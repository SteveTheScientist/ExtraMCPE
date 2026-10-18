#include "ChatSettingsScreen.h"
#include "OptionsScreen.h"
#include "CustomizationsScreen.h"
#include "../../Minecraft.h"
#include "../../../util/Mth.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <algorithm>

ChatSettingsScreen::ChatSettingsScreen()
    : btnDone(NULL),
      btnReset(NULL),
      m_scrollY(0.0f),
      m_maxScroll(0.0f),
      m_isDraggingScrollbar(false)
{
}

ChatSettingsScreen::~ChatSettingsScreen() {
    delete btnDone;
    delete btnReset;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        delete m_controls[i];
    }
    m_controls.clear();
}

void ChatSettingsScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    m_controls.clear();
    int idSeq = 3000;

    m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_CHAT_SHOWN, "Chat"));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_TEXT_OPACITY, "Text Opacity", false, 10.0f, 100.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_BG_OPACITY, "Background Opacity", false, 0.0f, 100.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_TEXT_SIZE, "Text Size", false, 50.0f, 100.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_LINE_SPACING, "Line Spacing", false, 0.0f, 100.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_WIDTH, "Width", false, 40.0f, 320.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_FOCUSED_HEIGHT, "Focused Height", false, 20.0f, 180.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_UNFOCUSED_HEIGHT, "Unfocused Height", false, 20.0f, 90.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_CHAT_MESSAGE_TIME, "Message Time"));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_POS_X, "Chat Position X", false, 0.0f, 320.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_CHAT_POS_Y, "Chat Position Y", false, 0.0f, 240.0f));

    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->updateLabel(minecraft);
        buttons.push_back(m_controls[i]);
    }

    btnReset = new Button(2000, 0, 0, 310, 20, "Reset Chat Settings");
    buttons.push_back(btnReset);
}

void ChatSettingsScreen::updateScrollBounds() {
    int totalRows = (int)((m_controls.size() + 1) / 2) + 1; // +1 for full-width reset button
    float contentH = (float)(totalRows * 24 + 10);
    float viewportH = (float)(height - 68);
    m_maxScroll = std::max(0.0f, contentH - viewportH);
    m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void ChatSettingsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
    }

    // Calculate dynamic max range for Chat Position X and Y
    float screenW = (float)width;
    float screenHF = (float)height;

    float chatW = minecraft->options.getProgressValue(OPTIONS_CHAT_WIDTH);
    chatW = Mth::clamp(chatW, 40.0f, 320.0f);
    if (chatW > screenW - 4.0f) chatW = screenW - 4.0f;

    float focusedH = minecraft->options.getProgressValue(OPTIONS_CHAT_FOCUSED_HEIGHT);
    focusedH = Mth::clamp(focusedH, 20.0f, 180.0f);
    if (focusedH > screenHF) focusedH = screenHF;

    float maxPosX = std::max(0.0f, screenW - chatW);
    float maxPosY = std::max(0.0f, screenHF - focusedH);

    // Re-clamp options if Width/Height or screen scale changed
    float curX = minecraft->options.getProgressValue(OPTIONS_CHAT_POS_X);
    if (curX > maxPosX) {
        minecraft->options.set(OPTIONS_CHAT_POS_X, maxPosX);
    }
    float curY = minecraft->options.getProgressValue(OPTIONS_CHAT_POS_Y);
    if (curY > maxPosY) {
        minecraft->options.set(OPTIONS_CHAT_POS_Y, maxPosY);
    }

    for (size_t i = 0; i < m_controls.size(); ++i) {
        OptionControl* ctrl = m_controls[i];
        if (ctrl->optionId == OPTIONS_CHAT_POS_X) {
            ctrl->maxVal = maxPosX;
        } else if (ctrl->optionId == OPTIONS_CHAT_POS_Y) {
            ctrl->maxVal = maxPosY;
        }
    }

    updateScrollBounds();

    int centerX = width / 2;
    int topY = 36;

    for (size_t i = 0; i < m_controls.size(); ++i) {
        OptionControl* ctrl = m_controls[i];
        int r = (int)(i / 2);
        int c = (int)(i % 2);

        int itemX = (c == 0) ? centerX - 155 : centerX + 5;
        int itemW = 150;
        int itemY = (int)(topY + 4 + r * 24 - m_scrollY);

        ctrl->x = itemX;
        ctrl->y = itemY;
        ctrl->width = itemW;
        ctrl->height = 20;

        ctrl->visible = (itemY + 20 >= 34 && itemY < height - 34);
        ctrl->updateLabel(minecraft);
    }

    if (btnReset) {
        int resetRow = (int)((m_controls.size() + 1) / 2);
        int itemY = (int)(topY + 4 + resetRow * 24 - m_scrollY);
        btnReset->x = centerX - 155;
        btnReset->y = itemY;
        btnReset->width = 310;
        btnReset->height = 20;
        btnReset->visible = (itemY + 20 >= 34 && itemY < height - 34);
    }
}

void ChatSettingsScreen::tick() {
    Screen::tick();

    int xm = Mouse::getX();
    int ym = Mouse::getY();
    toGUICoordinate(xm, ym);

    for (size_t i = 0; i < m_controls.size(); ++i) {
        OptionControl* ctrl = m_controls[i];
        if (ctrl->isDraggingSlider) {
            if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
                ctrl->updateSliderValue(minecraft, xm);
            } else {
                ctrl->isDraggingSlider = false;
            }
        }
    }

    if (m_isDraggingScrollbar) {
        if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
            float viewportH = (float)(height - 68);
            if (viewportH > 0 && m_maxScroll > 0) {
                float relY = (float)(ym - 34) / (viewportH - 15);
                m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
                setupPositions();
            }
        } else {
            m_isDraggingScrollbar = false;
        }
    }
}

void ChatSettingsScreen::render(int xm, int ym, float a) {
    if (minecraft->isLevelGenerated()) {
        renderBackground();
    } else {
        renderDirtBackground(0);
    }

    Font* font = minecraft->font;
    Tesselator& t = Tesselator::instance;
    int centerX = width / 2;
    int topY = 34;
    int bottomY = height - 34;

    glEnable(GL_SCISSOR_TEST);
    int scissorX = (int)((centerX - 165) * Gui::GuiScale);
    int scissorW = (int)(330 * Gui::GuiScale);
    int physY = minecraft->height - (int)((height - 34) * Gui::GuiScale);
    int scissorH = (int)((height - 68) * Gui::GuiScale);
    glScissor(scissorX < 0 ? 0 : scissorX, physY < 0 ? 0 : physY, scissorW < 0 ? 0 : scissorW, scissorH < 0 ? 0 : scissorH);

    Screen::render(xm, ym, a);

    glDisable(GL_SCISSOR_TEST);

    fill(0, 0, width, 34, 0x70000000);
    fill(0, height - 34, width, height, 0x70000000);

    if (m_maxScroll > 0) {
        int trackX = centerX + 160;
        int trackW = 6;
        int trackY = topY + 2;
        int trackH = bottomY - topY - 4;

        minecraft->textures->loadAndBindTexture("gui/Widget_scroller_background.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.vertexUV((float)trackX, (float)(trackY + trackH), blitOffset, 0, 1);
        t.vertexUV((float)(trackX + trackW), (float)(trackY + trackH), blitOffset, 1, 1);
        t.vertexUV((float)(trackX + trackW), (float)trackY, blitOffset, 1, 0);
        t.vertexUV((float)trackX, (float)trackY, blitOffset, 0, 0);
        t.draw();

        int thumbW = 12;
        int thumbH = 15;
        int thumbX = centerX + 157;
        int thumbY = trackY + (int)((trackH - thumbH) * (m_scrollY / m_maxScroll));

        minecraft->textures->loadAndBindTexture("gui/Scroller.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.vertexUV((float)thumbX, (float)(thumbY + thumbH), blitOffset, 0, 1);
        t.vertexUV((float)(thumbX + thumbW), (float)(thumbY + thumbH), blitOffset, 1, 1);
        t.vertexUV((float)(thumbX + thumbW), (float)thumbY, blitOffset, 1, 0);
        t.vertexUV((float)thumbX, (float)thumbY, blitOffset, 0, 0);
        t.draw();
    }

    if (btnDone) btnDone->render(minecraft, xm, ym);

    drawCenteredString(font, "Chat Settings", width / 2, 10, 0xffffffff);
}

void ChatSettingsScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        minecraft->options.save();
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }

    if (button == btnReset) {
        minecraft->options.set(OPTIONS_CHAT_SHOWN, true);
        minecraft->options.set(OPTIONS_CHAT_TEXT_OPACITY, 100.0f);
        minecraft->options.set(OPTIONS_CHAT_BG_OPACITY, 50.0f);
        minecraft->options.set(OPTIONS_CHAT_TEXT_SIZE, 100.0f);
        minecraft->options.set(OPTIONS_CHAT_LINE_SPACING, 0.0f);
        minecraft->options.set(OPTIONS_CHAT_WIDTH, 320.0f);
        minecraft->options.set(OPTIONS_CHAT_FOCUSED_HEIGHT, 180.0f);
        minecraft->options.set(OPTIONS_CHAT_UNFOCUSED_HEIGHT, 90.0f);
        minecraft->options.set(OPTIONS_CHAT_MESSAGE_TIME, 1); // 10s
        minecraft->options.set(OPTIONS_CHAT_POS_X, 2.0f);
        minecraft->options.set(OPTIONS_CHAT_POS_Y, 40.0f);
        minecraft->options.save();

        for (size_t i = 0; i < m_controls.size(); ++i) {
            m_controls[i]->updateLabel(minecraft);
        }
        setupPositions();
        return;
    }

    OptionControl* ctrl = dynamic_cast<OptionControl*>(button);
    if (ctrl) {
        if (ctrl->controlType == CONTROL_TOGGLE) {
            ctrl->toggleValue(minecraft);
            minecraft->options.save();
            setupPositions();
        }
    }
}

void ChatSettingsScreen::mouseClicked(int x, int y, int buttonNum) {
    // Requirement 2: Header/footer buttons hit-tested first
    if (btnDone && btnDone->clicked(minecraft, x, y)) {
        btnDone->setPressed();
        clickedButton = btnDone;
        return;
    }

    if (buttonNum == MouseAction::ACTION_LEFT) {
        int centerX = width / 2;
        int topY = 34;
        int bottomY = height - 34;

        // Requirement 1 & 3: List area touch/scroll only between header and footer
        if (y >= topY && y < bottomY) {
            if (m_maxScroll > 0 && x >= centerX + 155 && x <= centerX + 172) {
                m_isDraggingScrollbar = true;
                float viewportH = (float)(height - 68);
                float relY = (float)(y - 34) / (viewportH - 15);
                m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
                setupPositions();
                return;
            }
            Screen::mouseClicked(x, y, buttonNum);
        }
    }
}

void ChatSettingsScreen::mouseReleased(int x, int y, int buttonNum) {
    m_isDraggingScrollbar = false;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->isDraggingSlider = false;
    }
    Screen::mouseReleased(x, y, buttonNum);
}

void ChatSettingsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy != 0 && m_maxScroll > 0) {
        m_scrollY -= (float)dy * 12.0f;
        m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
        setupPositions();
    }
}

void ChatSettingsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->options.save();
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }
    Screen::keyPressed(eventKey);
}
