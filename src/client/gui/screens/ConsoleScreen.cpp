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
:   _inputBox(1, 2, 0, 100, 14, "")
{
}

void ConsoleScreen::init()
{
    _inputBox = TextBox(1, 2, height - 16, width - 4, 14, "");
    _inputBox.setFocus(minecraft);

    textBoxes.clear();
    textBoxes.push_back(&_inputBox);
}

void ConsoleScreen::tick()
{
    super::tick();
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
        return;
    }

    if (eventKey == Keyboard::KEY_RETURN) {
        execute();
        return;
    }

    super::keyPressed(eventKey);
}

void ConsoleScreen::charPressed(char inputChar)
{
    super::charPressed(inputChar);
}

void ConsoleScreen::mouseClicked(int x, int y, int buttonNum) {
    super::mouseClicked(x, y, buttonNum);
}

void ConsoleScreen::mouseReleased(int x, int y, int buttonNum) {
    super::mouseReleased(x, y, buttonNum);
}

void ConsoleScreen::execute()
{
    if (!minecraft->level) return;

    if (_inputBox.text.empty()) {
        return minecraft->setScreen(NULL);
    }

    if (minecraft->netCallback && !minecraft->raknetInstance->isServer()) {
        MessagePacket packet(_inputBox.text.c_str());
        minecraft->raknetInstance->send(packet);
    } else if (_inputBox.text[0] == '/') {
        std::string cmd = Util::stringTrim(_inputBox.text.substr(1));

        std::istringstream iss(
            minecraft->commandManager().execute(*minecraft, *minecraft->player, cmd)
        );

        for (std::string line; std::getline(iss, line);) {
            minecraft->gui.addMessage(line);
        }
    } else {
        if (minecraft->raknetInstance->isServer()) {
            MessagePacket packet(_inputBox.text.c_str());
            minecraft->raknetInstance->send(packet);
        }

        minecraft->gui.addMessage("<" + minecraft->player->name + "> " + _inputBox.text);
    }

    _inputBox.text = "";
    minecraft->setScreen(NULL);
}

void ConsoleScreen::render(int xm, int ym, float a)
{
    super::render(xm, ym, a);
}
