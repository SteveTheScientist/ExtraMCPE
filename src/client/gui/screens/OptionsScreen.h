#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <vector>
#include <string>

enum ControlType {
    CONTROL_TOGGLE,
    CONTROL_SLIDER,
    CONTROL_KEY,
    CONTROL_ACTION
};

enum ActionId {
    ACTION_USERNAME,
    ACTION_KEYBINDS,
    ACTION_CREDITS
};

class OptionControl : public Button {
    typedef Button super;
public:
    OptionControl(int id, ControlType type, OptionId optId, const std::string& name, bool fullWidth = false, float minVal = 0.0f, float maxVal = 1.0f, ActionId actId = ACTION_USERNAME);
    virtual ~OptionControl();

    virtual void render(Minecraft* minecraft, int xm, int ym) override;
    virtual bool clicked(Minecraft* minecraft, int mx, int my) override;
    virtual void released(int mx, int my) override;

    void updateLabel(Minecraft* minecraft);
    void toggleValue(Minecraft* minecraft);
    void updateSliderValue(Minecraft* minecraft, int mx);

    ControlType controlType;
    OptionId optionId;
    ActionId actionId;
    std::string baseName;
    bool isFullWidth;
    float minVal;
    float maxVal;
    bool isDraggingSlider;
    bool isListeningForKey;
};

class OptionsScreen: public Screen
{
	typedef Screen super;

public:
	OptionsScreen();
	virtual ~OptionsScreen();

	virtual void init() override;
	virtual void setupPositions() override;
	virtual void tick() override;
	virtual void render(int xm, int ym, float a) override;

	virtual void buttonClicked(Button* button) override;
	virtual void mouseClicked(int x, int y, int buttonNum) override;
	virtual void mouseReleased(int x, int y, int buttonNum) override;
	virtual void mouseWheel(int dx, int dy, int xm, int ym) override;
	virtual void keyPressed(int eventKey) override;

private:
	void updateScrollBounds();

	Button* btnDone;
	std::vector<OptionControl*> m_controls;
	OptionControl* m_activeKeyControl;

	float m_scrollY;
	float m_maxScroll;
	bool m_isDraggingScrollbar;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__*/
