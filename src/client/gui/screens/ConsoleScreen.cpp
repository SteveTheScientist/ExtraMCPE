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

// ---------------------------------------------------------------------------
// render
// ---------------------------------------------------------------------------
void ConsoleScreen::render(int /*xm*/, int /*ym*/, float /*a*/)
{
    // Java Edition 1.8 style input bar: solid black at 50% opacity, 12px tall, 2px from bottom/sides, no border, 4px left padding, blinking cursor
    const int boxH  = 12;
    const int boxY  = height - 14;
    const int boxX0 = 2;
    const int boxX1 = width - 2;
    const int boxW  = boxX1 - boxX0;

    // Solid black at 50% opacity background with no border/outline
    fill(boxX0, boxY, boxX1, boxY + boxH, 0x80000000);

    // Input text + blinking cursor
    std::string displayed = _input;
    if ((_cursorBlink / 10) % 2 == 0)
        displayed += '_';

    float textW = Gui::getColoredWidth(font, displayed);
    float maxVisibleW = (float)(boxW - 8); // 4px padding on each side inside bar

    float textX = (float)(boxX0 + 4);
    if (textW > maxVisibleW) {
        textX = (float)(boxX0 + 4) - (textW - maxVisibleW);
    }

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

    // White text with shadow
    font->drawShadow(displayed, textX, (float)(boxY + 2), 0xffffffff);

    glDisable(GL_SCISSOR_TEST);
}
