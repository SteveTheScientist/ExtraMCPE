#include "DeathScreen.h"
#include "../components/Button.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include <sstream>

template<typename T>
static std::string to_str(T val) {
	std::ostringstream ss;
	ss << val;
	return ss.str();
}

DeathScreen::DeathScreen()
:	bRespawn(NULL),
	bTitle(NULL)
{
}

DeathScreen::~DeathScreen()
{
	delete bRespawn;
	delete bTitle;
}

void DeathScreen::init()
{
	buttons.clear();
	tabButtons.clear();

	bRespawn = new Button(1, 0, 0, 200, 20, "Respawn");
	bTitle = new Button(2, 0, 0, 200, 20, "Title Screen");

	buttons.push_back(bRespawn);
	buttons.push_back(bTitle);

	tabButtons.push_back(bRespawn);
	tabButtons.push_back(bTitle);
}

void DeathScreen::setupPositions()
{
	int buttonWidth = 200;
	if (buttonWidth > width - 20) {
		buttonWidth = width - 20;
	}

	bRespawn->width = buttonWidth;
	bTitle->width = buttonWidth;

	int centerX = (width - buttonWidth) / 2;
	bRespawn->x = centerX;
	bTitle->x = centerX;

	int startY = height / 2;
	bRespawn->y = startY;
	bTitle->y = startY + 24;
}

void DeathScreen::tick() {
	super::tick();
}

void DeathScreen::render(int xm, int ym, float a)
{
	// Dark red tint over the world
	fillGradient(0, 0, width, height, 0x60500000, 0xa0803030);

	Font* font = minecraft->font;

	// "You Died!" in big text (2x), centered near the top
	glPushMatrix2();
	glScalef2(2.0f, 2.0f, 2.0f);
	drawCenteredString(font, "You Died!", width / 4, height / 12, 0xffffffff);
	glPopMatrix2();

	int currentY = height / 6 + 24;

	// Score line: show only if score exists / > 0
	if (minecraft->player) {
		int score = minecraft->player->getScore();
		if (score > 0) {
			std::string scoreText = "Score: \xa7" "e" + to_str(score);
			drawCenteredString(font, scoreText, width / 2, currentY, 0xffffffff);
			currentY += 12;
		}
	}

	super::render(xm, ym, a);
}

void DeathScreen::buttonClicked(Button* button)
{
	if (button == bRespawn) {
		if (minecraft->player) {
			minecraft->player->respawn();
		}
		minecraft->setScreen(NULL);
	} else if (button == bTitle) {
		minecraft->leaveGame();
	}
}
