#include "ChatScreen.h"
#include "DialogDefinitions.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/input/Mouse.h"
#include "../../../util/Mth.h"
#include <algorithm>

ChatScreen::ChatScreen()
	: m_input(""),
	  m_cursorPos(0),
	  m_blinkTimer(0),
	  m_hasNativeDialog(false),
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
	m_hasNativeDialog = false;

	if (minecraft->platform() && minecraft->platform()->supportsTouchscreen()) {
		m_hasNativeDialog = true;
		minecraft->platform()->createUserInput(DialogDefinitions::DIALOG_NEW_CHAT_MESSAGE);
	}
}

void ChatScreen::tick() {
	super::tick();
	m_blinkTimer++;

	if (m_hasNativeDialog) {
		int status = minecraft->platform()->getUserInputStatus();
		if (status > -1) {
			if (status == 1) {
				std::vector<std::string> v = minecraft->platform()->getUserInput();
				if (!v.empty() && !v[0].empty()) {
					minecraft->gui.addMessage("<" + minecraft->player->name + "> " + v[0]);
				}
			}
			minecraft->setScreen(NULL);
		}
	}
}

void ChatScreen::render(int xm, int ym, float a) {
	if (m_hasNativeDialog) {
		return;
	}

	Font* font = minecraft->font;
	const GuiMessageList& msgs = minecraft->gui.getGuiMessages();

	int totalMsgs = (int)msgs.size();
	int visibleLines = std::min(totalMsgs, 20);

	float lineH = 9.0f;
	float contentH = (float)visibleLines * lineH;
	float historyTop = (float)(height - 14 - 4);
	float historyBoxH = 20.0f * lineH;
	m_maxScroll = std::max(0.0f, contentH - historyBoxH);
	m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);

	// Render up to 20 last lines fully visible
	for (int i = 0; i < visibleLines; i++) {
		const GuiMessage& msg = msgs[i];
		float lineY = historyTop - (i + 1) * lineH + m_scrollY;

		if (lineY >= 0 && lineY < height - 14) {
			float msgW = Gui::getColoredWidth(font, msg.message) + 4.0f;
			if (msgW < 120.0f) msgW = 120.0f;
			if (msgW > width - 8) msgW = (float)(width - 8);

			fill(2, (int)lineY - 1, 4 + (int)msgW, (int)lineY + 8, 0x80000000);
			glEnable(GL_BLEND);
			Gui::drawColoredString(font, msg.message, 4.0f, lineY, 255);
		}
	}

	// Dark semi-transparent input bar at bottom
	fill(0, height - 14, width, height, 0xc0000000);

	std::string prompt = "> " + m_input;
	bool showCursor = (m_blinkTimer / 10) % 2 == 0;
	if (showCursor) {
		prompt += "_";
	}

	font->drawShadow(prompt, 4.0f, (float)(height - 11), 0xffffffff);

	super::render(xm, ym, a);
}

void ChatScreen::buttonClicked(Button* button) {
}

void ChatScreen::keyPressed(int key) {
	if (m_hasNativeDialog) return;

	if (key == Keyboard::KEY_ESCAPE) {
		minecraft->setScreen(NULL);
		return;
	}

	if (key == Keyboard::KEY_RETURN) {
		if (!m_input.empty()) {
			minecraft->gui.addMessage("<" + minecraft->player->name + "> " + m_input);
			m_input = "";
		}
		minecraft->setScreen(NULL);
		return;
	}

	if (key == Keyboard::KEY_BACKSPACE) {
		if (!m_input.empty()) {
			m_input.pop_back();
		}
		return;
	}

	super::keyPressed(key);
}

void ChatScreen::keyboardNewChar(char c) {
	if (m_hasNativeDialog) return;

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
	super::mouseClicked(x, y, buttonNum);
}

void ChatScreen::mouseReleased(int x, int y, int buttonNum) {
	m_isDragging = false;
	super::mouseReleased(x, y, buttonNum);
}

void ChatScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	if (dy != 0) {
		m_scrollY += (float)dy * 9.0f;
		m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
	}
}
