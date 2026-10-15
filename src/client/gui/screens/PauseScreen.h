#ifndef NET_MINECRAFT_CLIENT_GUI__PauseScreen_H__
#define NET_MINECRAFT_CLIENT_GUI__PauseScreen_H__

#include "../Screen.h"

class Button;

class PauseScreen: public Screen
{
	typedef Screen super;
public:
	PauseScreen(bool wasBackPaused);
	~PauseScreen();

	void init();
	void setupPositions();

	void tick();
	void render(int xm, int ym, float a);
protected:
	void buttonClicked(Button* button);
private:
	int visibleTime;
	bool wasBackPaused;

	Button* bContinue;
	Button* bCustomizations;
	Button* bQuit;
	Button* bOptions;
	Button* bCredits;
};

#endif /*NET_MINECRAFT_CLIENT_GUI__PauseScreen_H__*/
