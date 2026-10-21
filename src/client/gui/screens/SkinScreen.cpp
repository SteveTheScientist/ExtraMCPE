#include "SkinScreen.h"
#include "CustomizationsScreen.h"
#include "../../Minecraft.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "../../../util/Mth.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include "../../player/LocalPlayer.h"
#include "../../../AppPlatform.h"
#include <algorithm>

SkinScreen* SkinScreen::currentSkinScreen = NULL;

SkinScreen::SkinScreen()
    : btnDone(NULL),
      btnImportSkin(NULL),
      btnImportCape(NULL),
      btnSkinOrCapeSkin(NULL),
      btnSkinOrCapeCape(NULL),
      btnSkinOrCapeCancel(NULL),
      btnConfirmImport(NULL),
      btnCancelImport(NULL),
      txtPathInput(NULL),
      m_scrollY(0.0f),
      m_maxScroll(0.0f),
      m_isDraggingScrollbar(false),
      m_showPathInput(false),
      m_importingIsSkin(true),
      m_showDropDialog(false),
      m_statusMessage(""),
      m_statusIsError(false),
      m_statusTimer(0)
{
    currentSkinScreen = this;
}

SkinScreen::~SkinScreen() {
    if (currentSkinScreen == this) {
        currentSkinScreen = NULL;
    }
    delete btnDone;
    delete btnImportSkin;
    delete btnImportCape;
    delete btnSkinOrCapeSkin;
    delete btnSkinOrCapeCape;
    delete btnSkinOrCapeCancel;
    delete btnConfirmImport;
    delete btnCancelImport;
    delete txtPathInput;

    for (size_t i = 0; i < m_controls.size(); ++i) {
        delete m_controls[i];
    }
    m_controls.clear();
}

void SkinScreen::init() {
    btnDone = new Button(100, 0, 0, 200, 20, "Done");
    buttons.push_back(btnDone);
    tabButtons.push_back(btnDone);

    btnImportSkin = new Button(101, 0, 0, 150, 20, "Import Skin...");
    buttons.push_back(btnImportSkin);

    btnImportCape = new Button(102, 0, 0, 150, 20, "Import Cape...");
    buttons.push_back(btnImportCape);

    m_controls.clear();
    int idSeq = 400;

    m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_SKIN_MODEL, "Skin Model"));
    m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_SKIN_OVERLAY, "Skin Overlay"));

    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->updateLabel(minecraft);
        buttons.push_back(m_controls[i]);
    }

    txtPathInput = new TextBox(103, 0, 0, 200, 20, "Enter PNG path...");
    btnConfirmImport = new Button(104, 0, 0, 95, 20, "Import");
    btnCancelImport = new Button(105, 0, 0, 95, 20, "Cancel");

    btnSkinOrCapeSkin = new Button(106, 0, 0, 95, 20, "Skin");
    btnSkinOrCapeCape = new Button(107, 0, 0, 95, 20, "Cape");
    btnSkinOrCapeCancel = new Button(108, 0, 0, 200, 20, "Cancel");
}

void SkinScreen::setStatus(const std::string& msg, bool isError) {
    m_statusMessage = msg;
    m_statusIsError = isError;
    m_statusTimer = 180; // ~6 seconds at 30 fps
}

void SkinScreen::handleDroppedFile(const std::string& path) {
    m_droppedFilePath = path;
    m_showDropDialog = true;
    m_showPathInput = false;
}

void SkinScreen::updateScrollBounds() {
    int totalItems = (int)m_controls.size() + 2; // +2 for Import Skin / Import Cape
    int totalRows = (totalItems + 1) / 2;
    float contentH = (float)(totalRows * 24 + 10);
    float viewportH = (float)(height - 68);
    m_maxScroll = std::max(0.0f, contentH - viewportH);
    m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void SkinScreen::setupPositions() {
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

    int importStartR = (int)(m_controls.size() / 2);

    if (btnImportSkin) {
        int itemY = (int)(topY + 4 + importStartR * 24 - m_scrollY);
        btnImportSkin->x = centerX - 155;
        btnImportSkin->y = itemY;
        btnImportSkin->width = 150;
        btnImportSkin->height = 20;
        btnImportSkin->visible = (itemY + 20 >= 34 && itemY < height - 34);
    }

    if (btnImportCape) {
        int itemY = (int)(topY + 4 + importStartR * 24 - m_scrollY);
        btnImportCape->x = centerX + 5;
        btnImportCape->y = itemY;
        btnImportCape->width = 150;
        btnImportCape->height = 20;
        btnImportCape->visible = (itemY + 20 >= 34 && itemY < height - 34);
    }

    if (txtPathInput) {
        txtPathInput->x = centerX - 100;
        txtPathInput->y = height / 2 - 15;
        txtPathInput->width = 200;
        txtPathInput->height = 20;
    }

    if (btnConfirmImport && btnCancelImport) {
        btnConfirmImport->x = centerX - 100;
        btnConfirmImport->y = height / 2 + 10;
        btnConfirmImport->width = 95;
        btnConfirmImport->height = 20;

        btnCancelImport->x = centerX + 5;
        btnCancelImport->y = height / 2 + 10;
        btnCancelImport->width = 95;
        btnCancelImport->height = 20;
    }

    if (btnSkinOrCapeSkin && btnSkinOrCapeCape && btnSkinOrCapeCancel) {
        btnSkinOrCapeSkin->x = centerX - 100;
        btnSkinOrCapeSkin->y = height / 2 - 10;
        btnSkinOrCapeSkin->width = 95;
        btnSkinOrCapeSkin->height = 20;

        btnSkinOrCapeCape->x = centerX + 5;
        btnSkinOrCapeCape->y = height / 2 - 10;
        btnSkinOrCapeCape->width = 95;
        btnSkinOrCapeCape->height = 20;

        btnSkinOrCapeCancel->x = centerX - 100;
        btnSkinOrCapeCancel->y = height / 2 + 15;
        btnSkinOrCapeCancel->width = 200;
        btnSkinOrCapeCancel->height = 20;
    }
}

void SkinScreen::tick() {
    super::tick();

    if (m_statusTimer > 0) {
        m_statusTimer--;
    }

    if (m_showPathInput && txtPathInput) {
        txtPathInput->tick();
    }

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

void SkinScreen::render(int xm, int ym, float a) {
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

    drawCenteredString(font, "Skin Customization", width / 2, 10, 0xffffffff);

    // Status message overlay
    if (m_statusTimer > 0 && !m_statusMessage.empty()) {
        int color = m_statusIsError ? 0xffff5555 : 0xff55ff55;
        drawCenteredString(font, m_statusMessage, width / 2, height - 48, color);
    }

    // Modal dialog for path input
    if (m_showPathInput) {
        fill(0, 0, width, height, 0xcc000000);
        drawCenteredString(font, m_importingIsSkin ? "Import Skin PNG Path" : "Import Cape PNG Path", centerX, height / 2 - 35, 0xffffffff);
        if (txtPathInput) txtPathInput->render(minecraft, xm, ym);
        if (btnConfirmImport) btnConfirmImport->render(minecraft, xm, ym);
        if (btnCancelImport) btnCancelImport->render(minecraft, xm, ym);
    }

    // Modal dialog for dropped file
    if (m_showDropDialog) {
        fill(0, 0, width, height, 0xcc000000);
        drawCenteredString(font, "Import Dropped File", centerX, height / 2 - 35, 0xffffffff);
        drawCenteredString(font, "Import as Skin or Cape?", centerX, height / 2 - 22, 0xffaaaaaa);
        if (btnSkinOrCapeSkin) btnSkinOrCapeSkin->render(minecraft, xm, ym);
        if (btnSkinOrCapeCape) btnSkinOrCapeCape->render(minecraft, xm, ym);
        if (btnSkinOrCapeCancel) btnSkinOrCapeCancel->render(minecraft, xm, ym);
    }
}

void SkinScreen::buttonClicked(Button* button) {
    if (m_showDropDialog) {
        if (button == btnSkinOrCapeSkin) {
            m_showDropDialog = false;
            if (minecraft && minecraft->player) {
                std::string err;
                if (minecraft->player->setSkinFromFile(m_droppedFilePath, err)) {
                    setStatus("Skin loaded.");
                } else {
                    setStatus("Invalid file: " + err, true);
                }
            }
            return;
        }
        if (button == btnSkinOrCapeCape) {
            m_showDropDialog = false;
            if (minecraft && minecraft->player) {
                std::string err;
                if (minecraft->player->setCapeFromFile(m_droppedFilePath, err)) {
                    setStatus("Cape loaded.");
                } else {
                    setStatus("Invalid file: " + err, true);
                }
            }
            return;
        }
        if (button == btnSkinOrCapeCancel) {
            m_showDropDialog = false;
            return;
        }
        return;
    }

    if (m_showPathInput) {
        if (button == btnConfirmImport) {
            m_showPathInput = false;
            if (txtPathInput && minecraft && minecraft->player) {
                std::string path = txtPathInput->getText();
                std::string err;
                if (m_importingIsSkin) {
                    if (minecraft->player->setSkinFromFile(path, err)) {
                        setStatus("Skin loaded.");
                    } else {
                        setStatus("Invalid file: " + err, true);
                    }
                } else {
                    if (minecraft->player->setCapeFromFile(path, err)) {
                        setStatus("Cape loaded.");
                    } else {
                        setStatus("Invalid file: " + err, true);
                    }
                }
            }
            return;
        }
        if (button == btnCancelImport) {
            m_showPathInput = false;
            return;
        }
        return;
    }

    if (button == btnDone) {
        minecraft->options.save();
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }

    if (button == btnImportSkin) {
#if defined(__ANDROID__) || defined(ANDROID)
        if (minecraft->platform) {
            minecraft->platform->importSkinOrCape(0);
        }
#else
        m_showPathInput = true;
        m_importingIsSkin = true;
        if (txtPathInput) txtPathInput->setText("");
#endif
        return;
    }

    if (button == btnImportCape) {
#if defined(__ANDROID__) || defined(ANDROID)
        if (minecraft->platform) {
            minecraft->platform->importSkinOrCape(1);
        }
#else
        m_showPathInput = true;
        m_importingIsSkin = false;
        if (txtPathInput) txtPathInput->setText("");
#endif
        return;
    }

    OptionControl* ctrl = dynamic_cast<OptionControl*>(button);
    if (ctrl) {
        if (ctrl->controlType == CONTROL_TOGGLE) {
            if (ctrl->optionId == OPTIONS_SKIN_MODEL) {
                std::string cur = minecraft->options.getStringValue(OPTIONS_SKIN_MODEL);
                if (cur == "Auto") minecraft->options.set(OPTIONS_SKIN_MODEL, "Classic");
                else if (cur == "Classic") minecraft->options.set(OPTIONS_SKIN_MODEL, "Slim");
                else minecraft->options.set(OPTIONS_SKIN_MODEL, "Auto");
                ctrl->updateLabel(minecraft);
            } else {
                ctrl->toggleValue(minecraft);
            }
            minecraft->options.save();
            setupPositions();
        }
    }
}

void SkinScreen::mouseClicked(int x, int y, int buttonNum) {
    if (m_showDropDialog) {
        if (btnSkinOrCapeSkin && btnSkinOrCapeSkin->clicked(minecraft, x, y)) {
            buttonClicked(btnSkinOrCapeSkin);
            return;
        }
        if (btnSkinOrCapeCape && btnSkinOrCapeCape->clicked(minecraft, x, y)) {
            buttonClicked(btnSkinOrCapeCape);
            return;
        }
        if (btnSkinOrCapeCancel && btnSkinOrCapeCancel->clicked(minecraft, x, y)) {
            buttonClicked(btnSkinOrCapeCancel);
            return;
        }
        return;
    }

    if (m_showPathInput) {
        if (txtPathInput) txtPathInput->mouseClicked(x, y, buttonNum);
        if (btnConfirmImport && btnConfirmImport->clicked(minecraft, x, y)) {
            buttonClicked(btnConfirmImport);
            return;
        }
        if (btnCancelImport && btnCancelImport->clicked(minecraft, x, y)) {
            buttonClicked(btnCancelImport);
            return;
        }
        return;
    }

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

void SkinScreen::mouseReleased(int x, int y, int buttonNum) {
    m_isDraggingScrollbar = false;
    for (size_t i = 0; i < m_controls.size(); ++i) {
        m_controls[i]->isDraggingSlider = false;
    }
    super::mouseReleased(x, y, buttonNum);
}

void SkinScreen::mouseWheel(int dx, int dy, int xm, int ym) {
    if (dy != 0 && m_maxScroll > 0) {
        m_scrollY -= (float)dy * 12.0f;
        m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
        setupPositions();
    }
}

void SkinScreen::keyPressed(int eventKey) {
    if (m_showPathInput) {
        if (eventKey == Keyboard::KEY_RETURN) {
            buttonClicked(btnConfirmImport);
            return;
        }
        if (eventKey == Keyboard::KEY_ESCAPE) {
            m_showPathInput = false;
            return;
        }
        if (txtPathInput) txtPathInput->keyPressed(eventKey);
        return;
    }

    if (m_showDropDialog) {
        if (eventKey == Keyboard::KEY_ESCAPE) {
            m_showDropDialog = false;
            return;
        }
        return;
    }

    if (eventKey == Keyboard::KEY_ESCAPE) {
        minecraft->options.save();
        minecraft->setScreen(new CustomizationsScreen());
        return;
    }
    super::keyPressed(eventKey);
}
