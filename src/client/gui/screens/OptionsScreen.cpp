#include "OptionsScreen.h"

#include "StartMenuScreen.h"
#include "UsernameScreen.h"
#include "DialogDefinitions.h"
#include "../../Minecraft.h"
#include "../../../AppPlatform.h"
#include "CreditsScreen.h"

#include "../components/ImageButton.h"
#include "../components/OptionsGroup.h"
#include "platform/input/Keyboard.h"

OptionsScreen::OptionsScreen()
	: btnClose(NULL),
	bHeader(NULL),
	btnCredits(NULL),
	selectedCategory(0) {
}

OptionsScreen::~OptionsScreen() {
	if (btnClose != NULL) {
		delete btnClose;
		btnClose = NULL;
	}

	if (bHeader != NULL) {
		delete bHeader;
		bHeader = NULL;
	}

	if (btnCredits != NULL) {
		delete btnCredits;
		btnCredits = NULL;
	}

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		if (*it != NULL) {
			delete* it;
			*it = NULL;
		}
	}

	for (std::vector<OptionsGroup*>::iterator it = optionPanes.begin(); it != optionPanes.end(); ++it) {
		if (*it != NULL) {
			delete* it;
			*it = NULL;
		}
	}

	categoryButtons.clear();
}

void OptionsScreen::init() {
	bHeader = new Touch::THeader(0, "Options");

	btnClose = new ImageButton(1, "");

	ImageDef def;
	def.name = "gui/touchgui.png";
	def.width = 34;
	def.height = 26;

	def.setSrc(IntRectangle(150, 0, (int)def.width, (int)def.height));
	btnClose->setImageDef(def, true);

	categoryButtons.push_back(new Touch::TButton(2, "General"));
	categoryButtons.push_back(new Touch::TButton(3, "Game"));
	categoryButtons.push_back(new Touch::TButton(4, "Controls"));
	categoryButtons.push_back(new Touch::TButton(5, "Graphics"));
	categoryButtons.push_back(new Touch::TButton(6, "Tweaks"));

	btnCredits = new Touch::TButton(11, "Credits");

	buttons.push_back(bHeader);
	buttons.push_back(btnClose);
	buttons.push_back(btnCredits);

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		buttons.push_back(*it);
		tabButtons.push_back(*it);
	}

	generateOptionScreens();
	selectCategory(0);
}

void OptionsScreen::setupPositions() {
	int headerH = btnClose->height;

	btnClose->x = width - btnClose->width - 4;
	btnClose->y = 2;

	bHeader->x = 0;
	bHeader->y = 0;
	bHeader->width = width - btnClose->width - 8;
	bHeader->height = headerH;

	int categoryWidth = 76;
	int offsetNum = 0;

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		(*it)->width = categoryWidth;
		(*it)->height = 26;
		(*it)->x = 2;
		(*it)->y = bHeader->height + 4 + offsetNum * 28;

		offsetNum++;
	}

	if (btnCredits != NULL) {
		btnCredits->width = 66;
		btnCredits->height = 24;
		btnCredits->x = width - btnCredits->width - 6;
		btnCredits->y = height - btnCredits->height - 4;
	}

	for (std::vector<OptionsGroup*>::iterator it = optionPanes.begin(); it != optionPanes.end(); ++it) {
		if (categoryButtons.size() > 0 && categoryButtons[0] != NULL) {
			(*it)->x = categoryWidth + 8;
			(*it)->y = bHeader->height + 4;
			(*it)->width = width - (categoryWidth + 8) - 10;
			(*it)->height = height - bHeader->height - 8;

			(*it)->setupPositions();
		}
	}
}

void OptionsScreen::render(int xm, int ym, float a) {
	renderBackground();

	int xmm = xm * width / minecraft->width;
	int ymm = ym * height / minecraft->height - 1;

	if (currentOptionsGroup != NULL)
		currentOptionsGroup->render(minecraft, xmm, ymm);

	super::render(xm, ym, a);
}

void OptionsScreen::removed() {
}

void OptionsScreen::buttonClicked(Button* button) {
	if (button == btnClose) {
		minecraft->options.save();
		if (minecraft->screen != NULL) {
			minecraft->setScreen(NULL);
		} else {
			minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
		}
	}
	else if (button->id > 1 && button->id < 7) {
		int categoryButton = button->id - categoryButtons[0]->id;
		selectCategory(categoryButton);
	}
	else if (button == btnCredits) {
		minecraft->setScreen(new CreditsScreen());
	}
}

void OptionsScreen::selectCategory(int index) {
	int currentIndex = 0;

	for (std::vector<Touch::TButton*>::iterator it = categoryButtons.begin(); it != categoryButtons.end(); ++it) {
		if (index == currentIndex)
			(*it)->selected = true;
		else
			(*it)->selected = false;

		currentIndex++;
	}

	if (index < (int)optionPanes.size())
		currentOptionsGroup = optionPanes[index];
}

void OptionsScreen::generateOptionScreens() {
	optionPanes.push_back(new OptionsGroup("options.group.general"));
	optionPanes.push_back(new OptionsGroup("options.group.game"));
	optionPanes.push_back(new OptionsGroup("options.group.controls"));
	optionPanes.push_back(new OptionsGroup("options.group.graphics"));
	optionPanes.push_back(new OptionsGroup("options.group.tweaks"));

	// General Pane
	optionPanes[0]->addOptionItem(OPTIONS_USERNAME, minecraft)
		.addOptionItem(OPTIONS_SENSITIVITY, minecraft);

	// Game Pane
	optionPanes[1]->addOptionItem(OPTIONS_DIFFICULTY, minecraft)
		.addOptionItem(OPTIONS_SERVER_VISIBLE, minecraft)
		.addOptionItem(OPTIONS_THIRD_PERSON_VIEW, minecraft)
		.addOptionItem(OPTIONS_WINDOW_SCALE, minecraft)
		.addOptionItem(OPTIONS_GUI_SCALE, minecraft)
		.addOptionItem(OPTIONS_SENSITIVITY, minecraft)
		.addOptionItem(OPTIONS_MUSIC_VOLUME, minecraft)
		.addOptionItem(OPTIONS_SOUND_VOLUME, minecraft)
		.addOptionItem(OPTIONS_SMOOTH_CAMERA, minecraft)
		.addOptionItem(OPTIONS_DESTROY_VIBRATION, minecraft)
		.addOptionItem(OPTIONS_IS_LEFT_HANDED, minecraft);

	// Controls Pane
	optionPanes[2]->addOptionItem(OPTIONS_INVERT_Y_MOUSE, minecraft)
		.addOptionItem(OPTIONS_USE_TOUCHSCREEN, minecraft)
		.addOptionItem(OPTIONS_AUTOJUMP, minecraft)
		.addOptionItem(OPTIONS_BLOCK_OUTLINE, minecraft)
		.addOptionItem(OPTIONS_IS_JOY_TOUCH_AREA, minecraft);

	for (int i = OPTIONS_KEY_FORWARD; i <= OPTIONS_KEY_USE; i++) {
		optionPanes[2]->addOptionItem((OptionId)i, minecraft);
	}

	// Graphics Pane
	optionPanes[3]->addOptionItem(OPTIONS_FANCY_GRAPHICS, minecraft)
		.addOptionItem(OPTIONS_LIMIT_FRAMERATE, minecraft)
		.addOptionItem(OPTIONS_VSYNC, minecraft)
		.addOptionItem(OPTIONS_VIEW_DISTANCE, minecraft)
		.addOptionItem(OPTIONS_RENDER_DEBUG, minecraft)
		.addOptionItem(OPTIONS_ANAGLYPH_3D, minecraft)
		.addOptionItem(OPTIONS_VIEW_BOBBING, minecraft)
		.addOptionItem(OPTIONS_AMBIENT_OCCLUSION, minecraft)
		.addOptionItem(OPTIONS_NORMAL_LIGHTING, minecraft)
		.addOptionItem(OPTIONS_BEAUTIFUL_SKY, minecraft)
		.addOptionItem(OPTIONS_VIGNETTE, minecraft);

	optionPanes[4]->addOptionItem(OPTIONS_ALLOW_SPRINT, minecraft)
		.addOptionItem(OPTIONS_BAR_ON_TOP, minecraft)
		.addOptionItem(OPTIONS_MENU_STYLE, minecraft)
		.addOptionItem(OPTIONS_RPI_CURSOR, minecraft)
#ifndef __APPLE__
		.addOptionItem(OPTIONS_FOLIAGE_TINT, minecraft)
		.addOptionItem(OPTIONS_TINTED_SIDE, minecraft)
#endif
		.addOptionItem(OPTIONS_JAVA_HUD, minecraft)
		.addOptionItem(OPTIONS_FOG_TYPE, minecraft)
		.addOptionItem(OPTIONS_BETA_SKY, minecraft)
		.addOptionItem(OPTIONS_RESTORED_ANIMS, minecraft)
		.addOptionItem(OPTIONS_DEBUG_STYLE, minecraft);
}

void OptionsScreen::mouseClicked(int x, int y, int buttonNum) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->mouseClicked(minecraft, x, y, buttonNum);

	super::mouseClicked(x, y, buttonNum);
}

void OptionsScreen::mouseReleased(int x, int y, int buttonNum) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->mouseReleased(minecraft, x, y, buttonNum);

	super::mouseReleased(x, y, buttonNum);
}

void OptionsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	if (currentOptionsGroup != NULL && currentOptionsGroup->pointInside(xm, ym) && dy != 0) {
		currentOptionsGroup->scrollByPixels((float)dy * 18.0f);
	}
}

void OptionsScreen::keyPressed(int eventKey) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->keyPressed(minecraft, eventKey);
	if (eventKey == Keyboard::KEY_ESCAPE)
		minecraft->options.save();

	super::keyPressed(eventKey);
}

void OptionsScreen::charPressed(char inputChar) {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->charPressed(minecraft, inputChar);

	super::keyPressed(inputChar);
}

void OptionsScreen::tick() {
	if (currentOptionsGroup != NULL)
		currentOptionsGroup->tick(minecraft);

	super::tick();
}