#include "SimpleChooseLevelScreen.h"
#include "ProgressScreen.h"
#include "ScreenChooser.h"
#include "../components/Button.h"
#include "../components/ImageButton.h"
#include "../../Minecraft.h"
#include "../../../world/level/LevelSettings.h"
#include "../../../platform/time.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/log.h"
#include <locale/I18n.h>

SimpleChooseLevelScreen::SimpleChooseLevelScreen(const std::string& levelName)
:   bHeader(nullptr),
    bGamemode(nullptr),
    bCheats(nullptr),
    bBack(nullptr),
    bCreate(nullptr),
    levelName(levelName),
    hasChosen(false),
    gamemode(GameType::Survival),
    cheatsEnabled(false),
    tLevelName(0, "World name"),
    tSeed(1, "World seed")
{
}

SimpleChooseLevelScreen::~SimpleChooseLevelScreen()
{
    if (bHeader) delete bHeader;
    delete bGamemode;
    delete bCheats;
    delete bBack;
    delete bCreate;
}

void SimpleChooseLevelScreen::init()
{
    ChooseLevelScreen::init();

    tLevelName.text = I18n::get("selectWorld.newWorld");

    bHeader = new Touch::THeader(0, "Create New World");

    bBack = new ImageButton(2, "");
    {
        ImageDef def;
        def.name = "gui/touchgui.png";
        def.width = 34;
        def.height = 26;
        def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
        bBack->setImageDef(def, true);
    }

    bGamemode = new Button(1, "Game Mode: Survival");
    bCheats  = new Button(4, "Cheats: Off");
    bCreate  = new Button(3, "Create World");

    buttons.push_back(bHeader);
    buttons.push_back(bBack);
    buttons.push_back(bGamemode);
    buttons.push_back(bCheats);
    buttons.push_back(bCreate);

    tabButtons.push_back(bGamemode);
    tabButtons.push_back(bCheats);
    tabButtons.push_back(bBack);
    tabButtons.push_back(bCreate);

    textBoxes.push_back(&tLevelName);
    textBoxes.push_back(&tSeed);
}

void SimpleChooseLevelScreen::setupPositions()
{
    int headerH = bBack->height;

    bBack->x = width - bBack->width - 4;
    bBack->y = 2;

    if (bHeader) {
        bHeader->x = 0;
        bHeader->y = 0;
        bHeader->width = width - bBack->width - 8;
        bHeader->height = headerH;
    }

    int centerX = width / 2;

    tLevelName.width = tSeed.width = 200;
    tLevelName.x = centerX - 100;
    tLevelName.y = headerH + 20;

    tSeed.x = centerX - 100;
    tSeed.y = tLevelName.y + 32;

    bGamemode->width = 200;
    bGamemode->height = 20;
    bGamemode->x = centerX - 100;
    bGamemode->y = tSeed.y + 32;

    bCheats->width = 200;
    bCheats->height = 20;
    bCheats->x = centerX - 100;
    bCheats->y = bGamemode->y + 32;

    bCreate->width = 200;
    bCreate->height = 20;
    bCreate->x = centerX - 100;
    bCreate->y = height - 28;
}

void SimpleChooseLevelScreen::tick()
{
    for (auto* tb : textBoxes)
        tb->tick(minecraft);
}

void SimpleChooseLevelScreen::render( int xm, int ym, float a )
{
    renderDirtBackground(0);
    glEnable2(GL_BLEND);

    drawString(minecraft->font, "World Name:", tLevelName.x, tLevelName.y - Font::DefaultLineHeight - 1, 0xffffffff);
    drawString(minecraft->font, "Seed (leave blank for random):", tSeed.x, tSeed.y - Font::DefaultLineHeight - 1, 0xffffffff);

    const char* modeDesc = NULL;
    if (gamemode == GameType::Survival) {
        modeDesc = "Mobs, health and gather resources";
    } else if (gamemode == GameType::Creative) {
        modeDesc = "Unlimited resources and flying";
    }
    if (modeDesc) {
        drawCenteredString(minecraft->font, modeDesc, width / 2, bGamemode->y + bGamemode->height + 2, 0xffcccccc);
    }

    Screen::render(xm, ym, a);
    glDisable2(GL_BLEND);
}

void SimpleChooseLevelScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == MouseAction::ACTION_LEFT) {
        int lvlTop = tLevelName.y - (Font::DefaultLineHeight + 4);
        int lvlBottom = tLevelName.y + tLevelName.height;
        int lvlLeft = tLevelName.x;
        int lvlRight = tLevelName.x + tLevelName.width;
        bool clickedLevel = x >= lvlLeft && x < lvlRight && y >= lvlTop && y < lvlBottom;

        int seedTop = tSeed.y - (Font::DefaultLineHeight + 4);
        int seedBottom = tSeed.y + tSeed.height;
        int seedLeft = tSeed.x;
        int seedRight = tSeed.x + tSeed.width;
        bool clickedSeed  = x >= seedLeft && x < seedRight && y >= seedTop && y < seedBottom;

        if (clickedLevel) {
            tLevelName.setFocus(minecraft);
            tSeed.loseFocus(minecraft);
        } else if (clickedSeed) {
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
    if (hasChosen)
        return;

    if (button == bGamemode) {
        gamemode ^= 1;
        bGamemode->msg = (gamemode == GameType::Survival) ? "Game Mode: Survival" : "Game Mode: Creative";
        return;
    }

    if (button == bCheats) {
        cheatsEnabled = !cheatsEnabled;
        bCheats->msg = cheatsEnabled ? "Cheats: On" : "Cheats: Off";
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

    if (button == bBack) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
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
