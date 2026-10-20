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

    // 1. Format Code Parsing Unit Tests
    char outCode = 0;

    // UTF-8 section sign §a (\xc2\xa7 + "aHello")
    const char* utf8Code = "\xc2\xa7" "aHello";
    int lenUtf8 = Font::getFormatCodeLength(utf8Code, true, outCode);
    assert(lenUtf8 == 3);
    assert(outCode == 'a');

    // Single-byte section sign \xa7b
    const char* sbCode = "\xa7" "bWorld";
    int lenSb = Font::getFormatCodeLength(sbCode, true, outCode);
    assert(lenSb == 2);
    assert(outCode == 'b');

    // Ampersand code &c (formatting option ON)
    const char* ampCode = "&cText";
    int lenAmp = Font::getFormatCodeLength(ampCode, true, outCode);
    assert(lenAmp == 2);
    assert(outCode == 'c');

    // Ampersand code &c (formatting option OFF)
    int lenAmpOff = Font::getFormatCodeLength(ampCode, false, outCode);
    assert(lenAmpOff == 0);

    // Section sign code \xa7c (formatting option OFF)
    int lenSecOff = Font::getFormatCodeLength(sbCode, false, outCode);
    assert(lenSecOff == 2);
    assert(outCode == 'b');

    // Case-insensitivity: &L, §R
    const char* upperCode1 = "&LBold";
    int lenUpper1 = Font::getFormatCodeLength(upperCode1, true, outCode);
    assert(lenUpper1 == 2);
    assert(outCode == 'l');

    const char* upperCode2 = "\xa7" "RReset";
    int lenUpper2 = Font::getFormatCodeLength(upperCode2, true, outCode);
    assert(lenUpper2 == 2);
    assert(outCode == 'r');

    // Unknown format codes: &z, §z
    const char* unknown1 = "&zTest";
    int lenUnknown1 = Font::getFormatCodeLength(unknown1, true, outCode);
    assert(lenUnknown1 == 0);

    const char* unknown2 = "\xa7" "zTest";
    int lenUnknown2 = Font::getFormatCodeLength(unknown2, true, outCode);
    assert(lenUnknown2 == 0);

    // End of string edge cases
    const char* endStr = "&";
    int lenEnd = Font::getFormatCodeLength(endStr, true, outCode);
    assert(lenEnd == 0);

    // 2. Sanitization Unit Tests
    std::string s1 = Font::sanitize("\xc2\xa7" "aHello &bWorld", true);
    assert(s1 == "Hello World");

    std::string s2 = Font::sanitize("&aHello &zWorld", true);
    assert(s2 == "Hello &zWorld");

    std::string s3 = Font::sanitize("&aHello &bWorld", false);
    assert(s3 == "&aHello &bWorld");

    // 3. Drop Shadow Color Darkening Calculation Test
    int colorGreen = 0x55FF55;
    int shadowGreen = (colorGreen & 0xFCFCFC) >> 2;
    assert(shadowGreen == 0x153F15);

    int colorWhite = 0xFFFFFF;
    int shadowWhite = (colorWhite & 0xFCFCFC) >> 2;
    assert(shadowWhite == 0x3F3F3F);

    std::cout << "All Font Formatting unit tests passed successfully!" << std::endl;
    return 0;
}
