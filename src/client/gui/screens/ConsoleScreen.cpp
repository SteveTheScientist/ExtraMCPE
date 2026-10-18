#include "ConsoleScreen.h"
#include "../Gui.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../../platform/input/Keyboard.h"
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
    _cursorBlink(0)
{
}

void ConsoleScreen::init()
{
}

void ConsoleScreen::tick()
{
    _cursorBlink++;
}

bool ConsoleScreen::handleBackEvent(bool /*isDown*/)
{
    minecraft->setScreen(NULL);
    return true;
}

void ConsoleScreen::keyPressed(int eventKey)
{
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->setScreen(NULL);
    } else if (eventKey == Keyboard::KEY_RETURN) {
        execute();
    } else if (eventKey == Keyboard::KEY_BACKSPACE) {
        if (!_input.empty())
            _input.erase(_input.size() - 1, 1);
    } else {
        Screen::keyPressed(eventKey);
    }
}

void ConsoleScreen::charPressed(char inputChar)
{
    if (inputChar >= 32 && inputChar < 127) {
        // Limit input to 100 characters like Java Edition
        if (_input.length() < 100) {
            _input += inputChar;
        }
    }
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

// ---------------------------------------------------------------------------
// render
// ---------------------------------------------------------------------------
void ConsoleScreen::render(int /*xm*/, int /*ym*/, float /*a*/)
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

    // Solid black at 50% opacity background with no border/outline
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
        font->drawShadow(lineStr, (float)(boxX0 + 4), lineY, 0xffffffff);
    }

    glDisable(GL_SCISSOR_TEST);
}
