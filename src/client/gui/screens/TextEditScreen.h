#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__TextEditScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__TextEditScreen_H__

#include "../Screen.h"
#include "../components/ImageButton.h"
#include "../components/TextBox.h"
#include <string>

class SignTileEntity;
class Button;

class TextEditScreen: public Screen
{
    typedef Screen super;
public:
	TextEditScreen(SignTileEntity* signEntity);
	~TextEditScreen();
    void init();
	void tick();
	bool handleBackEvent(bool isDown);
	void render(int xm, int ym, float a);
	virtual void lostFocus();
	virtual void keyPressed(int eventKey);
	virtual void charPressed(char inputChar);
	virtual void mouseClicked(int x, int y, int buttonNum);
	virtual void mouseReleased(int x, int y, int buttonNum);
	void setupPositions();
	void buttonClicked(Button* button);
protected:
	bool isShowingKeyboard;
	SignTileEntity* sign;
	int frame;
	int line;
private:
	ImageButton btnClose;
	TextBox m_lineBoxes[4];
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__TextEditScreen_H__*/
