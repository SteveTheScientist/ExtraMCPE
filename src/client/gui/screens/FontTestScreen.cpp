#include "FontTestScreen.h"
#include "StartMenuScreen.h"
#include "../../Minecraft.h"
#include "../Font.h"

FontTestScreen::FontTestScreen()
    : btnBack(NULL)
{
}

FontTestScreen::~FontTestScreen() {
    delete btnBack;
}

void FontTestScreen::init() {
    btnBack = new Button(1, 0, 0, 200, 20, "Back");
    buttons.push_back(btnBack);
    tabButtons.push_back(btnBack);
}

void FontTestScreen::setupPositions() {
    if (btnBack) {
        btnBack->width = 200;
        btnBack->height = 20;
        btnBack->x = (width - 200) / 2;
        btnBack->y = height - 28;
    }
}

void FontTestScreen::render(int xm, int ym, float a) {
    renderDirtBackground(0);

    Font* font = minecraft->font;

    int yPos = 10;
    font->drawShadow("Font Rendering System Test", 10, (float)yPos, 0xffffff);
    yPos += 14;

    font->drawShadow("ASCII Printable Range (U+0020 .. U+007E):", 10, (float)yPos, 0xffff55);
    yPos += 12;

    std::string asciiRange = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
    font->draw(asciiRange, 10, (float)yPos, 0xffffffff);
    yPos += 16;

    font->drawShadow("Sample Menu Strings:", 10, (float)yPos, 0xffff55);
    yPos += 12;

    font->drawShadow("Username", 10, (float)yPos, 0xffffffff);
    yPos += 12;
    font->drawShadow("Customizations...", 10, (float)yPos, 0xffffffff);
    yPos += 12;
    font->drawShadow("Please choose a username so others can easily identify you in chat.", 10, (float)yPos, 0xaaaaaa);
    yPos += 12;
    font->drawShadow("Max Framerate: Unlimited", 10, (float)yPos, 0xffffffff);

    super::render(xm, ym, a);
}

void FontTestScreen::buttonClicked(Button* button) {
    if (button == btnBack) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
    }
}
