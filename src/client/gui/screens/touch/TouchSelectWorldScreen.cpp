#include "TouchSelectWorldScreen.h"
#include "../StartMenuScreen.h"
#include "../ProgressScreen.h"
#include "../DialogDefinitions.h"
#include "../../components/ImageButton.h"
#include "../../../renderer/Textures.h"
#include "../../../renderer/Tesselator.h"
#include "../../../../world/level/LevelSettings.h"
#include "../../../../AppPlatform.h"
#include "../../../../util/StringUtils.h"
#include "../../../../util/Mth.h"
#include "../../../../platform/input/Mouse.h"
#include "../../../../Performance.h"

#include <algorithm>
#include <set>
#include "../SimpleChooseLevelScreen.h"

namespace Touch {

TouchWorldSelectionList::TouchWorldSelectionList( Minecraft* minecraft, int width, int height )
:	_height(height),
	hasPickedLevel(false),
	pickedIndex(-1),
	currentTick(0),
	stoppedTick(-1),
	mode(0),
	_newWorldSelected(false),
	RolledSelectionListH(minecraft, width, height, 0, width, 24, height-32, 120)
{
	_renderBottomBorder = false;
}

int TouchWorldSelectionList::getNumberOfItems() {
	return (int)levels.size();
}

void TouchWorldSelectionList::selectItem( int item, bool doubleClick ) {
	if (item < 0 || item >= (int)levels.size())
		return;

	if (selectedItem != item) {
		selectedItem = item;
		int targetX = (int)((float)(selectedItem * itemWidth) - (float)(width - itemWidth) * 0.5f);
		td.start = xo;
		td.stop = (float)targetX;
		td.cur = 0;
		td.dur = 8;
		mode = 1;
		tweenInited();
		return;
	}

	if (!hasPickedLevel) {
		hasPickedLevel = true;
		pickedIndex = item;
		pickedLevel = levels[item];
	}
}

bool TouchWorldSelectionList::isSelectedItem( int item ) {
	return item == selectedItem;
}

void TouchWorldSelectionList::selectStart(int item, int localX, int localY) {
	if (selectedItem != (int) levels.size() || item != selectedItem)
		return;
	_newWorldSelected = true;
}

void TouchWorldSelectionList::selectCancel() {
	_newWorldSelected = false;
}

void TouchWorldSelectionList::renderItem( int i, int x, int y, int h, Tesselator& t ) {
	int centerx = x + itemWidth/2;
	float a0 = Mth::Max(1.1f - std::abs( width / 2 - centerx ) * 0.0055f, 0.2f);
	if (a0 > 1) a0 = 1;
	int textColor =  (int)(255.0f * a0) * 0x010101;
	int textColor2 = (int)(140.0f * a0) * 0x010101;
	const int TX = centerx - itemWidth / 2 + 5;
	const int TY = y + 44;

	if (i < (int)levels.size()) {
		StringVector v = _descriptions[i];
		drawString(minecraft->font, v[0].c_str(), TX, TY +  0, textColor);
		drawString(minecraft->font, v[1].c_str(), TX, TY + 10, textColor2);
		drawString(minecraft->font, v[2].c_str(), TX, TY + 20, textColor2);
		drawString(minecraft->font, v[3].c_str(), TX, TY + 30, textColor2);

		minecraft->textures->loadAndBindTexture(_imageNames[i]);
		t.color(0.3f, 1.0f, 0.2f);

		const float IY = (float)y - 8;
		t.begin();
			t.color(textColor);
			t.vertexUV((float)(centerx-32), IY,      blitOffset, 0, 0.125f);
			t.vertexUV((float)(centerx-32), IY + 48, blitOffset, 0, 0.875f);
			t.vertexUV((float)(centerx+32), IY + 48, blitOffset, 1, 0.875f);
			t.vertexUV((float)(centerx+32), IY,      blitOffset, 1, 0.125f);
		t.draw();
	}
}

void TouchWorldSelectionList::stepLeft() {
	if (selectedItem > 0) {
		int xoffset = (int)(xo - ((float)(selectedItem * itemWidth) + ((float)(itemWidth-width)) * 0.5f));
		td.start = xo;
		td.stop = xo - itemWidth - xoffset;
		td.cur = 0;
		td.dur = 8;
		mode = 1;
		tweenInited();
	}
}

void TouchWorldSelectionList::stepRight() {
	if (selectedItem >= 0 && selectedItem < getNumberOfItems()-1) {
		int xoffset = (int)(xo - ((float)(selectedItem * itemWidth) + ((float)(itemWidth-width)) * 0.5f));
		td.start = xo;
		td.stop = xo + itemWidth - xoffset;
		td.cur = 0;
		td.dur = 8;
		mode = 1;
		tweenInited();
	}
}

void TouchWorldSelectionList::commit() {
	for (unsigned int i = 0; i < levels.size(); ++i) {
		LevelSummary& level = levels[i];

		std::stringstream ss;
		ss << level.name << "/preview.png";
		TextureId id = Textures::InvalidId;

		if (id != Textures::InvalidId) {
			_imageNames.push_back( ss.str() );
		} else {
			_imageNames.push_back("gui/default_world.png");
		}

		StringVector lines;
		lines.push_back(levels[i].name);
		lines.push_back(minecraft->platform()->getDateString(levels[i].lastPlayed));
		lines.push_back(levels[i].id);
		lines.push_back(LevelSettings::gameTypeToString(level.gameType));
		_descriptions.push_back(lines);

		selectedItem = 0;
	}
}

static float quadraticInOut(float t, float dur, float start, float stop) {
	const float delta = stop - start;
	const float T = (t / dur) * 2.0f;
	if (T < 1) return 0.5f*delta*T*T + start;
	return -0.5f*delta * ((T-1)*(T-3) - 1) + start;
}

void TouchWorldSelectionList::tick()
{
	RolledSelectionListH::tick();

	++currentTick;

	if (selectedItem < 0 && getNumberOfItems() > 0) {
		selectedItem = 0;
	}

	if (Mouse::isButtonDown(MouseAction::ACTION_LEFT) || dragState == 0) {
		return;
	}

	if (mode == 1) {
		if (++td.cur == td.dur) {
			mode = 0;
			xInertia = 0;
			xoo = xo = td.stop;
			int centerItem = getItemAtPosition(width/2, height/2);
			if (centerItem >= 0 && selectedItem < 0) selectedItem = centerItem;
		} else {
			tweenInited();
		}
		return;
	}

	float speed = Mth::abs(xInertia);
	bool slowEnoughToBeBothered = speed < 5.0f;
	if (!slowEnoughToBeBothered) {
		xInertia = xInertia * .9f;
		return;
	}

	xInertia *= 0.8f;

	if (speed < 1 && dragState < 0) {
		const int offsetx = (width-itemWidth) / 2;
		const float pxo = xo + offsetx;
		int index = getItemAtXPositionRaw((int)(pxo - 10*xInertia));
		int indexPos = index*itemWidth;

		float diff = (float)indexPos - pxo;
		if (diff < -itemWidth/2) {
			diff += itemWidth;
			index++;
		}
		if (Mth::abs(diff) < 1 && speed < 0.1f) {
			return;
		}

		td.start = xo;
		td.stop = xo + diff;
		td.cur = 0;
		td.dur = (float) Mth::Min(7, 1 + (int)(Mth::abs(diff) * 0.25f));
		mode = 1;
		tweenInited();
	}
}

float TouchWorldSelectionList::getPos( float alpha )
{
	if (mode != 1) return RolledSelectionListH::getPos(alpha);

	float x0 = quadraticInOut(td.cur, td.dur, td.start, td.stop);
	float x1 = quadraticInOut(td.cur+1, td.dur, td.start, td.stop);
	return x0 + (x1-x0)*alpha;
}

bool TouchWorldSelectionList::capXPosition() {
	bool capped = RolledSelectionListH::capXPosition();
	if (capped) mode = 0;
	return capped;
}

void TouchWorldSelectionList::tweenInited() {
	float x0 = quadraticInOut(td.cur,   td.dur, td.start, td.stop);
	float x1 = quadraticInOut(td.cur+1, td.dur, td.start, td.stop);
	_xinertia = 0;
	xInertia = x0-x1;
}

//
// Select World Screen
//
SelectWorldScreen::SelectWorldScreen()
:	bDelete (1, "Delete"),
	bCreate (2, "Create new"),
	bBack   (3, "Back"),
	bHeader (0, "Select world"),
	bWorldView(4, ""),
	worldsList(NULL),
	_hasStartedLevel(false)
{
	bDelete.active = false;
}

SelectWorldScreen::~SelectWorldScreen()
{
	delete worldsList;
}

void SelectWorldScreen::init()
{
	worldsList = new TouchWorldSelectionList(minecraft, width, height);
	loadLevelSource();
	worldsList->commit();

	buttons.push_back(&bDelete);
	buttons.push_back(&bCreate);
	buttons.push_back(&bBack);

	_mouseHasBeenUp = !Mouse::getButtonState(MouseAction::ACTION_LEFT);

	tabButtons.push_back(&bWorldView);
	tabButtons.push_back(&bDelete);
	tabButtons.push_back(&bCreate);
	tabButtons.push_back(&bBack);
}

void SelectWorldScreen::setupPositions() {
	int yBase = height - 28;
	int btnW = 98;
	int gap = 6;
	int totalW = btnW * 3 + gap * 2;
	int startX = (width - totalW) / 2;

	bDelete.width = bCreate.width = bBack.width = btnW;
	bDelete.height = bCreate.height = bBack.height = 20;

	bDelete.y = bCreate.y = bBack.y = yBase;

	bDelete.x = startX;
	bCreate.x = startX + btnW + gap;
	bBack.x   = startX + (btnW + gap) * 2;
}

void SelectWorldScreen::buttonClicked(Button* button)
{
	if (button->id == bCreate.id) {
		if (!_hasStartedLevel) {
			std::string name = getUniqueLevelName("World");
			minecraft->setScreen(new SimpleChooseLevelScreen(name));
		}
	}
	if (button->id == bDelete.id) {
		if (isIndexValid(worldsList->selectedItem)) {
			LevelSummary level = worldsList->levels[worldsList->selectedItem];
			minecraft->setScreen( new TouchDeleteWorldScreen(level) );
		}
	}
	if (button->id == bBack.id) {
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
	if (button->id == bWorldView.id) {
		worldsList->selectItem( worldsList->getItemAtPosition(width/2, height/2), false );
	}
}

bool SelectWorldScreen::handleBackEvent(bool isDown)
{
	if (!isDown)
	{
		minecraft->cancelLocateMultiplayer();
		minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
	}
	return true;
}

bool SelectWorldScreen::isIndexValid( int index )
{
	return worldsList && index >= 0 && index < worldsList->getNumberOfItems();
}

void SelectWorldScreen::mouseWheel(int dx, int dy, int xm, int ym)
{
	if (!worldsList) return;
	if (dy == 0) return;
	int num = worldsList->getNumberOfItems();
	int idx = worldsList->selectedItem;
	if (dy > 0) {
		if (idx > 0) {
			idx--;
			worldsList->stepLeft();
		}
	} else {
		if (idx < num - 1) {
			idx++;
			worldsList->stepRight();
		}
	}
	worldsList->selectedItem = idx;
}

void SelectWorldScreen::tick()
{
	worldsList->tick();

	if (worldsList->hasPickedLevel) {
		minecraft->selectLevel(worldsList->pickedLevel.id, worldsList->pickedLevel.name, LevelSettings::None());
		minecraft->hostMultiplayer();
		minecraft->setScreen(new ProgressScreen());
		_hasStartedLevel = true;
		return;
	}

	bDelete.active = isIndexValid(worldsList->selectedItem);
}

void SelectWorldScreen::render( int xm, int ym, float a )
{
	renderBackground();

	worldsList->setComponentSelected(bWorldView.selected);

	if (_mouseHasBeenUp)
		worldsList->render(xm, ym, a);
	else {
		worldsList->render(0, 0, a);
		_mouseHasBeenUp = !Mouse::getButtonState(MouseAction::ACTION_LEFT);
	}

	Screen::render(xm, ym, a);
	drawCenteredString(minecraft->font, "Select world", width / 2, 8, 0xffffffff);
}

void SelectWorldScreen::loadLevelSource()
{
	LevelStorageSource* levelSource = minecraft->getLevelSource();
	levelSource->getLevelList(levels);
	std::sort(levels.begin(), levels.end());

	for (unsigned int i = 0; i < levels.size(); ++i) {
		if (levels[i].id != LevelStorageSource::TempLevelId)
			worldsList->levels.push_back( levels[i] );
	}
}

std::string SelectWorldScreen::getUniqueLevelName( const std::string& level )
{
	std::set<std::string> Set;
	for (unsigned int i = 0; i < levels.size(); ++i)
		Set.insert(levels[i].id);

	std::string s = level;
	while ( Set.find(s) != Set.end() )
		s += "-";
	return s;
}

bool SelectWorldScreen::isInGameScreen() { return true;  }

void SelectWorldScreen::keyPressed( int eventKey )
{
	if (bWorldView.selected) {
		if (eventKey == minecraft->options.getIntValue(OPTIONS_KEY_LEFT))
			worldsList->stepLeft();
		if (eventKey == minecraft->options.getIntValue(OPTIONS_KEY_RIGHT))
			worldsList->stepRight();
	}

	Screen::keyPressed(eventKey);
}

TouchDeleteWorldScreen::TouchDeleteWorldScreen(const LevelSummary& level)
:	ConfirmScreen(NULL, "Are you sure you want to delete this world?",
						"'" + level.name + "' will be lost forever!",
						"Delete", "Cancel", 0),
	_level(level)
{
	tabButtonIndex = 1;
}

void TouchDeleteWorldScreen::postResult( bool isOk )
{
	if (isOk) {
		LevelStorageSource* storageSource = minecraft->getLevelSource();
		storageSource->deleteLevel(_level.id);
	}
	minecraft->screenChooser.setScreen(SCREEN_SELECTWORLD);
}

};
