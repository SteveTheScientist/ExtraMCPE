#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__FontTestScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__FontTestScreen_H__

#include "../Screen.h"
#include "../components/Button.h"

class FontTestScreen : public Screen {
    typedef Screen super;
public:
    FontTestScreen();
    virtual ~FontTestScreen();

    virtual void init() override;
    virtual void setupPositions() override;
    virtual void render(int xm, int ym, float a) override;
    virtual void buttonClicked(Button* button) override;

private:
    Button* btnBack;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__FontTestScreen_H__*/
