#include "TextBox.h"
#include "../Gui.h"
#include "../Screen.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Mouse.h"

// delegate constructors
TextBox::TextBox(int id, const std::string& msg)
 : TextBox(id, 0, 0, msg)
{
}

TextBox::TextBox(int id, int x, int y, const std::string& msg)
 : TextBox(id, x, y, 24, Font::DefaultLineHeight + 4, msg)
{
}

TextBox::TextBox(int id, int x, int y, int w, int h, const std::string& msg)
 : GuiElement(true, true, x, y, w, h),
   id(id), hint(msg), focused(false), blink(false), blinkTicks(0)
{
}

void TextBox::setFocus(Minecraft* minecraft) {
    if (!focused) {
        minecraft->platform()->showKeyboard();
        focused = true;
        blinkTicks = 0;
        blink = false;
    }
}

bool TextBox::loseFocus(Minecraft* minecraft) {
    if (focused) {
        minecraft->platform()->hideKeyboard();
        focused = false;
        return true;
    }
    return false;
}

void TextBox::mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (pointInside(x, y)) {
            setFocus(minecraft);
        } else {
            loseFocus(minecraft);
        }
    }
}

void TextBox::charPressed(Minecraft* minecraft, char c)  {
    if (focused && c >= 32 && c < 127 && (int)text.size() < 256) {
        text.push_back(c);
    }
}

void TextBox::keyPressed(Minecraft* minecraft, int key) {
    if (focused) {
        if ((key == Keyboard::KEY_BACKSPACE || key == 127) && !text.empty()) {
            text.pop_back();
        }
    }
}

void TextBox::tick(Minecraft* minecraft) {
    blinkTicks++;
    if (blinkTicks >= 5) {
        blink = !blink;
        blinkTicks = 0;
    }
}

void TextBox::render(Minecraft* minecraft, int xm, int ym) {
    uint32_t bgColor = focused ? 0xffa0a0a0 : 0xffa0a0a0;
    uint32_t borderColor = focused ? 0xff000000 : 0xff000000;
    fill(x, y, x + width, y + height, bgColor);
    fill(x + 1, y + 1, x + width - 1, y + height - 1, borderColor);

	int _y = y + (height - Font::DefaultLineHeight) / 2;

    if (text.empty() && !focused) {
        drawString(minecraft->font, hint, x + 2, _y, 0xff5e5e5e);
        return;
    }

    std::string displayed = text;
    if (focused && blink) displayed.push_back('_');

    Font* font = minecraft->font;
    float textW = Gui::getColoredWidth(font, displayed);
    float maxVisibleW = (float)(width - 4); // 2px margin inside box

    float textX = (float)(x + 2);
    if (textW > maxVisibleW) {
        textX = (float)(x + 2) - (textW - maxVisibleW);
    }

    glEnable(GL_SCISSOR_TEST);
    int scissorX = (int)((x + 1) * Gui::GuiScale);
    int scissorW = (int)((width - 2) * Gui::GuiScale);
    int physY = minecraft->height - (int)((y + height - 1) * Gui::GuiScale);
    int scissorH = (int)((height - 2) * Gui::GuiScale);

    if (scissorX < 0) scissorX = 0;
    if (physY < 0) physY = 0;
    if (scissorW < 0) scissorW = 0;
    if (scissorH < 0) scissorH = 0;

    glScissor(scissorX, physY, scissorW, scissorH);

    drawString(font, displayed, textX, _y, 0xffffffff);

    glDisable(GL_SCISSOR_TEST);
}
