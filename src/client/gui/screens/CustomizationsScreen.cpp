#include "CustomizationsScreen.h"
#include "StartMenuScreen.h"
#include "PauseScreen.h"
#include "../../Minecraft.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"

CustomizationsScreen::CustomizationsScreen()
    : btnDone(NULL),
      sliderFov(NULL)
{
}

CustomizationsScreen::~CustomizationsScreen() {
    delete btnDone;
    delete sliderFov;
}

void CustomizationsScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    sliderFov = new OptionControl(101, CONTROL_SLIDER, OPTIONS_SENSITIVITY, "Field of View", false, 0.0f, 1.0f);
    sliderFov->updateLabel(minecraft);
    buttons.push_back(sliderFov);
}

void CustomizationsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
    }

    if (sliderFov) {
        sliderFov->width = 200;
        sliderFov->height = 20;
        sliderFov->x = (width - 200) / 2;
        sliderFov->y = height / 2 - 10;
        sliderFov->visible = true;
        sliderFov->updateLabel(minecraft);
    }
}

void CustomizationsScreen::tick() {
    super::tick();

    int xm = Mouse::getX();
    int ym = Mouse::getY();
    toGUICoordinate(xm, ym);

    if (sliderFov && sliderFov->isDraggingSlider) {
        if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
            sliderFov->updateSliderValue(minecraft, xm);
        } else {
            sliderFov->isDraggingSlider = false;
        }
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

void CustomizationsScreen::mouseClicked(int x, int y, int buttonNum) {
    super::mouseClicked(x, y, buttonNum);
}

void CustomizationsScreen::mouseReleased(int x, int y, int buttonNum) {
    if (sliderFov) {
        sliderFov->isDraggingSlider = false;
    }
    super::mouseReleased(x, y, buttonNum);
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
