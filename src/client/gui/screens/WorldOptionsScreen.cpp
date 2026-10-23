#include "WorldOptionsScreen.h"
#include "../../Minecraft.h"
#include "../../player/LocalPlayer.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../../util/Mth.h"
#include "../../../network/RakNetInstance.h"
#include "../../../world/level/Level.h"
#include "../../../world/entity/player/Player.h"
#include "../../../AppPlatform.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <algorithm>

WorldOptionsScreen::WorldOptionsScreen()
    : btnDone(NULL),
      btnAllowCommands(NULL),
      ctrlDifficulty(NULL),
      ctrlItemDropDelay(NULL),
      btnResetItemDropDelay(NULL),
      m_scrollY(0.0f),
      m_maxScroll(0.0f),
      m_isDraggingScrollbar(false)
{
}

WorldOptionsScreen::~WorldOptionsScreen() {
    delete btnDone;
    delete btnAllowCommands;
    delete ctrlDifficulty;
    delete ctrlItemDropDelay;
    delete btnResetItemDropDelay;
}

bool WorldOptionsScreen::canEditSettings() {
    if (!minecraft || !minecraft->isLevelGenerated() || minecraft->level == NULL) {
        return false;
    }

    if (minecraft->raknetInstance && !minecraft->raknetInstance->isServer()) {
        if (minecraft->player != NULL) {
            std::string pName = minecraft->player->name;
            std::transform(pName.begin(), pName.end(), pName.begin(), ::tolower);
            if (minecraft->level->ops.find(pName) == minecraft->level->ops.end()) {
                return false;
            }
        } else {
            return false;
        }
    }

    return true;
}

void WorldOptionsScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    bool allowCmds = true;
    if (minecraft && minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
        allowCmds = minecraft->level->getLevelData()->getAllowCheats();
    }
    std::string cmdText = allowCmds ? "Allow Commands: ON" : "Allow Commands: OFF";

    btnAllowCommands = new Button(201, 0, 0, 150, 20, cmdText);
    ctrlDifficulty = new OptionControl(202, CONTROL_TOGGLE, OPTIONS_DIFFICULTY, "Difficulty");

    buttons.push_back(btnAllowCommands);
    buttons.push_back(ctrlDifficulty);

    ctrlDifficulty->updateLabel(minecraft);

    if (minecraft && minecraft->platform() && minecraft->platform()->supportsTouchscreen()) {
        ctrlItemDropDelay = new OptionControl(203, CONTROL_SLIDER, OPTIONS_ITEM_DROP_DELAY, "Item Drop Delay", false, 100.0f, 1500.0f);
        btnResetItemDropDelay = new Button(204, 0, 0, 150, 20, "Reset Drop Delay");
        buttons.push_back(ctrlItemDropDelay);
        buttons.push_back(btnResetItemDropDelay);
        ctrlItemDropDelay->updateLabel(minecraft);
    }

    bool editable = canEditSettings();
    btnAllowCommands->active = editable;
    ctrlDifficulty->active = editable;
}

void WorldOptionsScreen::updateScrollBounds() {
    int totalRows = 1;
    if (ctrlItemDropDelay != NULL) {
        totalRows = 2;
    }
    float contentH = (float)(totalRows * 24 + 10);
    float viewportH = (float)(height - 68);
    m_maxScroll = std::max(0.0f, contentH - viewportH);
    m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void WorldOptionsScreen::setupPositions() {
    if (btnDone) {
        btnDone->width = 200;
        btnDone->height = 20;
        btnDone->x = (width - 200) / 2;
        btnDone->y = height - 28;
    }

    updateScrollBounds();

    int centerX = width / 2;
    int topY = 36;
    int itemY = (int)(topY + 4 - m_scrollY);

    if (btnAllowCommands) {
        btnAllowCommands->x = centerX - 155;
        btnAllowCommands->y = itemY;
        btnAllowCommands->width = 150;
        btnAllowCommands->height = 20;
        btnAllowCommands->visible = (itemY + 20 >= 34 && itemY < height - 34);
    }

    if (ctrlDifficulty) {
        ctrlDifficulty->x = centerX + 5;
        ctrlDifficulty->y = itemY;
        ctrlDifficulty->width = 150;
        ctrlDifficulty->height = 20;
        ctrlDifficulty->visible = (itemY + 20 >= 34 && itemY < height - 34);
        ctrlDifficulty->updateLabel(minecraft);
    }

    if (ctrlItemDropDelay && btnResetItemDropDelay) {
        int itemY1 = (int)(topY + 4 + 24 - m_scrollY);

        ctrlItemDropDelay->x = centerX - 155;
        ctrlItemDropDelay->y = itemY1;
        ctrlItemDropDelay->width = 150;
        ctrlItemDropDelay->height = 20;
        ctrlItemDropDelay->visible = (itemY1 + 20 >= 34 && itemY1 < height - 34);
        ctrlItemDropDelay->updateLabel(minecraft);

        btnResetItemDropDelay->x = centerX + 5;
        btnResetItemDropDelay->y = itemY1;
        btnResetItemDropDelay->width = 150;
        btnResetItemDropDelay->height = 20;
        btnResetItemDropDelay->visible = (itemY1 + 20 >= 34 && itemY1 < height - 34);
    }

    bool editable = canEditSettings();
    if (btnAllowCommands) btnAllowCommands->active = editable;
    if (ctrlDifficulty) ctrlDifficulty->active = editable;
}

void WorldOptionsScreen::tick() {
    super::tick();

    int xm = Mouse::getX();
    int ym = Mouse::getY();
    toGUICoordinate(xm, ym);

    if (ctrlItemDropDelay && ctrlItemDropDelay->isDraggingSlider) {
        if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
            ctrlItemDropDelay->updateSliderValue(minecraft, xm);
        } else {
            ctrlItemDropDelay->isDraggingSlider = false;
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

void WorldOptionsScreen::render(int xm, int ym, float a) {
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

    super::render(xm, ym, a);

    if (!minecraft->isLevelGenerated() || minecraft->level == NULL) {
        int itemY = (int)(topY + 4 - m_scrollY);
        drawCenteredString(font, "Open a world to change these.", width / 2, itemY + 26, 0xffa0a0a0);
    }

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

    drawCenteredString(font, "World Options", width / 2, 10, 0xffffffff);
}

void WorldOptionsScreen::buttonClicked(Button* button) {
    if (button == btnDone) {
        minecraft->options.save();
        minecraft->setScreen(new OptionsScreen());
        return;
    }

    if (button == btnAllowCommands) {
        if (canEditSettings() && minecraft->level != NULL && minecraft->level->getLevelData() != NULL) {
            bool newAllow = !minecraft->level->getLevelData()->getAllowCheats();
            minecraft->level->getLevelData()->setAllowCheats(newAllow);
            btnAllowCommands->msg = newAllow ? "Allow Commands: ON" : "Allow Commands: OFF";
        }
        return;
    }

    if (button == ctrlDifficulty) {
        if (canEditSettings()) {
            ctrlDifficulty->toggleValue(minecraft);
            minecraft->options.save();
            setupPositions();
        }
        return;
    }

    if (button == btnResetItemDropDelay) {
        minecraft->options.set(OPTIONS_ITEM_DROP_DELAY, 500.0f);
        if (ctrlItemDropDelay) {
            ctrlItemDropDelay->updateLabel(minecraft);
        }
        minecraft->options.save();
        return;
    }
}

void WorldOptionsScreen::mouseClicked(int x, int y, int buttonNum) {
    if (btnDone && btnDone->clicked(minecraft, x, y)) {
        btnDone->setPressed();
        clickedButton = btnDone;
        return;
    }

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
            super::mouseClicked(x, y, buttonNum);
        }
    }
}

void WorldOptionsScreen::mouseReleased(int x, int y, int buttonNum) {
    m_isDraggingScrollbar = false;
    if (ctrlItemDropDelay) {
        ctrlItemDropDelay->isDraggingSlider = false;
    }
    super::mouseReleased(x, y, buttonNum);
}

void WorldOptionsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy != 0 && m_maxScroll > 0) {
        m_scrollY -= (float)dy * 12.0f;
        m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
        setupPositions();
    }
}

void WorldOptionsScreen::keyPressed(int eventKey) {
    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->options.save();
        minecraft->setScreen(new OptionsScreen());
        return;
    }
    super::keyPressed(eventKey);
}
