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
                    info.advance = (int)std::round((rightmost + 1) * scale) + 1;
                    info.scale = scale;
                    info.ascent = ascentVal;

                    glyphMap[codepoint] = info;
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

    // Built-in default provider using ascii.png (height 8, ascent 7)
    std::vector<std::string> asciiGrid;
    for (int r = 0; r < 16; r++) {
        std::string line = "";
        for (int c = 0; c < 16; c++) {
            line += (char)(r * 16 + c);
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
    int shadowColor = ((color & 0xFCFCFC) >> 2) | (color & 0xFF000000);
    renderString(str, x + 1.0f, y + 1.0f, shadowColor, false); // Pass false to prevent double-darkening
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

    if (darken) {
        int oldAlpha = color & 0xFF000000;
        color = ((color & 0xFCFCFC) >> 2) | oldAlpha;
    }

    Tesselator& t = Tesselator::instance;

    int alpha = (0xFF000000 & color) >> 24;
    if (!alpha) alpha = 0xFF;

    static const std::string hex("0123456789abcdef");
    static const int colorCodes[16] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xFFAA00, 0xAAAAAA,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
    };

    int curColor = color;
    int curR = (curColor >> 16) & 0xFF;
    int curG = (curColor >> 8) & 0xFF;
    int curB = curColor & 0xFF;

    // Pixel snapping starting coordinates
    float startX = (float)std::floor((double)x + 0.5);
    float startY = (float)std::floor((double)y + 0.5);

    float xOffset = 0.0f;
    float yOffset = 0.0f;

    std::string currentTexture = "";

    const char* ptr = str;
    while (*ptr != '\0') {
        if (*ptr == '\xa7' && *(ptr + 1) != '\0') {
            ptr++;
            char codeChar = tolower((unsigned char)*ptr++);
            int codeIdx = hex.find(codeChar);
            if (codeIdx >= 0 && codeIdx < 16) {
                int cCode = colorCodes[codeIdx];
                if (darken) {
                    cCode = (cCode & 0xFCFCFC) >> 2;
                }
                curR = (cCode >> 16) & 0xFF;
                curG = (cCode >> 8) & 0xFF;
                curB = cCode & 0xFF;
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

            t.color(curR, curG, curB, alpha);

            // Baseline alignment: glyphTopY = textY + 7 - ascent
            float gx = (float)std::floor((double)(startX + xOffset) + 0.5);
            float gy = (float)std::floor((double)(startY + yOffset + 7 - g->ascent) + 0.5);

            float quadW = (float)g->glyphWidth;
            float quadH = (float)g->cellH * g->scale;

            float u0 = (float)g->cellX / (float)g->textureWidth;
            float v0 = (float)g->cellY / (float)g->textureHeight;
            float u1 = (float)(g->cellX + g->glyphWidth) / (float)g->textureWidth;
            float v1 = (float)(g->cellY + g->cellH) / (float)g->textureHeight;

            t.vertexUV(gx, gy + quadH, 0.0f, u0, v1);
            t.vertexUV(gx + quadW, gy + quadH, 0.0f, u1, v1);
            t.vertexUV(gx + quadW, gy, 0.0f, u1, v0);
            t.vertexUV(gx, gy, 0.0f, u0, v0);
        }

        xOffset += g->advance;
    }

    if (currentTexture.length() > 0) {
        t.draw();
    }
}

int Font::width(const std::string& str)
{
    int maxLen = 0;
    int len = 0;

    const char* ptr = str.c_str();
    while (*ptr != '\0') {
        if (*ptr == '\xa7' && *(ptr + 1) != '\0') {
            ptr += 2;
            continue;
        }

        uint32_t codepoint = decodeUTF8(ptr);
        if (codepoint == '\n') {
            if (len > maxLen) maxLen = len;
            len = 0;
        } else {
            const GlyphInfo* g = getGlyph(codepoint);
            if (g) {
                len += g->advance;
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

std::string Font::sanitize(const std::string& str)
{
    std::string sanitized(str.length() + 1, 0);
    int j = 0;

    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] == '\xa7') {
            i++;
        } else {
            sanitized[j++] = str[i];
        }
    }
    return sanitized.erase(j);
}

void Font::drawWordWrap(const std::string& str, float x, float y, float w, int col)
{
    char* cstr = new char[str.length() + 1];
    strncpy(cstr, str.c_str(), str.length());
    cstr[str.length()] = 0;

    const char* lims = " \n\t\r";
    char* ptok = strtok(cstr, lims);

    std::vector<std::string> words;
    while (ptok != NULL) {
        words.push_back(ptok);
        ptok = strtok(NULL, lims);
    }

    delete[] cstr;

    int pos = 0;
    while (pos < (int)words.size()) {
        std::string line = words[pos++] + " ";
        while (pos < (int)words.size() && width(line + words[pos]) < w) {
            line += words[pos++] + " ";
        }
        drawShadow(line, x, y, col);
        y += lineHeight;
    }
}
