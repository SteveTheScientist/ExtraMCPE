#include "ConsoleScreen.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../platform/input/Keyboard.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/time.h"
#include "../../../world/level/Level.h"
#include "../../../network/RakNetInstance.h"
#include "../../../network/ServerSideNetworkHandler.h"
#include "../../../network/packet/MessagePacket.h"
#include "../../../platform/log.h"
#include "util/StringUtils.h"

#include <sstream>
#include <cstdlib>
#include <cctype>

ConsoleScreen::ConsoleScreen()
:   _input(""),
    _cursorBlink(0),
    _isPressingInput(false),
    _pressStartTime(0),
    _showPopup(false),
    _popupX(0),
    _popupY(0),
    _popupW(180),
    _popupH(20)
{
}

void ConsoleScreen::init()
{
    _input = "";
    _isPressingInput = false;
    _showPopup = false;
}

void ConsoleScreen::tick()
{
    _cursorBlink++;

    int boxX0 = 2;
    int boxX1 = width - 2;
    int boxW  = boxX1 - boxX0;
    int boxH  = 12;
    int boxY  = height - 2 - boxH;

    if (_isPressingInput) {
        if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
            long elapsed = getTimeMs() - _pressStartTime;
            if (elapsed >= 500) {
                _isPressingInput = false;
                _showPopup = true;

                _popupW = 180;
                _popupH = 20;
                _popupX = boxX0 + (boxW - _popupW) / 2;
                if (_popupX < 2) _popupX = 2;
                if (_popupX + _popupW > width - 2) _popupX = width - _popupW - 2;

                _popupY = boxY - _popupH - 4;
                if (_popupY < 2) _popupY = boxY + 18;
            }
        } else {
            _isPressingInput = false;
        }
    }
}

bool ConsoleScreen::handleBackEvent(bool /*isDown*/)
{
    minecraft->setScreen(NULL);
    return true;
}

void ConsoleScreen::keyPressed(int eventKey)
{
    bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);

    if (ctrlDown) {
        if (eventKey == Keyboard::KEY_C || eventKey == 'c' || eventKey == 'C') {
            if (!_input.empty()) {
                minecraft->platform()->setClipboardText(_input);
            }
            return;
        }
        if (eventKey == Keyboard::KEY_X || eventKey == 'x' || eventKey == 'X') {
            if (!_input.empty()) {
                minecraft->platform()->setClipboardText(_input);
                _input.clear();
            }
            return;
        }
        if (eventKey == Keyboard::KEY_V || eventKey == 'v' || eventKey == 'V') {
            std::string clip = minecraft->platform()->getClipboardText();
            if (!clip.empty()) {
                std::string cleaned = Util::cleanPastedText(clip, 100 - _input.length());
                _input += cleaned;
            }
            return;
        }
        if (eventKey == Keyboard::KEY_A || eventKey == 'a' || eventKey == 'A') {
            return;
        }
    }

    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
    } else if (eventKey == Keyboard::KEY_RETURN) {
        execute();
    } else if (eventKey == Keyboard::KEY_BACKSPACE || eventKey == 127) {
        if (!_input.empty())
            _input.erase(_input.size() - 1, 1);
    }
}

void ConsoleScreen::charPressed(char inputChar)
{
    bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);
    if (ctrlDown || (unsigned char)inputChar < 32 || inputChar == 127) return;

    if (inputChar >= 32 && inputChar < 127) {
        if (_input.length() < 100) {
            _input += inputChar;
        }
    }
}

void ConsoleScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (_showPopup) {
            if (x >= _popupX && x <= _popupX + _popupW && y >= _popupY && y <= _popupY + _popupH) {
                int btnIndex = (x - _popupX) / (_popupW / 4);
                if (btnIndex < 0) btnIndex = 0;
                if (btnIndex > 3) btnIndex = 3;

                if (btnIndex == 0) { // Select All
                } else if (btnIndex == 1) { // Copy
                    if (!_input.empty()) {
                        minecraft->platform()->setClipboardText(_input);
                    }
                } else if (btnIndex == 2) { // Cut
                    if (!_input.empty()) {
                        minecraft->platform()->setClipboardText(_input);
                        _input.clear();
                    }
                } else if (btnIndex == 3) { // Paste
                    std::string clip = minecraft->platform()->getClipboardText();
                    if (!clip.empty()) {
                        std::string cleaned = Util::cleanPastedText(clip, 100 - _input.length());
                        _input += cleaned;
                    }
                }
                _showPopup = false;
                return;
            } else {
                _showPopup = false;
            }
        }

        const int boxX0 = 2;
        const int boxX1 = width - 2;
        const int boxH  = 12;
        const int boxY  = height - 2 - boxH;
        if (x >= boxX0 && x <= boxX1 && y >= boxY - 4 && y <= height) {
            _isPressingInput = true;
            _pressStartTime = getTimeMs();
        }
    }
    super::mouseClicked(x, y, buttonNum);
}

void ConsoleScreen::mouseReleased(int x, int y, int buttonNum) {
    _isPressingInput = false;
    super::mouseReleased(x, y, buttonNum);
}

void ConsoleScreen::execute()
{
    if (!minecraft->level) return;

    if (_input.empty()) {
        return minecraft->setScreen(NULL);
    }

    if (minecraft->netCallback && !minecraft->raknetInstance->isServer()) {
        MessagePacket packet(_input.c_str());
        minecraft->raknetInstance->send(packet);
    } else if (_input[0] == '/') {
        _input = Util::stringTrim(_input.substr(1));

        std::istringstream iss(
            minecraft->commandManager().execute(*minecraft, *minecraft->player, _input)
        );

        for (std::string line; std::getline(iss, line);) {
            minecraft->gui.addMessage(line);
        }
    }
    else {
        if (minecraft->raknetInstance->isServer()) {
            MessagePacket packet(_input.c_str());
            minecraft->raknetInstance->send(packet);
        }

        minecraft->gui.addMessage("<" + minecraft->player->name + "> " + _input);
    }

    minecraft->setScreen(NULL);
}

static void wrapInputTextConsole(Font* font, const std::string& input, float maxW, std::vector<std::string>& outLines) {
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
					if (Gui::getColoredWidth(font, currentLine + " " + currentWord) <= maxW) {
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
			if (Gui::getColoredWidth(font, testLine) > maxW) {
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
				while (Gui::getColoredWidth(font, currentWord) > maxW) {
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
		if (!currentLine.empty() && Gui::getColoredWidth(font, currentLine + " " + currentWord) <= maxW) {
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

void ConsoleScreen::render(int xm, int ym, float a)
{
    const int boxX0 = 2;
    const int boxX1 = width - 2;
    const int boxW  = boxX1 - boxX0;
    float maxTextW = (float)(boxW - 8);

    std::string displayed = _input;
    if ((_cursorBlink / 10) % 2 == 0)
        displayed += '_';

    std::vector<std::string> inputLines;
    wrapInputTextConsole(font, displayed, maxTextW, inputLines);

    int lineCount = (int)inputLines.size();
    int visibleLines = std::min(lineCount, 4); // Grow up to 4 lines upward
    int boxH = visibleLines * 12;
    int boxY = height - 2 - boxH;

    fill(boxX0, boxY, boxX1, boxY + boxH, 0x80000000);

    int startLineIdx = std::max(0, lineCount - 4);

    glEnable(GL_SCISSOR_TEST);
    int scissorX = (int)((boxX0 + 2) * Gui::GuiScale);
    int scissorW = (int)((boxW - 4) * Gui::GuiScale);
    int physY = minecraft->height - (int)((boxY + boxH) * Gui::GuiScale);
    int scissorH = (int)(boxH * Gui::GuiScale);

    if (scissorX < 0) scissorX = 0;
    if (physY < 0) physY = 0;
    if (scissorW < 0) scissorW = 0;
    if (scissorH < 0) scissorH = 0;

    glScissor(scissorX, physY, scissorW, scissorH);

    for (int i = 0; i < visibleLines && (startLineIdx + i) < lineCount; i++) {
        const std::string& lineStr = inputLines[startLineIdx + i];
        float lineY = (float)(boxY + 2 + i * 12);
        font->drawShadow(lineStr, (float)(boxX0 + 4), lineY, 0xffffffff, true);
    }

    glDisable(GL_SCISSOR_TEST);

    if (_showPopup) {
        fill(_popupX - 1, _popupY - 1, _popupX + _popupW + 1, _popupY + _popupH + 1, 0xff555555);
        fill(_popupX, _popupY, _popupX + _popupW, _popupY + _popupH, 0xeb101010);

        int btnW = _popupW / 4;
        static const char* btnLabels[4] = { "Select All", "Copy", "Cut", "Paste" };

        for (int i = 0; i < 4; i++) {
            int bx0 = _popupX + i * btnW;
            int bx1 = bx0 + btnW;

            bool isHover = (xm >= bx0 && xm < bx1 && ym >= _popupY && ym < _popupY + _popupH);
            if (isHover) {
                fill(bx0, _popupY, bx1, _popupY + _popupH, 0x50ffffff);
            }

            if (i > 0) {
                fill(bx0, _popupY + 2, bx0 + 1, _popupY + _popupH - 2, 0xff555555);
            }

            int labelX = bx0 + (btnW - font->width(btnLabels[i])) / 2;
            int labelY = _popupY + (_popupH - Font::DefaultLineHeight) / 2;
            font->drawShadow(btnLabels[i], (float)labelX, (float)labelY, isHover ? 0xffffff55 : 0xffffffff);
        }
    }
}
