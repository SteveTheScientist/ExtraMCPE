#include "InBedChatScreen.h"
#include "../components/Button.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../network/packet/MessagePacket.h"
#include "../../../network/RakNetInstance.h"
#include "commands/CommandManager.hpp"
#include "util/StringUtils.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <sstream>

InBedChatScreen::InBedChatScreen()
	: btnLeaveBed(NULL)
{
}

InBedChatScreen::~InBedChatScreen() {
	delete btnLeaveBed;
}

void InBedChatScreen::init() {
	ChatScreen::init();

	btnLeaveBed = new Button(1, 0, 0, 200, 20, "Leave Bed");
	buttons.push_back(btnLeaveBed);
	tabButtons.push_back(btnLeaveBed);
}

void InBedChatScreen::setupPositions() {
	ChatScreen::setupPositions();

	if (btnLeaveBed) {
		btnLeaveBed->width = 200;
		btnLeaveBed->height = 20;
		btnLeaveBed->x = (width - 200) / 2;
		btnLeaveBed->y = height - 42;
	}
}

void InBedChatScreen::render(int xm, int ym, float a) {
	ChatScreen::render(xm, ym, a);

	if (btnLeaveBed) {
		btnLeaveBed->render(minecraft, xm, ym);
	}
}

void InBedChatScreen::buttonClicked(Button* button) {
	if (button == btnLeaveBed) {
		if (minecraft && minecraft->player) {
			minecraft->player->stopSleepInBed(true, true, true);
		}
		minecraft->setScreen(NULL);
	}
}

void InBedChatScreen::execute() {
	if (!minecraft->level) return;

	if (m_input.empty()) {
		return;
	}

	if (minecraft->netCallback && !minecraft->raknetInstance->isServer()) {
		MessagePacket packet(m_input.c_str());
		minecraft->raknetInstance->send(packet);
	} else if (m_input[0] == '/') {
		std::string cmd = Util::stringTrim(m_input.substr(1));

		std::istringstream iss(
			minecraft->commandManager().execute(*minecraft, *minecraft->player, cmd)
		);

		for (std::string line; std::getline(iss, line);) {
			minecraft->gui.addMessage(line);
		}
	} else {
		if (minecraft->raknetInstance->isServer()) {
			MessagePacket packet(m_input.c_str());
			minecraft->raknetInstance->send(packet);
		}

		minecraft->gui.addMessage("<" + minecraft->player->name + "> " + m_input);
	}

	m_input = "";
	// The screen STAYS open while in bed
}

void InBedChatScreen::keyPressed(int key) {
	if (key == Keyboard::KEY_ESCAPE) {
		if (minecraft && minecraft->player) {
			minecraft->player->stopSleepInBed(true, true, true);
		}
		minecraft->setScreen(NULL);
		return;
	}

	ChatScreen::keyPressed(key);
}

bool InBedChatScreen::handleBackEvent(bool isDown) {
	if (isDown) {
		if (minecraft && minecraft->player) {
			minecraft->player->stopSleepInBed(true, true, true);
		}
		minecraft->setScreen(NULL);
	}
	return true;
}

void InBedChatScreen::mouseClicked(int x, int y, int buttonNum) {
	if (btnLeaveBed && btnLeaveBed->clicked(minecraft, x, y)) {
		btnLeaveBed->setPressed();
		clickedButton = btnLeaveBed;
		buttonClicked(btnLeaveBed);
		return;
	}

	ChatScreen::mouseClicked(x, y, buttonNum);
}
