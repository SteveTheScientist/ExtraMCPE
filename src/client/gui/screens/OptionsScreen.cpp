#include "OptionsScreen.h"
#include "StartMenuScreen.h"
#include "PauseScreen.h"
#include "CreditsScreen.h"
#include "UsernameScreen.h"
#include "KeyBindsScreen.h"
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

OptionControl::OptionControl(int id, ControlType type, OptionId optId, const std::string& name, bool fullWidth, float minVal, float maxVal, ActionId actId)
    : super(id, 0, 0, fullWidth ? 310 : 150, 20, name),
      controlType(type),
      optionId(optId),
      actionId(actId),
      baseName(name),
      isFullWidth(fullWidth),
      minVal(minVal),
      maxVal(maxVal),
      isDraggingSlider(false),
      isListeningForKey(false)
{
}

OptionControl::~OptionControl() {
}

void OptionControl::updateLabel(Minecraft* minecraft) {
    if (controlType == CONTROL_ACTION) {
        if (actionId == ACTION_USERNAME) {
            std::string uname = minecraft->options.getStringValue(OPTIONS_USERNAME);
            if (uname.empty()) uname = "Steve";
            msg = "Username: " + uname;
            return;
        }
        msg = baseName;
        return;
    }

    if (controlType == CONTROL_KEY) {
        if (isListeningForKey) {
            msg = baseName + ": > ??? <";
        } else {
            int keyVal = minecraft->options.getIntValue(optionId);
            const char* keyName = Keyboard::getKeyName(keyVal);
            msg = baseName + ": " + (keyName ? keyName : "NONE");
        }
        return;
    }

    if (controlType == CONTROL_SLIDER) {
        float val = minecraft->options.getProgressValue(optionId);
        int pct = (int)(val * 100.0f + 0.5f);
        if (optionId == OPTIONS_MUSIC_VOLUME || optionId == OPTIONS_SOUND_VOLUME) {
            if (pct <= 0) {
                msg = baseName + ": OFF";
                return;
            }
        }
        msg = baseName + ": " + to_str(pct) + "%";
        return;
    }

    if (optionId == OPTIONS_VIEW_DISTANCE) {
        int dist = minecraft->options.getIntValue(OPTIONS_VIEW_DISTANCE);
        switch (dist) {
            case 0: msg = "Render Distance: Far"; break;
            case 1: msg = "Render Distance: Normal"; break;
            case 2: msg = "Render Distance: Short"; break;
            default: msg = "Render Distance: Tiny"; break;
        }
        return;
    }

    if (optionId == OPTIONS_GUI_SCALE) {
        int scale = minecraft->options.getIntValue(OPTIONS_GUI_SCALE);
        switch (scale) {
            case 0: msg = "GUI Scale: Auto"; break;
            case 1: msg = "GUI Scale: Normal"; break;
            default: msg = "GUI Scale: Large"; break;
        }
        return;
    }

    if (optionId == OPTIONS_WINDOW_SCALE) {
        int scale = minecraft->options.getIntValue(OPTIONS_WINDOW_SCALE);
        msg = "Window Scale: " + to_str(scale + 1) + "x";
        return;
    }

    if (optionId == OPTIONS_MENU_STYLE) {
        int st = minecraft->options.getIntValue(OPTIONS_MENU_STYLE);
        switch (st) {
            case 0: msg = "Menu Style: Modern"; break;
            case 1: msg = "Menu Style: Classic"; break;
            default: msg = "Menu Style: Java"; break;
        }
        return;
    }

    if (optionId == OPTIONS_DIFFICULTY) {
        int diff = minecraft->options.getIntValue(OPTIONS_DIFFICULTY);
        switch (diff) {
            case 0: msg = "Difficulty: Peaceful"; break;
            case 1: msg = "Difficulty: Easy"; break;
            case 2: msg = "Difficulty: Normal"; break;
            default: msg = "Difficulty: Hard"; break;
        }
        return;
    }

    if (optionId == OPTIONS_FANCY_GRAPHICS) {
        bool fancy = minecraft->options.getBooleanValue(OPTIONS_FANCY_GRAPHICS);
        msg = fancy ? "Graphics: Fancy" : "Graphics: Fast";
        return;
    }

    if (optionId == OPTIONS_LIMIT_FRAMERATE) {
        bool limit = minecraft->options.getBooleanValue(OPTIONS_LIMIT_FRAMERATE);
        msg = limit ? "Max Framerate: 60 FPS" : "Max Framerate: Unlimited";
        return;
    }

    if (optionId == OPTIONS_FOG_TYPE) {
        int fog = minecraft->options.getIntValue(OPTIONS_FOG_TYPE);
        msg = (fog == 0) ? "Fog Type: Fancy" : "Fog Type: Fast";
        return;
    }

    if (optionId == OPTIONS_DEBUG_STYLE) {
        int st = minecraft->options.getIntValue(OPTIONS_DEBUG_STYLE);
        msg = (st == 0) ? "Debug HUD Style: New" : "Debug HUD Style: Classic";
        return;
    }

    if (optionId == OPTIONS_TALL_CLOUDS) {
        bool state = minecraft->options.getBooleanValue(OPTIONS_TALL_CLOUDS);
        msg = "Tall Clouds: " + std::string(state ? "ON" : "OFF");
        return;
    }

    if (optionId == OPTIONS_CLOUD_THICKNESS) {
        float val = minecraft->options.getProgressValue(OPTIONS_CLOUD_THICKNESS);
        std::ostringstream ss;
        ss.precision(1);
        ss << std::fixed << "Cloud Thickness: " << val << "x";
        msg = ss.str();
        return;
    }

    if (optionId == OPTIONS_CLOUD_FADE) {
        float val = minecraft->options.getProgressValue(OPTIONS_CLOUD_FADE);
        int pct = (int)(val * 100.0f + 0.5f);
        msg = "Cloud Fade: " + to_str(pct) + "%";
        return;
    }

    if (optionId == OPTIONS_CLOUD_OPACITY) {
        float val = minecraft->options.getProgressValue(OPTIONS_CLOUD_OPACITY);
        int pct = (int)(val * 100.0f + 0.5f);
        msg = "Cloud Opacity: " + to_str(pct) + "%";
        return;
    }

    if (optionId == OPTIONS_CLOUD_FLAT_SHADING) {
        bool state = minecraft->options.getBooleanValue(OPTIONS_CLOUD_FLAT_SHADING);
        msg = "Flat Shading: " + std::string(state ? "ON" : "OFF");
        return;
    }

    if (optionId == OPTIONS_CLOUD_Y_LEVEL) {
        float val = minecraft->options.getProgressValue(OPTIONS_CLOUD_Y_LEVEL);
        int yLvl = (int)(val + 0.5f);
        msg = "Cloud Height: " + to_str(yLvl);
        return;
    }

    if (optionId == OPTIONS_BETA_SKY) {
        bool tallOn = minecraft->options.getBooleanValue(OPTIONS_TALL_CLOUDS);
        active = !tallOn;
    }

    bool state = minecraft->options.getBooleanValue(optionId);
    msg = baseName + (state ? ": ON" : ": OFF");
}

void OptionControl::toggleValue(Minecraft* minecraft) {
    if (controlType == CONTROL_ACTION || controlType == CONTROL_KEY) return;

    if (optionId == OPTIONS_BETA_SKY && minecraft->options.getBooleanValue(OPTIONS_TALL_CLOUDS)) {
        return;
    }

    if (optionId == OPTIONS_VIEW_DISTANCE) {
        int cur = minecraft->options.getIntValue(OPTIONS_VIEW_DISTANCE);
        minecraft->options.set(OPTIONS_VIEW_DISTANCE, (cur + 1) % 4);
    } else if (optionId == OPTIONS_GUI_SCALE) {
        int cur = minecraft->options.getIntValue(OPTIONS_GUI_SCALE);
        minecraft->options.set(OPTIONS_GUI_SCALE, (cur + 1) % 3);
    } else if (optionId == OPTIONS_WINDOW_SCALE) {
        int cur = minecraft->options.getIntValue(OPTIONS_WINDOW_SCALE);
        minecraft->options.set(OPTIONS_WINDOW_SCALE, (cur + 1) % 3);
    } else if (optionId == OPTIONS_MENU_STYLE) {
        int cur = minecraft->options.getIntValue(OPTIONS_MENU_STYLE);
        minecraft->options.set(OPTIONS_MENU_STYLE, (cur + 1) % 3);
    } else if (optionId == OPTIONS_DIFFICULTY) {
        int cur = minecraft->options.getIntValue(OPTIONS_DIFFICULTY);
        minecraft->options.set(OPTIONS_DIFFICULTY, (cur + 1) % 4);
    } else if (optionId == OPTIONS_FOG_TYPE) {
        int cur = minecraft->options.getIntValue(OPTIONS_FOG_TYPE);
        minecraft->options.set(OPTIONS_FOG_TYPE, (cur + 1) % 2);
    } else if (optionId == OPTIONS_DEBUG_STYLE) {
        int cur = minecraft->options.getIntValue(OPTIONS_DEBUG_STYLE);
        minecraft->options.set(OPTIONS_DEBUG_STYLE, (cur + 1) % 2);
    } else {
        minecraft->options.toggle(optionId);
    }

    updateLabel(minecraft);
}

void OptionControl::updateSliderValue(Minecraft* minecraft, int mx) {
    float pct = (float)(mx - x) / (float)width;
    pct = Mth::clamp(pct, 0.0f, 1.0f);
    float newVal = minVal + pct * (maxVal - minVal);
    minecraft->options.set(optionId, newVal);
    updateLabel(minecraft);
}

bool OptionControl::clicked(Minecraft* minecraft, int mx, int my) {
    if (!visible || !active) return false;

    // Bounds check to ensure control isn't covered by top header bar or bottom footer bar
    if (my < 34 || (minecraft->screen && my >= minecraft->screen->height - 34)) return false;

    if (mx >= x && my >= y && mx < x + width && my < y + height) {
        _currentlyDown = true;
        if (controlType == CONTROL_SLIDER) {
            isDraggingSlider = true;
            updateSliderValue(minecraft, mx);
        }
        return true;
    }
    return false;
}

void OptionControl::released(int mx, int my) {
    _currentlyDown = false;
    isDraggingSlider = false;
}

void OptionControl::render(Minecraft* minecraft, int xm, int ym) {
    if (!visible) return;

    Font* font = minecraft->font;
    Tesselator& t = Tesselator::instance;

    bool isHovered = (xm >= x && ym >= y && xm < x + width && ym < y + height);

    if (controlType == CONTROL_SLIDER) {
        minecraft->textures->loadAndBindTexture(isHovered ? "gui/Widget_slider_highlighted.png" : "gui/Widget_slider.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.vertexUV((float)x, (float)(y + height), blitOffset, 0, 1);
        t.vertexUV((float)(x + width), (float)(y + height), blitOffset, 1, 1);
        t.vertexUV((float)(x + width), (float)y, blitOffset, 1, 0);
        t.vertexUV((float)x, (float)y, blitOffset, 0, 0);
        t.draw();

        float val = minecraft->options.getProgressValue(optionId);
        float pct = Mth::clamp((val - minVal) / (maxVal - minVal), 0.0f, 1.0f);
        float handleW = 8.0f;
        float handleX = (float)x + ((float)width - handleW) * pct;
        bool isHandleHovered = isHovered || isDraggingSlider;

        minecraft->textures->loadAndBindTexture(isHandleHovered ? "gui/Widget_slider_handle_highlighted.png" : "gui/Widget_slider_handle.png");
        glColor4f2(1, 1, 1, 1);
        t.begin();
        t.vertexUV(handleX, (float)(y + height), blitOffset, 0, 1);
        t.vertexUV(handleX + handleW, (float)(y + height), blitOffset, 1, 1);
        t.vertexUV(handleX + handleW, (float)y, blitOffset, 1, 0);
        t.vertexUV(handleX, (float)y, blitOffset, 0, 0);
        t.draw();

        drawCenteredString(font, msg, x + width / 2, y + (height - 8) / 2, 0xffffffff);
    } else {
        super::render(minecraft, xm, ym);
    }
}

OptionsScreen::OptionsScreen()
	: btnDone(NULL),
	  m_activeKeyControl(NULL),
	  m_scrollY(0.0f),
	  m_maxScroll(0.0f),
	  m_isDraggingScrollbar(false)
{
}

OptionsScreen::~OptionsScreen() {
	delete btnDone;
	for (size_t i = 0; i < m_controls.size(); ++i) {
		delete m_controls[i];
	}
	m_controls.clear();
}

void OptionsScreen::init() {
	btnDone = new Button(100, 0, 0, 200, 20, "Done");
	buttons.push_back(btnDone);
	tabButtons.push_back(btnDone);

	m_controls.clear();
	int idSeq = 10;

	// Row 0: Username & Music Slider
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_USERNAME, "Username", false, 0.0f, 1.0f, ACTION_USERNAME));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_MUSIC_VOLUME, "Music", false, 0.0f, 1.0f));

	// Audio & Mouse Sensitivity
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_SOUND_VOLUME, "Sound", false, 0.0f, 1.0f));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_SLIDER, OPTIONS_SENSITIVITY, "Sensitivity", false, 0.0f, 1.0f));

	// General Controls & Display
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_INVERT_Y_MOUSE, "Invert Mouse"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_VIEW_DISTANCE, "Render Distance"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_FANCY_GRAPHICS, "Graphics"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_VIEW_BOBBING, "View Bobbing"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_LIMIT_FRAMERATE, "Max Framerate"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_VSYNC, "VSync"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_GUI_SCALE, "GUI Scale"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_WINDOW_SCALE, "Window Scale"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_MENU_STYLE, "Menu Style"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_AMBIENT_OCCLUSION, "Smooth Lighting"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_SMOOTH_CAMERA, "Smooth Camera"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_RENDER_DEBUG, "Render Debug"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_ANAGLYPH_3D, "3D Anaglyph"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_AUTOJUMP, "Auto-Jump"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_BLOCK_OUTLINE, "Block Outline"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_SERVER_VISIBLE, "Local Server Visible"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_DESTROY_VIBRATION, "Vibrate on Destroy"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_JAVA_HUD, "Java HUD"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_BETA_SKY, "Beta Sky"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_BEAUTIFUL_SKY, "Beautiful Skies"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_VIGNETTE, "Vignette"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_RESTORED_ANIMS, "Restored Animations"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_IS_LEFT_HANDED, "Lefty Mode"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_USE_TOUCHSCREEN, "Touchscreen"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_DIFFICULTY, "Difficulty"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_THIRD_PERSON_VIEW, "Third Person View"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_ALLOW_SPRINT, "Allow Sprint"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_BAR_ON_TOP, "Hotbar On Top"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_RPI_CURSOR, "Custom Cursor"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_FOLIAGE_TINT, "Foliage Tinting"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_TINTED_SIDE, "Tinted Sides"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_FOG_TYPE, "Fog Type"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_DEBUG_STYLE, "Debug HUD Style"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_NORMAL_LIGHTING, "Classic Lighting"));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_TOGGLE, OPTIONS_IS_JOY_TOUCH_AREA, "D-Pad Touch Area"));

	// Submenus: Key Binds & Credits
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_COUNT, "Key Binds...", false, 0.0f, 1.0f, ACTION_KEYBINDS));
	m_controls.push_back(new OptionControl(idSeq++, CONTROL_ACTION, OPTIONS_COUNT, "Credits...", false, 0.0f, 1.0f, ACTION_CREDITS));

	for (size_t i = 0; i < m_controls.size(); ++i) {
		m_controls[i]->updateLabel(minecraft);
		buttons.push_back(m_controls[i]);
	}
}

void OptionsScreen::updateScrollBounds() {
	int totalRows = (int)((m_controls.size() + 1) / 2);
	float contentH = (float)(totalRows * 24 + 10);
	float viewportH = (float)(height - 68);
	m_maxScroll = std::max(0.0f, contentH - viewportH);
	m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
}

void OptionsScreen::setupPositions() {
	if (btnDone) {
		btnDone->width = 200;
		btnDone->height = 20;
		btnDone->x = (width - 200) / 2;
		btnDone->y = height - 28;
	}

	updateScrollBounds();

	int centerX = width / 2;
	int topY = 36;

	for (size_t i = 0; i < m_controls.size(); ++i) {
		OptionControl* ctrl = m_controls[i];
		int r = (int)(i / 2);
		int c = (int)(i % 2);

		int itemX = (c == 0) ? centerX - 155 : centerX + 5;
		int itemW = 150;
		int itemY = (int)(topY + 4 + r * 24 - m_scrollY);

		ctrl->x = itemX;
		ctrl->y = itemY;
		ctrl->width = itemW;
		ctrl->height = 20;

		// Visible within header and footer margins
		ctrl->visible = (itemY + 20 >= 34 && itemY < height - 34);
		ctrl->updateLabel(minecraft);
	}
}

void OptionsScreen::tick() {
	super::tick();

	int xm = Mouse::getX();
	int ym = Mouse::getY();
	toGUICoordinate(xm, ym);

	for (size_t i = 0; i < m_controls.size(); ++i) {
		OptionControl* ctrl = m_controls[i];
		if (ctrl->isDraggingSlider) {
			if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
				ctrl->updateSliderValue(minecraft, xm);
			} else {
				ctrl->isDraggingSlider = false;
			}
		}
	}

	if (m_isDraggingScrollbar) {
		if (Mouse::isButtonDown(MouseAction::ACTION_LEFT)) {
			float viewportH = (float)(height - 68);
			if (viewportH > 0 && m_maxScroll > 0) {
				float relY = (float)(ym - 34) / (viewportH - 15);
				m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
				setupPositions();
			}
		} else {
			m_isDraggingScrollbar = false;
		}
	}
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

	// Top and bottom header/footer shadow masks rendered behind buttons
	fill(0, 0, width, 32, 0x70000000);
	fill(0, height - 34, width, height, 0x70000000);

	// Render vertical scrollbar track and thumb if content exceeds viewport
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

	super::render(xm, ym, a);

	drawCenteredString(font, "Options", width / 2, 10, 0xffffffff);
}

void OptionsScreen::buttonClicked(Button* button) {
	if (button == btnDone) {
		minecraft->options.save();
		if (minecraft->isLevelGenerated()) {
			minecraft->setScreen(new PauseScreen(false));
		} else {
			minecraft->screenChooser.setScreen(SCREEN_STARTMENU);
		}
		return;
	}

	OptionControl* ctrl = dynamic_cast<OptionControl*>(button);
	if (ctrl) {
		if (ctrl->controlType == CONTROL_KEY) {
			if (m_activeKeyControl && m_activeKeyControl != ctrl) {
				m_activeKeyControl->isListeningForKey = false;
				m_activeKeyControl->updateLabel(minecraft);
			}
			m_activeKeyControl = ctrl;
			ctrl->isListeningForKey = true;
			ctrl->updateLabel(minecraft);
			return;
		}

		if (ctrl->controlType == CONTROL_TOGGLE) {
			ctrl->toggleValue(minecraft);
			minecraft->options.save();

			// If scale changed, re-initialize screen so GUI coordinates immediately update
			if (ctrl->optionId == OPTIONS_GUI_SCALE || ctrl->optionId == OPTIONS_WINDOW_SCALE) {
				minecraft->setScreen(new OptionsScreen());
				return;
			}

			setupPositions();
		} else if (ctrl->controlType == CONTROL_ACTION) {
			if (ctrl->actionId == ACTION_USERNAME) {
				minecraft->setScreen(new UsernameScreen());
			} else if (ctrl->actionId == ACTION_KEYBINDS) {
				minecraft->setScreen(new KeyBindsScreen());
			} else if (ctrl->actionId == ACTION_CREDITS) {
				minecraft->setScreen(new CreditsScreen(CREDITS_RETURN_OPTIONS));
			}
		}
	}
}

void OptionsScreen::mouseClicked(int x, int y, int buttonNum) {
	if (buttonNum == MouseAction::ACTION_LEFT) {
		int centerX = width / 2;
		int topY = 34;
		int bottomY = height - 34;

		if (y >= topY && y < bottomY) {
			if (m_maxScroll > 0 && x >= centerX + 155 && x <= centerX + 172) {
				m_isDraggingScrollbar = true;
				float viewportH = (float)(height - 68);
				float relY = (float)(y - 34) / (viewportH - 15);
				m_scrollY = Mth::clamp(relY * m_maxScroll, 0.0f, m_maxScroll);
				setupPositions();
				return;
			}
		}
	}

	super::mouseClicked(x, y, buttonNum);
}

void OptionsScreen::mouseReleased(int x, int y, int buttonNum) {
	m_isDraggingScrollbar = false;
	for (size_t i = 0; i < m_controls.size(); ++i) {
		m_controls[i]->isDraggingSlider = false;
	}
	super::mouseReleased(x, y, buttonNum);
}

void OptionsScreen::mouseWheel(int dx, int dy, int xm, int ym) {
	if (dy != 0 && m_maxScroll > 0) {
		m_scrollY -= (float)dy * 12.0f;
		m_scrollY = Mth::clamp(m_scrollY, 0.0f, m_maxScroll);
		setupPositions();
	}
}

void OptionsScreen::keyPressed(int eventKey) {
	if (m_activeKeyControl && m_activeKeyControl->isListeningForKey) {
		if (eventKey != Keyboard::KEY_ESCAPE) {
			minecraft->options.set(m_activeKeyControl->optionId, eventKey);
			minecraft->options.save();
		}
		m_activeKeyControl->isListeningForKey = false;
		m_activeKeyControl->updateLabel(minecraft);
		m_activeKeyControl = NULL;
		return;
	}

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
