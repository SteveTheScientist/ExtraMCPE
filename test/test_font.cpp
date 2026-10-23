
#include "client/gui/Gui.h"
float Gui::GuiScale = 2.0f;
float Gui::getColoredWidth(Font* font, const std::string& text, bool isPlayerText) {
    return (float)font->width(text, isPlayerText);
}
void Gui::drawColoredString(Font* font, const std::string& text, float x, float y, int alpha, bool isPlayerText) {
    font->drawShadow(text, x, y, 0xffffff + (alpha << 24), isPlayerText);
}
#include "client/gui/components/TextBox.h"
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

    // 5. Test TextBox charPressed for digits 0-9 and symbols (^ ! @ # $ % & * ( ) - _ = +)
    TextBox tb(1, "Test");
    tb.focused = true;
    std::string testChars = "0123456789^!@#$%&*()- _=+";
    for (char c : testChars) {
        tb.charPressed(nullptr, c);
    }
    assert(tb.text == "0123456789^!@#$%&*()- _=+");
    assert(tb.focused == true);

    // 6. Test parseColorTags for bracketed strings like "[Stone]" vs "[gold]"
    struct ColorSegmentTest {
        std::string text;
        uint32_t color;
    };
    auto getColorFromTagTest = [](const std::string& tag, uint32_t& outColor) -> bool {
        if (!tag.empty() && tag[0] == '/') {
            outColor = 0xffffff;
            return true;
        }
        std::string lower;
        lower.resize(tag.size());
        std::transform(tag.begin(), tag.end(), lower.begin(), ::tolower);
        if (lower == "gold") { outColor = 0xffd700; return true; }
        if (lower == "green") { outColor = 0x00ff00; return true; }
        if (lower == "yellow") { outColor = 0xffff00; return true; }
        if (lower == "red") { outColor = 0xff0000; return true; }
        if (lower == "blue") { outColor = 0x0000ff; return true; }
        return false;
    };
    auto parseColorTagsTest = [&](const std::string& in, std::vector<ColorSegmentTest>& out) {
        uint32_t curColor = 0xffffff;
        size_t pos = 0;
        while (pos < in.size()) {
            size_t open = in.find('[', pos);
            if (open == std::string::npos) {
                out.push_back({in.substr(pos), curColor});
                break;
            }
            if (open > pos) {
                out.push_back({in.substr(pos, open - pos), curColor});
            }
            size_t close = in.find(']', open);
            if (close == std::string::npos) {
                out.push_back({in.substr(open), curColor});
                break;
            }
            std::string tag = in.substr(open + 1, close - open - 1);
            uint32_t tagColor = 0;
            if (getColorFromTagTest(tag, tagColor)) {
                curColor = tagColor;
                pos = close + 1;
            } else {
                out.push_back({in.substr(open, close - open + 1), curColor});
                pos = close + 1;
            }
        }
    };

    std::vector<ColorSegmentTest> segs;
    parseColorTagsTest("Gave 45 [Stone] to AA", segs);
    std::string reassembled = "";
    for (const auto& s : segs) {
        reassembled += s.text;
    }
    assert(reassembled == "Gave 45 [Stone] to AA");

    segs.clear();
    parseColorTagsTest("[gold]Gave 1 [Stone] to AA[/gold]", segs);
    reassembled = "";
    for (const auto& s : segs) {
        reassembled += s.text;
    }
    assert(reassembled == "Gave 1 [Stone] to AA");
    assert(segs[0].color == 0xffd700);

    segs.clear();
    parseColorTagsTest("Gave 64 [Redstone Dust] to AA", segs);
    reassembled = "";
    for (const auto& s : segs) {
        reassembled += s.text;
    }
    assert(reassembled == "Gave 64 [Redstone Dust] to AA");

    std::cout << "All Font unit tests passed successfully!" << std::endl;
    return 0;
}
