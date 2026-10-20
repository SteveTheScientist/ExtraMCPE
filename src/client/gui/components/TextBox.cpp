#include "TextBox.h"
#include "../Gui.h"
#include "../Screen.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/time.h"
#include "../../../util/StringUtils.h"

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
   id(id), hint(msg), focused(false), blink(false), blinkTicks(0),
   isPressing(false), pressStartTime(0), pressStartX(0), pressStartY(0),
   showPopup(false), popupX(0), popupY(0), popupW(180), popupH(20)
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
    showPopup = false;
    isPressing = false;
    if (focused) {
        minecraft->platform()->hideKeyboard();
        focused = false;
        return true;
    }
    return false;
}

void TextBox::mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum) {
    if (buttonNum == MouseAction::ACTION_LEFT) {
        if (showPopup) {
            // Check popup button hits
            if (x >= popupX && x <= popupX + popupW && y >= popupY && y <= popupY + popupH) {
                int btnIndex = (x - popupX) / (popupW / 4);
                if (btnIndex < 0) btnIndex = 0;
                if (btnIndex > 3) btnIndex = 3;

                if (btnIndex == 0) { // Select All
                    // Select All
                } else if (btnIndex == 1) { // Copy
                    if (!text.empty()) {
                        minecraft->platform()->setClipboardText(text);
                    }
                } else if (btnIndex == 2) { // Cut
                    if (!text.empty()) {
                        minecraft->platform()->setClipboardText(text);
                        text.clear();
                    }
                } else if (btnIndex == 3) { // Paste
                    std::string clip = minecraft->platform()->getClipboardText();
                    if (!clip.empty()) {
                        std::string cleaned = Util::cleanPastedText(clip, 256 - text.length());
                        text += cleaned;
                    }
                }
                showPopup = false;
                return;
            } else {
                showPopup = false;
            }
        }

        if (pointInside(x, y)) {
            setFocus(minecraft);
            isPressing = true;
            pressStartTime = getTimeMs();
            pressStartX = x;
            pressStartY = y;
        } else {
            loseFocus(minecraft);
        }
    }
}

void TextBox::charPressed(Minecraft* minecraft, char c)  {
    bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);

    if (ctrlDown || (unsigned char)c < 32 || c == 127) {
        return;
    }

    if (focused && c >= 32 && c < 127 && (int)text.size() < 256) {
        text.push_back(c);
    }
}

void TextBox::keyPressed(Minecraft* minecraft, int key) {
    if (!focused) return;

    bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);

    if (ctrlDown) {
        if (key == Keyboard::KEY_C || key == 'c' || key == 'C') {
            if (!text.empty()) {
                minecraft->platform()->setClipboardText(text);
            }
            return;
        }
        if (key == Keyboard::KEY_X || key == 'x' || key == 'X') {
            if (!text.empty()) {
                minecraft->platform()->setClipboardText(text);
                text.clear();
            }
            return;
        }
        if (key == Keyboard::KEY_V || key == 'v' || key == 'V') {
            std::string clip = minecraft->platform()->getClipboardText();
            if (!clip.empty()) {
                std::string cleaned = Util::cleanPastedText(clip, 256 - text.length());
                text += cleaned;
            }
            return;
        }
        if (key == Keyboard::KEY_A || key == 'a' || key == 'A') {
            // Select All
            return;
        }
    }

    if ((key == Keyboard::KEY_BACKSPACE || key == 127) && !text.empty()) {
        text.pop_back();
    }
}

void TextBox::tick(Minecraft* minecraft) {
    blinkTicks++;
    if (blinkTicks >= 5) {
        blink = !blink;
        blinkTicks = 0;
    }

    if (isPressing && focused) {
        if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
            long elapsed = getTimeMs() - pressStartTime;
            if (elapsed >= 500) { // 500ms long press
                isPressing = false;
                showPopup = true;

                int screenW = (int)(minecraft->width * Gui::InvGuiScale);
                popupW = 180;
                popupH = 20;

                popupX = x + (width - popupW) / 2;
                if (popupX < 2) popupX = 2;
                if (popupX + popupW > screenW - 2) popupX = screenW - popupW - 2;

                if (y - popupH - 4 >= 2) {
                    popupY = y - popupH - 4; // Position above text box
                } else {
                    popupY = y + height + 4; // Position below text box
                }
            }
        } else {
            isPressing = false;
        }
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
    float textW = (float)font->width(displayed, true);
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

    drawString(font, displayed, (int)textX, _y, 0xffffffff, true);

    glDisable(GL_SCISSOR_TEST);

    if (showPopup) {
        // Draw popup container
        fill(popupX - 1, popupY - 1, popupX + popupW + 1, popupY + popupH + 1, 0xff555555);
        fill(popupX, popupY, popupX + popupW, popupY + popupH, 0xeb101010);

        int btnW = popupW / 4;
        static const char* btnLabels[4] = { "Select All", "Copy", "Cut", "Paste" };

        for (int i = 0; i < 4; i++) {
            int bx0 = popupX + i * btnW;
            int bx1 = bx0 + btnW;

            bool isHover = (xm >= bx0 && xm < bx1 && ym >= popupY && ym < popupY + popupH);
            if (isHover) {
                fill(bx0, popupY, bx1, popupY + popupH, 0x50ffffff);
            }

            if (i > 0) {
                fill(bx0, popupY + 2, bx0 + 1, popupY + popupH - 2, 0xff555555);
            }

            int labelX = bx0 + (btnW - font->width(btnLabels[i])) / 2;
            int labelY = popupY + (popupH - Font::DefaultLineHeight) / 2;
            drawString(font, btnLabels[i], labelX, labelY, isHover ? 0xffffff55 : 0xffffffff);
        }
    }
}
