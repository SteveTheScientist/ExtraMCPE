#include "TouchStartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../OptionsScreen.h"
#include "../PauseScreen.h"

#include "../../Font.h"
#include "../../components/ScrolledSelectionList.h"
#include "../../components/GuiElement.h"

#include "../../../Minecraft.h"
#include "../../../renderer/Tesselator.h"
#include "../../../renderer/Textures.h"
#include "../../../renderer/TextureData.h"
#include "../../../../SharedConstants.h"
#include "../../../../AppPlatform.h"
#include "../../../../LicenseCodes.h"
#include "../../../../util/Mth.h"

#include "../DialogDefinitions.h"
#include "../SimpleChooseLevelScreen.h"

namespace Touch {

StartMenuScreen::StartMenuScreen()
{
	bHost = new Button(    2, 0, 0, 200, 20, "Start Game");
	bJoin = new Button(    3, 0, 0, 200, 20, "Join Game");
	bOptions = new Button( 4, 0, 0, 200, 20, "Options");
	bQuit = new Button(    5, 0, 0, 200, 20, "Quit Game");
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
	buttons.push_back(bHost);
	buttons.push_back(bJoin);
	buttons.push_back(bOptions);
	buttons.push_back(bQuit);

	tabButtons.push_back(bHost);
	tabButtons.push_back(bJoin);
	tabButtons.push_back(bOptions);
	tabButtons.push_back(bQuit);

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
		version = versionString;
	#endif

    #ifdef APPLE_DEMO_PROMOTION
        version = versionString + " (Demo)";
    #endif

	bJoin->active = bHost->active = bOptions->active = bQuit->active = true;
}

void StartMenuScreen::setupPositions() {
	int yBase = (height / 2) - 10;
	bHost->width = bJoin->width = bOptions->width = bQuit->width = 200;
	bHost->height = bJoin->height = bOptions->height = bQuit->height = 20;

	bHost->x = (width - 200) / 2;
	bHost->y = yBase;

	bJoin->x = (width - 200) / 2;
	bJoin->y = bHost->y + 24;

	bOptions->x = (width - 200) / 2;
	bOptions->y = bJoin->y + 24;

	bQuit->x = (width - 200) / 2;
	bQuit->y = bOptions->y + 24;

	copyrightPosX = width - minecraft->font->width(copyright) - 6;
	versionPosX = (width - minecraft->font->width(version)) / 2;
}

void StartMenuScreen::buttonClicked(::Button* button) {

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
        #ifdef APPLE_DEMO_PROMOTION
            minecraft->platform()->createUserInput(DialogDefinitions::DIALOG_DEMO_FEATURE_DISABLED);
        #else
            minecraft->locateMultiplayer();
            minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
        #endif
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

    glEnable2(GL_BLEND);
    glBlendFunc2(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	drawString(font, version, width - font->width(version) - 6, height - 22, 0xddcccccc);
	drawString(font, copyright, width - font->width(copyright) - 6, height - 12, 0xddffffff);

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

} // namespace Touch
