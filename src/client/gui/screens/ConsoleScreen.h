#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__

#include "../Screen.h"
#include "../components/TextBox.h"
#include <string>

class ConsoleScreen: public Screen
{
    typedef Screen super;
public:
    ConsoleScreen();

    virtual void init();
    virtual void tick();
    virtual bool handleBackEvent(bool isDown);
    virtual void render(int xm, int ym, float a);
    virtual void keyPressed(int eventKey);
    virtual void charPressed(char inputChar);
    virtual void mouseClicked(int x, int y, int buttonNum);
    virtual void mouseReleased(int x, int y, int buttonNum);

    virtual bool renderGameBehind() const { return true; }

private:
    void execute();

    TextBox _inputBox;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ConsoleScreen_H__*/
