#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__CustomizationsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__CustomizationsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"

class CustomizationsScreen : public Screen {
    typedef Screen super;
public:
    CustomizationsScreen();
    virtual ~CustomizationsScreen();

    virtual void init() override;
    virtual void setupPositions() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;
    virtual void keyPressed(int eventKey) override;

private:
    Button* btnDone;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__CustomizationsScreen_H__*/
