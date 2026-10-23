#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__InBedChatScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__InBedChatScreen_H__

#include "ChatScreen.h"

class Button;

class InBedChatScreen : public ChatScreen {
public:
	InBedChatScreen();
	virtual ~InBedChatScreen();

	virtual void init() override;
	virtual void setupPositions() override;
	virtual void render(int xm, int ym, float a) override;
	virtual void buttonClicked(Button* button) override;
	virtual void keyPressed(int key) override;
	virtual bool handleBackEvent(bool isDown) override;
	virtual void mouseClicked(int x, int y, int buttonNum) override;

protected:
	virtual void execute() override;

private:
	Button* btnLeaveBed;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__InBedChatScreen_H__*/
