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

    void draw(const char* str, float x, float y, int color, bool darken = false, bool isPlayerText = false);
    void draw(const std::string& str, float x, float y, int color, bool darken = false, bool isPlayerText = false);
    void drawShadow(const std::string& str, float x, float y, int color, bool isPlayerText = false);
    void drawShadow(const char* str, float x, float y, int color, bool isPlayerText = false);
    void drawWordWrap(const std::string& str, float x, float y, float w, int col, bool isPlayerText = false);

    int width(const std::string& str, bool isPlayerText = false);
    int height(const std::string& str);

    static std::string sanitize(const std::string& str, bool allowAmpersand = true);

    int getGuiScale() const;

    void setFormattingOption(bool enabled) { formattingOption = enabled; }
    bool isFormattingOptionEnabled() const { return formattingOption; }
    bool isFormattingEnabled(bool isPlayerText = false) const;

    int getFormatCodeLength(const char* ptr, bool allowAmpersand, char& outCode, bool isPlayerText = false) const;

public:
    int fontTexture;
    int lineHeight;
    static const int DefaultLineHeight = 9;
    bool formattingOption;

private:
    void loadFontProviders();
    void addBitmapProvider(const std::string& relativeTexPath, int heightVal, int ascentVal, const std::vector<std::string>& charGrid);
    void renderString(const char* str, float x, float y, int color, bool darken, bool isPlayerText = false);
    const GlyphInfo* getGlyph(uint32_t codepoint) const;
    const GlyphInfo* getRandomGlyphWithWidth(int width) const;

private:
    std::string fontName;
    Textures* _textures;
    Options* options;

    std::map<uint32_t, GlyphInfo> glyphMap;
    std::map<int, std::vector<GlyphInfo>> glyphsByWidth;
    GlyphInfo missingGlyph;
    bool hasMissingGlyph;
};

#endif /*NET_MINECRAFT_CLIENT_GUI__Font_H__*/
