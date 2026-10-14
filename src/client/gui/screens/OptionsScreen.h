#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include <vector>
#include <string>

enum OptionItemType {
    OPTION_TYPE_TOGGLE,
    OPTION_TYPE_SLIDER,
    OPTION_TYPE_ACTION
};

struct OptionItem {
    OptionItemType type;
    OptionId optionId;
    std::string name;
    bool isFullWidth;

    // For sliders
    float minVal;
    float maxVal;

    // Action callback ID
    int actionId;
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
	std::string getOptionText(const OptionItem& item);
	void toggleOption(const OptionItem& item);

	Button* btnDone;
	std::vector<OptionItem> m_items;

	float m_scrollY;
	float m_maxScroll;
	bool m_isDraggingScrollbar;
	int m_activeSliderIndex;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__OptionsScreen_H__*/
