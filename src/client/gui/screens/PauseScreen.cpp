#include "PauseScreen.h"
#include "StartMenuScreen.h"
#include "../components/ImageButton.h"
#include "../../Minecraft.h"
#include "../../../util/Mth.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/ServerSideNetworkHandler.h"
#include "client/Options.h"
#include "client/gui/components/Button.h"
#include "client/gui/screens/OptionsScreen.h"
#include <algorithm>

PauseScreen::PauseScreen(bool wasBackPaused)
	:	saveStep(0),
	visibleTime(0),
	bContinue(0),
	bQuit(0),
	bOptions(0),
	bQuitAndSaveLocally(0),
	bServerVisibility(0),
	wasBackPaused(wasBackPaused),
	bThirdPerson(OPTIONS_THIRD_PERSON_VIEW),
	bHideGui(OPTIONS_HIDEGUI)
{
	ImageDef def;
	def.setSrc(IntRectangle(160, 144, 39, 31));
	def.name = "gui/touchgui.png";
	IntRectangle& defSrc = *def.getSrc();

	def.width = defSrc.w * 0.666667f;
	def.height = defSrc.h * 0.666667f;

	defSrc.y += defSrc.h;
	bThirdPerson.setImageDef(def, true);
	bHideGui.setImageDef(def, true);
}

PauseScreen::~PauseScreen() {
	delete bContinue;
	delete bQuit;
	delete bQuitAndSaveLocally;
	delete bServerVisibility;
	delete bOptions;
}

void PauseScreen::init() {
	if (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 0) {
		bContinue = new Touch::TButton(1, "Back to Game");
		bOptions = new Touch::TButton(5, "Options");
		bQuit = new Touch::TButton(2, "Quit to Title");
		bQuitAndSaveLocally = new Touch::TButton(3, "Quit and Copy Map");
		bServerVisibility = new Touch::TButton(4, "");
	} else if (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 1) {
		bContinue = new Button(1, "Back to Game");
		bOptions = new Button(5, "Options");
		bQuit = new Button(2, "Quit to Title");
		bQuitAndSaveLocally = new Button(3, "Quit and Copy Map");
		bServerVisibility = new Button(4, "");
	} else {
		bContinue = new Button(1, 0, 0, 200, 20, "Back to Game");
		bServerVisibility = new Button(4, 0, 0, 200, 20, "");
		bOptions = new Button(5, 0, 0, 200, 20, "Options...");
		bQuit = new Button(2, 0, 0, 200, 20, "Save and Quit to Title");
		bQuitAndSaveLocally = new Button(3, 0, 0, 200, 20, "Copy and Quit Map");
	}

	buttons.push_back(bContinue);
	buttons.push_back(bOptions);

#if !defined(APPLE_DEMO_PROMOTION) && !defined(RPI)
	if (minecraft->raknetInstance) {
		if (minecraft->raknetInstance->isServer()) {
			updateServerVisibilityText();
			buttons.push_back(bServerVisibility);
		} else {
#if !defined(DEMO_MODE)
			buttons.push_back(bQuitAndSaveLocally);
#endif
		}
	}
#endif

	buttons.push_back(bQuit);

	bThirdPerson.updateImage(&minecraft->options);
	bHideGui.updateImage(&minecraft->options);

	buttons.push_back(&bThirdPerson);
	buttons.push_back(&bHideGui);

	for (unsigned int i = 0; i < buttons.size(); ++i) {
		if (buttons[i] == &bThirdPerson || buttons[i] == &bHideGui) continue;
		tabButtons.push_back(buttons[i]);
	}
}

void PauseScreen::setupPositions() {
	saveStep = 0;

	bool isJavaStyle = (minecraft->options.getIntValue(OPTIONS_MENU_STYLE) == 2);
	int btnW = isJavaStyle ? 200 : 160;
	int btnH = isJavaStyle ? 20 : 24;
	int spacing = isJavaStyle ? 24 : 28;

	std::vector<Button*> activeStack;
	if (bContinue && std::find(buttons.begin(), buttons.end(), bContinue) != buttons.end()) activeStack.push_back(bContinue);
	if (bOptions && std::find(buttons.begin(), buttons.end(), bOptions) != buttons.end()) activeStack.push_back(bOptions);
	if (bServerVisibility && std::find(buttons.begin(), buttons.end(), bServerVisibility) != buttons.end()) activeStack.push_back(bServerVisibility);
	if (bQuitAndSaveLocally && std::find(buttons.begin(), buttons.end(), bQuitAndSaveLocally) != buttons.end()) activeStack.push_back(bQuitAndSaveLocally);
	if (bQuit && std::find(buttons.begin(), buttons.end(), bQuit) != buttons.end()) activeStack.push_back(bQuit);

	int stackSize = (int)activeStack.size();
	if (stackSize > 0) {
		int totalHeight = stackSize * btnH + (stackSize - 1) * (spacing - btnH);
		int yStart = (height - totalHeight) / 2 + 8;

		for (int i = 0; i < stackSize; i++) {
			Button* btn = activeStack[i];
			btn->width = btnW;
			btn->height = btnH;
			btn->x = (width - btnW) / 2;
			btn->y = yStart + i * spacing;
		}
	}

	// Quick action toggle buttons top-right
	bThirdPerson.x = width - bThirdPerson.width - 6;
	bThirdPerson.y = 4;

	bHideGui.x = bThirdPerson.x - bHideGui.width - 4;
	bHideGui.y = 4;
}

void PauseScreen::tick() {
	super::tick();
	visibleTime++;
}

void PauseScreen::render(int xm, int ym, float a) {
	// Dark semi-transparent pause backdrop
	fillGradient(0, 0, width, height, 0xd00a0a0a, 0xe0141414);

	// Header bar top
	fill(0, 0, width, 26, 0x50000000);
	fill(0, 25, width, 26, 0x40ffffff);

	drawCenteredString(font, "Game Menu", width / 2, 8, 0xffffffff);

	super::render(xm, ym, a);
}

void PauseScreen::buttonClicked(Button* button) {
	if (button->id == bContinue->id) {
		minecraft->setScreen(NULL);
	}
	if (button->id == bQuit->id) {
		minecraft->leaveGame();
	}
	if (button->id == bQuitAndSaveLocally->id) {
		minecraft->leaveGame(true);
	}
	if (button->id == bOptions->id) {
		minecraft->setScreen(new OptionsScreen());
	}
	if (button->id == bServerVisibility->id) {
		if (minecraft->raknetInstance && minecraft->netCallback && minecraft->raknetInstance->isServer()) {
			ServerSideNetworkHandler* ss = (ServerSideNetworkHandler*) minecraft->netCallback;
			bool allows = !ss->allowsIncomingConnections();
			ss->allowIncomingConnections(allows);

			updateServerVisibilityText();
		}
	}

	if (button == &bThirdPerson || button == &bHideGui) {
		OptionButton* optBtn = (OptionButton*)button;
		optBtn->toggle(&minecraft->options);
	}
}

void PauseScreen::updateServerVisibilityText()
{
	if (!minecraft->raknetInstance || !minecraft->raknetInstance->isServer())
		return;

	ServerSideNetworkHandler* ss = (ServerSideNetworkHandler*) minecraft->netCallback;
	bServerVisibility->msg = ss->allowsIncomingConnections()?
		"Server is Visible"
		:   "Server is Invisible";
}
