#include "client/gui/Font.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/Textures.h"
#include "util/StringUtils.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <cctype>

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

static void testGetWordBounds(const std::string& str, int idx, int& outStart, int& outEnd) {
    int len = (int)str.length();
    if (len == 0) {
        outStart = outEnd = 0;
        return;
    }
    if (idx < 0) idx = 0;
    if (idx >= len) idx = len - 1;

    bool isWordChar = isalnum((unsigned char)str[idx]) || str[idx] == '_';

    int s = idx;
    while (s > 0) {
        bool prevWord = isalnum((unsigned char)str[s - 1]) || str[s - 1] == '_';
        if (prevWord != isWordChar) break;
        s--;
    }

    int e = idx;
    while (e < len) {
        bool currWord = isalnum((unsigned char)str[e]) || str[e] == '_';
        if (currWord != isWordChar) break;
        e++;
    }

    outStart = s;
    outEnd = e;
}

int main() {
    std::cout << "Running Font Formatting, Clipboard & TextBox Selection Unit Tests..." << std::endl;

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
    font.setFormattingOption(true);
    int wPlain = font.width("Hello", true);
    int wCodeOn = font.width("&8Hello", true);
    assert(wCodeOn == wPlain);

    font.setFormattingOption(false);
    int wCodeOff = font.width("&8Hello", true);
    assert(wCodeOff > wPlain);

    int wBuiltin = font.width("\xa7" "8Hello", false);
    assert(wBuiltin == font.width("Hello", false));

    font.setFormattingOption(true);

    // 3. TextBox Word Bounds Unit Tests
    std::string sampleText = "&aHello &bWorld";

    int wStart, wEnd;
    testGetWordBounds(sampleText, 3, wStart, wEnd); // "Hello"
    assert(wStart == 2);
    assert(wEnd == 7);

    testGetWordBounds(sampleText, 10, wStart, wEnd); // "World"
    assert(wStart == 10);
    assert(wEnd == 15);

    // 4. Clean Pasted Text Unit Tests
    std::string multilinePasted = "Line1\r\nLine2\nLine3";
    std::string cleaned1 = Util::cleanPastedText(multilinePasted, 256);
    assert(cleaned1 == "Line1 Line2 Line3");

    std::cout << "All Font Formatting, Clipboard & TextBox Selection unit tests passed successfully!" << std::endl;
    return 0;
}
