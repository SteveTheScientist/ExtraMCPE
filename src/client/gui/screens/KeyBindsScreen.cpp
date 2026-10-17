#include "KeyBindsScreen.h"
#include "OptionsScreen.h"
#include "../../Minecraft.h"
#include "../../sound/SoundEngine.h"
#include "../../../util/Mth.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <algorithm>

KeyBindsScreen::KeyBindsScreen()
    : btnDone(NULL),
      btnResetAll(NULL),
      m_listeningIndex(-1),
      m_scrollY(0.0f),
      m_maxScroll(0.0f),
      m_isDraggingScrollbar(false)
{
}

KeyBindsScreen::~KeyBindsScreen() {
    delete btnDone;
    delete btnResetAll;
    for (size_t i = 0; i < m_entries.size(); ++i) {
        delete m_entries[i].keyBtn;
        delete m_entries[i].resetBtn;
    }
    m_entries.clear();
}

void KeyBindsScreen::init() {
    btnResetAll = new Button(200, 0, 0, 150, 20, "Reset Keys");
    btnDone = new Button(201, 0, 0, 150, 20, "Done");

    buttons.push_back(btnResetAll);
    buttons.push_back(btnDone);
    tabButtons.push_back(btnResetAll);
    tabButtons.push_back(btnDone);

    m_entries.clear();
    int btnId = 300;

    // Movement Category
    KeyBindEntry headerMov;
    headerMov.category = "Movement";
    headerMov.keyBtn = NULL;
    headerMov.resetBtn = NULL;
    m_entries.push_back(headerMov);

    auto addKey = [&](const std::string& label, OptionId optId, int defKey) {
        KeyBindEntry entry;
        entry.category = "";
        entry.label = label;
        entry.optionId = optId;
        entry.defaultKey = defKey;
        entry.keyBtn = new Button(btnId++, 0, 0, 95, 20, "");
        entry.resetBtn = new Button(btnId++, 0, 0, 48, 20, "Reset");
        m_entries.push_back(entry);

        buttons.push_back(entry.keyBtn);
        buttons.push_back(entry.resetBtn);
    };

    addKey("Jump", OPTIONS_KEY_JUMP, Keyboard::KEY_SPACE);
    addKey("Sneak", OPTIONS_KEY_SNEAK, Keyboard::KEY_LSHIFT);
    addKey("Strafe Left", OPTIONS_KEY_LEFT, Keyboard::KEY_A);
    addKey("Strafe Right", OPTIONS_KEY_RIGHT, Keyboard::KEY_D);
    addKey("Walk Backward", OPTIONS_KEY_BACK, Keyboard::KEY_S);
    addKey("Walk Forward", OPTIONS_KEY_FORWARD, Keyboard::KEY_W);

    // Miscellaneous Category
    KeyBindEntry headerMisc;
    headerMisc.category = "Miscellaneous";
    headerMisc.keyBtn = NULL;
    headerMisc.resetBtn = NULL;
    m_entries.push_back(headerMisc);

    addKey("Inventory", OPTIONS_KEY_INVENTORY, Keyboard::KEY_E);
    addKey("Drop Item", OPTIONS_KEY_DROP, Keyboard::KEY_Q);
    addKey("Chat", OPTIONS_KEY_CHAT, Keyboard::KEY_T);
    addKey("Toggle Fog", OPTIONS_KEY_FOG, Keyboard::KEY_F);
    addKey("Use / Place", OPTIONS_KEY_USE, Keyboard::KEY_U);

    updateKeyLabels();
}

void KeyBindsScreen::updateKeyLabels() {
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].keyBtn == NULL) continue;

        if ((int)i == m_listeningIndex) {
            m_entries[i].keyBtn->msg = "> ??? <";
        } else {
            int keyVal = minecraft->options.getIntValue(m_entries[i].optionId);
            const char* keyName = Keyboard::getKeyName(keyVal);
            m_entries[i].keyBtn->msg = keyName ? keyName : "NONE";
        }

        int currentKey = minecraft->options.getIntValue(m_entries[i].optionId);
        m_entries[i].resetBtn->active = (currentKey != m_entries[i].defaultKey);
    }
}

void KeyBindsScreen::resetAllKeys() {
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].keyBtn == NULL) continue;
        minecraft->options.set(m_entries[i].optionId, m_entries[i].defaultKey);
    }
    minecraft->options.save();
    updateKeyLabels();
}

void KeyBindsScreen::updateScrollBounds() {
    int totalHeight = 10;
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (!m_entries[i].category.empty()) {
            totalHeight += 16;
        } else {
            totalHeight += 24;
        }
    }
    float viewportH = (float)(height - 68);
    m_maxScroll = std::max(0.0f, (float)totalHeight - viewportH);
    m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void KeyBindsScreen::setupPositions() {
    int centerX = width / 2;

    if (btnResetAll && btnDone) {
        btnResetAll->width = 145;
        btnResetAll->height = 20;
        btnResetAll->x = centerX - 150;
        btnResetAll->y = height - 28;

        btnDone->width = 145;
        btnDone->height = 20;
        btnDone->x = centerX + 5;
        btnDone->y = height - 28;
    }

    updateScrollBounds();

    int curY = (int)(38 - m_scrollY);

    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (!m_entries[i].category.empty()) {
            curY += 16;
            continue;
        }

        Button* kb = m_entries[i].keyBtn;
        Button* rb = m_entries[i].resetBtn;

        if (kb && rb) {
            kb->x = centerX + 5;
            kb->y = curY;
            kb->width = 95;
            kb->height = 20;

            rb->x = centerX + 105;
            rb->y = curY;
            rb->width = 48;
            rb->height = 20;

            bool inBounds = (curY + 20 >= 34 && curY < height - 34);
            kb->visible = inBounds;
            rb->visible = inBounds;
        }
        curY += 24;
    }
}

void KeyBindsScreen::tick() {
    super::tick();

    int xm = Mouse::getX();
    int ym = Mouse::getY();
    toGUICoordinate(xm, ym);

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

void KeyBindsScreen::render(int xm, int ym, float a) {
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

    // Scissor clipping for list area
    glEnable(GL_SCISSOR_TEST);
    int scissorX = (int)((centerX - 165) * Gui::GuiScale);
    int scissorW = (int)(330 * Gui::GuiScale);
    int physY = minecraft->height - (int)((height - 34) * Gui::GuiScale);
    int scissorH = (int)((height - 68) * Gui::GuiScale);
    glScissor(scissorX < 0 ? 0 : scissorX, physY < 0 ? 0 : physY, scissorW < 0 ? 0 : scissorW, scissorH < 0 ? 0 : scissorH);

    // Render category titles and action labels
    int curY = (int)(38 - m_scrollY);
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (!m_entries[i].category.empty()) {
            if (curY >= topY - 10 && curY < bottomY) {
                drawCenteredString(font, m_entries[i].category, centerX, curY + 3, 0xffffffff);
            }
            curY += 16;
            continue;
        }

        if (curY >= topY - 10 && curY < bottomY) {
            drawString(font, m_entries[i].label, centerX - 150, curY + 6, 0xffffffff);
        }
        curY += 24;
    }

    super::render(xm, ym, a);

    glDisable(GL_SCISSOR_TEST);

    // Header & Footer shadow overlays rendered on top of scrolling list
    fill(0, 0, width, 34, 0x70000000);
    fill(0, height - 34, width, height, 0x70000000);

    // Scrollbar track & thumb
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

    if (btnResetAll) btnResetAll->render(minecraft, xm, ym);
    if (btnDone) btnDone->render(minecraft, xm, ym);

    drawCenteredString(font, "Key Binds", width / 2, 10, 0xffffffff);
}

void KeyBindsScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        minecraft->options.save();
        minecraft->setScreen(new OptionsScreen());
        return;
    }

    if (button == btnResetAll) {
        resetAllKeys();
        return;
    }

    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (m_entries[i].keyBtn == button) {
            m_listeningIndex = (int)i;
            updateKeyLabels();
            return;
        }

        if (m_entries[i].resetBtn == button) {
            minecraft->options.set(m_entries[i].optionId, m_entries[i].defaultKey);
            minecraft->options.save();
            updateKeyLabels();
            return;
        }
    }
}

void KeyBindsScreen::mouseClicked(int x, int y, int buttonNum) {
    if (buttonNum == MouseAction::ACTION_LEFT) {
        int centerX = width / 2;
        int topY = 34;
        int bottomY = height - 34;

        if (y >= topY && y < bottomY) {
            if (m_maxScroll > 0 && x >= centerX + 155 && x <= centerX + 172) {
                m_isDraggingScrollbar = true;
                float viewportH = (float)(height - 68);
                float relY = (float)(y - 34) / (viewportH - 15);
                m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
                setupPositions();
                return;
            }
        }
    }

    super::mouseClicked(x, y, buttonNum);
}

void KeyBindsScreen::mouseReleased(int x, int y, int buttonNum) {
    m_isDraggingScrollbar = false;
    super::mouseReleased(x, y, buttonNum);
}

void KeyBindsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy != 0 && m_maxScroll > 0) {
        m_scrollY -= (float)dy * 12.0f;
        m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
        setupPositions();
    }
}

void KeyBindsScreen::keyPressed(int eventKey) {
    if (m_listeningIndex >= 0 && m_listeningIndex < (int)m_entries.size()) {
        if (eventKey != Keyboard::KEY_ESCAPE) {
            minecraft->options.set(m_entries[m_listeningIndex].optionId, eventKey);
            minecraft->options.save();
        }
        m_listeningIndex = -1;
        updateKeyLabels();
        return;
    }

    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->options.save();
        minecraft->setScreen(new OptionsScreen());
        return;
    }
    super::keyPressed(eventKey);
}
