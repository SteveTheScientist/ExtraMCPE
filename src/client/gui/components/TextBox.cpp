#include "TextBox.h"
#include "../Gui.h"
#include "../Screen.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/time.h"
#include "../../../util/StringUtils.h"
#include "../../../util/Mth.h"
#include <algorithm>
#include <cmath>
#include <cctype>

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
   id(id), hint(msg), focused(false), blink(false), isChatStyle(false), blinkTicks(0),
   cursorPos(0), selectAnchor(0),
   isPressing(false), isDraggingMouse(false), isDraggingHandleStart(false), isDraggingHandleEnd(false),
   pressStartTime(0), pressStartX(0), pressStartY(0),
   lastClickTime(0), lastClickX(0), lastClickY(0),
   showPopup(false), popupX(0), popupY(0), popupW(180), popupH(20)
{
}

std::string TextBox::getSelectedText() const {
    int s = getSelectionStart();
    int e = getSelectionEnd();
    if (s >= e || s < 0 || e > (int)text.length()) return "";
    return text.substr(s, e - s);
}

bool TextBox::deleteSelection() {
    int s = getSelectionStart();
    int e = getSelectionEnd();
    if (s >= e || s < 0 || e > (int)text.length()) return false;
    text.erase(s, e - s);
    cursorPos = s;
    selectAnchor = s;
    return true;
}

void TextBox::selectAll() {
    selectAnchor = 0;
    cursorPos = (int)text.length();
}

void TextBox::getWordBounds(const std::string& str, int idx, int& outStart, int& outEnd) {
    int len = (int)str.length();
    if (len == 0) {
        outStart = outEnd = 0;
        return;
    }
    if (idx < 0) idx = 0;
    if (idx >= len) idx = len - 1;

    bool isWordChar = isalnum((unsigned char)str[idx]) || str[idx] == '_';

    int s = idx;
    while (s > 0) {
        bool prevWord = isalnum((unsigned char)str[s - 1]) || str[s - 1] == '_';
        if (prevWord != isWordChar) break;
        s--;
    }

    int e = idx;
    while (e < len) {
        bool currWord = isalnum((unsigned char)str[e]) || str[e] == '_';
        if (currWord != isWordChar) break;
        e++;
    }

    outStart = s;
    outEnd = e;
}

void TextBox::selectWordAt(int idx) {
    getWordBounds(text, idx, selectAnchor, cursorPos);
}

void TextBox::insertText(Minecraft* minecraft, const std::string& inputStr) {
    if (!focused || inputStr.empty()) return;

    int selLen = hasSelection() ? (getSelectionEnd() - getSelectionStart()) : 0;
    int maxAllowed = 256 - (int)text.length() + selLen;
    if (maxAllowed <= 0) return;

    std::string cleaned = Util::cleanPastedText(inputStr, maxAllowed);
    if (cleaned.empty()) return;

    if (hasSelection()) {
        deleteSelection();
    }

    if (cursorPos < 0) cursorPos = 0;
    if (cursorPos > (int)text.length()) cursorPos = (int)text.length();

    text.insert(cursorPos, cleaned);
    cursorPos += (int)cleaned.length();
    selectAnchor = cursorPos;
    blink = true;
    blinkTicks = 0;
}

int TextBox::getCharIndexAtX(Font* font, const std::string& str, int clickX, float textX) {
    int len = (int)str.length();
    if (len == 0 || clickX <= textX) return 0;

    int bestIdx = 0;
    float bestDist = std::abs((float)clickX - textX);

    for (int i = 1; i <= len; i++) {
        float px = textX + (float)font->width(str.substr(0, i), true);
        float dist = std::abs((float)clickX - px);
        if (dist < bestDist) {
            bestDist = dist;
            bestIdx = i;
        }
    }
    return bestIdx;
}

void TextBox::setFocus(Minecraft* minecraft) {
    if (!focused) {
        minecraft->platform()->showKeyboard();
        focused = true;
        blinkTicks = 0;
        blink = true;
    }
}

bool TextBox::loseFocus(Minecraft* minecraft) {
    showPopup = false;
    isPressing = false;
    isDraggingMouse = false;
    isDraggingHandleStart = false;
    isDraggingHandleEnd = false;

    if (hasSelection()) {
        cursorPos = getSelectionEnd();
        selectAnchor = cursorPos;
    }

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
            Font* font = minecraft->font;
            static const char* btnLabels[4] = { "Select All", "Copy", "Cut", "Paste" };
            int btnW[4];
            int totalW = 0;
            for (int i = 0; i < 4; i++) {
                btnW[i] = font->width(btnLabels[i]) + 12;
                totalW += btnW[i];
            }

            if (x >= popupX && x <= popupX + totalW && y >= popupY && y <= popupY + popupH) {
                int curX = popupX;
                int btnIndex = -1;
                for (int i = 0; i < 4; i++) {
                    if (x >= curX && x < curX + btnW[i]) {
                        btnIndex = i;
                        break;
                    }
                    curX += btnW[i];
                }

                if (btnIndex == 0) { // Select All
                    selectAll();
                } else if (btnIndex == 1) { // Copy
                    std::string sel = getSelectedText();
                    if (sel.empty()) sel = text;
                    if (!sel.empty()) {
                        minecraft->platform()->setClipboardText(sel);
                    }
                } else if (btnIndex == 2) { // Cut
                    std::string sel = getSelectedText();
                    if (sel.empty()) sel = text;
                    if (!sel.empty()) {
                        minecraft->platform()->setClipboardText(sel);
                        if (hasSelection()) {
                            deleteSelection();
                        } else {
                            text.clear();
                            cursorPos = selectAnchor = 0;
                        }
                    }
                } else if (btnIndex == 3) { // Paste
                    std::string clip = minecraft->platform()->getClipboardText();
                    if (!clip.empty()) {
                        insertText(minecraft, clip);
                    }
                }
                showPopup = false;
                return;
            } else {
                showPopup = false;
            }
        }

        if (pointInside(x, y)) {
            Font* font = minecraft->font;
            int _y = y + (height - Font::DefaultLineHeight) / 2;
            float fullTextW = (float)font->width(text, true);
            float maxVisibleW = (float)(width - 4);

            float scrollX = 0.0f;
            float cursorPixelOffset = (float)font->width(text.substr(0, cursorPos), true);
            if (cursorPixelOffset > maxVisibleW) {
                scrollX = cursorPixelOffset - maxVisibleW;
            }
            if (fullTextW <= maxVisibleW) {
                scrollX = 0.0f;
            }
            float textX = (float)(x + 2) - scrollX;

            // Check if clicking on selection handles
            if (hasSelection() && focused) {
                int selS = getSelectionStart();
                int selE = getSelectionEnd();
                float xStart = textX + (float)font->width(text.substr(0, selS), true);
                float xEnd = textX + (float)font->width(text.substr(0, selE), true);

                if (std::abs((float)x - xStart) <= 8 && std::abs((float)y - (_y + Font::DefaultLineHeight)) <= 10) {
                    isDraggingHandleStart = true;
                    return;
                }
                if (std::abs((float)x - xEnd) <= 8 && std::abs((float)y - (_y + Font::DefaultLineHeight)) <= 10) {
                    isDraggingHandleEnd = true;
                    return;
                }
            }

            setFocus(minecraft);

            // Check double click
            long now = getTimeMs();
            if (now - lastClickTime < 350 && std::abs(x - lastClickX) < 6 && std::abs(y - lastClickY) < 6) {
                int idx = getCharIndexAtX(font, text, x, textX);
                selectWordAt(idx);
                lastClickTime = 0;
                isDraggingMouse = false;
                isPressing = false;
                return;
            }
            lastClickTime = now;
            lastClickX = x;
            lastClickY = y;

            int clickIdx = getCharIndexAtX(font, text, x, textX);
            bool shiftDown = Keyboard::isKeyDown(Keyboard::KEY_LSHIFT) || Keyboard::isKeyDown(Keyboard::KEY_RSHIFT);

            if (shiftDown) {
                cursorPos = clickIdx;
            } else {
                cursorPos = clickIdx;
                selectAnchor = clickIdx;
            }

            isDraggingMouse = true;
            isPressing = true;
            pressStartTime = now;
            pressStartX = x;
            pressStartY = y;
        } else {
            loseFocus(minecraft);
        }
    }
}

void TextBox::charPressed(Minecraft* minecraft, char c) {
    bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);

    if (ctrlDown || (unsigned char)c < 32 || c == 127) {
        return;
    }

    if (focused && c >= 32 && c < 127) {
        insertText(minecraft, std::string(1, c));
    }
}

void TextBox::keyPressed(Minecraft* minecraft, int key) {
    if (!focused) return;

    bool ctrlDown = Keyboard::isKeyDown(Keyboard::KEY_LEFT_CTRL) || Keyboard::isKeyDown(232) || Keyboard::isKeyDown(224) || Keyboard::isKeyDown(228);
    bool shiftDown = Keyboard::isKeyDown(Keyboard::KEY_LSHIFT) || Keyboard::isKeyDown(Keyboard::KEY_RSHIFT);

    if (ctrlDown) {
        if (key == Keyboard::KEY_C || key == 'c' || key == 'C') {
            std::string sel = getSelectedText();
            if (sel.empty()) sel = text;
            if (!sel.empty()) {
                minecraft->platform()->setClipboardText(sel);
            }
            return;
        }
        if (key == Keyboard::KEY_X || key == 'x' || key == 'X') {
            std::string sel = getSelectedText();
            if (sel.empty()) sel = text;
            if (!sel.empty()) {
                minecraft->platform()->setClipboardText(sel);
                if (hasSelection()) {
                    deleteSelection();
                } else {
                    text.clear();
                    cursorPos = selectAnchor = 0;
                }
            }
            return;
        }
        if (key == Keyboard::KEY_V || key == 'v' || key == 'V') {
            std::string clip = minecraft->platform()->getClipboardText();
            if (!clip.empty()) {
                insertText(minecraft, clip);
            }
            return;
        }
        if (key == Keyboard::KEY_A || key == 'a' || key == 'A') {
            selectAll();
            return;
        }
    }

    if (key == Keyboard::KEY_LEFT) {
        if (shiftDown) {
            if (cursorPos > 0) cursorPos--;
        } else {
            if (hasSelection()) {
                cursorPos = getSelectionStart();
            } else if (cursorPos > 0) {
                cursorPos--;
            }
            selectAnchor = cursorPos;
        }
        blink = true;
        blinkTicks = 0;
        return;
    }

    if (key == Keyboard::KEY_RIGHT) {
        if (shiftDown) {
            if (cursorPos < (int)text.length()) cursorPos++;
        } else {
            if (hasSelection()) {
                cursorPos = getSelectionEnd();
            } else if (cursorPos < (int)text.length()) {
                cursorPos++;
            }
            selectAnchor = cursorPos;
        }
        blink = true;
        blinkTicks = 0;
        return;
    }

    if (key == Keyboard::KEY_HOME) {
        cursorPos = 0;
        if (!shiftDown) selectAnchor = 0;
        blink = true;
        blinkTicks = 0;
        return;
    }

    if (key == Keyboard::KEY_END) {
        cursorPos = (int)text.length();
        if (!shiftDown) selectAnchor = cursorPos;
        blink = true;
        blinkTicks = 0;
        return;
    }

    if (key == Keyboard::KEY_BACKSPACE || key == 127) {
        if (hasSelection()) {
            deleteSelection();
        } else if (cursorPos > 0) {
            text.erase(cursorPos - 1, 1);
            cursorPos--;
            selectAnchor = cursorPos;
        }
        blink = true;
        blinkTicks = 0;
        return;
    }

    if (key == Keyboard::KEY_DELETE || key == 255) {
        if (hasSelection()) {
            deleteSelection();
        } else if (cursorPos < (int)text.length()) {
            text.erase(cursorPos, 1);
        }
        blink = true;
        blinkTicks = 0;
        return;
    }
}

void TextBox::tick(Minecraft* minecraft) {
    blinkTicks++;
    if (blinkTicks >= 8) {
        blink = !blink;
        blinkTicks = 0;
    }

    if (focused) {
        Font* font = minecraft->font;
        float fullTextW = (float)font->width(text, true);
        float maxVisibleW = (float)(width - 4);

        float scrollX = 0.0f;
        float cursorPixelOffset = (float)font->width(text.substr(0, cursorPos), true);
        if (cursorPixelOffset > maxVisibleW) {
            scrollX = cursorPixelOffset - maxVisibleW;
        }
        if (fullTextW <= maxVisibleW) {
            scrollX = 0.0f;
        }
        float textX = (float)(x + 2) - scrollX;

        int xm = Mouse::getX() / (int)Gui::GuiScale;
        int ym = Mouse::getY() / (int)Gui::GuiScale;

        if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
            if (isDraggingHandleStart) {
                selectAnchor = getCharIndexAtX(font, text, xm, textX);
            } else if (isDraggingHandleEnd) {
                cursorPos = getCharIndexAtX(font, text, xm, textX);
            } else if (isDraggingMouse) {
                int dragIdx = getCharIndexAtX(font, text, xm, textX);
                cursorPos = dragIdx;

                if (std::abs(xm - pressStartX) > 4 || std::abs(ym - pressStartY) > 4) {
                    isPressing = false;
                }
            }

            if (isPressing) {
                long elapsed = getTimeMs() - pressStartTime;
                if (elapsed >= 500) { // 500ms long press on phone
                    isPressing = false;
                    isDraggingMouse = false;

                    int touchIdx = getCharIndexAtX(font, text, pressStartX, textX);
                    selectWordAt(touchIdx);

                    showPopup = true;
                    static const char* btnLabels[4] = { "Select All", "Copy", "Cut", "Paste" };
                    int totalW = 0;
                    for (int i = 0; i < 4; i++) {
                        totalW += font->width(btnLabels[i]) + 12;
                    }

                    int screenW = (int)(minecraft->width * Gui::InvGuiScale);
                    int screenH = (int)(minecraft->height * Gui::InvGuiScale);

                    popupW = totalW;
                    popupH = 20;

                    popupX = x + (width - popupW) / 2;
                    popupX = Mth::clamp(popupX, 2, screenW - popupW - 2);

                    if (y - popupH - 4 >= 2) {
                        popupY = y - popupH - 4;
                    } else {
                        popupY = y + height + 4;
                    }
                    popupY = Mth::clamp(popupY, 2, screenH - popupH - 2);
                }
            }
        } else {
            isPressing = false;
            isDraggingMouse = false;
            isDraggingHandleStart = false;
            isDraggingHandleEnd = false;
        }
    }
}

void TextBox::render(Minecraft* minecraft, int xm, int ym) {
    if (isChatStyle) {
        fill(x, y, x + width, y + height, 0x80000000); // 50% transparent black bar with no border
    } else {
        uint32_t bgColor = focused ? 0xffa0a0a0 : 0xffa0a0a0;
        uint32_t borderColor = focused ? 0xff000000 : 0xff000000;
        fill(x, y, x + width, y + height, bgColor);
        fill(x + 1, y + 1, x + width - 1, y + height - 1, borderColor);
    }

    int _y = y + (height - Font::DefaultLineHeight) / 2;

    if (text.empty() && !focused) {
        drawString(minecraft->font, hint, x + 2, _y, 0xff5e5e5e);
        return;
    }

    Font* font = minecraft->font;
    float fullTextW = (float)font->width(text, true);
    float maxVisibleW = (float)(width - 4);

    if (cursorPos < 0) cursorPos = 0;
    if (cursorPos > (int)text.length()) cursorPos = (int)text.length();
    if (selectAnchor < 0) selectAnchor = 0;
    if (selectAnchor > (int)text.length()) selectAnchor = (int)text.length();

    float cursorPixelOffset = (float)font->width(text.substr(0, cursorPos), true);
    float scrollX = 0.0f;
    if (cursorPixelOffset > maxVisibleW) {
        scrollX = cursorPixelOffset - maxVisibleW;
    }
    if (fullTextW <= maxVisibleW) {
        scrollX = 0.0f;
    }

    float textX = (float)(x + 2) - scrollX;

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

    // Draw translucent blue selection highlight background
    if (hasSelection()) {
        int selS = getSelectionStart();
        int selE = getSelectionEnd();
        float xStart = textX + (float)font->width(text.substr(0, selS), true);
        float xEnd = textX + (float)font->width(text.substr(0, selE), true);

        fill((int)xStart, _y - 1, (int)xEnd, _y + Font::DefaultLineHeight + 1, 0x802a62ac);
    }

    // Draw text
    drawString(font, text, (int)textX, _y, 0xffffffff, true);

    // Draw cursor
    if (focused && blink) {
        float cursorX = textX + (float)font->width(text.substr(0, cursorPos), true);
        if (isChatStyle) {
            drawString(font, "_", (int)cursorX, _y, 0xffffffff, true);
        } else {
            fill((int)cursorX, _y - 1, (int)cursorX + 1, _y + Font::DefaultLineHeight + 1, 0xffffffff);
        }
    }

    // Draw selection handles (for phone/touch interface)
    if (hasSelection() && focused) {
        int selS = getSelectionStart();
        int selE = getSelectionEnd();
        float xStart = textX + (float)font->width(text.substr(0, selS), true);
        float xEnd = textX + (float)font->width(text.substr(0, selE), true);

        int handleY = _y + Font::DefaultLineHeight;

        // Start handle
        fill((int)xStart - 3, handleY, (int)xStart + 3, handleY + 6, 0xff2a62ac);
        fill((int)xStart - 2, handleY + 1, (int)xStart + 2, handleY + 5, 0xffffffff);

        // End handle
        fill((int)xEnd - 3, handleY, (int)xEnd + 3, handleY + 6, 0xff2a62ac);
        fill((int)xEnd - 2, handleY + 1, (int)xEnd + 2, handleY + 5, 0xffffffff);
    }

    glDisable(GL_SCISSOR_TEST);

    if (showPopup) {
        static const char* btnLabels[4] = { "Select All", "Copy", "Cut", "Paste" };
        int btnW[4];
        int totalW = 0;
        for (int i = 0; i < 4; i++) {
            btnW[i] = font->width(btnLabels[i]) + 12;
            totalW += btnW[i];
        }
        popupW = totalW;

        fill(popupX - 1, popupY - 1, popupX + popupW + 1, popupY + popupH + 1, 0xff555555);
        fill(popupX, popupY, popupX + popupW, popupY + popupH, 0xeb101010);

        int curX = popupX;
        for (int i = 0; i < 4; i++) {
            int bx0 = curX;
            int bx1 = bx0 + btnW[i];

            bool isHover = (xm >= bx0 && xm < bx1 && ym >= popupY && ym < popupY + popupH);
            if (isHover) {
                fill(bx0, popupY, bx1, popupY + popupH, 0x50ffffff);
            }

            if (i > 0) {
                fill(bx0, popupY + 2, bx0 + 1, popupY + popupH - 2, 0xff555555);
            }

            int labelX = bx0 + (btnW[i] - font->width(btnLabels[i])) / 2;
            int labelY = popupY + (popupH - Font::DefaultLineHeight) / 2;
            drawString(font, btnLabels[i], labelX, labelY, isHover ? 0xffffff55 : 0xffffffff);

            curX += btnW[i];
        }
    }
}
