#include "FogScreen.h"
#include "CustomizationsScreen.h"
#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../../util/Mth.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <algorithm>

FogScreen::FogScreen()
    : btnDone(NULL),
      btnReset(NULL),
      m_scrollY(0.0f),
      m_maxScroll(0.0f),
      m_isDraggingScrollbar(false)
{
}

FogScreen::~FogScreen() {
    delete btnDone;
    delete btnReset;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        delete m_controls[i];
    }
    m_controls.clear();
}

void FogScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    m_controls.clear();
    int idSeq = 400;

    m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_ENHANCED_FOG_ENABLED, "Enhanced Fog"));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_FOG_HORIZON_SYNC, "Horizon Sky Sync"));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_FOG_DISTANCE_SCALE, "Distance Scale", false, 0.5f, 2.0f));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_FOG_SUNSET_BIAS, "Sunset Chromatic Bias", false, 0.0f, 1.0f));

    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->updateLabel(minecraft);
        buttons.push_back(m_controls[i]);
    }

    btnReset = new Button(499, 0, 0, 150, 20, "Reset Fog Defaults");
    buttons.push_back(btnReset);
}

void FogScreen::updateScrollBounds() {
    int totalItems = (int)m_controls.size() + 1; // +1 for Reset button
    int totalRows = (totalItems + 1) / 2;
    float contentH = (float)(totalRows * 24 + 10);
    float viewportH = (float)(height - 68);
    m_maxScroll = std::max(0.0f, contentH - viewportH);
    m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void FogScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
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
        size_t resetIdx = m_controls.size();
        int r = (int)(resetIdx / 2);
        int c = (int)(resetIdx % 2);

        int itemX = (c == 0) ? centerX - 155 : centerX + 5;
        int itemY = (int)(topY + 4 + r * 24 - m_scrollY);

        btnReset->x = itemX;
        btnReset->y = itemY;
        btnReset->width = 150;
        btnReset->height = 20;
        btnReset->visible = (itemY + 20 >= 34 && itemY < height - 34);
    }
}

void FogScreen::tick() {
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

void FogScreen::render(int xm, int ym, float a) {
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

    drawCenteredString(font, "Atmospheric Fog Customization", width / 2, 10, 0xffffffff);
}

void FogScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        minecraft->options.save();
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }

    if (button == btnReset) {
        minecraft->options.set(OPTIONS_ENHANCED_FOG_ENABLED, true);
        minecraft->options.set(OPTIONS_FOG_DISTANCE_SCALE, 1.0f);
        minecraft->options.set(OPTIONS_FOG_SUNSET_BIAS, 0.5f);
        minecraft->options.set(OPTIONS_FOG_HORIZON_SYNC, true);
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

void FogScreen::mouseClicked(int x, int y, int buttonNum) {
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

void FogScreen::mouseReleased(int x, int y, int buttonNum) {
    m_isDraggingScrollbar = false;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->isDraggingSlider = false;
    }
    super::mouseReleased(x, y, buttonNum);
}

void FogScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy != 0 && m_maxScroll > 0) {
        m_scrollY -= (float)dy * 12.0f;
        m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
        setupPositions();
    }
}

void FogScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->options.save();
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }
    super::keyPressed(eventKey);
}
