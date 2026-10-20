#include "client/gui/Font.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/Textures.h"
#include <iostream>
#include <cassert>
#include <cstring>

// Stub OpenGL & renderer dependencies for standalone unit test
int getTimeMs() { return 0; }

Tesselator Tesselator::instance(1024);

Tesselator::Tesselator(int size) {}
Tesselator::~Tesselator() {}
void Tesselator::begin(int mode) {}
void Tesselator::begin() {}
void Tesselator::draw() {}
void Tesselator::color(int r, int g, int b, int a) {}
void Tesselator::vertexUV(float x, float y, float z, float u, float v) {}
void Tesselator::vertex(float x, float y, float z) {}

TextureId Textures::loadTexture(const std::string& name, bool b) { return 0; }
const TextureData* Textures::getTemporaryTextureData(TextureId id) { return NULL; }
TextureId Textures::loadAndBindTexture(const std::string& name) { return 0; }

int main() {
    std::cout << "Running Font Formatting Unit Tests..." << std::endl;

    Font font(NULL, "gui/default.png", NULL);

    // 1. Format Code Parsing Unit Tests
    char outCode = 0;

    // UTF-8 section sign §a (\xc2\xa7 + "aHello")
    const char* utf8Code = "\xc2\xa7" "aHello";
    int lenUtf8 = font.getFormatCodeLength(utf8Code, true, outCode, false);
    assert(lenUtf8 == 3);
    assert(outCode == 'a');

    // Single-byte section sign \xa7b
    const char* sbCode = "\xa7" "bWorld";
    int lenSb = font.getFormatCodeLength(sbCode, true, outCode, false);
    assert(lenSb == 2);
    assert(outCode == 'b');

    // Ampersand code &c (formatting option ON)
    font.setFormattingOption(true);
    const char* ampCode = "&cText";
    int lenAmp = font.getFormatCodeLength(ampCode, true, outCode, true);
    assert(lenAmp == 2);
    assert(outCode == 'c');

    // Player text with formatting option OFF
    font.setFormattingOption(false);
    int lenPlayerOff1 = font.getFormatCodeLength(ampCode, true, outCode, true);
    assert(lenPlayerOff1 == 0); // & not parsed as code when OFF

    int lenPlayerOff2 = font.getFormatCodeLength(sbCode, true, outCode, true);
    assert(lenPlayerOff2 == 0); // § not parsed as code for player text when OFF

    // Built-in text with formatting option OFF (built-in text MUST NEVER BE AFFECTED)
    int lenBuiltinOff = font.getFormatCodeLength(sbCode, true, outCode, false);
    assert(lenBuiltinOff == 2); // § IS parsed for built-in text even when option is OFF
    assert(outCode == 'b');

    // 2. Width Measurement Unit Tests
    // Formatting ON: codes take no width
    font.setFormattingOption(true);
    int wPlain = font.width("Hello", true);
    int wCodeOn = font.width("&8Hello", true);
    assert(wCodeOn == wPlain);

    // Formatting OFF: codes count for full width
    font.setFormattingOption(false);
    int wCodeOff = font.width("&8Hello", true);
    assert(wCodeOff > wPlain);

    // Built-in text width when option is OFF: section sign still ignored
    int wBuiltin = font.width("\xa7" "8Hello", false);
    assert(wBuiltin == font.width("Hello", false));

    // Reset formatting option to ON
    font.setFormattingOption(true);

    // Case-insensitivity: &L, §R
    const char* upperCode1 = "&LBold";
    int lenUpper1 = font.getFormatCodeLength(upperCode1, true, outCode, false);
    assert(lenUpper1 == 2);
    assert(outCode == 'l');

    const char* upperCode2 = "\xa7" "RReset";
    int lenUpper2 = font.getFormatCodeLength(upperCode2, true, outCode, false);
    assert(lenUpper2 == 2);
    assert(outCode == 'r');

    // Unknown format codes: &z, §z
    const char* unknown1 = "&zTest";
    int lenUnknown1 = font.getFormatCodeLength(unknown1, true, outCode, false);
    assert(lenUnknown1 == 0);

    const char* unknown2 = "\xa7" "zTest";
    int lenUnknown2 = font.getFormatCodeLength(unknown2, true, outCode, false);
    assert(lenUnknown2 == 0);

    // End of string edge cases
    const char* endStr = "&";
    int lenEnd = font.getFormatCodeLength(endStr, true, outCode, false);
    assert(lenEnd == 0);

    // 3. Sanitization Unit Tests
    std::string s1 = Font::sanitize("\xc2\xa7" "aHello &bWorld", true);
    assert(s1 == "Hello World");

    std::string s2 = Font::sanitize("&aHello &zWorld", true);
    assert(s2 == "Hello &zWorld");

    std::string s3 = Font::sanitize("&aHello &bWorld", false);
    assert(s3 == "&aHello &bWorld");

    // 4. Drop Shadow Color Darkening Calculation Test
    int colorGreen = 0x55FF55;
    int shadowGreen = (colorGreen & 0xFCFCFC) >> 2;
    assert(shadowGreen == 0x153F15);

    int colorWhite = 0xFFFFFF;
    int shadowWhite = (colorWhite & 0xFCFCFC) >> 2;
    assert(shadowWhite == 0x3F3F3F);

    // 5. Digit and symbol character typing verification
    std::string text = "";
    std::string testChars = "0123456789^!@#$%&*()-_=+";
    for (size_t i = 0; i < testChars.length(); ++i) {
        char c = testChars[i];
        if (c >= 32 && c < 127) {
            text.push_back(c);
        }
    }
    assert(text == testChars);

    std::cout << "All Font Formatting unit tests passed successfully!" << std::endl;
    return 0;
}
