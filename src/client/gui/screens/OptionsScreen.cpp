#include "OptionsScreen.h"
#include "StartMenuScreen.h"
#include "PauseScreen.h"
#include "CreditsScreen.h"
#include "../../Minecraft.h"
#include "../../sound/SoundEngine.h"
#include "../../../util/Mth.h"
#include "../../renderer/Tesselator.h"
#include "../../renderer/Textures.h"
#include "platform/input/Keyboard.h"
#include "platform/input/Mouse.h"
#include <algorithm>
#include <sstream>

template<typename T>
static std::string to_str(T val) {
    std::ostringstream ss;
    ss << val;
    return ss.str();
}

OptionsScreen::OptionsScreen()
	: btnDone(NULL),
	  m_scrollY(0.0f),
	  m_maxScroll(0.0f),
	  m_isDraggingScrollbar(false),
	  m_activeSliderIndex(-1)
{
}

OptionsScreen::~OptionsScreen() {
	delete btnDone;
}

void OptionsScreen::init() {
	btnDone = new Button(100, 0, 0, 200, 20, "Done");
	buttons.push_back(btnDone);
	tabButtons.push_back(btnDone);

	m_items.clear();
	m_items.push_back({ OPTION_TYPE_SLIDER, OPTIONS_MUSIC_VOLUME, "Music", false, 0.0f, 1.0f, 0 });
	m_items.push_back({ OPTION_TYPE_SLIDER, OPTIONS_SOUND_VOLUME, "Sound", false, 0.0f, 1.0f, 0 });
	m_items.push_back({ OPTION_TYPE_SLIDER, OPTIONS_SENSITIVITY, "Sensitivity", false, 0.0f, 1.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_INVERT_Y_MOUSE, "Invert Mouse", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_VIEW_DISTANCE, "Render Distance", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_FANCY_GRAPHICS, "Graphics", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_VIEW_BOBBING, "View Bobbing", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_LIMIT_FRAMERATE, "Max Framerate", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_VSYNC, "VSync", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_GUI_SCALE, "GUI Scale", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_AMBIENT_OCCLUSION, "Smooth Lighting", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_ANAGLYPH_3D, "3D Anaglyph", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_AUTOJUMP, "Auto-Jump", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_BLOCK_OUTLINE, "Block Outline", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_SERVER_VISIBLE, "Local Server Visible", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_DESTROY_VIBRATION, "Vibrate on Destroy", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_JAVA_HUD, "Java HUD", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_BETA_SKY, "Beta Sky", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_BEAUTIFUL_SKY, "Beautiful Skies", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_VIGNETTE, "Vignette", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_RESTORED_ANIMS, "Restored Animations", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_TOGGLE, OPTIONS_IS_LEFT_HANDED, "Lefty Mode", false, 0.0f, 0.0f, 0 });
	m_items.push_back({ OPTION_TYPE_ACTION, OPTIONS_COUNT, "Credits...", true, 0.0f, 0.0f, 1 });
}

void OptionsScreen::setupPositions() {
	if (btnDone) {
		btnDone->width = 200;
		btnDone->height = 20;
		btnDone->x = (width - 200) / 2;
		btnDone->y = height - 28;
	}

	int totalRows = (int)((m_items.size() + 1) / 2);
	float contentH = (float)(totalRows * 24 + 10);
	float viewportH = (float)(height - 68);
	m_maxScroll = std::max(0.0f, contentH - viewportH);
	m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void OptionsScreen::tick() {
	super::tick();

	if (m_activeSliderIndex >= 0 && m_activeSliderIndex < (int)m_items.size()) {
		if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
			int xm = Mouse::getX() * width / minecraft->width;
			int ym = Mouse::getY() * height / minecraft->height - 1;

			const OptionItem& item = m_items[m_activeSliderIndex];
			int r = m_activeSliderIndex / 2;
			int c = m_activeSliderIndex % 2;
			int centerX = width / 2;
			int itemX = (c == 0 || item.isFullWidth) ? centerX - 155 : centerX + 5;
			int itemW = item.isFullWidth ? 310 : 150;

			float pct = (float)(xm - itemX) / (float)itemW;
			pct = Mth::clamp(pct, 0.0f, 1.0f);
			float newVal = item.minVal + pct * (item.maxVal - item.minVal);
			minecraft->options.set(item.optionId, newVal);
		} else {
			m_activeSliderIndex = -1;
		}
	}

	if (m_isDraggingScrollbar) {
		if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
			int ym = Mouse::getY() * height / minecraft->height - 1;
			float viewportH = (float)(height - 68);
			if (viewportH > 0 && m_maxScroll > 0) {
				float relY = (float)(ym - 34) / (viewportH - 15);
				m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
			}
		} else {
			m_isDraggingScrollbar = false;
		}
	}
}

std::string OptionsScreen::getOptionText(const OptionItem& item) {
	if (item.type == OPTION_TYPE_ACTION) {
		return item.name;
	}

	if (item.type == OPTION_TYPE_SLIDER) {
		float val = minecraft->options.getProgressValue(item.optionId);
		int pct = (int)(val * 100.0f + 0.5f);
		if (item.optionId == OPTIONS_MUSIC_VOLUME || item.optionId == OPTIONS_SOUND_VOLUME) {
			if (pct <= 0) return item.name + ": OFF";
		}
		return item.name + ": " + to_str(pct) + "%";
	}

	if (item.optionId == OPTIONS_VIEW_DISTANCE) {
		int dist = minecraft->options.getIntValue(OPTIONS_VIEW_DISTANCE);
		switch (dist) {
			case 0: return "Render Distance: Far";
			case 1: return "Render Distance: Normal";
			case 2: return "Render Distance: Short";
			default: return "Render Distance: Tiny";
		}
	}

	if (item.optionId == OPTIONS_GUI_SCALE) {
		int scale = minecraft->options.getIntValue(OPTIONS_GUI_SCALE);
		switch (scale) {
			case 0: return "GUI Scale: Auto";
			case 1: return "GUI Scale: Normal";
			default: return "GUI Scale: Large";
		}
	}

	if (item.optionId == OPTIONS_FANCY_GRAPHICS) {
		bool fancy = minecraft->options.getBooleanValue(OPTIONS_FANCY_GRAPHICS);
		return fancy ? "Graphics: Fancy" : "Graphics: Fast";
	}

	if (item.optionId == OPTIONS_LIMIT_FRAMERATE) {
		bool limit = minecraft->options.getBooleanValue(OPTIONS_LIMIT_FRAMERATE);
		return limit ? "Max Framerate: 60 FPS" : "Max Framerate: Unlimited";
	}

	bool state = minecraft->options.getBooleanValue(item.optionId);
	return item.name + (state ? ": ON" : ": OFF");
}

void OptionsScreen::toggleOption(const OptionItem& item) {
	if (item.type == OPTION_TYPE_ACTION) {
		if (item.actionId == 1) {
			minecraft->setScreen(new CreditsScreen(CREDITS_RETURN_OPTIONS));
		}
		return;
	}

	if (item.optionId == OPTIONS_VIEW_DISTANCE) {
		int cur = minecraft->options.getIntValue(OPTIONS_VIEW_DISTANCE);
		minecraft->options.set(OPTIONS_VIEW_DISTANCE, (cur + 1) % 4);
		return;
	}

	if (item.optionId == OPTIONS_GUI_SCALE) {
		int cur = minecraft->options.getIntValue(OPTIONS_GUI_SCALE);
		minecraft->options.set(OPTIONS_GUI_SCALE, (cur + 1) % 3);
		return;
	}

	bool curState = minecraft->options.getBooleanValue(item.optionId);
	minecraft->options.set(item.optionId, !curState);
}

void OptionsScreen::render(int xm, int ym, float a) {
	if (minecraft->isLevelGenerated()) {
		renderBackground();
	} else {
		renderDirtBackground(0);
	}

	Font* font = minecraft->font;
	Tesselator& t = Tesselator::instance;

	int centerX = width / 2;
	int topY = 34;
	int bottomY = height - 34;

	// Render items list inside central viewport area
	for (size_t i = 0; i < m_items.size(); ++i) {
		const OptionItem& item = m_items[i];
		int r = (int)(i / 2);
		int c = (int)(i % 2);

		int itemX = (c == 0 || item.isFullWidth) ? centerX - 155 : centerX + 5;
		int itemW = item.isFullWidth ? 310 : 150;
		int itemY = (int)(topY + 4 + r * 24 - m_scrollY);
		int itemH = 20;

		// Only draw items visible within header and footer margins
		if (itemY + itemH < topY || itemY > bottomY) {
			continue;
		}

		bool isHovered = (xm >= itemX && ym >= itemY && xm < itemX + itemW && ym < itemY + itemH && ym >= topY && ym < bottomY);

		if (item.type == OPTION_TYPE_SLIDER) {
			// Slider track background texture
			minecraft->textures->loadAndBindTexture(isHovered ? "gui/Widget_slider_highlighted.png" : "gui/Widget_slider.png");
			glColor4f2(1, 1, 1, 1);
			t.begin();
			t.vertexUV((float)itemX, (float)(itemY + itemH), blitOffset, 0, 1);
			t.vertexUV((float)(itemX + itemW), (float)(itemY + itemH), blitOffset, 1, 1);
			t.vertexUV((float)(itemX + itemW), (float)itemY, blitOffset, 1, 0);
			t.vertexUV((float)itemX, (float)itemY, blitOffset, 0, 0);
			t.draw();

			// Slider handle thumb texture
			float val = minecraft->options.getProgressValue(item.optionId);
			float pct = Mth::clamp((val - item.minVal) / (item.maxVal - item.minVal), 0.0f, 1.0f);
			float handleW = 8.0f;
			float handleX = itemX + (itemW - handleW) * pct;
			bool isHandleHovered = isHovered || (m_activeSliderIndex == (int)i);

			minecraft->textures->loadAndBindTexture(isHandleHovered ? "gui/Widget_slider_handle_highlighted.png" : "gui/Widget_slider_handle.png");
			glColor4f2(1, 1, 1, 1);
			t.begin();
			t.vertexUV(handleX, (float)(itemY + itemH), blitOffset, 0, 1);
			t.vertexUV(handleX + handleW, (float)(itemY + itemH), blitOffset, 1, 1);
			t.vertexUV(handleX + handleW, (float)itemY, blitOffset, 1, 0);
			t.vertexUV(handleX, (float)itemY, blitOffset, 0, 0);
			t.draw();

			std::string label = getOptionText(item);
			drawCenteredString(font, label, itemX + itemW / 2, itemY + (itemH - 8) / 2, 0xffffffff);
		} else {
			// Standard button texture using Button renderer
			std::string label = getOptionText(item);
			Button b(0, itemX, itemY, itemW, itemH, label);
			b.render(minecraft, xm, ym);
		}
	}

	// Render vertical scrollbar if list exceeds viewport height
	if (m_maxScroll > 0) {
		int trackX = centerX + 160;
		int trackW = 6;
		int trackY = topY + 2;
		int trackH = bottomY - topY - 4;

		minecraft->textures->loadAndBindTexture("gui/Widget_scroller_background.png");
		glColor4f2(1, 1, 1, 1);
		t.begin();
		t.vertexUV((float)trackX, (float)(trackY + trackH), blitOffset, 0, 1);
		t.vertexUV((float)(trackX + trackW), (float)(trackY + trackH), blitOffset, 1, 1);
		t.vertexUV((float)(trackX + trackW), (float)trackY, blitOffset, 1, 0);
		t.vertexUV((float)trackX, (float)trackY, blitOffset, 0, 0);
		t.draw();

		int thumbW = 12;
		int thumbH = 15;
		int thumbX = centerX + 157;
		int thumbY = trackY + (int)((trackH - thumbH) * (m_scrollY / m_maxScroll));

		minecraft->textures->loadAndBindTexture("gui/Scroller.png");
		glColor4f2(1, 1, 1, 1);
		t.begin();
		t.vertexUV((float)thumbX, (float)(thumbY + thumbH), blitOffset, 0, 1);
		t.vertexUV((float)(thumbX + thumbW), (float)(thumbY + thumbH), blitOffset, 1, 1);
		t.vertexUV((float)(thumbX + thumbW), (float)thumbY, blitOffset, 1, 0);
		t.vertexUV((float)thumbX, (float)thumbY, blitOffset, 0, 0);
		t.draw();
	}

	// Top and bottom header/footer shadow masks
	fill(0, 0, width, 32, 0xe0000000);
	fill(0, height - 34, width, height, 0xe0000000);

	drawCenteredString(font, "Options", width / 2, 10, 0xffffffff);

	super::render(xm, ym, a);
}

void OptionsScreen::buttonClicked(Button* button) {
	if (button->id == 100) {
		minecraft->options.save();
		if (minecraft->isLevelGenerated()) {
			minecraft->setScreen(new PauseScreen(false));
		} else {
			minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
		}
	}
}

void OptionsScreen::mouseClicked(int x, int y, int buttonNum) {
	if (buttonNum == MouseAction::ACTION_LEFT) {
		int centerX = width / 2;
		int topY = 34;
		int bottomY = height - 34;

		if (y >= topY && y < bottomY) {
			// Check scrollbar hit
			if (m_maxScroll > 0 && x >= centerX + 155 && x <= centerX + 172) {
				m_isDraggingScrollbar = true;
				float viewportH = (float)(height - 68);
				float relY = (float)(y - 34) / (viewportH - 15);
				m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
				return;
			}

			// Check option items hit
			for (size_t i = 0; i < m_items.size(); ++i) {
				const OptionItem& item = m_items[i];
				int r = (int)(i / 2);
				int c = (int)(i % 2);

				int itemX = (c == 0 || item.isFullWidth) ? centerX - 155 : centerX + 5;
				int itemW = item.isFullWidth ? 310 : 150;
				int itemY = (int)(topY + 4 + r * 24 - m_scrollY);
				int itemH = 20;

				if (x >= itemX && y >= itemY && x < itemX + itemW && y < itemY + itemH) {
					if (item.type == OPTION_TYPE_SLIDER) {
						m_activeSliderIndex = (int)i;
						float pct = (float)(x - itemX) / (float)itemW;
						pct = Mth::clamp(pct, 0.0f, 1.0f);
						float newVal = item.minVal + pct * (item.maxVal - item.minVal);
						minecraft->options.set(item.optionId, newVal);
					} else {
						toggleOption(item);
					}
					minecraft->soundEngine->playUI("random.click", 1, 1);
					return;
				}
			}
		}
	}

	super::mouseClicked(x, y, buttonNum);
}

void OptionsScreen::mouseReleased(int x, int y, int buttonNum) {
	m_isDraggingScrollbar = false;
	m_activeSliderIndex = -1;
	super::mouseReleased(x, y, buttonNum);
}

void OptionsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	if (dy != 0) {
		m_scrollY -= (float)dy * 12.0f;
		m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
	}
}

void OptionsScreen::keyPressed(int eventKey) {
	if (eventKey == Keyboard::KEY_ESCAPE) {
		minecraft->options.save();
		if (minecraft->isLevelGenerated()) {
			minecraft->setScreen(new PauseScreen(false));
		} else {
			minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
		}
		return;
	}
	super::keyPressed(eventKey);
}
