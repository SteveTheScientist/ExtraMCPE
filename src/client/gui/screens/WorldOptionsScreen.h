#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__WorldOptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__WorldOptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "OptionsScreen.h"
#include <vector>

class WorldOptionsScreen : public Screen {
    typedef Screen super;
public:
    WorldOptionsScreen();
    virtual ~WorldOptionsScreen();

    virtual void init() override;
    virtual void setupPositions() override;
    virtual void tick() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;
    virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual void mouseReleased(int x, int y, int buttonNum) override;
    virtual void mouseWheel(int dx, int dy, int xm, int ym) override;
    virtual void keyPressed(int eventKey) override;

private:
    void updateScrollBounds();
    bool canEditSettings();

    Button* btnDone;
    Button* btnAllowCommands;
    OptionControl* ctrlDifficulty;
    OptionControl* ctrlItemDropDelay;
    Button* btnResetItemDropDelay;

    float m_scrollY;
    float m_maxScroll;
    bool m_isDraggingScrollbar;
};

#endif /* NET_MINECRAFT_CLIENT_GUI_SCREENS__WorldOptionsScreen_H__ */
