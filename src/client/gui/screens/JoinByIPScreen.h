
#include "../Screen.h"
#include "../components/Button.h"
#include "../../Minecraft.h"
#include "../components/TextBox.h"

class JoinByIPScreen: public Screen
{
public:
	JoinByIPScreen();
	virtual ~JoinByIPScreen();

	void init() override;
	void setupPositions() override;

	virtual void tick() override;
    void render(int xm, int ym, float a) override;

	virtual void keyPressed(int eventKey) override;
	void buttonClicked(Button* button) override;
	virtual void mouseClicked(int x, int y, int buttonNum) override;
    virtual bool handleBackEvent(bool isDown) override;
private:
    TextBox tName;
    TextBox tIP;
	Button bJoin;
	Button bBack;
};
