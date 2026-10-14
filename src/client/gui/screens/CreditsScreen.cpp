#include "CreditsScreen.h"
#include "StartMenuScreen.h"
#include "OptionsScreen.h"
#include "PauseScreen.h"
#include "../../Minecraft.h"
#include "../components/Button.h"
#include "platform/input/Mouse.h"

CreditsScreen::CreditsScreen(CreditsReturnTarget returnTarget)
: btnDone(NULL), m_returnTarget(returnTarget), _scrollY(0.0f), _scrollSpeed(0.5f)
{}

CreditsScreen::~CreditsScreen() {
    delete btnDone;
}

void CreditsScreen::init() {
    btnDone = new Button(1, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    _lines.clear();
    _lines.push_back("ExtraMC / Minecraft: Pocket Edition");
    _lines.push_back("Original game by Mojang AB");
    _lines.push_back("");
    _lines.push_back("--- ExtraMC Developers ---");
    _lines.push_back("mschiller890");
    _lines.push_back("InviseDivine");
    _lines.push_back("Kolyah35");
    _lines.push_back("karson");
    _lines.push_back("deepfriedwaffles");
    _lines.push_back("EpikIzCool");
    _lines.push_back("fileshredder");
    _lines.push_back("");
    _lines.push_back("Special Thanks to the Minecraft PE Community");
    _lines.push_back("");
    _lines.push_back("Join our Discord server:");
    _lines.push_back("https://discord.gg/c58YesBxve");

    _scrollSpeed = 0.5f;
    _scrollY = (float)height - 40; // start below screen title area
}

void CreditsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
    }
}

void CreditsScreen::tick() {
    super::tick();

    _scrollY -= _scrollSpeed;
    float totalHeight = _lines.size() * (minecraft->font->lineHeight + 6);
    if (_scrollY + totalHeight < 40) {
        _scrollY = (float)height - 40;
    }

    if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
        _scrollSpeed = 1.8f;
    } else {
        _scrollSpeed = 0.5f;
    }
}

void CreditsScreen::render(int xm, int ym, float a) {
    if (m_returnTarget == CREDITS_RETURN_PAUSE && minecraft->isLevelGenerated()) {
        renderBackground();
    } else {
        renderDirtBackground(0);
    }

    Font* font = minecraft->font;
    float y = _scrollY;
    const float lineHeight = font->lineHeight + 6;

    for (size_t i = 0; i < _lines.size(); ++i) {
        const std::string& line = _lines[i];
        if (y > 25 && y < height - 35) {
            float lineWidth = Gui::getColoredWidth(font, line);
            Gui::drawColoredString(font, line, (width - (int)lineWidth) / 2, (int)y, 255);

            if (line.find("http") != std::string::npos || line.find("discord.gg") != std::string::npos) {
                float x0 = (width - lineWidth) / 2;
                float y0 = y + font->lineHeight;
                fill(x0, y0, x0 + lineWidth, y0 + 1, 0xff55ffff);
            }
        }
        y += lineHeight;
    }

    // Top and bottom header/footer shadow masks
    fill(0, 0, width, 32, 0x70000000);
    fill(0, height - 36, width, height, 0x70000000);

    drawCenteredString(font, "Credits", width / 2, 10, 0xffffffff);

    super::render(xm, ym, a);
}

void CreditsScreen::buttonClicked(Button* button) {
    if (button->id == 1) {
        if (m_returnTarget == CREDITS_RETURN_OPTIONS) {
            minecraft->setScreen(new OptionsScreen());
        } else if (m_returnTarget == CREDITS_RETURN_PAUSE) {
            minecraft->setScreen(new PauseScreen(false));
        } else {
            minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        }
    }
}

void CreditsScreen::mouseClicked(int x, int y, int buttonNum) {
    const float lineHeight = minecraft->font->lineHeight + 6;
    for (size_t i = 0; i < _lines.size(); ++i) {
        float lineY = _scrollY + i * lineHeight;
        if (y >= lineY && y < lineY + lineHeight && y > 30 && y < height - 35) {
            const std::string& line = _lines[i];
            size_t start = line.find("http");
            if (start == std::string::npos)
                start = line.find("discord.gg");
            if (start != std::string::npos) {
                size_t end = line.find(' ', start);
                std::string url = line.substr(start, (end == std::string::npos) ? std::string::npos : end - start);
                minecraft->platform()->openURL(url);
                return;
            }
        }
    }
    super::mouseClicked(x, y, buttonNum);
}
