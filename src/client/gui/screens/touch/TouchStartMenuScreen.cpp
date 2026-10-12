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

//
// Start menu screen implementation
//

StartMenuScreen::StartMenuScreen()
:	bHost(    2, "Start Game"),
	bJoin(    3, "Join Game"),
	bOptions( 4, "Options"),
	bQuit(    5, "")
{
	ImageDef def;
	bJoin.width = 75;
	def.width = def.height = (float) bJoin.width;

	def.setSrc(IntRectangle(0, 26, (int)def.width, (int)def.width));
	def.name = "gui/touchgui.png";
	IntRectangle& defSrc = *def.getSrc();

	bOptions.setImageDef(def, true);

	defSrc.y += defSrc.h;
	bHost.setImageDef(def, true);

	defSrc.y += defSrc.h;
	bJoin.setImageDef(def, true);
}

StartMenuScreen::~StartMenuScreen()
{
}

void StartMenuScreen::init()
{
	buttons.push_back(&bHost);
	buttons.push_back(&bJoin);
	buttons.push_back(&bOptions);

	// add quit icon (top-right X button)
	{
		ImageDef def;
		def.name = "gui/touchgui.png";
		def.width = 34;
		def.height = 26;
		def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
		bQuit.setImageDef(def, true);
		bQuit.scaleWhenPressed = false;
		buttons.push_back(&bQuit);
	}

	tabButtons.push_back(&bHost);
	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bOptions);

	#ifdef DEMO_MODE
		buttons.push_back(&bBuy);
		tabButtons.push_back(&bBuy);
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
		version = versionString;
	#endif

    #ifdef APPLE_DEMO_PROMOTION
        version = versionString + " (Demo)";
    #endif

	bJoin.active = bHost.active = bOptions.active = true;
}

void StartMenuScreen::setupPositions() {
	int yBase = (height - bHost.height) / 2 + 10;
	int buttonWidth = bHost.width;
	float spacing = (width - (3.0f * buttonWidth)) / 4;

	// Center buttons horizontally
	bJoin.x		= 0*buttonWidth + (int)(1*spacing);
	bHost.x		= 1*buttonWidth + (int)(2*spacing);
	bOptions.x	= 2*buttonWidth + (int)(3*spacing);

	bJoin.y = yBase;
	bHost.y = yBase;
	bOptions.y = yBase;

	// quit icon top-right with margin
	bQuit.x = width - bQuit.width - 4;
	bQuit.y = 4;

	copyrightPosX = width - minecraft->font->width(copyright) - 6;
	versionPosX = (width - minecraft->font->width(version)) / 2;
}

void StartMenuScreen::buttonClicked(::Button* button) {

	if (button->id == bHost.id)
	{
		#if defined(DEMO_MODE) || defined(APPLE_DEMO_PROMOTION)
			minecraft->setScreen( new SimpleChooseLevelScreen("_DemoLevel") );
		#else
			minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
		#endif
	}
	if (button->id == bJoin.id)
	{
        #ifdef APPLE_DEMO_PROMOTION
            minecraft->platform()->createUserInput(DialogDefinitions::DIALOG_DEMO_FEATURE_DISABLED);
        #else
            minecraft->locateMultiplayer();
            minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
        #endif
	}
	if (button->id == bOptions.id)
	{
		minecraft->setScreen(new OptionsScreen());
	}
	if (button == &bQuit)
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
		const float wh = 0.5f * Mth::Min((float)width * 0.7f, (float)data->w);
		const float scale = wh / (float)data->w;
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

		drawCenteredString(font, version, width / 2, (int)(y + h) + 2, 0xffcccccc);
	}

	// Footer copyright & links
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
