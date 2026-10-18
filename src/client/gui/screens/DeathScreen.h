#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__DeathScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__DeathScreen_H__

#include "../Screen.h"

class Button;

class DeathScreen: public Screen
{
public:
	DeathScreen();
	virtual ~DeathScreen();

	virtual void init();
	virtual void setupPositions();
	virtual void tick();
	virtual void render(int xm, int ym, float a);
	virtual void buttonClicked(Button* button);

	virtual bool renderGameBehind() const { return true; }

private:
	Button* bRespawn;
	Button* bTitle;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__DeathScreen_H__*/
