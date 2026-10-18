#include "ChatScreen.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/input/Mouse.h"
#include "../../../util/Mth.h"
#include "../../../world/level/Level.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/packet/MessagePacket.h"
#include "util/StringUtils.h"
#include <algorithm>
#include <sstream>

ChatScreen::ChatScreen()
	: m_input(""),
	  m_cursorPos(0),
	  m_blinkTimer(0),
	  m_scrollY(0.0f),
	  m_maxScroll(0.0f),
	  m_isDragging(false),
	  m_dragStartY(0),
	  m_dragStartScrollY(0.0f)
{
}

ChatScreen::~ChatScreen() {
}

void ChatScreen::init() {
	m_input = "";
	m_cursorPos = 0;
	m_blinkTimer = 0;
	m_scrollY = 0.0f;
}

void ChatScreen::tick() {
	Screen::tick();
	m_blinkTimer++;
}

bool ChatScreen::handleBackEvent(bool isDown) {
	if (isDown) {
		minecraft->setScreen(NULL);
	}
	return true;
}

void ChatScreen::render(int xm, int ym, float a) {
	// Java 1.8 style input bar at bottom:
	// Black 50% opacity bar with no border, 12px tall, 2px from bottom and sides (y = height - 14, x = 2, w = width - 4)
	int barX = 2;
	int barY = height - 14;
	int barW = width - 4;
	int barH = 12;

	fill(barX, barY, barX + barW, barY + barH, 0x80000000); // 50% opacity black

	std::string text = m_input;
	if ((m_blinkTimer / 10) % 2 == 0) {
		text += "_";
	}

	Font* font = minecraft->font;
	// White text with shadow, 4px left padding (barX + 4 = 6px)
	font->drawShadow(text, (float)(barX + 4), (float)(barY + 2), 0xffffffff);

	Screen::render(xm, ym, a);
}

void ChatScreen::execute() {
	if (!minecraft->level) return;

	if (m_input.empty()) {
		minecraft->setScreen(NULL);
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
	minecraft->setScreen(NULL);
}

void ChatScreen::keyPressed(int key) {
	if (key == Keyboard::KEY_ESCAPE) {
		minecraft->setScreen(NULL);
		return;
	}

	if (key == Keyboard::KEY_RETURN) {
		execute();
		return;
	}

	if (key == Keyboard::KEY_BACKSPACE) {
		if (!m_input.empty()) {
			m_input.pop_back();
		}
		return;
	}

	Screen::keyPressed(key);
}

void ChatScreen::keyboardNewChar(char c) {
	if (c >= 32 && c < 127) {
		if (m_input.length() < 100) {
			m_input += c;
		}
	}
}

void ChatScreen::mouseClicked(int x, int y, int buttonNum) {
	if (buttonNum == MouseAction::ACTION_LEFT && y < height - 14) {
		m_isDragging = true;
		m_dragStartY = y;
		m_dragStartScrollY = m_scrollY;
	}
	Screen::mouseClicked(x, y, buttonNum);
}

void ChatScreen::mouseReleased(int x, int y, int buttonNum) {
	if (m_isDragging) {
		m_isDragging = false;
	}
	Screen::mouseReleased(x, y, buttonNum);
}

void ChatScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	if (dy != 0) {
		minecraft->gui.scrollChat(dy);
	}
}
