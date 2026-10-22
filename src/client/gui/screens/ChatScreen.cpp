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
#include "commands/CommandManager.hpp"
#include "util/StringUtils.h"
#include <algorithm>
#include <sstream>
#include <cctype>

namespace {

std::string toLower(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return lower;
}

static bool isIntegerToken(const std::string& token) {
    if (token.empty()) return false;
    size_t start = 0;
    if (token[0] == '-' || token[0] == '+') start = 1;
    if (start >= token.length()) return false;
    for (size_t i = start; i < token.length(); ++i) {
        if (token[i] < '0' || token[i] > '9') return false;
    }
    return true;
}

static bool isNumberToken(const std::string& token) {
    if (token.empty()) return false;
    size_t start = 0;
    if (token[0] == '-' || token[0] == '+') start = 1;
    if (start >= token.length()) return false;
    bool hasDot = false;
    for (size_t i = start; i < token.length(); ++i) {
        if (token[i] == '.') {
            if (hasDot) return false;
            hasDot = true;
        } else if (token[i] < '0' || token[i] > '9') {
            return false;
        }
    }
    return true;
}

static bool isCoordinateToken(const std::string& token) {
    if (token.empty()) return false;
    if (token[0] == '~') {
        if (token.length() == 1) return true;
        return isNumberToken(token.substr(1));
    }
    return isNumberToken(token);
}

static bool isEnumToken(const std::string& token, const std::vector<std::string>& allowed, bool isLastTokenBeingTyped) {
    if (token.empty()) return false;
    std::string lowerToken = toLower(token);
    for (size_t i = 0; i < allowed.size(); ++i) {
        std::string lowerVal = toLower(allowed[i]);
        if (isLastTokenBeingTyped) {
            if (lowerVal.length() >= lowerToken.length() && lowerVal.substr(0, lowerToken.length()) == lowerToken) {
                return true;
            }
        } else {
            if (lowerVal == lowerToken) {
                return true;
            }
        }
    }
    return false;
}

struct ParsedArgToken {
    std::string leadingSpaces;
    std::string text;
    bool isLast;
};

static bool validateArgumentToken(Command* cmdObj, size_t argIdx, const std::string& token, bool isLastTokenBeingTyped, const std::vector<ParsedArgToken>& allArgTokens) {
    if (cmdObj == NULL) return false;

    const std::string& cName = cmdObj->getName();

    if (cName == "tp") {
        if (argIdx == 0) {
            return !token.empty();
        } else if (argIdx == 1) {
            if (isCoordinateToken(allArgTokens[0].text)) {
                return isCoordinateToken(token);
            }
            return !token.empty();
        } else if (argIdx == 2 || argIdx == 3) {
            return isCoordinateToken(token);
        } else {
            return false;
        }
    }

    if (cName == "time") {
        if (argIdx == 0) {
            return isEnumToken(token, {"set", "add"}, isLastTokenBeingTyped);
        } else if (argIdx == 1) {
            std::string subCmd = toLower(allArgTokens[0].text);
            if (subCmd == "set") {
                return isEnumToken(token, {"day", "noon", "night", "midnight"}, isLastTokenBeingTyped) || isIntegerToken(token);
            } else if (subCmd == "add") {
                return isIntegerToken(token);
            }
            return false;
        } else {
            return false;
        }
    }

    if (cName == "spawnpoint") {
        if (argIdx == 0) {
            return !token.empty();
        } else if (argIdx == 1 || argIdx == 2 || argIdx == 3) {
            return isCoordinateToken(token);
        } else {
            return false;
        }
    }

    if (cName == "setworldspawn") {
        if (argIdx < 3) {
            return isCoordinateToken(token);
        } else {
            return false;
        }
    }

    const auto& syntax = cmdObj->getSyntax();

    if (argIdx < syntax.size()) {
        const auto& argDef = syntax[argIdx];
        switch (argDef.type) {
            case CommandArgType::Integer:
                return isIntegerToken(token);
            case CommandArgType::Number:
                return isNumberToken(token);
            case CommandArgType::Coordinate:
                return isCoordinateToken(token);
            case CommandArgType::Enum:
                return isEnumToken(token, argDef.allowedValues, isLastTokenBeingTyped);
            case CommandArgType::Player:
            case CommandArgType::Item:
            case CommandArgType::Text:
            default:
                return !token.empty();
        }
    } else {
        if (!syntax.empty() && syntax.back().type == CommandArgType::Text) {
            return !token.empty();
        }
        return false;
    }
}

static std::string getHighlightedCommandInput(Minecraft* minecraft, const std::string& rawInput) {
    if (rawInput.empty() || rawInput[0] != '/') {
        return rawInput;
    }

    bool cmdColorsOn = minecraft->options.getBooleanValue(OPTIONS_COMMAND_COLORS);
    if (!cmdColorsOn) {
        return "\xc2\xa7f" + rawInput;
    }

    bool allowCmds = true;
    if (minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
        allowCmds = minecraft->level->getLevelData()->getAllowCheats();
    }

    const auto& allCmds = minecraft->commandManager().getAllCommands();
    std::vector<Command*> availableCmds;
    for (auto cmd : allCmds) {
        if (cmd != NULL) {
            if (!allowCmds && cmd->isOperatorCommand()) {
                continue;
            }
            availableCmds.push_back(cmd);
        }
    }

    std::string afterSlash = rawInput.substr(1);
    size_t firstSpace = afterSlash.find(' ');
    bool hasSpaceAfterCmd = (firstSpace != std::string::npos);

    std::string cmdToken = hasSpaceAfterCmd ? afterSlash.substr(0, firstSpace) : afterSlash;
    std::string lookupCmd = toLower(cmdToken);
    if (lookupCmd == "?") lookupCmd = "help";

    Command* matchedCmd = NULL;
    bool isCmdValid = false;

    if (!hasSpaceAfterCmd) {
        if (lookupCmd.empty()) {
            isCmdValid = true;
        } else {
            for (auto cmd : availableCmds) {
                std::string cName = toLower(cmd->getName());
                if (cName.length() >= lookupCmd.length() && cName.substr(0, lookupCmd.length()) == lookupCmd) {
                    isCmdValid = true;
                    if (cName == lookupCmd) {
                        matchedCmd = cmd;
                    }
                    break;
                }
            }
        }
    } else {
        for (auto cmd : availableCmds) {
            if (toLower(cmd->getName()) == lookupCmd) {
                matchedCmd = cmd;
                isCmdValid = true;
                break;
            }
        }
    }

    if (!isCmdValid) {
        return "\xc2\xa7c" + rawInput;
    }

    std::string result = "\xc2\xa7f/" + cmdToken;

    if (!hasSpaceAfterCmd) {
        return result;
    }

    std::string rest = afterSlash.substr(firstSpace);

    std::vector<ParsedArgToken> argTokens;
    size_t pos = 0;
    while (pos < rest.length()) {
        size_t spaceStart = pos;
        while (pos < rest.length() && rest[pos] == ' ') {
            pos++;
        }
        std::string leadingSpaces = rest.substr(spaceStart, pos - spaceStart);
        if (pos >= rest.length()) {
            ParsedArgToken pt;
            pt.leadingSpaces = leadingSpaces;
            pt.text = "";
            pt.isLast = true;
            argTokens.push_back(pt);
            break;
        }
        size_t tokenStart = pos;
        while (pos < rest.length() && rest[pos] != ' ') {
            pos++;
        }
        std::string text = rest.substr(tokenStart, pos - tokenStart);
        bool isLast = (pos >= rest.length());

        ParsedArgToken pt;
        pt.leadingSpaces = leadingSpaces;
        pt.text = text;
        pt.isLast = isLast;
        argTokens.push_back(pt);
    }

    const char* ARG_COLORS[] = {
        "\xc2\xa7" "b", // 1. Light Blue / Aqua (§b)
        "\xc2\xa7" "e", // 2. Yellow (§e)
        "\xc2\xa7" "a", // 3. Green (§a)
        "\xc2\xa7" "d", // 4. Light Purple (§d)
        "\xc2\xa7" "6"  // 5. Gold (§6)
    };

    bool inError = false;

    for (size_t i = 0; i < argTokens.size(); ++i) {
        const auto& pt = argTokens[i];
        if (pt.text.empty()) {
            if (inError) {
                result += "\xc2\xa7c" + pt.leadingSpaces;
            } else {
                result += "\xc2\xa7f" + pt.leadingSpaces;
            }
            continue;
        }

        if (inError) {
            result += "\xc2\xa7c" + pt.leadingSpaces + pt.text;
            continue;
        }

        bool valid = validateArgumentToken(matchedCmd, i, pt.text, pt.isLast, argTokens);

        if (!valid) {
            inError = true;
            result += "\xc2\xa7c" + pt.leadingSpaces + pt.text;
        } else {
            const char* color = ARG_COLORS[i % 5];
            result += color + pt.leadingSpaces + pt.text;
        }
    }

    return result;
}

} // namespace

ChatScreen::ChatScreen()
	: m_input(""),
	  m_cursorPos(0),
	  m_blinkTimer(0),
	  m_scrollY(0.0f),
	  m_maxScroll(0.0f),
	  m_isDragging(false),
	  m_dragStartY(0),
	  m_startTouchY(0),
	  m_dragAccumulatorY(0.0f),
	  m_lastRawInputForHighlight("\xFF"),
	  m_lastAllowCmdsForHighlight(true),
	  m_lastCmdColorsForHighlight(true)
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
	m_lastRawInputForHighlight = "\xFF";
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

// Helper to compute gray hint string for ChatScreen
static std::string getCommandHintText(Minecraft* minecraft, const std::string& input) {
	if (input.empty() || input[0] != '/') {
		return "";
	}

	bool allowCmds = true;
	if (minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
		allowCmds = minecraft->level->getLevelData()->getAllowCheats();
	}

	const auto& allCmds = minecraft->commandManager().getAllCommands();
	std::vector<Command*> availableCmds;
	for (auto cmd : allCmds) {
		if (cmd != NULL) {
			if (!allowCmds && cmd->isOperatorCommand()) {
				continue;
			}
			availableCmds.push_back(cmd);
		}
	}

	std::string afterSlash = input.substr(1);
	size_t firstSpace = afterSlash.find(' ');

	if (firstSpace == std::string::npos) {
		// Typing command name or only "/" or "/?"
		std::string typedCmd = afterSlash;
		if (typedCmd == "?") typedCmd = "help";

		std::string hint = "";

		if (typedCmd.empty()) {
			// Show first match completion (e.g. "help")
			if (!availableCmds.empty()) {
				hint = availableCmds[0]->getName();
			}
		} else {
			// Match prefix
			for (auto cmd : availableCmds) {
				const std::string& cName = cmd->getName();
				if (cName.length() >= typedCmd.length() && cName.substr(0, typedCmd.length()) == typedCmd) {
					hint = cName.substr(typedCmd.length());
					break;
				}
			}
		}
		return hint;
	}

	// Command name has been typed, followed by space(s) and optional arguments
	std::string cmdName = afterSlash.substr(0, firstSpace);
	if (cmdName == "?") cmdName = "help";

	Command* targetCmd = NULL;
	for (auto cmd : availableCmds) {
		if (cmd->getName() == cmdName) {
			targetCmd = cmd;
			break;
		}
	}

	if (targetCmd == NULL) {
		return "";
	}

	// Count arguments typed
	std::string argsStr = afterSlash.substr(firstSpace + 1);
	std::istringstream iss(argsStr);
	std::vector<std::string> typedArgs;
	std::string tok;
	while (iss >> tok) {
		typedArgs.push_back(tok);
	}

	size_t typedCount = typedArgs.size();
	if (!argsStr.empty() && argsStr[argsStr.length() - 1] == ' ') {
		// Currently on next argument
	} else if (typedCount > 0) {
		typedCount -= 1; // Currently typing this argument
	}

	const auto& syntax = targetCmd->getSyntax();
	if (typedCount >= syntax.size()) {
		return "";
	}

	std::ostringstream hintStream;
	for (size_t i = typedCount; i < syntax.size(); ++i) {
		if (i > typedCount || (!argsStr.empty() && argsStr[argsStr.length() - 1] == ' ')) {
			hintStream << " ";
		}
		const auto& arg = syntax[i];
		if (arg.optional) {
			hintStream << "[" << arg.name << "]";
		} else {
			hintStream << "<" << arg.name << ">";
		}
	}

	return hintStream.str();
}

void ChatScreen::render(int xm, int ym, float a) {
	Font* font = minecraft->font;

	int barX = 2;
	int barW = width - 4;
	float maxTextW = (float)(barW - 8);

	bool allowCmds = true;
	if (minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
		allowCmds = minecraft->level->getLevelData()->getAllowCheats();
	}
	bool cmdColors = minecraft->options.getBooleanValue(OPTIONS_COMMAND_COLORS);

	if (m_input != m_lastRawInputForHighlight ||
		allowCmds != m_lastAllowCmdsForHighlight ||
		cmdColors != m_lastCmdColorsForHighlight) {
		m_lastRawInputForHighlight = m_input;
		m_lastAllowCmdsForHighlight = allowCmds;
		m_lastCmdColorsForHighlight = cmdColors;
		m_lastHighlightedInput = getHighlightedCommandInput(minecraft, m_input);
	}

	std::string displayedText = m_lastHighlightedInput;
	std::string hintText = getCommandHintText(minecraft, m_input);

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

		// Draw gray hint on the last line (where the cursor is)
		if (startLineIdx + i == lineCount - 1 && !hintText.empty()) {
			float lineW = (float)Gui::getColoredWidth(font, lineStr, true);
			font->drawShadow(hintText, (float)(barX + 4) + lineW, lineY, 0x808080ff, true);
		}
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
