#include "TouchJoinGameScreen.h"
#include "../StartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../JoinByIPScreen.h"
#include "../../Font.h"
#include "../../../Minecraft.h"
#include "../../../renderer/Textures.h"

namespace Touch {

void AvailableGamesList::selectStart( int item) {
	startSelected = item;
}

void AvailableGamesList::selectCancel() {
	startSelected = -1;
}

void AvailableGamesList::selectItem( int item, bool doubleClick ) {
	selectedItem = item;
}

void AvailableGamesList::renderItem( int i, int x, int y, int h, Tesselator& t )
{
	if (startSelected == i && Multitouch::getFirstActivePointerIdEx() >= 0) {
		fill((int)x0, y, (int)x1, y+h, 0x809E684F);
	}

	const PingedCompatibleServer& s = copiedServerList[i];
	unsigned int color  = s.isSpecial? 0x6090a0 : 0xffffb0;
	unsigned int color2 = 0xffffa0;

	int xx1 = (int)x0 + 24;
	int xx2 = xx1;

	if (s.isSpecial) {
		xx1 += 50;

		glEnable2(GL_TEXTURE_2D);
        glColor4f2(1,1,1,1);
        glEnable2(GL_BLEND);
		minecraft->textures->loadAndBindTexture("gui/badge/minecon140.png");
		blit(xx2, y + 6, 0, 0, 37, 8, 140, 240);
	}

	drawString(minecraft->font, s.name.C_String(), xx1, y + 4 + 2, color);
	drawString(minecraft->font, s.address.ToString(false), xx2, y + 18, color2);
}

JoinGameScreen::JoinGameScreen()
:	bJoin(  2, "Join Server"),
	bBack(  3, "Cancel"),
	bJoinByIp(4, "Direct Connect"),
	bHeader(0, ""),
	gamesList(NULL)
{
	bJoin.active = false;
	bJoinByIp.active = true;
}

JoinGameScreen::~JoinGameScreen()
{
	delete gamesList;
}

void JoinGameScreen::init()
{
	buttons.push_back(&bJoin);
	buttons.push_back(&bJoinByIp);
	buttons.push_back(&bBack);

	minecraft->raknetInstance->clearServerList();
	gamesList = new AvailableGamesList(minecraft, width, height);

	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bJoinByIp);
	tabButtons.push_back(&bBack);
}

void JoinGameScreen::setupPositions() {
	int yBase = height - 48;
	int centerX = width / 2;

	bJoin.width = 98;
	bJoinByIp.width = 98;
	bBack.width = 200;

	bJoin.height = bJoinByIp.height = bBack.height = 20;

	bJoin.x = centerX - 100;
	bJoin.y = yBase;

	bJoinByIp.x = centerX + 2;
	bJoinByIp.y = yBase;

	bBack.x = centerX - 100;
	bBack.y = yBase + 24;

	if (gamesList) {
		gamesList->setupPositions(width, height, 32, height - 52);
	}
}

void JoinGameScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{
		if (isIndexValid(gamesList->selectedItem))
		{
			PingedCompatibleServer selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
			minecraft->joinMultiplayer(selectedServer);
			{
				bJoin.active = false;
				bBack.active = false;
				minecraft->setScreen(new ProgressScreen());
			}
		}
	}
	if (button->id == bJoinByIp.id) {
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINBYIP);
	}

	if (button->id == bBack.id)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
}

bool JoinGameScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
	return true;
}

bool JoinGameScreen::isIndexValid( int index )
{
	return gamesList && index >= 0 && index < gamesList->getNumberOfItems();
}

void JoinGameScreen::tick()
{
	const ServerList& orgServerList = minecraft->raknetInstance->getServerList();
	ServerList serverList;
	for (unsigned int i = 0; i < orgServerList.size(); ++i)
		if (orgServerList[i].name.GetLength() > 0)
			serverList.push_back(orgServerList[i]);

	if (serverList.size() != gamesList->copiedServerList.size())
	{
		PingedCompatibleServer selectedServer;
		bool hasSelection = false;
		if (isIndexValid(gamesList->selectedItem))
		{
			selectedServer = gamesList->copiedServerList[gamesList->selectedItem];
			hasSelection = true;
		}

		gamesList->copiedServerList = serverList;
		gamesList->selectItem(-1, false);

		if (hasSelection)
		{
			for (unsigned int i = 0; i < gamesList->copiedServerList.size(); i++)
			{
				if (gamesList->copiedServerList[i].address == selectedServer.address)
				{
					gamesList->selectItem(i, false);
					break;
				}
			}
		}
	} else {
		for (int i = (int)gamesList->copiedServerList.size()-1; i >= 0 ; --i) {
			for (int j = 0; j < (int) serverList.size(); ++j)
				if (serverList[j].address == gamesList->copiedServerList[i].address)
					gamesList->copiedServerList[i].name = serverList[j].name;
		}
	}

	bJoin.active = isIndexValid(gamesList->selectedItem);
}

void JoinGameScreen::render( int xm, int ym, float a )
{
	bool hasNetwork = minecraft->platform()->isNetworkEnabled(true);
#ifdef WIN32
	hasNetwork = hasNetwork && !GetAsyncKeyState(VK_TAB);
#endif

	renderBackground();
	if (hasNetwork) gamesList->render(xm, ym, a);
	Screen::render(xm, ym, a);

	if (hasNetwork) {
		std::string s = "Play Multiplayer";
		drawCenteredString(minecraft->font, s, width / 2, 8, 0xffffffff);

		const int textWidth = minecraft->font->width(s);
		const int spinnerX = width / 2 + textWidth / 2 + 6;

		static const char* spinnerTexts[] = {"-", "\\", "|", "/"};
		int n = ((int)(5.5f * getTimeS()) % 4);
		drawCenteredString(minecraft->font, spinnerTexts[n], spinnerX, 8, 0xffffffff);
	} else {
		drawCenteredString(minecraft->font, "WiFi is disabled", width / 2, 8, 0xffffffff);
	}
}

bool JoinGameScreen::isInGameScreen() { return false; }

};
