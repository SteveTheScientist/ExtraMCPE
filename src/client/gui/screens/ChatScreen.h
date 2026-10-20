#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatScreen_H__

#include "../Screen.h"
#include "../components/TextBox.h"
#include <string>

class ChatScreen: public Screen
{
public:
	ChatScreen();
	virtual ~ChatScreen();

	virtual void init();
	virtual void tick();
	virtual bool handleBackEvent(bool isDown);
	virtual void render(int xm, int ym, float a);
	virtual void buttonClicked(Button* button) {}
	virtual void keyPressed(int key);
	virtual void charPressed(char c);
	virtual void mouseClicked(int x, int y, int buttonNum);
	virtual void mouseReleased(int x, int y, int buttonNum);
	virtual void mouseWheel(int dx, int dy, int xm, int ym);

	virtual bool renderGameBehind() const { return true; }

private:
	void execute();

	TextBox m_inputBox;
	float m_scrollY;
	float m_maxScroll;
	bool m_isDragging;
	int m_dragStartY;
	int m_startTouchY;
	float m_dragAccumulatorY;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__ChatScreen_H__*/
