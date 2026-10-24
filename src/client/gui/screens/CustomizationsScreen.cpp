#include "CustomizationsScreen.h"
#include "CloudsScreen.h"
#include "FogScreen.h"
#include "ChatSettingsScreen.h"
#include "SkinsScreen.h"
#include "StartMenuScreen.h"
#include "PauseScreen.h"
#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../../util/Mth.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <algorithm>

CustomizationsScreen::CustomizationsScreen()
    : btnDone(NULL),
      m_scrollY(0.0f),
      m_maxScroll(0.0f),
      m_isDraggingScrollbar(false)
{
}

CustomizationsScreen::~CustomizationsScreen() {
    delete btnDone;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        delete m_controls[i];
    }
    m_controls.clear();
}

void CustomizationsScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    m_controls.clear();
    int idSeq = 200;

    // Customization Options: sub-menu buttons
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_COUNT, "Skins...", false, 0.0f, 1.0f, ACTION_SKINS));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_COUNT, "Clouds...", false, 0.0f, 1.0f, ACTION_CLOUDS));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_COUNT, "Fog...", false, 0.0f, 1.0f, ACTION_FOG));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_COUNT, "Chat...", false, 0.0f, 1.0f, ACTION_CHAT_SETTINGS));

    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->updateLabel(minecraft);
        buttons.push_back(m_controls[i]);
    }
}

void CustomizationsScreen::updateScrollBounds() {
    int currentRow = 0;
    int col = 0;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        OptionControl* ctrl = m_controls[i];
        if (ctrl->isFullWidth) {
            if (col != 0) {
                currentRow++;
                col = 0;
            }
            currentRow++;
            col = 0;
        } else {
            col++;
            if (col >= 2) {
                col = 0;
                currentRow++;
            }
        }
    }
    if (col != 0) currentRow++;

    float contentH = (float)(currentRow * 24 + 10);
    float viewportH = (float)(height - 68);
    m_maxScroll = std::max(0.0f, contentH - viewportH);
    m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void CustomizationsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
    }

    updateScrollBounds();

    int centerX = width / 2;
    int topY = 36;
    int currentRow = 0;
    int col = 0;

    for (size_t i = 0; i < m_controls.size(); ++i) {
        OptionControl* ctrl = m_controls[i];
        if (ctrl->isFullWidth) {
            if (col != 0) {
                currentRow++;
                col = 0;
            }
            int itemX = centerX - 155;
            int itemW = 310;
            int itemY = (int)(topY + 4 + currentRow * 24 - m_scrollY);

            ctrl->x = itemX;
            ctrl->y = itemY;
            ctrl->width = itemW;
            ctrl->height = 20;

            currentRow++;
            col = 0;
        } else {
            int itemX = (col == 0) ? centerX - 155 : centerX + 5;
            int itemW = 150;
            int itemY = (int)(topY + 4 + currentRow * 24 - m_scrollY);

            ctrl->x = itemX;
            ctrl->y = itemY;
            ctrl->width = itemW;
            ctrl->height = 20;

            col++;
            if (col >= 2) {
                col = 0;
                currentRow++;
            }
        }

        ctrl->visible = (ctrl->y + ctrl->height >= 34 && ctrl->y < height - 34);
        ctrl->updateLabel(minecraft);
    }
}

void CustomizationsScreen::tick() {
    super::tick();

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

void CustomizationsScreen::render(int xm, int ym, float a) {
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

    // Enable scissor clipping for scrollable area
    glEnable(GL_SCISSOR_TEST);
    int scissorX = (int)((centerX - 165) * Gui::GuiScale);
    int scissorW = (int)(330 * Gui::GuiScale);
    int physY = minecraft->height - (int)((height - 34) * Gui::GuiScale);
    int scissorH = (int)((height - 68) * Gui::GuiScale);
    glScissor(scissorX < 0 ? 0 : scissorX, physY < 0 ? 0 : physY, scissorW < 0 ? 0 : scissorW, scissorH < 0 ? 0 : scissorH);

    super::render(xm, ym, a);

    glDisable(GL_SCISSOR_TEST);

    // Top and bottom header/footer translucent bars
    fill(0, 0, width, 34, 0x70000000);
    fill(0, height - 34, width, height, 0x70000000);

    // Render vertical scrollbar track and thumb
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

    drawCenteredString(font, "Customizations", width / 2, 10, 0xffffffff);
}

void CustomizationsScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        minecraft->options.save();
        if (minecraft->isLevelGenerated()) {
            minecraft->setScreen(new PauseScreen(false));
        } else {
            minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        }
        return;
    }

    OptionControl* ctrl = dynamic_cast<OptionControl*>(button);
    if (ctrl) {
        if (ctrl->controlType == CONTROL_TOGGLE) {
            ctrl->toggleValue(minecraft);
            minecraft->options.save();
            setupPositions();
        } else if (ctrl->controlType == CONTROL_ACTION) {
            if (ctrl->actionId == ACTION_SKINS) {
                minecraft->setScreen(new SkinsScreen());
            } else if (ctrl->actionId == ACTION_CLOUDS) {
                minecraft->setScreen(new CloudsScreen());
            } else if (ctrl->actionId == ACTION_FOG) {
                minecraft->setScreen(new FogScreen());
            } else if (ctrl->actionId == ACTION_CHAT_SETTINGS) {
                minecraft->setScreen(new ChatSettingsScreen());
            }
        }
    }
}

void CustomizationsScreen::mouseClicked(int x, int y, int buttonNum) {
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
            super::mouseClicked(x, y, buttonNum);
        }
    }
}

void CustomizationsScreen::mouseReleased(int x, int y, int buttonNum) {
    m_isDraggingScrollbar = false;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->isDraggingSlider = false;
    }
    super::mouseReleased(x, y, buttonNum);
}

void CustomizationsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy != 0 && m_maxScroll > 0) {
        m_scrollY -= (float)dy * 12.0f;
        m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
        setupPositions();
    }
}

void CustomizationsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        if (minecraft->isLevelGenerated()) {
            minecraft->setScreen(new PauseScreen(false));
        } else {
            minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
        }
        return;
    }
    super::keyPressed(eventKey);
}
