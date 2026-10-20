#ifndef NET_MINECRAFT_CLIENT_GUI_COMPONENTS__TextBox_H__
#define NET_MINECRAFT_CLIENT_GUI_COMPONENTS__TextBox_H__

#include <string>
#include "GuiElement.h"
#include "../../Options.h"
#include "../../../platform/input/Mouse.h"
#include "../../../platform/input/Keyboard.h"

class Font;
class Minecraft;

class TextBox: public GuiElement
{
public:
	TextBox(int id, const std::string& msg);
	TextBox(int id, int x, int y, const std::string& msg);
	TextBox(int id, int x, int y, int w, int h, const std::string& msg);

	virtual void mouseClicked(Minecraft* minecraft, int x, int y, int buttonNum);

	virtual void setFocus(Minecraft* minecraft);
	virtual bool loseFocus(Minecraft* minecraft);

	virtual void render(Minecraft* minecraft, int xm, int ym);

	virtual void keyPressed(Minecraft* minecraft, int key);
	virtual void charPressed(Minecraft* minecraft, char c);
	virtual void tick(Minecraft* minecraft);

	// Selection, cursor and text insertion helpers
	bool hasSelection() const { return cursorPos != selectAnchor; }
	int getSelectionStart() const { return cursorPos < selectAnchor ? cursorPos : selectAnchor; }
	int getSelectionEnd() const { return cursorPos > selectAnchor ? cursorPos : selectAnchor; }
	std::string getSelectedText() const;
	bool deleteSelection();
	void selectAll();
	void selectWordAt(int idx);
	void insertText(Minecraft* minecraft, const std::string& inputStr);

	static int getCharIndexAtX(Font* font, const std::string& str, int clickX, float textX);
	static void getWordBounds(const std::string& str, int idx, int& outStart, int& outEnd);

public:
	std::string hint;
	std::string text;
	int id;

	int blinkTicks;

	bool focused;
	bool blink;

	int cursorPos;
	int selectAnchor;

	bool isPressing;
	bool isDraggingMouse;
	bool isDraggingHandleStart;
	bool isDraggingHandleEnd;

	long pressStartTime;
	int pressStartX;
	int pressStartY;

	long lastClickTime;
	int lastClickX;
	int lastClickY;

	bool showPopup;
	int popupX;
	int popupY;
	int popupW;
	int popupH;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_COMPONENTS__TextBox_H__*/
