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
:   bTabGame(nullptr),
    bTabWorld(nullptr),
    bTabMore(nullptr),
    bGamemode(nullptr),
    bDifficulty(nullptr),
    bCheats(nullptr),
    bWorldType(nullptr),
    bGenerateStructures(nullptr),
    bBonusChest(nullptr),
    bCreate(nullptr),
    bCancel(nullptr),
    activeTab(0),
    difficulty(2), // Normal
    gamemode(GameType::Survival),
    cheatsEnabled(false),
    generateStructures(true),
    bonusChest(false),
    hasChosen(false),
    levelName(levelName),
    tLevelName(0, "New World"),
    tSeed(1, "Leave blank for random")
{
}

SimpleChooseLevelScreen::~SimpleChooseLevelScreen()
{
    delete bTabGame;
    delete bTabWorld;
    delete bTabMore;
    delete bGamemode;
    delete bDifficulty;
    delete bCheats;
    delete bWorldType;
    delete bGenerateStructures;
    delete bBonusChest;
    delete bCreate;
    delete bCancel;
}

void SimpleChooseLevelScreen::init()
{
    ChooseLevelScreen::init();

    tLevelName.text = "New World";

    // Tabs
    bTabGame  = new Button(10, "Game");
    bTabWorld = new Button(11, "World");
    bTabMore  = new Button(12, "More");

    // Tab 0
    bGamemode   = new Button(1, "Game Mode: Survival");
    bDifficulty = new Button(2, "Difficulty: Normal");
    bCheats     = new Button(4, "Allow Commands: OFF");

    // Tab 1
    bWorldType  = new Button(20, "World Type: Default");

    // Tab 2
    bGenerateStructures = new Button(30, "Generate Structures: ON");
    bBonusChest         = new Button(31, "Bonus Chest: OFF");

    // Bottom
    bCreate = new Button(3, "Create New World");
    bCancel = new Button(5, "Cancel");

    buttons.push_back(bTabGame);
    buttons.push_back(bTabWorld);
    buttons.push_back(bTabMore);

    buttons.push_back(bGamemode);
    buttons.push_back(bDifficulty);
    buttons.push_back(bCheats);

    buttons.push_back(bWorldType);

    buttons.push_back(bGenerateStructures);
    buttons.push_back(bBonusChest);

    buttons.push_back(bCreate);
    buttons.push_back(bCancel);

    textBoxes.push_back(&tLevelName);
    textBoxes.push_back(&tSeed);

    setupPositions();
}

void SimpleChooseLevelScreen::setupPositions()
{
    int centerX = width / 2;

    // Tabs
    int tabW = 90;
    int tabH = 20;
    bTabGame->width  = tabW; bTabGame->height  = tabH;
    bTabWorld->width = tabW; bTabWorld->height = tabH;
    bTabMore->width  = tabW; bTabMore->height  = tabH;

    bTabGame->x  = centerX - 140; bTabGame->y  = 6;
    bTabWorld->x = centerX - 45;  bTabWorld->y = 6;
    bTabMore->x  = centerX + 50;  bTabMore->y  = 6;

    // Bottom bar
    int btnW = 145;
    int btnH = 20;
    int bottomY = height - 26;

    bCreate->width = btnW; bCreate->height = btnH;
    bCreate->x = centerX - 150; bCreate->y = bottomY;

    bCancel->width = btnW; bCancel->height = btnH;
    bCancel->x = centerX + 5; bCancel->y = bottomY;

    // Tab 0 (Game)
    int fieldW = 210;
    int fieldX = centerX - fieldW / 2;

    tLevelName.width = fieldW; tLevelName.height = 20;
    tLevelName.x = fieldX; tLevelName.y = 52;

    bGamemode->width = fieldW; bGamemode->height = btnH;
    bGamemode->x = fieldX; bGamemode->y = 84;

    bDifficulty->width = fieldW; bDifficulty->height = btnH;
    bDifficulty->x = fieldX; bDifficulty->y = 110;

    bCheats->width = fieldW; bCheats->height = btnH;
    bCheats->x = fieldX; bCheats->y = 136;

    // Tab 1 (World)
    tSeed.width = fieldW; tSeed.height = 20;
    tSeed.x = fieldX; tSeed.y = 52;

    bWorldType->width = fieldW; bWorldType->height = btnH;
    bWorldType->x = fieldX; bWorldType->y = 96;

    // Tab 2 (More)
    bGenerateStructures->width = fieldW; bGenerateStructures->height = btnH;
    bGenerateStructures->x = fieldX; bGenerateStructures->y = 52;

    bBonusChest->width = fieldW; bBonusChest->height = btnH;
    bBonusChest->x = fieldX; bBonusChest->y = 78;

    // Visibility management based on activeTab
    bGamemode->visible   = (activeTab == 0);
    bDifficulty->visible = (activeTab == 0);
    bCheats->visible     = (activeTab == 0);

    bWorldType->visible  = (activeTab == 1);

    bGenerateStructures->visible = (activeTab == 2);
    bBonusChest->visible         = (activeTab == 2);
}

void SimpleChooseLevelScreen::tick()
{
    if (activeTab == 0) tLevelName.tick(minecraft);
    else if (activeTab == 1) tSeed.tick(minecraft);
}

void SimpleChooseLevelScreen::render( int xm, int ym, float a )
{
    renderDirtBackground(0);
    glEnable2(GL_BLEND);

    // Separator lines
    fill(0, 30, width, 31, 0xff555555);
    fill(0, height - 32, width, height - 31, 0xff555555);

    // Highlight active tab
    Button* activeTabBtn = (activeTab == 0)? bTabGame : ((activeTab == 1)? bTabWorld : bTabMore);
    if (activeTabBtn) {
        fill(activeTabBtn->x, activeTabBtn->y + activeTabBtn->height - 2, activeTabBtn->x + activeTabBtn->width, activeTabBtn->y + activeTabBtn->height, 0xffffffff);
    }

    if (activeTab == 0) {
        drawString(minecraft->font, "World Name", tLevelName.x, tLevelName.y - Font::DefaultLineHeight - 1, 0xffa0a0a0);
        tLevelName.render(minecraft, xm, ym);
    } else if (activeTab == 1) {
        drawString(minecraft->font, "World Seed", tSeed.x, tSeed.y - Font::DefaultLineHeight - 1, 0xffa0a0a0);
        tSeed.render(minecraft, xm, ym);
        drawString(minecraft->font, "Leave blank for random seed", tSeed.x, tSeed.y + tSeed.height + 3, 0xff808080);
    }

    // Render buttons
    for (unsigned int i = 0; i < buttons.size(); i++) {
        buttons[i]->render(minecraft, xm, ym);
    }

    glDisable2(GL_BLEND);
}

void SimpleChooseLevelScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (activeTab == 0) {
            if (tLevelName.pointInside(x, y)) {
                tLevelName.setFocus(minecraft);
            } else {
                tLevelName.loseFocus(minecraft);
            }
        } else if (activeTab == 1) {
            if (tSeed.pointInside(x, y)) {
                tSeed.setFocus(minecraft);
            } else {
                tSeed.loseFocus(minecraft);
            }
        }
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void SimpleChooseLevelScreen::buttonClicked( Button* button )
{
    if (hasChosen) return;

    if (button == bTabGame)  { activeTab = 0; setupPositions(); return; }
    if (button == bTabWorld) { activeTab = 1; setupPositions(); return; }
    if (button == bTabMore)  { activeTab = 2; setupPositions(); return; }

    if (button == bGamemode) {
        gamemode ^= 1;
        bGamemode->msg = (gamemode == GameType::Survival) ? "Game Mode: Survival" : "Game Mode: Creative";
        return;
    }

    if (button == bDifficulty) {
        difficulty = (difficulty + 1) % 4;
        const char* diffNames[] = { "Peaceful", "Easy", "Normal", "Hard" };
        bDifficulty->msg = std::string("Difficulty: ") + diffNames[difficulty];
        return;
    }

    if (button == bCheats) {
        cheatsEnabled = !cheatsEnabled;
        bCheats->msg = cheatsEnabled ? "Allow Commands: ON" : "Allow Commands: OFF";
        return;
    }

    if (button == bWorldType) {
        bWorldType->msg = "World Type: Default";
        return;
    }

    if (button == bGenerateStructures) {
        generateStructures = !generateStructures;
        bGenerateStructures->msg = generateStructures ? "Generate Structures: ON" : "Generate Structures: OFF";
        return;
    }

    if (button == bBonusChest) {
        bonusChest = !bonusChest;
        bBonusChest->msg = bonusChest ? "Bonus Chest: ON" : "Bonus Chest: OFF";
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
