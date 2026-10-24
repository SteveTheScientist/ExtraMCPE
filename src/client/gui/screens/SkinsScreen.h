#ifndef NET_MINECRAFT_CLIENT_GUI_SCREENS__SkinsScreen_H__
#define NET_MINECRAFT_CLIENT_GUI_SCREENS__SkinsScreen_H__

#include "../Screen.h"
#include "../components/Button.h"
#include "../components/RolledSelectionListH.h"
#include <vector>
#include <string>

class Level;
class LevelStorage;
class Player;
class SkinsScreen;

struct SkinTweenData {
    float start;
    float stop;
    float cur;
    float dur;
};

class SkinList : public RolledSelectionListH {
    typedef RolledSelectionListH super;
public:
    SkinList(SkinsScreen* screen, Minecraft* minecraft, int width, int height);

    virtual int getNumberOfItems() override;
    virtual void selectItem(int item, bool doubleClick) override;
    virtual bool isSelectedItem(int item) override;
    virtual void renderItem(int i, int x, int y, int h, Tesselator& t) override;
    virtual void renderBackground() override {}

    virtual void tick() override;
    virtual float getPos(float alpha) override;
    virtual bool capXPosition() override;

    void stepLeft();
    void stepRight();
    void tweenInited();

    int selectedItem;
    int mode;
    SkinTweenData td;

private:
    SkinsScreen* m_screen;
};

class SkinsScreen : public Screen {
    typedef Screen super;
    friend class SkinList;
public:
    SkinsScreen();
    virtual ~SkinsScreen();

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
    void loadSkins();
    void renderPlayer(float xo, float yo, const std::string& skinTexturePath);

    Button* btnReset;
    Button* btnSelect;
    Button* btnDone;
    Button* btnLeft;
    Button* btnRight;

    SkinList* m_skinList;

    std::vector<std::string> m_skinFiles;
    std::vector<std::string> m_skinNames;

    Player* m_previewPlayer;
    Level* m_dummyLevel;
    LevelStorage* m_dummyStorage;
    bool m_ownsPreviewPlayer;
};

#endif /*NET_MINECRAFT_CLIENT_GUI_SCREENS__SkinsScreen_H__*/
