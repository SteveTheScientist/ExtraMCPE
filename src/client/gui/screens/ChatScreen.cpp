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
	  m_startTouchY(0),
	  m_dragAccumulatorY(0.0f)
{
}

ChatScreen::~ChatScreen() {
}

void ChatScreen::init() {
	m_input = "";
	m_cursorPos = 0;
	m_blinkTimer = 0;
	m_scrollY = 0.0f;
	m_isDragging = false;
	m_dragAccumulatorY = 0.0f;
}

void ChatScreen::tick() {
	Screen::tick();
	m_blinkTimer++;

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

// Helper function to wrap typed input string into lines matching max width
static void wrapInputText(Font* font, const std::string& input, float maxW, std::vector<std::string>& outLines) {
	outLines.clear();
	if (input.empty()) {
		outLines.push_back("");
		return;
	}

	std::string currentLine = "";
	std::string currentWord = "";

	for (size_t i = 0; i < input.length(); i++) {
		char c = input[i];
		if (c == ' ') {
			if (!currentWord.empty()) {
				if (!currentLine.empty()) {
					if (Gui::getColoredWidth(font, currentLine + " " + currentWord, true) <= maxW) {
						currentLine += " " + currentWord;
					} else {
						outLines.push_back(currentLine);
						currentLine = currentWord;
					}
				} else {
					currentLine = currentWord;
				}
				currentWord = "";
			}
		} else {
			currentWord += c;
			std::string testLine = currentLine.empty() ? currentWord : (currentLine + " " + currentWord);
			if (Gui::getColoredWidth(font, testLine, true) > maxW) {
				if (!currentLine.empty()) {
					std::string prefix = currentLine + " ";
					size_t k = 0;
					while (k < currentWord.length() && Gui::getColoredWidth(font, prefix + currentWord.substr(0, k + 1)) <= maxW) {
						k++;
					}
					if (k > 0) {
						outLines.push_back(prefix + currentWord.substr(0, k));
						currentLine = "";
						currentWord = currentWord.substr(k);
					} else {
						outLines.push_back(currentLine);
						currentLine = "";
					}
				}
				while (Gui::getColoredWidth(font, currentWord, true) > maxW) {
					size_t k = 1;
					while (k < currentWord.length() && Gui::getColoredWidth(font, currentWord.substr(0, k + 1)) <= maxW) {
						k++;
					}
					outLines.push_back(currentWord.substr(0, k));
					currentWord = currentWord.substr(k);
				}
			}
		}
	}

	if (!currentWord.empty()) {
		if (!currentLine.empty() && Gui::getColoredWidth(font, currentLine + " " + currentWord, true) <= maxW) {
			currentLine += " " + currentWord;
		} else if (currentLine.empty()) {
			currentLine = currentWord;
		} else {
			outLines.push_back(currentLine);
			currentLine = currentWord;
		}
	}

	if (!currentLine.empty()) {
		outLines.push_back(currentLine);
	}

	if (outLines.empty()) {
		outLines.push_back("");
	}
}

void ChatScreen::render(int xm, int ym, float a) {
	Font* font = minecraft->font;

	int barX = 2;
	int barW = width - 4;
	float maxTextW = (float)(barW - 8);

	std::string displayedText = m_input;
	if ((m_blinkTimer / 10) % 2 == 0) {
		displayedText += "_";
	}

	std::vector<std::string> inputLines;
	wrapInputText(font, displayedText, maxTextW, inputLines);

	int lineCount = (int)inputLines.size();
	int visibleLines = std::min(lineCount, 4); // Grow up to 4 lines
	int barH = visibleLines * 12;
	int barY = height - 2 - barH;

	fill(barX, barY, barX + barW, barY + barH, 0x80000000); // 50% opacity black

	// Scroll lines so the cursor line stays visible at the bottom
	int startLineIdx = std::max(0, lineCount - 4);

	glEnable(GL_SCISSOR_TEST);
	int scissorX = (int)((barX + 2) * Gui::GuiScale);
	int scissorW = (int)((barW - 4) * Gui::GuiScale);
	int physY = minecraft->height - (int)((barY + barH) * Gui::GuiScale);
	int scissorH = (int)(barH * Gui::GuiScale);

	if (scissorX < 0) scissorX = 0;
	if (physY < 0) physY = 0;
	if (scissorW < 0) scissorW = 0;
	if (scissorH < 0) scissorH = 0;

	glScissor(scissorX, physY, scissorW, scissorH);

	for (int i = 0; i < visibleLines && (startLineIdx + i) < lineCount; i++) {
		const std::string& lineStr = inputLines[startLineIdx + i];
		float lineY = (float)(barY + 2 + i * 12);
		font->drawShadow(lineStr, (float)(barX + 4), lineY, 0xffffffff, true);
	}

	glDisable(GL_SCISSOR_TEST);

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

	if (key == Keyboard::KEY_BACKSPACE || key == 127) {
		if (!m_input.empty()) {
			m_input.pop_back();
		}
		return;
	}
}

void ChatScreen::keyboardNewChar(char c) {
	if (c >= 32 && c < 127) {
		// Limit input to 100 characters like Java Edition
		if (m_input.length() < 100) {
			m_input += c;
		}
	}
}

void ChatScreen::mouseClicked(int x, int y, int buttonNum) {
	if (buttonNum == MouseAction::ACTION_LEFT) {
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

		// Chat box touch region
		if (x >= posX && x <= posX + boxWidth && y >= boxTop - 4.0f && y <= baseY + lineH + 4.0f) {
			m_isDragging = true;
			m_dragStartY = y;
			m_startTouchY = y;
			m_dragAccumulatorY = 0.0f;
			return;
		}
	}
	Screen::mouseClicked(x, y, buttonNum);
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
