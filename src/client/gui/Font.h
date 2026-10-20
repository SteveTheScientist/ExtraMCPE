#ifndef NET_MINECRAFT_CLIENT_GUI__Font_H__
#define NET_MINECRAFT_CLIENT_GUI__Font_H__

#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "../renderer/gles.h"

class Textures;
class Options;

struct GlyphInfo {
    std::string textureName;
    int textureWidth;
    int textureHeight;
    int cellX;
    int cellY;
    int cellW;
    int cellH;
    int glyphWidth;
    int advance;
    float scale;
    int ascent;
};

class Font
{
public:
    Font(Options* options, const std::string& name, Textures* textures);
    ~Font();

    void init(Options* options);
    void onGraphicsReset();

    void draw(const char* str, float x, float y, int color);
    void draw(const std::string& str, float x, float y, int color);
    void draw(const char* str, float x, float y, int color, bool darken);
    void draw(const std::string& str, float x, float y, int color, bool darken);
    void drawShadow(const std::string& str, float x, float y, int color);
    void drawShadow(const char* str, float x, float y, int color);
    void drawWordWrap(const std::string& str, float x, float y, float w, int col);

    int width(const std::string& str);
    int height(const std::string& str);

    static std::string sanitize(const std::string& str, bool allowAmpersand = true);

    int getGuiScale() const;

    static bool parseFormattingCode(const char* ptr, bool allowAmpersand, int& codeLength, char& codeChar);
    static std::string updateFormatPrefix(const std::string& currentPrefix, char codeChar);
    std::vector<std::string> wrapText(const std::string& str, float w);

public:
    int fontTexture;
    int lineHeight;
    static const int DefaultLineHeight = 9;

private:
    void loadFontProviders();
    void addBitmapProvider(const std::string& relativeTexPath, int heightVal, int ascentVal, const std::vector<std::string>& charGrid);
    void renderString(const char* str, float x, float y, int color, bool darken);
    const GlyphInfo* getGlyph(uint32_t codepoint) const;

private:
    std::string fontName;
    Textures* _textures;
    Options* options;

    std::map<uint32_t, GlyphInfo> glyphMap;
    std::vector<uint32_t> glyphsByAdvance[32];
    GlyphInfo missingGlyph;
    bool hasMissingGlyph;
};

#endif /*NET_MINECRAFT_CLIENT_GUI__Font_H__*/
