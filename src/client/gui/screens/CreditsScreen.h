#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__CreditsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__CreditsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <vector>
#include <string>

enum CreditsReturnTarget {
    CREDITS_RETURN_STARTMENU,
    CREDITS_RETURN_OPTIONS,
    CREDITS_RETURN_PAUSE
};

class CreditsScreen: public Screen {
public:
    typedef Screen super;
    CreditsScreen(CreditsReturnTarget returnTarget = CREDITS_RETURN_STARTMENU);
    virtual ~CreditsScreen();
    virtual void init();
    virtual void setupPositions();
    virtual void tick();
    virtual void render(int xm, int ym, float a);
    virtual void buttonClicked(Button* button);
    virtual void mouseClicked(int x, int y, int buttonNum);
private:
    Button* btnDone;
    CreditsReturnTarget m_returnTarget;
    std::vector<std::string> _lines;
    float _scrollY;
    float _scrollSpeed;
};

#endif /* NET_MINECRAFT_CLIENT_GUI_SCREENS__CreditsScreen_H__ */
