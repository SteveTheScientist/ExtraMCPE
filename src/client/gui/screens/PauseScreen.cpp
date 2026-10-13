#include "PauseScreen.h"
#include "StartMenuScreen.h"
#include "OptionsScreen.h"
#include "CreditsScreen.h"
#include "../../Minecraft.h"
#include "../../../network/RakNetInstance.h"
#include "client/gui/components/Button.h"

PauseScreen::PauseScreen(bool wasBackPaused)
	: visibleTime(0),
	  wasBackPaused(wasBackPaused),
	  bContinue(NULL),
	  bQuit(NULL),
	  bOptions(NULL),
	  bCredits(NULL)
{
}

PauseScreen::~PauseScreen() {
	delete bContinue;
	delete bQuit;
	delete bOptions;
	delete bCredits;
}

void PauseScreen::init() {
	bool isMP = (minecraft->raknetInstance && !minecraft->raknetInstance->isServer());
	std::string quitText = isMP ? "Disconnect" : "Save and Quit to Title";

	bContinue = new Button(1, 0, 0, 200, 20, "Back to Game");
	bOptions = new Button(2, 0, 0, 98, 20, "Options...");
	bCredits = new Button(3, 0, 0, 98, 20, "Credits...");
	bQuit = new Button(4, 0, 0, 200, 20, quitText);

	buttons.push_back(bContinue);
	buttons.push_back(bOptions);
	buttons.push_back(bCredits);
	buttons.push_back(bQuit);

	tabButtons.push_back(bContinue);
	tabButtons.push_back(bOptions);
	tabButtons.push_back(bCredits);
	tabButtons.push_back(bQuit);
}

void PauseScreen::setupPositions() {
	int centerX = width / 2;
	int centerY = height / 2;

	bContinue->width = 200;
	bContinue->height = 20;
	bContinue->x = centerX - 100;
	bContinue->y = centerY - 28;

	bOptions->width = 98;
	bOptions->height = 20;
	bOptions->x = centerX - 100;
	bOptions->y = centerY - 4;

	bCredits->width = 98;
	bCredits->height = 20;
	bCredits->x = centerX + 2;
	bCredits->y = centerY - 4;

	bQuit->width = 200;
	bQuit->height = 20;
	bQuit->x = centerX - 100;
	bQuit->y = centerY + 20;
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

	drawCenteredString(font, "Game Menu", width / 2, height / 2 - 50, 0xffffffff);

	super::render(xm, ym, a);
}

void PauseScreen::buttonClicked(Button* button) {
	if (button->id == bContinue->id) {
		minecraft->setScreen(NULL);
	} else if (button->id == bOptions->id) {
		minecraft->setScreen(new OptionsScreen());
	} else if (button->id == bCredits->id) {
		minecraft->setScreen(new CreditsScreen());
	} else if (button->id == bQuit->id) {
		minecraft->leaveGame();
	}
}
