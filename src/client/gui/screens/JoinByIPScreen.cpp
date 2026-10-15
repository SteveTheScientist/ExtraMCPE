#include "JoinByIPScreen.h"

#include "JoinGameScreen.h"
#include "StartMenuScreen.h"
#include "ProgressScreen.h"
#include "../Font.h"
#include "../../../network/RakNetInstance.h"
#include "client/Options.h"
#include "client/gui/Screen.h"
#include "client/gui/components/TextBox.h"
#include "network/ClientSideNetworkHandler.h"

JoinByIPScreen::JoinByIPScreen() :
    tName(0, "Minecraft Server"),
    tIP(1, "Server IP"),
	bJoin(2, "Add Server"),
	bBack(3, "Cancel")
{
	bJoin.active = false;
}

JoinByIPScreen::~JoinByIPScreen()
{
}

void JoinByIPScreen::init()
{
    tName.text = "Minecraft Server";
    tIP.text = minecraft->options.getStringValue(OPTIONS_LAST_IP);

	buttons.push_back(&bJoin);
	buttons.push_back(&bBack);

    textBoxes.push_back(&tName);
    textBoxes.push_back(&tIP);

	tabButtons.push_back(&bJoin);
	tabButtons.push_back(&bBack);
}

void JoinByIPScreen::setupPositions() {
    int centerX = width / 2;
    int fieldW = 210;
    int btnH = 20;
    int fieldX = centerX - fieldW / 2;

    tName.width = fieldW; tName.height = 20;
    tName.x = fieldX; tName.y = 52;

    tIP.width = fieldW; tIP.height = 20;
    tIP.x = fieldX; tIP.y = 96;

    int bottomY = height - 28;
    int btnW = 145;

    bJoin.width = btnW; bJoin.height = btnH;
    bJoin.x = centerX - 150; bJoin.y = bottomY;

    bBack.width = btnW; bBack.height = btnH;
    bBack.x = centerX + 5; bBack.y = bottomY;
}

void JoinByIPScreen::tick()
{
	tName.tick(minecraft);
	tIP.tick(minecraft);
	bJoin.active = !tIP.text.empty();
}

void JoinByIPScreen::render( int xm, int ym, float a )
{
	renderDirtBackground(0);
	glEnable2(GL_BLEND);

	drawCenteredString(minecraft->font, "Add External Server", width / 2, 12, 0xffffffff);

	drawString(minecraft->font, "Server Name", tName.x, tName.y - Font::DefaultLineHeight - 1, 0xffa0a0a0);
	tName.render(minecraft, xm, ym);

	drawString(minecraft->font, "Server Address", tIP.x, tIP.y - Font::DefaultLineHeight - 1, 0xffa0a0a0);
	tIP.render(minecraft, xm, ym);

	fill(0, height - 34, width, height - 33, 0xff555555);

	for (unsigned int i = 0; i < buttons.size(); i++) {
		buttons[i]->render(minecraft, xm, ym);
	}

	glDisable2(GL_BLEND);
}

void JoinByIPScreen::mouseClicked(int x, int y, int buttonNum)
{
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (tName.pointInside(x, y)) {
            tName.setFocus(minecraft);
            tIP.loseFocus(minecraft);
        } else if (tIP.pointInside(x, y)) {
            tIP.setFocus(minecraft);
            tName.loseFocus(minecraft);
        } else {
            tName.loseFocus(minecraft);
            tIP.loseFocus(minecraft);
        }
    }

    Screen::mouseClicked(x, y, buttonNum);
}

void JoinByIPScreen::buttonClicked(Button* button)
{
	if (button->id == bJoin.id)
	{            
        minecraft->isLookingForMultiplayer = true;
	    minecraft->netCallback = new ClientSideNetworkHandler(minecraft, minecraft->raknetInstance);

        minecraft->joinMultiplayerFromString(tIP.text);
        {
			minecraft->options.set(OPTIONS_LAST_IP, tIP.text);
            bJoin.active = false;
            bBack.active = false;
            minecraft->setScreen(new ProgressScreen());
        }
	}
	if (button->id == bBack.id)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
}

bool JoinByIPScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->screenChooser.setScreen(SCREEN_JOINGAME);
	}
	return true;
}

void JoinByIPScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        return;
    }
    // let base class handle navigation and text box keys
    Screen::keyPressed(eventKey);
}