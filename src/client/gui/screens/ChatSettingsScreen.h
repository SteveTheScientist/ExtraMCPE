#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatSettingsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatSettingsScreen_H__

#include "../Screen.h"
#include "OptionsScreen.h"
#include <vector>

class Button;

class ChatSettingsScreen : public Screen {
public:
    typedef Screen super;

    ChatSettingsScreen();
    virtual ~ChatSettingsScreen();

    virtual void init();
    virtual void setupPositions();
    virtual void tick();
    virtual void render(int xm, int ym, float a);

    virtual void buttonClicked(Button* button);
    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);
    virtual void mouseWheel(int dx, int dy, int xm, int ym);
    virtual void keyPressed(int eventKey);

    virtual bool renderGameBehind() const { return true; }

private:
    void updateScrollBounds();

    Button* btnDone;
    Button* btnReset;
    std::vector<OptionControl*> m_controls;

    float m_scrollY;
    float m_maxScroll;
    bool m_isDraggingScrollbar;
};

#endif /* NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatSettingsScreen_H__ */
