#ifndef NET_MINECRAFT_CLIENT__Options_H__
#define NET_MINECRAFT_CLIENT__Options_H__

#define SOUND_MIN_VALUE 0.0f
#define SOUND_MAX_VALUE 1.0f
#define MUSIC_MIN_VALUE 0.0f
#define MUSIC_MAX_VALUE 1.0f
#define SENSITIVITY_MIN_VALUE 0.0f
#define SENSITIVITY_MAX_VALUE 1.0f
#define PIXELS_PER_MILLIMETER_MIN_VALUE 3.0f
#define PIXELS_PER_MILLIMETER_MAX_VALUE 4.0f

#include <string>
#include <cstdio>
#include "../platform/input/Keyboard.h"
#include "../util/StringUtils.h"
#include "OptionsFile.h"
#include "Option.h"
#include <array>

enum OptionId {
    // General
    OPTIONS_DIFFICULTY,
    OPTIONS_HIDEGUI,
    OPTIONS_THIRD_PERSON_VIEW,
    OPTIONS_GUI_SCALE,
    OPTIONS_DESTROY_VIBRATION,
    OPTIONS_MUSIC_VOLUME,
    OPTIONS_SOUND_VOLUME,
    OPTIONS_SKIN,
    OPTIONS_USERNAME,
    OPTIONS_SERVER_VISIBLE,
    OPTIONS_BAR_ON_TOP,
    OPTIONS_ALLOW_SPRINT,
    OPTIONS_AUTOJUMP,
	OPTIONS_WINDOW_SCALE,

    // Graphics
    OPTIONS_RENDER_DEBUG,
    OPTIONS_SMOOTH_CAMERA,
    OPTIONS_FIXED_CAMERA,
    OPTIONS_VIEW_DISTANCE,
    OPTIONS_VIEW_BOBBING,
    OPTIONS_AMBIENT_OCCLUSION,
    OPTIONS_ANAGLYPH_3D,
    OPTIONS_LIMIT_FRAMERATE,
    OPTIONS_VSYNC,
    OPTIONS_FANCY_GRAPHICS,
	OPTIONS_NORMAL_LIGHTING,

    // Cheats / debug
    OPTIONS_FLY_SPEED,
    OPTIONS_CAMERA_SPEED,
    OPTIONS_IS_FLYING,

    // Control
	OPTIONS_BLOCK_OUTLINE,
    OPTIONS_USE_MOUSE_FOR_DIGGING,
    OPTIONS_IS_LEFT_HANDED,
    OPTIONS_IS_JOY_TOUCH_AREA,
    OPTIONS_SENSITIVITY,
    OPTIONS_INVERT_Y_MOUSE,
    OPTIONS_USE_TOUCHSCREEN,

    OPTIONS_KEY_FORWARD,
    OPTIONS_KEY_LEFT,
    OPTIONS_KEY_BACK,
    OPTIONS_KEY_RIGHT,
    OPTIONS_KEY_JUMP,
    OPTIONS_KEY_INVENTORY,
    OPTIONS_KEY_SNEAK,
    OPTIONS_KEY_DROP,
    OPTIONS_KEY_CHAT,
    OPTIONS_KEY_FOG,
    OPTIONS_KEY_USE,

    OPTIONS_KEY_MENU_NEXT,
    OPTIONS_KEY_MENU_PREV,
    OPTIONS_KEY_MENU_OK,
    OPTIONS_KEY_MENU_CANCEL,

    OPTIONS_FIRST_LAUNCH,
    OPTIONS_LAST_IP,

    OPTIONS_RPI_CURSOR,
	OPTIONS_FOLIAGE_TINT,
    OPTIONS_FOG_TYPE,
	OPTIONS_JAVA_HUD,
	OPTIONS_RESTORED_ANIMS,
	OPTIONS_TINTED_SIDE,
	OPTIONS_BEAUTIFUL_SKY,
	OPTIONS_VIGNETTE,
	OPTIONS_DEBUG_STYLE,
	OPTIONS_COMPLETE_LIGHTING,
	OPTIONS_MENU_STYLE,
	OPTIONS_TALL_CLOUDS,
	OPTIONS_CLOUD_THICKNESS,
	OPTIONS_CLOUD_FADE,
	OPTIONS_CLOUD_OPACITY,
	OPTIONS_CLOUD_FLAT_SHADING,
	OPTIONS_CLOUD_Y_LEVEL,
	OPTIONS_ENHANCED_FOG,
	OPTIONS_FOG_START_PCT,
	OPTIONS_FOG_END_PCT,
	OPTIONS_FOG_TRANSITION_SPEED,
	OPTIONS_FOG_SUNSET_TINT,
	OPTIONS_FOG_HIGH_ALTITUDE_BLEND,
	OPTIONS_FOG_UNDERGROUND_FOG,
	OPTIONS_CHAT_SHOWN,
	OPTIONS_CHAT_TEXT_OPACITY,
	OPTIONS_CHAT_BG_OPACITY,
	OPTIONS_CHAT_TEXT_SIZE,
	OPTIONS_CHAT_LINE_SPACING,
	OPTIONS_CHAT_WIDTH,
	OPTIONS_CHAT_FOCUSED_HEIGHT,
	OPTIONS_CHAT_UNFOCUSED_HEIGHT,
	OPTIONS_CHAT_MESSAGE_TIME,
	OPTIONS_CHAT_POS_X,
	OPTIONS_CHAT_POS_Y,
	OPTIONS_TEXT_FORMATTING,
	// Should be last!
	OPTIONS_COUNT
};

struct FogSettings {
    bool enhancedFog = true;
    float startPct = 50.0f;
    float endPct = 100.0f;
    float transitionSpeed = 5.0f;
    bool sunsetTint = true;
    bool highAltitudeBlend = true;
    bool undergroundFog = true;
};

class Minecraft;
typedef std::vector<std::string> StringVector;

class Options
{
public:
    float sound = 1.0f;
    float music = 1.0f;

    static bool debugGl;

    Options(Minecraft* minecraft, const std::string& workingDirectory = "")
	: minecraft(minecraft) {
        m_options.fill(nullptr);
        initTable();
    }

    void initTable();

    int getIntValue(OptionId key) {
        auto option = opt<OptionInt>(key);
        return (option)? option->get() : 0;
    }

    std::string getStringValue(OptionId key) {
        auto option = opt<OptionString>(key);
        return (option)? option->get() : "";
    }

    float getProgressValue(OptionId key) {
        auto option = opt<OptionFloat>(key);
        return (option)? option->get() : 0.f;
    }

    bool getBooleanValue(OptionId key) {
        auto option = opt<OptionBool>(key);
        return (option)? option->get() : false;
    }

    float getProgrssMin(OptionId key) {
        auto option = opt<OptionFloat>(key);
        return (option)? option->getMin() : 0.f;
    }

    float getProgrssMax(OptionId key) {
        auto option = opt<OptionFloat>(key);
        return (option)? option->getMax() : 0.f;
    }

    Option* getOpt(OptionId id) { return m_options[id]; }

	FogSettings getFogSettings() {
		FogSettings settings;
		settings.enhancedFog = getBooleanValue(OPTIONS_ENHANCED_FOG);
		settings.startPct = getProgressValue(OPTIONS_FOG_START_PCT);
		settings.endPct = getProgressValue(OPTIONS_FOG_END_PCT);
		settings.transitionSpeed = getProgressValue(OPTIONS_FOG_TRANSITION_SPEED);
		settings.sunsetTint = getBooleanValue(OPTIONS_FOG_SUNSET_TINT);
		settings.highAltitudeBlend = getBooleanValue(OPTIONS_FOG_HIGH_ALTITUDE_BLEND);
		settings.undergroundFog = getBooleanValue(OPTIONS_FOG_UNDERGROUND_FOG);
		return settings;
	}

    void load();
    void save();
    void set(OptionId key, bool value);
    void set(OptionId key, int value);
    void set(OptionId key, float value);
    void set(OptionId key, const std::string& value);
	void setOptionsFilePath(const std::string& path);
	void toggle(OptionId key);

	void notifyOptionUpdate(OptionId key, bool value);
	void notifyOptionUpdate(OptionId key, float value);
	void notifyOptionUpdate(OptionId key, int value);
    void notifyOptionUpdate(OptionId key, const std::string& value) {}

private:
    template<typename T>
    T* opt(OptionId key) {
        if (m_options[key] == nullptr) return nullptr;
        return dynamic_cast<T*>(m_options[key]);
    }

	std::array<Option*, OPTIONS_COUNT> m_options;
	OptionsFile optionsFile;

	Minecraft* minecraft;
};

#endif /*NET_MINECRAFT_CLIENT__Options_H__*/
