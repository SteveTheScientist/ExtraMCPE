#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__KeyBindsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__KeyBindsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../../Options.h"
#include <vector>
#include <string>

struct KeyBindEntry {
    std::string category; // If non-empty, represents a category header
    std::string label;
    OptionId optionId;
    int defaultKey;
    Button* keyBtn;
    Button* resetBtn;
};

class KeyBindsScreen : public Screen {
    typedef Screen super;

public:
    KeyBindsScreen();
    virtual ~KeyBindsScreen();

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
    void updateKeyLabels();
    void resetAllKeys();

    Button* btnDone;
    Button* btnResetAll;

    std::vector<KeyBindEntry> m_entries;
    int m_listeningIndex; // Index of entry currently remapping key, or -1

    float m_scrollY;
    float m_maxScroll;
    bool m_isDraggingScrollbar;
};

#endif /* NET_MINECRAFT_CLIENT_GUI_SCREENS__KeyBindsScreen_H__ */
