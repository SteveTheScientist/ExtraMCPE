#include "ChatScreen.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/time.h"
#include "../../../util/Mth.h"
#include "../../../world/level/Level.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/packet/MessagePacket.h"
#include "util/StringUtils.h"
#include <algorithm>
#include <sstream>

ChatScreen::ChatScreen()
	: m_inputBox(1, 0, 0, 100, 14, ""),
	  m_scrollY(0.0f),
	  m_maxScroll(0.0f),
	  m_isDragging(false),
	  m_dragStartY(0),
	  m_startTouchY(0),
	  m_dragAccumulatorY(0.0f)
{
}

ChatScreen::~ChatScreen() {
}

void ChatScreen::init() {
	m_inputBox = TextBox(1, 0, height - 14, width, 14, "");
	m_inputBox.setChatStyle(true);
	m_inputBox.setFocus(minecraft);

	textBoxes.clear();
	textBoxes.push_back(&m_inputBox);

	m_scrollY = 0.0f;
	m_isDragging = false;
	m_dragAccumulatorY = 0.0f;
}

void ChatScreen::tick() {
	Screen::tick();

	int ym = Mouse::getY();
	int xm = Mouse::getX();
	toGUICoordinate(xm, ym);

	int totalLines = (int)minecraft->gui.getGuiMessages().size();

	float configuredHeightPx = minecraft->options.getProgressValue(OPTIONS_CHAT_FOCUSED_HEIGHT);
	if (configuredHeightPx < 20.0f) configuredHeightPx = 20.0f;
	if (configuredHeightPx > 180.0f) configuredHeightPx = 180.0f;

	float lineSpacingPct = minecraft->options.getProgressValue(OPTIONS_CHAT_LINE_SPACING) / 100.0f;
	float textSizeScale = minecraft->options.getProgressValue(OPTIONS_CHAT_TEXT_SIZE) / 100.0f;
	textSizeScale = Mth::clamp(textSizeScale, 0.5f, 1.0f);
	float lineH = (9.0f + lineSpacingPct * 9.0f) * textSizeScale;

	float screenHeightF = (float)height;
	float unfocusedH = minecraft->options.getProgressValue(OPTIONS_CHAT_UNFOCUSED_HEIGHT);
	unfocusedH = Mth::clamp(unfocusedH, 20.0f, 90.0f);
	float maxY = std::max(40.0f, screenHeightF - unfocusedH - 4.0f);
	float posY = minecraft->options.getProgressValue(OPTIONS_CHAT_POS_Y);
	posY = Mth::clamp(posY, 0.0f, maxY);

	int desiredMaxLines = std::max(1, (int)(configuredHeightPx / lineH));
	int allowedLines = (int)((screenHeightF - posY) / lineH);
	int maxLines = std::max(1, std::min(desiredMaxLines, allowedLines));
	int maxScroll = totalLines - maxLines;

	if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
		if (m_isDragging && maxScroll > 0) {
			float deltaY = (float)(ym - m_dragStartY);
			m_dragStartY = ym;
			m_dragAccumulatorY += deltaY;

			if (m_dragAccumulatorY >= lineH) {
				int steps = (int)(m_dragAccumulatorY / lineH);
				minecraft->gui.scrollChat(steps);
				m_dragAccumulatorY -= steps * lineH;
			} else if (m_dragAccumulatorY <= -lineH) {
				int steps = (int)(-m_dragAccumulatorY / lineH);
				minecraft->gui.scrollChat(-steps);
				m_dragAccumulatorY += steps * lineH;
			}
		}
	} else {
		m_isDragging = false;
		m_dragAccumulatorY = 0.0f;
	}
}

bool ChatScreen::handleBackEvent(bool isDown) {
	if (isDown) {
		minecraft->setScreen(NULL);
	}
	return true;
}

void ChatScreen::render(int xm, int ym, float a) {
	Screen::render(xm, ym, a);
}

void ChatScreen::execute() {
	if (!minecraft->level) return;

	if (m_inputBox.text.empty()) {
		minecraft->setScreen(NULL);
		return;
	}

	if (minecraft->netCallback && !minecraft->raknetInstance->isServer()) {
		MessagePacket packet(m_inputBox.text.c_str());
		minecraft->raknetInstance->send(packet);
	} else if (m_inputBox.text[0] == '/') {
		std::string cmd = Util::stringTrim(m_inputBox.text.substr(1));

		std::istringstream iss(
			minecraft->commandManager().execute(*minecraft, *minecraft->player, cmd)
		);

		for (std::string line; std::getline(iss, line);) {
			minecraft->gui.addMessage(line);
		}
	} else {
		if (minecraft->raknetInstance->isServer()) {
			MessagePacket packet(m_inputBox.text.c_str());
			minecraft->raknetInstance->send(packet);
		}

		minecraft->gui.addMessage("<" + minecraft->player->name + "> " + m_inputBox.text);
	}

	m_inputBox.text = "";
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

	Screen::keyPressed(key);
}

void ChatScreen::charPressed(char c) {
	Screen::charPressed(c);
}

void ChatScreen::mouseClicked(int x, int y, int buttonNum) {
	Screen::mouseClicked(x, y, buttonNum);

	if (buttonNum == MouseAction::ACTION_LEFT && !m_inputBox.pointInside(x, y)) {
		float configuredWidth = minecraft->options.getProgressValue(OPTIONS_CHAT_WIDTH);
		configuredWidth = Mth::clamp(configuredWidth, 40.0f, 320.0f);
		float boxWidth = std::min(configuredWidth, (float)width - 4.0f);

		float configuredHeightPx = minecraft->options.getProgressValue(OPTIONS_CHAT_FOCUSED_HEIGHT);
		configuredHeightPx = Mth::clamp(configuredHeightPx, 20.0f, 180.0f);

		float lineSpacingPct = minecraft->options.getProgressValue(OPTIONS_CHAT_LINE_SPACING) / 100.0f;
		float textSizeScale = minecraft->options.getProgressValue(OPTIONS_CHAT_TEXT_SIZE) / 100.0f;
		textSizeScale = Mth::clamp(textSizeScale, 0.5f, 1.0f);
		float lineH = (9.0f + lineSpacingPct * 9.0f) * textSizeScale;

		float screenWidth = (float)width;
		float screenHeightF = (float)height;

		float maxX = std::max(40.0f, screenWidth - boxWidth);
		float posX = minecraft->options.getProgressValue(OPTIONS_CHAT_POS_X);
		posX = Mth::clamp(posX, 0.0f, maxX);

		float unfocusedH = minecraft->options.getProgressValue(OPTIONS_CHAT_UNFOCUSED_HEIGHT);
		unfocusedH = Mth::clamp(unfocusedH, 20.0f, 90.0f);
		float maxY = std::max(40.0f, screenHeightF - unfocusedH - 4.0f);
		float posY = minecraft->options.getProgressValue(OPTIONS_CHAT_POS_Y);
		posY = Mth::clamp(posY, 0.0f, maxY);

		int desiredMaxLines = std::max(1, (int)(configuredHeightPx / lineH));
		int allowedLines = (int)((screenHeightF - posY) / lineH);
		int maxLines = std::max(1, std::min(desiredMaxLines, allowedLines));

		const float baseY = screenHeightF - posY - lineH;
		float boxTop = baseY - (maxLines - 1) * lineH - 1.0f;

		if (x >= posX && x <= posX + boxWidth && y >= boxTop - 4.0f && y <= baseY + lineH + 4.0f) {
			m_isDragging = true;
			m_dragStartY = y;
			m_startTouchY = y;
			m_dragAccumulatorY = 0.0f;
		}
	}
}

void ChatScreen::mouseReleased(int x, int y, int buttonNum) {
	m_isDragging = false;
	m_dragAccumulatorY = 0.0f;
	Screen::mouseReleased(x, y, buttonNum);
}

void ChatScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	if (dy != 0) {
		int steps = (dy > 0 ? 3 : -3);
		minecraft->gui.scrollChat(steps);
	}
}
