#include "PauseScreen.h"
#include "StartMenuScreen.h"
#include "OptionsScreen.h"
#include "CreditsScreen.h"
#include "CustomizationsScreen.h"
#include "../../Minecraft.h"
#include "../../../network/RakNetInstance.h"
#include "client/gui/components/Button.h"

#include "world/level/Level.h"

PauseScreen::PauseScreen(bool wasBackPaused)
	: visibleTime(0),
	  wasBackPaused(wasBackPaused),
	  bContinue(NULL),
	  bCustomizations(NULL),
	  bQuit(NULL),
	  bOptions(NULL),
	  bCredits(NULL),
	  bAllowCommands(NULL)
{
}

PauseScreen::~PauseScreen() {
	delete bContinue;
	delete bCustomizations;
	delete bQuit;
	delete bOptions;
	delete bCredits;
	delete bAllowCommands;
}

void PauseScreen::init() {
	bool isMP = (minecraft->raknetInstance && !minecraft->raknetInstance->isServer());
	std::string quitText = isMP ? "Disconnect" : "Save and Quit to Title";

	bool allowCmds = true;
	if (minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
		allowCmds = minecraft->level->getLevelData()->getAllowCheats();
	}
	std::string allowCmdsText = allowCmds ? "Allow Commands: ON" : "Allow Commands: OFF";

	bContinue = new Button(1, 0, 0, 200, 20, "Back to Game");
	bCustomizations = new Button(5, 0, 0, 200, 20, "Customizations...");
	bOptions = new Button(2, 0, 0, 98, 20, "Options...");
	bCredits = new Button(3, 0, 0, 98, 20, "Credits...");
	bAllowCommands = new Button(6, 0, 0, 200, 20, allowCmdsText);
	bQuit = new Button(4, 0, 0, 200, 20, quitText);

	buttons.push_back(bContinue);
	buttons.push_back(bCustomizations);
	buttons.push_back(bOptions);
	buttons.push_back(bCredits);
	buttons.push_back(bAllowCommands);
	buttons.push_back(bQuit);

	tabButtons.push_back(bContinue);
	tabButtons.push_back(bCustomizations);
	tabButtons.push_back(bOptions);
	tabButtons.push_back(bCredits);
	tabButtons.push_back(bAllowCommands);
	tabButtons.push_back(bQuit);
}

void PauseScreen::setupPositions() {
	int centerX = width / 2;
	int centerY = height / 2;

	bContinue->width = 200;
	bContinue->height = 20;
	bContinue->x = centerX - 100;
	bContinue->y = centerY - 52;

	bCustomizations->width = 200;
	bCustomizations->height = 20;
	bCustomizations->x = centerX - 100;
	bCustomizations->y = centerY - 28;

	bOptions->width = 98;
	bOptions->height = 20;
	bOptions->x = centerX - 100;
	bOptions->y = centerY - 4;

	bCredits->width = 98;
	bCredits->height = 20;
	bCredits->x = centerX + 2;
	bCredits->y = centerY - 4;

	bAllowCommands->width = 200;
	bAllowCommands->height = 20;
	bAllowCommands->x = centerX - 100;
	bAllowCommands->y = centerY + 20;

	bQuit->width = 200;
	bQuit->height = 20;
	bQuit->x = centerX - 100;
	bQuit->y = centerY + 44;
}

void PauseScreen::tick() {
	super::tick();
	visibleTime++;
}

void PauseScreen::render(int xm, int ym, float a) {
	// Screen background gradient overlay with smooth open animation progress
	int alpha1 = (int)(0xd0 * openAnimationProgress);
	int alpha2 = (int)(0xe0 * openAnimationProgress);
	fillGradient(0, 0, width, height, (alpha1 << 24) | 0x0a0a0a, (alpha2 << 24) | 0x141414);

	drawCenteredString(font, "Game Menu", width / 2, height / 2 - 62, 0xffffffff);

	super::render(xm, ym, a);
}

void PauseScreen::buttonClicked(Button* button) {
	if (button->id == bContinue->id) {
		minecraft->setScreen(NULL);
	} else if (button->id == bCustomizations->id) {
		minecraft->setScreen(new CustomizationsScreen());
	} else if (button->id == bOptions->id) {
		minecraft->setScreen(new OptionsScreen());
	} else if (button->id == bCredits->id) {
		minecraft->setScreen(new CreditsScreen(CREDITS_RETURN_PAUSE));
	} else if (button->id == bAllowCommands->id) {
		if (minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
			bool newAllow = !minecraft->level->getLevelData()->getAllowCheats();
			minecraft->level->getLevelData()->setAllowCheats(newAllow);
			bAllowCommands->msg = newAllow ? "Allow Commands: ON" : "Allow Commands: OFF";
		}
	} else if (button->id == bQuit->id) {
		minecraft->leaveGame();
	}
}
