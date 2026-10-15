#include "SimpleChooseLevelScreen.h"
#include "ProgressScreen.h"
#include "ScreenChooser.h"
#include "../components/Button.h"
#include "../../Minecraft.h"
#include "../../../world/level/LevelSettings.h"
#include "../../../platform/time.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/log.h"
#include <locale/I18n.h>

SimpleChooseLevelScreen::SimpleChooseLevelScreen(const std::string& levelName)
:   bGamemode(nullptr),
    bCheats(nullptr),
    bCreate(nullptr),
    bCancel(nullptr),
    gamemode(GameType::Survival),
    cheatsEnabled(false),
    hasChosen(false),
    levelName(levelName),
    tLevelName(0, "New World"),
    tSeed(1, "Leave blank for random")
{
}

SimpleChooseLevelScreen::~SimpleChooseLevelScreen()
{
    delete bGamemode;
    delete bCheats;
    delete bCreate;
    delete bCancel;
}

void SimpleChooseLevelScreen::init()
{
    ChooseLevelScreen::init();

    tLevelName.text = "New World";

    bGamemode = new Button(1, "Game Mode: Survival");
    bCheats   = new Button(4, "Allow Commands: OFF");
    bCreate   = new Button(3, "Create New World");
    bCancel   = new Button(5, "Cancel");

    buttons.push_back(bGamemode);
    buttons.push_back(bCheats);
    buttons.push_back(bCreate);
    buttons.push_back(bCancel);

    textBoxes.push_back(&tLevelName);
    textBoxes.push_back(&tSeed);

    setupPositions();
}

void SimpleChooseLevelScreen::setupPositions()
{
    int centerX = width / 2;
    int fieldW = 210;
    int btnH = 20;
    int fieldX = centerX - fieldW / 2;

    tLevelName.width = fieldW; tLevelName.height = 20;
    tLevelName.x = fieldX; tLevelName.y = 40;

    tSeed.width = fieldW; tSeed.height = 20;
    tSeed.x = fieldX; tSeed.y = 80;

    bGamemode->width = fieldW; bGamemode->height = btnH;
    bGamemode->x = fieldX; bGamemode->y = 120;

    bCheats->width = fieldW; bCheats->height = btnH;
    bCheats->x = fieldX; bCheats->y = 144;

    // Bottom action buttons
    int bottomY = height - 28;
    int btnW = 145;

    bCreate->width = btnW; bCreate->height = btnH;
    bCreate->x = centerX - 150; bCreate->y = bottomY;

    bCancel->width = btnW; bCancel->height = btnH;
    bCancel->x = centerX + 5; bCancel->y = bottomY;
}

void SimpleChooseLevelScreen::tick()
{
    tLevelName.tick(minecraft);
    tSeed.tick(minecraft);
}

void SimpleChooseLevelScreen::render( int xm, int ym, float a )
{
    renderDirtBackground(0);
    glEnable2(GL_BLEND);

    // Header title
    drawCenteredString(minecraft->font, "Create New World", width / 2, 12, 0xffffffff);

    // Field labels
    drawString(minecraft->font, "World Name", tLevelName.x, tLevelName.y - Font::DefaultLineHeight - 1, 0xffa0a0a0);
    tLevelName.render(minecraft, xm, ym);

    drawString(minecraft->font, "Seed for the World Generator", tSeed.x, tSeed.y - Font::DefaultLineHeight - 1, 0xffa0a0a0);
    tSeed.render(minecraft, xm, ym);

    // Separator line above bottom buttons
    fill(0, height - 34, width, height - 33, 0xff555555);

    // Render buttons
    for (unsigned int i = 0; i < buttons.size(); i++) {
        buttons[i]->render(minecraft, xm, ym);
    }

    glDisable2(GL_BLEND);
}

void SimpleChooseLevelScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (tLevelName.pointInside(x, y)) {
            tLevelName.setFocus(minecraft);
            tSeed.loseFocus(minecraft);
        } else if (tSeed.pointInside(x, y)) {
            tSeed.setFocus(minecraft);
            tLevelName.loseFocus(minecraft);
        } else {
            tLevelName.loseFocus(minecraft);
            tSeed.loseFocus(minecraft);
        }
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void SimpleChooseLevelScreen::buttonClicked( Button* button )
{
    if (hasChosen) return;

    if (button == bGamemode) {
        gamemode ^= 1;
        bGamemode->msg = (gamemode == GameType::Survival) ? "Game Mode: Survival" : "Game Mode: Creative";
        return;
    }

    if (button == bCheats) {
        cheatsEnabled = !cheatsEnabled;
        bCheats->msg = cheatsEnabled ? "Allow Commands: ON" : "Allow Commands: OFF";
        return;
    }

    if (button == bCancel) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return;
    }

    if (button == bCreate && !tLevelName.text.empty()) {
        int seed = getEpochTimeS();
        if (!tSeed.text.empty()) {
            std::string seedString = Util::stringTrim(tSeed.text);
            int tmpSeed;
            if (sscanf(seedString.c_str(), "%d", &tmpSeed) > 0) {
                seed = tmpSeed;
            } else {
                seed = Util::hashCode(seedString);
            }
        }
        std::string levelId = getUniqueLevelName(tLevelName.text);
        LevelSettings settings(seed, gamemode, cheatsEnabled);
        minecraft->selectLevel(levelId, levelId, settings);
        minecraft->hostMultiplayer();
        minecraft->setScreen(new ProgressScreen());
        hasChosen = true;
        return;
    }
}

void SimpleChooseLevelScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return;
    }
    Screen::keyPressed(eventKey);
}

bool SimpleChooseLevelScreen::handleBackEvent(bool isDown) {
	if (!isDown)
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	return true;
}
