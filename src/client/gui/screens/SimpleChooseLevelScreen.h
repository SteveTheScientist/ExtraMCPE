#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__

#include "ChooseLevelScreen.h"
#include "../components/TextBox.h"
#include "../components/Button.h"

class Button;

class SimpleChooseLevelScreen: public ChooseLevelScreen
{
public:
	SimpleChooseLevelScreen(const std::string& levelName);

	virtual ~SimpleChooseLevelScreen();

	void init();
	void setupPositions();
	void tick();

	void render(int xm, int ym, float a);

	void buttonClicked(Button* button);
	bool handleBackEvent(bool isDown);
	virtual void keyPressed(int eventKey);
	virtual void mouseClicked(int x, int y, int buttonNum);

private:
	// Tabs
	Button* bTabGame;
	Button* bTabWorld;
	Button* bTabMore;

	// Tab 0 (Game)
	Button* bGamemode;
	Button* bDifficulty;
	Button* bCheats;

	// Tab 1 (World)
	Button* bWorldType;

	// Tab 2 (More)
	Button* bGenerateStructures;
	Button* bBonusChest;

	// Bottom action buttons
	Button* bCreate;
	Button* bCancel;

	int activeTab; // 0 = Game, 1 = World, 2 = More
	int difficulty; // 0 = Peaceful, 1 = Easy, 2 = Normal, 3 = Hard
	int gamemode;
	bool cheatsEnabled;
	bool generateStructures;
	bool bonusChest;
	bool hasChosen;

	std::string levelName;

	TextBox tLevelName;
	TextBox tSeed;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__DemoChooseLevelScreen_H__*/
