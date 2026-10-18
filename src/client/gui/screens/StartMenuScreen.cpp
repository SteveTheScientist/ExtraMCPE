#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "SelectWorldScreen.h"
#include "ProgressScreen.h"
#include "JoinGameScreen.h"
#include "OptionsScreen.h"
#include "PauseScreen.h"
#include "CreditsScreen.h"
#include "CustomizationsScreen.h"
#include "PrerenderTilesScreen.h" // test button
#include "../components/ImageButton.h"

#include "../../../util/Mth.h"

#include "../Font.h"
#include "../components/ScrolledSelectionList.h"

#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../../AppPlatform.h"
#include "../../../LicenseCodes.h"
#include "SimpleChooseLevelScreen.h"
#include "../../renderer/Textures.h"
#include "../../../SharedConstants.h"

StartMenuScreen::StartMenuScreen()
{
}

StartMenuScreen::~StartMenuScreen()
{
	delete bHost;
	delete bJoin;
	delete bCustomizations;
	delete bOptions;
	delete bCredits;
	delete bQuit;
}

void StartMenuScreen::init()
{
	bHost = new Button(           2, 0, 0, 200, 20, "Singleplayer");
	bJoin = new Button(           3, 0, 0, 200, 20, "Multiplayer");
	bCustomizations = new Button( 7, 0, 0, 200, 20, "Customizations...");
	bOptions = new Button(        4, 0, 0, 98, 20, "Options...");
	bCredits = new Button(        6, 0, 0, 98, 20, "Credits...");
	bQuit = new Button(           5, 0, 0, 200, 20, "Quit Game");

	bJoin->active = bHost->active = bCustomizations->active = bOptions->active = bCredits->active = bQuit->active = true;

	if (minecraft->options.getStringValue(OPTIONS_USERNAME).empty()) {
		return; // tick() will redirect to UsernameScreen
	}

	buttons.push_back(bHost);
	buttons.push_back(bJoin);
	buttons.push_back(bCustomizations);
	buttons.push_back(bOptions);
	buttons.push_back(bCredits);
	buttons.push_back(bQuit);

	tabButtons.push_back(bHost);
	tabButtons.push_back(bJoin);
	tabButtons.push_back(bCustomizations);
	tabButtons.push_back(bOptions);
	tabButtons.push_back(bCredits);
	tabButtons.push_back(bQuit);

	std::string versionString = Common::getGameVersionString();

	std::string _username = minecraft->options.getStringValue(OPTIONS_USERNAME);
	if (_username.empty()) _username = "unknown";

	username = "Username: " + _username;

	#ifdef DEMO_MODE
	#ifdef __APPLE__
		version = versionString + " (Lite)";
	#else
		version = versionString + " (Demo)";
	#endif
	#else
		#ifdef RPI
			version = "v0.1.1 alpha";
		#else
			version = versionString;
		#endif
	#endif
}

void StartMenuScreen::setupPositions() {
	int centerX = width / 2;

	float logoY = 4.0f;
	float wh = Mth::Min((float)width * 0.35f, 85.0f);
	float scale = 2.0f * wh / 256.0f;
	float logoH = scale * 64.0f;
	int logoBottom = (int)(logoY + logoH);

	int available = height - logoBottom;
	int yBase = logoBottom + (available - 116) / 2;
	if (yBase < logoBottom + 4) yBase = logoBottom + 4;
	if (yBase + 116 > height - 6) yBase = height - 116 - 6;

	bHost->width = 200;
	bHost->height = 20;
	bHost->x = centerX - 100;
	bHost->y = yBase;

	bJoin->width = 200;
	bJoin->height = 20;
	bJoin->x = centerX - 100;
	bJoin->y = yBase + 24;

	bCustomizations->width = 200;
	bCustomizations->height = 20;
	bCustomizations->x = centerX - 100;
	bCustomizations->y = yBase + 48;

	bOptions->width = 98;
	bOptions->height = 20;
	bOptions->x = centerX - 100;
	bOptions->y = yBase + 72;

	bCredits->width = 98;
	bCredits->height = 20;
	bCredits->x = centerX + 2;
	bCredits->y = yBase + 72;

	bQuit->width = 200;
	bQuit->height = 20;
	bQuit->x = centerX - 100;
	bQuit->y = yBase + 96;
}

void StartMenuScreen::tick() {
}

void StartMenuScreen::buttonClicked(Button* button) {

	if (button->id == bHost->id)
	{
        #if defined(DEMO_MODE) || defined(APPLE_DEMO_PROMOTION)
			minecraft->setScreen( new SimpleChooseLevelScreen("_DemoLevel") );
		#else
			minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		#endif
	}
	if (button->id == bJoin->id)
	{
		minecraft->locateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	if (button->id == bCustomizations->id)
	{
		minecraft->setScreen(new CustomizationsScreen());
	}
	if (button->id == bOptions->id)
	{
		minecraft->setScreen(new OptionsScreen());
	}
	if (button->id == bCredits->id)
	{
		minecraft->setScreen(new CreditsScreen());
	}
	if (button->id == bQuit->id)
	{
		minecraft->quit();
	}
}

bool StartMenuScreen::isInGameScreen() { return false; }

void StartMenuScreen::render( int xm, int ym, float a )
{
	renderBackground();

	// Username pill badge top-left
	int unameW = font->width(username) + 12;
	fill(4, 4, 4 + unameW, 18, 0x60000000);
	drawString(font, username, 10, 6, 0xffffffff);

#if defined(RPI)
	TextureId id = minecraft->textures->loadTexture("gui/pi_title.png");
#else
	TextureId id = minecraft->textures->loadTexture("gui/title.png");
#endif
	const TextureData* data = minecraft->textures->getTemporaryTextureData(id);

	if (data) {
		minecraft->textures->bind(id);

		const float x = (float)width / 2;
		const float y = 4.0f;
		const float wh = Mth::Min((float)width * 0.35f, 85.0f);
		const float scale = 2.0f * wh / (float)data->w;
		const float h = scale * (float)data->h;

		// Render title logo
		Tesselator& t = Tesselator::instance;
		glColor4f2(1, 1, 1, 1);
		t.begin();
		t.vertexUV(x-wh, y+h, blitOffset, 0, 1);
		t.vertexUV(x+wh, y+h, blitOffset, 1, 1);
		t.vertexUV(x+wh, y+0, blitOffset, 1, 0);
		t.vertexUV(x-wh, y+0, blitOffset, 0, 0);
		t.draw();
	}

	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Screen::render(xm, ym, a);
	glDisable2(GL_BLEND);
}

void StartMenuScreen::mouseClicked(int x, int y, int buttonNum) {
	Screen::mouseClicked(x, y, buttonNum);
}

bool StartMenuScreen::handleBackEvent( bool isDown ) {
	minecraft->quit();
	return true;
}
