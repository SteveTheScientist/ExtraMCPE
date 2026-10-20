#include "client/gui/Font.h"
#include "client/Options.h"
#include "client/renderer/Tesselator.h"
#include "client/renderer/Textures.h"
#include "client/renderer/RenderChunk.h"
#include "client/Minecraft.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// Stubs for unit test binary
RenderChunk::RenderChunk() {}
RenderChunk::RenderChunk(unsigned int a, int b) {}
void anGenBuffers(int n, unsigned int* buffers) {}

TextureId Textures::loadTexture(const std::string& resourceName, bool inTextureFolder) { return 0; }
const TextureData* Textures::getTemporaryTextureData(TextureId id) { return nullptr; }
TextureId Textures::loadAndBindTexture(const std::string& resourceName) { return 0; }

void Minecraft::optionUpdated(OptionId key, bool value) {}
void Minecraft::optionUpdated(OptionId key, float value) {}
void Minecraft::optionUpdated(OptionId key, int value) {}

int getTimeMs() { return 0; }

int main() {
    std::cout << "Running Font Formatting Unit Tests..." << std::endl;

    // 1. Test parseFormattingCode
    int codeLen = 0;
    char codeChar = 0;

    // Valid section sign codes
    assert(Font::parseFormattingCode("§cHello", true, codeLen, codeChar) == true);
    assert(codeLen == 3 || codeLen == 2);
    assert(codeChar == 'c');

    assert(Font::parseFormattingCode("§1Text", true, codeLen, codeChar) == true);
    assert(codeChar == '1');

    assert(Font::parseFormattingCode("§lBold", true, codeLen, codeChar) == true);
    assert(codeChar == 'l');

    assert(Font::parseFormattingCode("§RReset", true, codeLen, codeChar) == true);
    assert(codeChar == 'R');

    // Ampersand codes when formatting option is ON vs OFF
    assert(Font::parseFormattingCode("&aGreen", true, codeLen, codeChar) == true);
    assert(codeLen == 2);
    assert(codeChar == 'a');

    assert(Font::parseFormattingCode("&aGreen", false, codeLen, codeChar) == false);

    // Unknown codes
    assert(Font::parseFormattingCode("§zUnknown", true, codeLen, codeChar) == false);
    assert(Font::parseFormattingCode("&xUnknown", true, codeLen, codeChar) == false);
    assert(Font::parseFormattingCode("&", true, codeLen, codeChar) == false);
    assert(Font::parseFormattingCode("§", true, codeLen, codeChar) == false);

    // 2. Test updateFormatPrefix
    std::string prefix = "";
    prefix = Font::updateFormatPrefix(prefix, 'c');
    assert(prefix == "§c");

    prefix = Font::updateFormatPrefix(prefix, 'l');
    assert(prefix == "§c§l");

    // Color code resets styles
    prefix = Font::updateFormatPrefix(prefix, 'a');
    assert(prefix == "§a");

    // Reset code
    prefix = Font::updateFormatPrefix(prefix, 'r');
    assert(prefix == "");

    // 3. Test sanitize
    assert(Font::sanitize("§cHello §aWorld", true) == "Hello World");
    assert(Font::sanitize("&cHello &aWorld", true) == "Hello World");
    assert(Font::sanitize("&cHello &aWorld", false) == "&cHello &aWorld");
    assert(Font::sanitize("§zHello", true) == "§zHello");
    assert(Font::sanitize("&xHello", true) == "&xHello");

    // 4. Test Options Integration
    Options options(nullptr, "");
    options.set(OPTIONS_TEXT_FORMATTING, true);
    assert(options.getBooleanValue(OPTIONS_TEXT_FORMATTING) == true);

    options.set(OPTIONS_TEXT_FORMATTING, false);
    assert(options.getBooleanValue(OPTIONS_TEXT_FORMATTING) == false);

    std::cout << "All Font unit tests passed successfully!" << std::endl;
    return 0;
}
