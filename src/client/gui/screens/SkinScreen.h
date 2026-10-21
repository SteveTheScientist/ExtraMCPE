#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__SkinScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__SkinScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../components/TextBox.h"
#include "OptionsScreen.h"
#include <vector>
#include <string>

class SkinScreen : public Screen {
    typedef Screen super;
public:
    SkinScreen();
    virtual ~SkinScreen();

    virtual void init() override;
    virtual void setupPositions() override;
    virtual void tick() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual void mouseReleased(int x, int y, int buttonNum) override;
    virtual void mouseWheel(int dx, int dy, int xm, int ym) override;
    virtual void keyPressed(int eventKey) override;

    void setStatus(const std::string& msg, bool isError = false);
    void handleDroppedFile(const std::string& path);

    static SkinScreen* currentSkinScreen;

private:
    void updateScrollBounds();

    Button* btnDone;
    Button* btnImportSkin;
    Button* btnImportCape;

    Button* btnSkinOrCapeSkin;
    Button* btnSkinOrCapeCape;
    Button* btnSkinOrCapeCancel;

    Button* btnConfirmImport;
    Button* btnCancelImport;

    TextBox* txtPathInput;

    std::vector<OptionControl*> m_controls;
    float m_scrollY;
    float m_maxScroll;
    bool m_isDraggingScrollbar;

    bool m_showPathInput;
    bool m_importingIsSkin;

    bool m_showDropDialog;
    std::string m_droppedFilePath;

    std::string m_statusMessage;
    bool m_statusIsError;
    int m_statusTimer;
};

#endif /* NET_MINECRAFT_CLIENT_GUI_SCREENS__SkinScreen_H__ */
