#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__FogScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__FogScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "OptionsScreen.h"
#include <vector>

class FogScreen : public Screen {
    typedef Screen super;
public:
    FogScreen();
    virtual ~FogScreen();

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

    Button* btnDone;
    Button* btnReset;
    std::vector<OptionControl*> m_controls;
    float m_scrollY;
    float m_maxScroll;
    bool m_isDraggingScrollbar;
};

#endif /* NET_MINECRAFT_CLIENT_GUI_SCREENS__FogScreen_H__ */
