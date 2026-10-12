#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "SelectWorldScreen.h"
#include "ProgressScreen.h"
#include "JoinGameScreen.h"
#include "OptionsScreen.h"
#include "PauseScreen.h"
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
	delete bOptions;
	delete bQuit;
}

void StartMenuScreen::init()
{
	if (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 2){
		bHost = new Button(    2, 0, 0, 200, 20, "Singleplayer");
		bJoin = new Button(    3, 0, 0, 200, 20, "Multiplayer");
		bOptions = new Button( 4, 0, 0, 200, 20, "Options...");
		bQuit = new Button( 5, 0, 0, 200, 20, "Quit Game");
	} else {
		bHost = new Button(    2, 0, 0, 160, 24, "Start Game");
		bJoin = new Button(    3, 0, 0, 160, 24, "Join Game");
		bOptions = new Button( 4, 0, 0, 160, 24, "Options");
		bQuit = new Button( 5, 0, 0, 160, 24, "Quit Game");
	}
	bJoin->active = bHost->active = bOptions->active = true;

	if (minecraft->options.getStringValue(OPTIONS_USERNAME).empty()) {
		return; // tick() will redirect to UsernameScreen
	}

	buttons.push_back(bHost);
	buttons.push_back(bJoin);
	buttons.push_back(bQuit);

	tabButtons.push_back(bHost);
	tabButtons.push_back(bJoin);
	tabButtons.push_back(bQuit);

	#ifndef RPI
		buttons.push_back(bOptions);
		tabButtons.push_back(bOptions);
	#endif

	copyright = "\xffMojang AB";

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
	if (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 2){
		int yBase = (height / 2) - 15;

		bHost->y =	 yBase;
		bJoin->y =	 bHost->y + 24;
		bOptions->y = bJoin->y + 24;
		bQuit->y = bOptions->y + 24;
	} else {
		int yBase = (height / 2) - 10;
		bHost->y =	 yBase;
		bJoin->y =	 bHost->y + 28;
		bOptions->y = bJoin->y + 28;
		bQuit->y = bOptions->y + 28;
	}

	// Center buttons
	bHost->x = (width - bHost->width) / 2;
	bJoin->x = (width - bJoin->width) / 2;
	bOptions->x = (width - bOptions->width) / 2;
	bQuit->x = (width - bQuit->width) / 2;
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
	if (button->id == bOptions->id)
	{
		minecraft->setScreen(new OptionsScreen());
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
		const float y = 10;
		const float wh = Mth::Min((float)width * 0.45f, (float)data->w / 2.0f);
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

#if defined(RPI)
	if (Textures::isTextureIdValid(minecraft->textures->loadAndBindTexture("gui/logo/raknet_high_72.png")))
		blit(0, height - 12, 0, 0, 43, 12, 256, 72+72);
#endif

	drawString(font, version, width - font->width(version) - 6, height - 22, 0xddcccccc);
	drawString(font, copyright, width - font->width(copyright) - 6, height - 12, 0xddffffff);

	glEnable2(GL_BLEND);
	glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (Textures::isTextureIdValid(minecraft->textures->loadAndBindTexture("gui/logo/github.png"))) {
		glColor4f2(1, 1, 1, 1);
		blit(6, height - 13, 0, 0, 8, 8, 256, 256);
	}

	std::string txt = "Kolyah35/minecraft-pe-0.6.1";
	int wtxt = font->width(txt);
	const int logoX = 6;
	const int logoW = 8 + 4 + wtxt;
	const int logoY = height - 14;
	const int logoH = 12;

	bool hoveredLink = (xm >= logoX && xm <= logoX + logoW && ym >= logoY && ym <= logoY + logoH);
	Gui::drawColoredString(font, txt, 18, height - 13, hoveredLink ? 255 : 220);
	if (hoveredLink) {
		fill(18, height - 4, 18 + wtxt, height - 3, 0xffffffff);
	}

	Screen::render(xm, ym, a);
	glDisable2(GL_BLEND);
}

void StartMenuScreen::mouseClicked(int x, int y, int buttonNum) {
	std::string txt = "Kolyah35/minecraft-pe-0.6.1";
	const int logoX = 6;
	const int logoW = 8 + 4 + font->width(txt);
	const int logoY = height - 14;
	const int logoH = 12;
	if (x >= logoX && x <= logoX + logoW && y >= logoY && y <= logoY + logoH)
		minecraft->platform()->openURL("https://gitea.sffempire.ru/Kolyah35/minecraft-pe-0.6.1");
	else
		Screen::mouseClicked(x, y, buttonNum);
}

bool StartMenuScreen::handleBackEvent( bool isDown ) {
	minecraft->quit();
	return true;
}
