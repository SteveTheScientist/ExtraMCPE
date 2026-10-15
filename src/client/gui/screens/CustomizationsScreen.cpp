#include "CustomizationsScreen.h"
#include "StartMenuScreen.h"
#include "PauseScreen.h"
#include "../../Minecraft.h"
#include "platform/input/Keyboard.h"

CustomizationsScreen::CustomizationsScreen()
    : btnDone(NULL)
{
}

CustomizationsScreen::~CustomizationsScreen() {
    delete btnDone;
}

void CustomizationsScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);
}

void CustomizationsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
    }
}

void CustomizationsScreen::render(int xm, int ym, float a) {
    if (minecraft->isLevelGenerated()) {
        renderBackground();
    } else {
        renderDirtBackground(0);
    }

    // Top and bottom header/footer translucent bars
    fill(0, 0, width, 32, 0x70000000);
    fill(0, height - 34, width, height, 0x70000000);

    super::render(xm, ym, a);

    drawCenteredString(minecraft->font, "Customizations", width / 2, 10, 0xffffffff);
}

void CustomizationsScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        if (minecraft->isLevelGenerated()) {
            minecraft->setScreen(new PauseScreen(false));
        } else {
            minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        }
    }
}

void CustomizationsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        if (minecraft->isLevelGenerated()) {
            minecraft->setScreen(new PauseScreen(false));
        } else {
            minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        }
        return;
    }
    super::keyPressed(eventKey);
}
