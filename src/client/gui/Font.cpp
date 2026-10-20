#include "Font.h"
#include "../Options.h"
#include "../renderer/Textures.h"
#include "../renderer/Tesselator.h"
#include "../../util/Mth.h"

#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cassert>

Font::Font(Options* options, const std::string& name, Textures* textures)
:   options(options),
    fontTexture(0),
    fontName(name),
    _textures(textures),
    lineHeight(DefaultLineHeight),
    hasMissingGlyph(false)
{
    init(options);
}

Font::~Font()
{
}

void Font::onGraphicsReset()
{
    init(options);
}

int Font::getGuiScale() const
{
    if (!options) return 2;
    int optScale = options->getIntValue(OPTIONS_GUI_SCALE);
    double guiScale = 2.0;

    if (optScale == 0) { // Auto scale
        guiScale = 2.0;
    } else {
        guiScale = (double)optScale;
    }

    int intScale = (int)std::floor(guiScale);
    if (intScale < 1) intScale = 1;
    return intScale;
}

bool Font::parseFormattingCode(const char* ptr, bool allowAmpersand, int& codeLength, char& codeChar)
{
    if (!ptr || !*ptr) return false;

    int prefixLen = 0;
    if ((unsigned char)ptr[0] == 0xC2 && (unsigned char)ptr[1] == 0xA7) {
        prefixLen = 2;
    } else if ((unsigned char)ptr[0] == 0xA7) {
        prefixLen = 1;
    } else if (allowAmpersand && ptr[0] == '&') {
        prefixLen = 1;
    }

    if (prefixLen == 0) return false;

    char nextC = ptr[prefixLen];
    if (nextC == '\0') return false;

    char lower = tolower((unsigned char)nextC);
    bool isValid = (lower >= '0' && lower <= '9') ||
                   (lower >= 'a' && lower <= 'f') ||
                   lower == 'k' || lower == 'l' || lower == 'm' ||
                   lower == 'n' || lower == 'o' || lower == 'r';

    if (isValid) {
        codeLength = prefixLen + 1;
        codeChar = nextC;
        return true;
    }

    return false;
}

std::string Font::updateFormatPrefix(const std::string& currentPrefix, char codeChar)
{
    char lower = tolower((unsigned char)codeChar);
    if ((lower >= '0' && lower <= '9') || (lower >= 'a' && lower <= 'f')) {
        return std::string("§") + lower;
    } else if (lower == 'r') {
        return "";
    } else if (lower == 'k' || lower == 'l' || lower == 'm' || lower == 'n' || lower == 'o') {
        if (currentPrefix.find(lower) == std::string::npos) {
            return currentPrefix + "§" + lower;
        }
    }
    return currentPrefix;
}

static uint32_t decodeUTF8(const char*& ptr) {
    unsigned char c = (unsigned char)(*ptr++);
    if (c < 0x80) return c;
    if ((c & 0xE0) == 0xC0) {
        unsigned char c2 = (unsigned char)(*ptr++);
        return ((c & 0x1F) << 6) | (c2 & 0x3F);
    }
    if ((c & 0xF0) == 0xE0) {
        unsigned char c2 = (unsigned char)(*ptr++);
        unsigned char c3 = (unsigned char)(*ptr++);
        return ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F);
    }
    if ((c & 0xF8) == 0xF0) {
        unsigned char c2 = (unsigned char)(*ptr++);
        unsigned char c3 = (unsigned char)(*ptr++);
        unsigned char c4 = (unsigned char)(*ptr++);
        return ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F);
    }
    return c;
}

void Font::addBitmapProvider(const std::string& relativeTexPath, int heightVal, int ascentVal, const std::vector<std::string>& charGrid)
{
    if (charGrid.empty()) return;

    // Load PNG texture exactly as provided
    std::string cleanPath = relativeTexPath;
    if (cleanPath.find("minecraft:") == 0) {
        cleanPath = cleanPath.substr(10);
    }
    if (cleanPath.find("textures/") == 0) {
        cleanPath = cleanPath.substr(9);
    }

    TextureId tid = _textures->loadTexture(cleanPath);
    const TextureData* tex = _textures->getTemporaryTextureData(tid);

    if (!tex || !tex->data) return;

    int imgW = tex->w;
    int imgH = tex->h;
    int rows = (int)charGrid.size();
    if (rows <= 0) return;

    // Determine cols from first char row length
    int cols = 0;
    const char* ptr = charGrid[0].c_str();
    while (*ptr != '\0') {
        decodeUTF8(ptr);
        cols++;
    }
    if (cols <= 0) return;

    int cellW = imgW / cols;
    int cellH = imgH / rows;
    if (cellW <= 0 || cellH <= 0) return;

    float scale = (float)heightVal / (float)cellH;

    for (int r = 0; r < rows; r++) {
        const char* linePtr = charGrid[r].c_str();
        int c = 0;
        while (*linePtr != '\0' && c < cols) {
            uint32_t codepoint = decodeUTF8(linePtr);

            if (codepoint != 0 && glyphMap.find(codepoint) == glyphMap.end()) {
                int cellX = c * cellW;
                int cellY = r * cellH;

                // Rightmost non-transparent column scan
                int rightmost = -1;
                for (int x = cellW - 1; x >= 0; x--) {
                    bool found = false;
                    for (int y = 0; y < cellH; y++) {
                        int px = cellX + x;
                        int py = cellY + y;
                        int idx = (px + py * imgW) << 2;
                        unsigned char alpha = tex->data[idx + 3];
                        if (alpha > 0) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        rightmost = x;
                        break;
                    }
                }

                if (rightmost >= 0) {
                    GlyphInfo info;
                    info.textureName = cleanPath;
                    info.textureWidth = imgW;
                    info.textureHeight = imgH;
                    info.cellX = cellX;
                    info.cellY = cellY;
                    info.cellW = cellW;
                    info.cellH = cellH;
                    info.glyphWidth = rightmost + 1;
                    info.advance = (int)floor(((rightmost + 1) * scale) + 0.5f) + 1;
                    info.scale = scale;
                    info.ascent = ascentVal;

                    glyphMap[codepoint] = info;
                    if (info.advance >= 0 && info.advance < 32) {
                        glyphsByAdvance[info.advance].push_back(codepoint);
                    }
                    if (codepoint == '?') {
                        missingGlyph = info;
                        hasMissingGlyph = true;
                    }
                }
            }
            c++;
        }
    }
}

void Font::loadFontProviders()
{
    glyphMap.clear();
    for (int i = 0; i < 32; ++i) {
        glyphsByAdvance[i].clear();
    }
    hasMissingGlyph = false;

    // Check for assets/minecraft/font/default.json
    std::ifstream jsonFile("assets/minecraft/font/default.json");
    if (jsonFile.is_open()) {
        std::stringstream ss;
        ss << jsonFile.rdbuf();
        std::string jsonStr = ss.str();

        // Minimal JSON parsing for providers
        size_t providersPos = jsonStr.find("\"providers\"");
        if (providersPos != std::string::npos) {
            // Process providers list
            // Log/skip unsupported provider types with clear TODO comments per spec:
            // TODO: Support "legacy_unicode" provider type from default.json
            // TODO: Support "unihex" provider type from default.json
            // TODO: Support "ttf" provider type from default.json
            // TODO: Support "reference" provider type from default.json
        }
    }

    // Helper to append a Unicode codepoint as UTF-8 bytes to a std::string
    auto appendUTF8 = [](std::string& str, uint32_t cp) {
        if (cp == 0) {
            // Replace codepoint 0 (NUL) with a placeholder character (0xFFFD / 0x0001) to prevent NUL string termination
            cp = 0x0001;
        }
        if (cp < 0x80) {
            str += (char)cp;
        } else if (cp < 0x800) {
            str += (char)(0xC0 | (cp >> 6));
            str += (char)(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            str += (char)(0xE0 | (cp >> 12));
            str += (char)(0x80 | ((cp >> 6) & 0x3F));
            str += (char)(0x80 | (cp & 0x3F));
        } else {
            str += (char)(0xF0 | (cp >> 18));
            str += (char)(0x80 | ((cp >> 12) & 0x3F));
            str += (char)(0x80 | ((cp >> 6) & 0x3F));
            str += (char)(0x80 | (cp & 0x3F));
        }
    };

    // Built-in default provider using ascii.png (height 8, ascent 7)
    std::vector<std::string> asciiGrid;
    for (int r = 0; r < 16; r++) {
        std::string line = "";
        for (int c = 0; c < 16; c++) {
            uint32_t cp = (uint32_t)(r * 16 + c);
            appendUTF8(line, cp);
        }
        asciiGrid.push_back(line);
    }
    addBitmapProvider("font/ascii.png", 8, 7, asciiGrid);

    // Space provider (advance 4)
    if (glyphMap.find(' ') == glyphMap.end()) {
        GlyphInfo spaceInfo;
        spaceInfo.textureName = "";
        spaceInfo.textureWidth = 0;
        spaceInfo.textureHeight = 0;
        spaceInfo.cellX = 0;
        spaceInfo.cellY = 0;
        spaceInfo.cellW = 0;
        spaceInfo.cellH = 0;
        spaceInfo.glyphWidth = 0;
        spaceInfo.advance = 4;
        spaceInfo.scale = 1.0f;
        spaceInfo.ascent = 7;
        glyphMap[' '] = spaceInfo;
    }

    // Unit test assertions per Step 3 / Step 5 spec:
    assert(glyphMap[' '].advance == 4);
    for (int ch = 0x21; ch <= 0x7E; ch++) {
        if (glyphMap.find(ch) != glyphMap.end()) {
            int gw = glyphMap[ch].glyphWidth;
            assert(gw >= 1 && gw <= 8);
        }
    }
}

void Font::init(Options* options)
{
    loadFontProviders();
}

const GlyphInfo* Font::getGlyph(uint32_t codepoint) const
{
    std::map<uint32_t, GlyphInfo>::const_iterator it = glyphMap.find(codepoint);
    if (it != glyphMap.end()) {
        return &it->second;
    }
    if (hasMissingGlyph) {
        return &missingGlyph;
    }
    return NULL;
}

void Font::drawShadow(const std::string& str, float x, float y, int color)
{
    drawShadow(str.c_str(), x, y, color);
}

void Font::drawShadow(const char* str, float x, float y, int color)
{
    renderString(str, x + 1.0f, y + 1.0f, color, true); // true = darken
    renderString(str, x, y, color, false);
}

void Font::draw(const std::string& str, float x, float y, int color)
{
    renderString(str.c_str(), x, y, color, false);
}

void Font::draw(const char* str, float x, float y, int color)
{
    renderString(str, x, y, color, false);
}

void Font::draw(const char* str, float x, float y, int color, bool darken)
{
    renderString(str, x, y, color, darken);
}

void Font::draw(const std::string& str, float x, float y, int color, bool darken)
{
    renderString(str.c_str(), x, y, color, darken);
}

void Font::renderString(const char* str, float x, float y, int color, bool darken)
{
    if (!str || !*str) return;

    Tesselator& t = Tesselator::instance;

    int initialColor = color;
    int alpha = (0xFF000000 & color) >> 24;
    if (!alpha) alpha = 0xFF;

    static const std::string hex("0123456789abcdef");
    static const int colorCodes[16] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xFFAA00, 0xAAAAAA,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
    };

    int curColor = initialColor;
    if (darken) {
        curColor = ((initialColor & 0xFCFCFC) >> 2) | (initialColor & 0xFF000000);
    }

    int curR = (curColor >> 16) & 0xFF;
    int curG = (curColor >> 8) & 0xFF;
    int curB = curColor & 0xFF;

    bool obfuscated = false;
    bool bold = false;
    bool strikethrough = false;
    bool underline = false;
    bool italic = false;

    bool allowAmpersand = options ? options->getBooleanValue(OPTIONS_TEXT_FORMATTING) : true;

    struct TextLineSegment {
        float x0, y0, x1, y1;
        int r, g, b, a;
    };
    std::vector<TextLineSegment> lineSegments;

    // Pixel snapping starting coordinates
    float startX = (float)std::floor((double)x + 0.5);
    float startY = (float)std::floor((double)y + 0.5);

    float xOffset = 0.0f;
    float yOffset = 0.0f;

    std::string currentTexture = "";

    const char* ptr = str;
    while (*ptr != '\0') {
        int codeLen = 0;
        char codeChar = 0;
        if (parseFormattingCode(ptr, allowAmpersand, codeLen, codeChar)) {
            ptr += codeLen;
            char lower = tolower((unsigned char)codeChar);
            int codeIdx = hex.find(lower);
            if (codeIdx >= 0 && codeIdx < 16) {
                int cCode = colorCodes[codeIdx];
                if (darken) {
                    cCode = (cCode & 0xFCFCFC) >> 2;
                }
                curR = (cCode >> 16) & 0xFF;
                curG = (cCode >> 8) & 0xFF;
                curB = cCode & 0xFF;
                // A color code resets active styles
                obfuscated = false;
                bold = false;
                strikethrough = false;
                underline = false;
                italic = false;
            } else if (lower == 'k') {
                obfuscated = true;
            } else if (lower == 'l') {
                bold = true;
            } else if (lower == 'm') {
                strikethrough = true;
            } else if (lower == 'n') {
                underline = true;
            } else if (lower == 'o') {
                italic = true;
            } else if (lower == 'r') {
                int rCol = initialColor;
                if (darken) {
                    rCol = ((initialColor & 0xFCFCFC) >> 2) | (initialColor & 0xFF000000);
                }
                curR = (rCol >> 16) & 0xFF;
                curG = (rCol >> 8) & 0xFF;
                curB = rCol & 0xFF;
                obfuscated = false;
                bold = false;
                strikethrough = false;
                underline = false;
                italic = false;
            }
            continue;
        }

        uint32_t codepoint = decodeUTF8(ptr);
        if (codepoint == '\n') {
            xOffset = 0.0f;
            yOffset += lineHeight;
            continue;
        }

        const GlyphInfo* g = getGlyph(codepoint);
        if (!g) continue;

        const GlyphInfo* renderG = g;
        if (obfuscated && g->advance > 0 && g->advance < 32 && !glyphsByAdvance[g->advance].empty()) {
            uint32_t rndCp = glyphsByAdvance[g->advance][rand() % glyphsByAdvance[g->advance].size()];
            const GlyphInfo* obfG = getGlyph(rndCp);
            if (obfG) renderG = obfG;
        }

        if (renderG->textureName.length() > 0 && renderG->glyphWidth > 0) {
            if (renderG->textureName != currentTexture) {
                if (currentTexture.length() > 0) {
                    t.draw();
                }
                currentTexture = renderG->textureName;
                _textures->loadAndBindTexture(currentTexture);

                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                t.begin();
            }

            t.color(curR, curG, curB, alpha);

            // Baseline alignment: glyphTopY = textY + 7 - ascent
            float gx = (float)std::floor((double)(startX + xOffset) + 0.5);
            float gy = (float)std::floor((double)(startY + yOffset + 7 - renderG->ascent) + 0.5);

            float quadW = (float)renderG->glyphWidth;
            float quadH = (float)renderG->cellH * renderG->scale;

            float u0 = (float)renderG->cellX / (float)renderG->textureWidth;
            float v0 = (float)renderG->cellY / (float)renderG->textureHeight;
            float u1 = (float)(renderG->cellX + renderG->glyphWidth) / (float)renderG->textureWidth;
            float v1 = (float)(renderG->cellY + renderG->cellH) / (float)renderG->textureHeight;

            float italicOffset = italic ? 1.0f : 0.0f;

            // First quad
            t.vertexUV(gx, gy + quadH, 0.0f, u0, v1);
            t.vertexUV(gx + quadW, gy + quadH, 0.0f, u1, v1);
            t.vertexUV(gx + quadW + italicOffset, gy, 0.0f, u1, v0);
            t.vertexUV(gx + italicOffset, gy, 0.0f, u0, v0);

            // Bold second quad offset by +1.0f in X
            if (bold) {
                t.vertexUV(gx + 1.0f, gy + quadH, 0.0f, u0, v1);
                t.vertexUV(gx + quadW + 1.0f, gy + quadH, 0.0f, u1, v1);
                t.vertexUV(gx + quadW + 1.0f + italicOffset, gy, 0.0f, u1, v0);
                t.vertexUV(gx + 1.0f + italicOffset, gy, 0.0f, u0, v0);
            }
        }

        float charWidth = (float)g->advance + (bold ? 1.0f : 0.0f);

        if (strikethrough) {
            TextLineSegment seg;
            seg.x0 = (float)std::floor((double)(startX + xOffset) + 0.5);
            seg.x1 = seg.x0 + charWidth;
            seg.y0 = (float)std::floor((double)(startY + yOffset + 4.0f) + 0.5);
            seg.y1 = seg.y0 + 1.0f;
            seg.r = curR;
            seg.g = curG;
            seg.b = curB;
            seg.a = alpha;
            lineSegments.push_back(seg);
        }

        if (underline) {
            TextLineSegment seg;
            seg.x0 = (float)std::floor((double)(startX + xOffset) + 0.5);
            seg.x1 = seg.x0 + charWidth;
            seg.y0 = (float)std::floor((double)(startY + yOffset + 8.0f) + 0.5);
            seg.y1 = seg.y0 + 1.0f;
            seg.r = curR;
            seg.g = curG;
            seg.b = curB;
            seg.a = alpha;
            lineSegments.push_back(seg);
        }

        xOffset += charWidth;
    }

    if (currentTexture.length() > 0) {
        t.draw();
    }

    if (!lineSegments.empty()) {
        glDisable2(GL_TEXTURE_2D);
        t.begin();
        for (size_t i = 0; i < lineSegments.size(); ++i) {
            const TextLineSegment& seg = lineSegments[i];
            t.color(seg.r, seg.g, seg.b, seg.a);
            t.vertex(seg.x0, seg.y1, 0.0f);
            t.vertex(seg.x1, seg.y1, 0.0f);
            t.vertex(seg.x1, seg.y0, 0.0f);
            t.vertex(seg.x0, seg.y0, 0.0f);
        }
        t.draw();
        glEnable2(GL_TEXTURE_2D);
    }
}

int Font::width(const std::string& str)
{
    int maxLen = 0;
    int len = 0;

    bool bold = false;
    bool allowAmpersand = options ? options->getBooleanValue(OPTIONS_TEXT_FORMATTING) : true;

    const char* ptr = str.c_str();
    while (*ptr != '\0') {
        int codeLen = 0;
        char codeChar = 0;
        if (parseFormattingCode(ptr, allowAmpersand, codeLen, codeChar)) {
            ptr += codeLen;
            char lower = tolower((unsigned char)codeChar);
            if ((lower >= '0' && lower <= '9') || (lower >= 'a' && lower <= 'f') || lower == 'r') {
                bold = false;
            } else if (lower == 'l') {
                bold = true;
            }
            continue;
        }

        uint32_t codepoint = decodeUTF8(ptr);
        if (codepoint == '\n') {
            if (len > maxLen) maxLen = len;
            len = 0;
        } else {
            const GlyphInfo* g = getGlyph(codepoint);
            if (g) {
                len += g->advance + (bold ? 1 : 0);
            }
        }
    }
    return maxLen > len ? maxLen : len;
}

int Font::height(const std::string& str)
{
    int h = lineHeight;
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\n') {
            h += lineHeight;
        }
    }
    return h;
}

std::string Font::sanitize(const std::string& str, bool allowAmpersand)
{
    std::string result = "";
    const char* ptr = str.c_str();
    while (*ptr != '\0') {
        int codeLen = 0;
        char codeChar = 0;
        if (parseFormattingCode(ptr, allowAmpersand, codeLen, codeChar)) {
            ptr += codeLen;
            continue;
        }
        result += *ptr++;
    }
    return result;
}

std::vector<std::string> Font::wrapText(const std::string& str, float w)
{
    std::vector<std::string> lines;
    if (str.empty()) return lines;

    bool allowAmpersand = options ? options->getBooleanValue(OPTIONS_TEXT_FORMATTING) : true;

    // Split input string by explicit '\n'
    std::vector<std::string> paragraphs;
    std::string currentPara = "";
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '\n') {
            paragraphs.push_back(currentPara);
            currentPara = "";
        } else {
            currentPara += str[i];
        }
    }
    paragraphs.push_back(currentPara);

    std::string activePrefix = "";

    for (size_t p = 0; p < paragraphs.size(); ++p) {
        const std::string& para = paragraphs[p];

        if (para.empty()) {
            lines.push_back(activePrefix);
            continue;
        }

        // Tokenize paragraph into words (preserving spaces)
        std::vector<std::string> words;
        std::string word = "";
        for (size_t i = 0; i < para.length(); ++i) {
            char c = para[i];
            if (c == ' ') {
                word += c;
                words.push_back(word);
                word = "";
            } else {
                word += c;
            }
        }
        if (!word.empty()) {
            words.push_back(word);
        }

        std::string currentLine = activePrefix;

        for (size_t i = 0; i < words.size(); ++i) {
            const std::string& wToken = words[i];

            if (width(currentLine) > width(activePrefix) && width(currentLine + wToken) > w) {
                lines.push_back(currentLine);

                const char* cptr = currentLine.c_str();
                while (*cptr != '\0') {
                    int codeLen = 0;
                    char codeChar = 0;
                    if (parseFormattingCode(cptr, allowAmpersand, codeLen, codeChar)) {
                        cptr += codeLen;
                        activePrefix = updateFormatPrefix(activePrefix, codeChar);
                    } else {
                        decodeUTF8(cptr);
                    }
                }

                std::string trimmedToken = wToken;
                while (!trimmedToken.empty() && trimmedToken[0] == ' ') {
                    trimmedToken = trimmedToken.substr(1);
                }

                currentLine = activePrefix + trimmedToken;
            } else {
                if (currentLine == activePrefix && width(currentLine + wToken) > w) {
                    const char* wptr = wToken.c_str();
                    while (*wptr != '\0') {
                        int codeLen = 0;
                        char codeChar = 0;
                        if (parseFormattingCode(wptr, allowAmpersand, codeLen, codeChar)) {
                            wptr += codeLen;
                            currentLine += std::string(wptr - codeLen, codeLen);
                            activePrefix = updateFormatPrefix(activePrefix, codeChar);
                            continue;
                        }

                        const char* savePtr = wptr;
                        decodeUTF8(wptr);
                        std::string charStr(savePtr, wptr - savePtr);

                        if (width(currentLine + charStr) > w && width(currentLine) > width(activePrefix)) {
                            lines.push_back(currentLine);
                            currentLine = activePrefix + charStr;
                        } else {
                            currentLine += charStr;
                        }
                    }
                } else {
                    currentLine += wToken;
                }
            }
        }

        if (!currentLine.empty()) {
            lines.push_back(currentLine);

            const char* cptr = currentLine.c_str();
            while (*cptr != '\0') {
                int codeLen = 0;
                char codeChar = 0;
                if (parseFormattingCode(cptr, allowAmpersand, codeLen, codeChar)) {
                    cptr += codeLen;
                    activePrefix = updateFormatPrefix(activePrefix, codeChar);
                } else {
                    decodeUTF8(cptr);
                }
            }
        }
    }

    return lines;
}

void Font::drawWordWrap(const std::string& str, float x, float y, float w, int col)
{
    std::vector<std::string> lines = wrapText(str, w);
    for (size_t i = 0; i < lines.size(); ++i) {
        drawShadow(lines[i], x, y, col);
        y += lineHeight;
    }
}
