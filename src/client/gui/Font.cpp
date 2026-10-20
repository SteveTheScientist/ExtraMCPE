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
#include <cctype>
#include <cstdlib>

Font::Font(Options* options, const std::string& name, Textures* textures)
:   options(options),
    fontTexture(0),
    fontName(name),
    _textures(textures),
    lineHeight(DefaultLineHeight),
    formattingOption(true),
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

int Font::getFormatCodeLength(const char* ptr, bool allowAmpersand, char& outCode, bool isPlayerText) const
{
    if (!ptr || *ptr == '\0') return 0;

    if (isPlayerText) {
        bool enabled = options ? options->getBooleanValue(OPTIONS_TEXT_FORMATTING) : formattingOption;
        if (!enabled) {
            return 0;
        }
    }

    int prefixLen = 0;
    if ((unsigned char)ptr[0] == 0xC2 && (unsigned char)ptr[1] == 0xA7) {
        prefixLen = 2;
    } else if ((unsigned char)ptr[0] == 0xA7) {
        prefixLen = 1;
    } else if ((allowAmpersand || isPlayerText) && ptr[0] == '&') {
        prefixLen = 1;
    }

    if (prefixLen == 0) return 0;

    char nextChar = ptr[prefixLen];
    if (nextChar == '\0') return 0;

    char c = tolower((unsigned char)nextChar);
    bool isValid = (c >= '0' && c <= '9') ||
                   (c >= 'a' && c <= 'f') ||
                   (c == 'k' || c == 'l' || c == 'm' || c == 'n' || c == 'o' || c == 'r');

    if (isValid) {
        outCode = c;
        return prefixLen + 1;
    }

    return 0;
}

const GlyphInfo* Font::getRandomGlyphWithWidth(int width) const
{
    std::map<int, std::vector<GlyphInfo>>::const_iterator it = glyphsByWidth.find(width);
    if (it != glyphsByWidth.end() && !it->second.empty()) {
        int idx = rand() % (int)it->second.size();
        return &it->second[idx];
    }
    return NULL;
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
                    glyphsByWidth[info.glyphWidth].push_back(info);

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
    glyphsByWidth.clear();
    hasMissingGlyph = false;

    // Check for assets/minecraft/font/default.json
    std::ifstream jsonFile("assets/minecraft/font/default.json");
    if (jsonFile.is_open()) {
        std::stringstream ss;
        ss << jsonFile.rdbuf();
        std::string jsonStr = ss.str();

        size_t providersPos = jsonStr.find("\"providers\"");
        if (providersPos != std::string::npos) {
            // Process providers list
        }
    }

    auto appendUTF8 = [](std::string& str, uint32_t cp) {
        if (cp == 0) {
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
    if (options) {
        formattingOption = options->getBooleanValue(OPTIONS_TEXT_FORMATTING);
    }
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

void Font::drawShadow(const std::string& str, float x, float y, int color, bool isPlayerText)
{
    drawShadow(str.c_str(), x, y, color, isPlayerText);
}

void Font::drawShadow(const char* str, float x, float y, int color, bool isPlayerText)
{
    renderString(str, x + 1.0f, y + 1.0f, color, true, isPlayerText);
    renderString(str, x, y, color, false, isPlayerText);
}

void Font::draw(const std::string& str, float x, float y, int color, bool darken, bool isPlayerText)
{
    renderString(str.c_str(), x, y, color, darken, isPlayerText);
}

void Font::draw(const char* str, float x, float y, int color, bool darken, bool isPlayerText)
{
    renderString(str, x, y, color, darken, isPlayerText);
}

void Font::renderString(const char* str, float x, float y, int color, bool darken, bool isPlayerText)
{
    if (!str || !*str) return;

    if (options) {
        formattingOption = options->getBooleanValue(OPTIONS_TEXT_FORMATTING);
    }

    Tesselator& t = Tesselator::instance;

    int initialAlpha = (0xFF000000 & color) >> 24;
    if (!initialAlpha) initialAlpha = 0xFF;

    static const int colorCodes[16] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xFFAA00, 0xAAAAAA,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
    };

    int baseColor = color;
    if (darken) {
        baseColor = ((color & 0xFCFCFC) >> 2) | (color & 0xFF000000);
    }

    int curColor = baseColor;
    int curR = (curColor >> 16) & 0xFF;
    int curG = (curColor >> 8) & 0xFF;
    int curB = curColor & 0xFF;

    bool bold = false;
    bool italic = false;
    bool underline = false;
    bool strikethrough = false;
    bool obfuscated = false;

    float startX = (float)std::floor((double)x + 0.5);
    float startY = (float)std::floor((double)y + 0.5);

    float xOffset = 0.0f;
    float yOffset = 0.0f;

    std::string currentTexture = "";
    bool allowAmpersand = formattingOption;

    const char* ptr = str;
    while (*ptr != '\0') {
        char codeChar = 0;
        int codeBytes = getFormatCodeLength(ptr, allowAmpersand, codeChar, isPlayerText);
        if (codeBytes > 0) {
            ptr += codeBytes;
            if ((codeChar >= '0' && codeChar <= '9') || (codeChar >= 'a' && codeChar <= 'f')) {
                int codeIdx = (codeChar >= '0' && codeChar <= '9') ? (codeChar - '0') : (codeChar - 'a' + 10);
                curColor = colorCodes[codeIdx];
                if (darken) {
                    curColor = (curColor & 0xFCFCFC) >> 2;
                }
                curR = (curColor >> 16) & 0xFF;
                curG = (curColor >> 8) & 0xFF;
                curB = curColor & 0xFF;
                bold = italic = underline = strikethrough = obfuscated = false;
            } else if (codeChar == 'k') {
                obfuscated = true;
            } else if (codeChar == 'l') {
                bold = true;
            } else if (codeChar == 'm') {
                strikethrough = true;
            } else if (codeChar == 'n') {
                underline = true;
            } else if (codeChar == 'o') {
                italic = true;
            } else if (codeChar == 'r') {
                curColor = baseColor;
                curR = (curColor >> 16) & 0xFF;
                curG = (curColor >> 8) & 0xFF;
                curB = curColor & 0xFF;
                bold = italic = underline = strikethrough = obfuscated = false;
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

        if (obfuscated && g->glyphWidth > 0) {
            const GlyphInfo* randG = getRandomGlyphWithWidth(g->glyphWidth);
            if (randG) {
                g = randG;
            }
        }

        int adv = g->advance;
        if (bold) {
            adv += 1;
        }

        if (g->textureName.length() > 0 && g->glyphWidth > 0) {
            if (g->textureName != currentTexture) {
                if (currentTexture.length() > 0) {
                    t.draw();
                }
                currentTexture = g->textureName;
                _textures->loadAndBindTexture(currentTexture);

                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri2(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                t.begin();
            }

            t.color(curR, curG, curB, initialAlpha);

            float gx = (float)std::floor((double)(startX + xOffset) + 0.5);
            float gy = (float)std::floor((double)(startY + yOffset + 7 - g->ascent) + 0.5);

            float quadW = (float)g->glyphWidth;
            float quadH = (float)g->cellH * g->scale;

            float u0 = (float)g->cellX / (float)g->textureWidth;
            float v0 = (float)g->cellY / (float)g->textureHeight;
            float u1 = (float)(g->cellX + g->glyphWidth) / (float)g->textureWidth;
            float v1 = (float)(g->cellY + g->cellH) / (float)g->textureHeight;

            float italicOffset = italic ? 1.0f : 0.0f;

            t.vertexUV(gx, gy + quadH, 0.0f, u0, v1);
            t.vertexUV(gx + quadW, gy + quadH, 0.0f, u1, v1);
            t.vertexUV(gx + quadW + italicOffset, gy, 0.0f, u1, v0);
            t.vertexUV(gx + italicOffset, gy, 0.0f, u0, v0);

            if (bold) {
                t.vertexUV(gx + 1.0f, gy + quadH, 0.0f, u0, v1);
                t.vertexUV(gx + 1.0f + quadW, gy + quadH, 0.0f, u1, v1);
                t.vertexUV(gx + 1.0f + quadW + italicOffset, gy, 0.0f, u1, v0);
                t.vertexUV(gx + 1.0f + italicOffset, gy, 0.0f, u0, v0);
            }
        }

        if (strikethrough || underline) {
            if (currentTexture.length() > 0) {
                t.draw();
                currentTexture = "";
            }

            float gx = (float)std::floor((double)(startX + xOffset) + 0.5);
            float lineY = startY + yOffset + (strikethrough ? 4.0f : 8.0f);

            glDisable2(GL_TEXTURE_2D);
            t.begin();
            t.color(curR, curG, curB, initialAlpha);
            t.vertex(gx - 1.0f, lineY + 1.0f, 0.0f);
            t.vertex(gx + adv, lineY + 1.0f, 0.0f);
            t.vertex(gx + adv, lineY, 0.0f);
            t.vertex(gx - 1.0f, lineY, 0.0f);
            t.draw();
            glEnable2(GL_TEXTURE_2D);
        }

        xOffset += adv;
    }

    if (currentTexture.length() > 0) {
        t.draw();
    }
}

int Font::width(const std::string& str, bool isPlayerText)
{
    if (str.empty()) return 0;

    if (options) {
        formattingOption = options->getBooleanValue(OPTIONS_TEXT_FORMATTING);
    }

    int maxLen = 0;
    int len = 0;
    bool bold = false;

    bool allowAmpersand = formattingOption;

    const char* ptr = str.c_str();
    while (*ptr != '\0') {
        char codeChar = 0;
        int codeBytes = getFormatCodeLength(ptr, allowAmpersand, codeChar, isPlayerText);
        if (codeBytes > 0) {
            ptr += codeBytes;
            if ((codeChar >= '0' && codeChar <= '9') || (codeChar >= 'a' && codeChar <= 'f') || codeChar == 'r') {
                bold = false;
            } else if (codeChar == 'l') {
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
                int adv = g->advance;
                if (bold) adv += 1;
                len += adv;
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
    std::string sanitized = "";
    sanitized.reserve(str.length());

    const char* ptr = str.c_str();
    while (*ptr != '\0') {
        char codeChar = 0;
        int codeBytes = 0;
        if ((unsigned char)ptr[0] == 0xC2 && (unsigned char)ptr[1] == 0xA7) {
            codeBytes = 2;
        } else if ((unsigned char)ptr[0] == 0xA7) {
            codeBytes = 1;
        } else if (allowAmpersand && ptr[0] == '&') {
            codeBytes = 1;
        }

        if (codeBytes > 0) {
            char nextChar = ptr[codeBytes];
            if (nextChar != '\0') {
                char c = tolower((unsigned char)nextChar);
                bool isValid = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') ||
                               (c == 'k' || c == 'l' || c == 'm' || c == 'n' || c == 'o' || c == 'r');
                if (isValid) {
                    ptr += codeBytes + 1;
                    continue;
                }
            }
        }

        sanitized += *ptr++;
    }
    return sanitized;
}

void Font::drawWordWrap(const std::string& str, float x, float y, float w, int col, bool isPlayerText)
{
    if (str.empty()) return;

    if (options) {
        formattingOption = options->getBooleanValue(OPTIONS_TEXT_FORMATTING);
    }

    bool allowAmpersand = formattingOption;

    struct FormatState {
        char colorCode = 0;
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool strikethrough = false;
        bool obfuscated = false;

        std::string getPrefix(bool ampersand) const {
            std::string res = "";
            char pfx = ampersand ? '&' : '\xa7';
            if (colorCode != 0) {
                res += pfx;
                res += colorCode;
            }
            if (bold) { res += pfx; res += 'l'; }
            if (italic) { res += pfx; res += 'o'; }
            if (underline) { res += pfx; res += 'n'; }
            if (strikethrough) { res += pfx; res += 'm'; }
            if (obfuscated) { res += pfx; res += 'k'; }
            return res;
        }

        void update(char codeChar) {
            if ((codeChar >= '0' && codeChar <= '9') || (codeChar >= 'a' && codeChar <= 'f')) {
                colorCode = codeChar;
                bold = italic = underline = strikethrough = obfuscated = false;
            } else if (codeChar == 'k') obfuscated = true;
            else if (codeChar == 'l') bold = true;
            else if (codeChar == 'm') strikethrough = true;
            else if (codeChar == 'n') underline = true;
            else if (codeChar == 'o') italic = true;
            else if (codeChar == 'r') {
                colorCode = 0;
                bold = italic = underline = strikethrough = obfuscated = false;
            }
        }
    };

    FormatState activeState;

    std::vector<std::string> lines;
    std::string currentLine = "";

    const char* ptr = str.c_str();
    while (*ptr != '\0') {
        if (*ptr == '\n') {
            lines.push_back(currentLine);
            currentLine = activeState.getPrefix(allowAmpersand);
            ptr++;
            continue;
        }

        const char* segStart = ptr;
        std::string wordCandidate = "";
        FormatState tempState = activeState;

        while (*ptr != '\0' && *ptr != '\n' && *ptr != ' ') {
            char codeChar = 0;
            int codeBytes = getFormatCodeLength(ptr, allowAmpersand, codeChar, isPlayerText);
            if (codeBytes > 0) {
                tempState.update(codeChar);
                for (int b = 0; b < codeBytes; b++) {
                    wordCandidate += *ptr++;
                }
                continue;
            }
            const char* prevPtr = ptr;
            decodeUTF8(ptr);
            while (prevPtr < ptr) {
                wordCandidate += *prevPtr++;
            }
        }

        if (*ptr == ' ') {
            wordCandidate += ' ';
            ptr++;
        }

        if (width(currentLine + wordCandidate, isPlayerText) <= w || currentLine.empty() || currentLine == activeState.getPrefix(allowAmpersand)) {
            currentLine += wordCandidate;
            activeState = tempState;
        } else {
            lines.push_back(currentLine);
            currentLine = activeState.getPrefix(allowAmpersand) + wordCandidate;
            activeState = tempState;
        }
    }

    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        drawShadow(lines[i], x, y, col, isPlayerText);
        y += lineHeight;
    }
}
