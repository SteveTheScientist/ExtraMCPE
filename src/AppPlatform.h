#ifndef APPPLATFORM_H__
#define APPPLATFORM_H__

#include <vector>
#include <string>
#include <cstring>
#include "client/renderer/TextureData.h"

typedef std::vector<std::string> StringVector;

class BinaryBlob {
public:
	BinaryBlob()
	:	data(NULL),
		size(-1) {}

	BinaryBlob(unsigned char* data, unsigned int size)
	:	data(data),
		size(size) {}

	unsigned char* data;
	int size;
};

class PlatformStringVars {
public:
	static const int DEVICE_BUILD_MODEL = 0;
};

class AppPlatform
{
public:
	AppPlatform() : keyboardVisible(false) {}
    virtual ~AppPlatform() {}

	virtual void saveScreenshot(const std::string& filename, int glWidth, int glHeight) {}
	virtual TextureData loadTexture(const std::string& filename_, bool textureFolder) { return TextureData(); }
	virtual TextureData loadTextureFromMemory(const unsigned char* data, size_t size) { return TextureData(); }

    virtual void playSound(const std::string& fn, float volume, float pitch) {}

	virtual void hideCursor(bool hide) {}

	virtual void showDialog(int dialogId) {}
    virtual void createUserInput() {}

	bool is_big_endian(void)  {
		union {
			unsigned int i;
			char c[4];
		} bint = {0x01020304};
		return bint.c[0] == 1;
	}

	void createUserInput(int dialogId)
	{
		showDialog(dialogId);
		createUserInput();
	}
	virtual int getUserInputStatus() { return 0; }
	virtual StringVector getUserInput() { return StringVector(); }

	virtual std::string getDateString(int s) { return ""; }

	virtual void uploadPlatformDependentData(int id, void* data) {}
	virtual BinaryBlob readAssetFile(const std::string& filename) { return BinaryBlob(); }
	virtual void _tick() {}

	virtual int getScreenWidth() { return 854; }
	virtual int getScreenHeight() { return 480; }
    virtual float getPixelsPerMillimeter() { return 10; }

	virtual bool isNetworkEnabled(bool onlyWifiAllowed) { return true; }

	virtual bool isPowerVR() {
		return false;
	}
	virtual int getKeyFromKeyCode(int keyCode, int metaState, int deviceId) {return 0;}
#ifdef __APPLE__
    virtual bool isSuperFast() = 0;
#endif

	virtual void openURL(const std::string& url) {}
	virtual void importSkinOrCape(int type) {}

	virtual void finish() {}

	virtual bool supportsTouchscreen() { return false; }

	virtual void vibrate(int milliSeconds) {}

	virtual std::string getPlatformStringVar(int stringId) {
		return "<getPlatformStringVar NotImplemented>";
	}

	virtual void showKeyboard() {
		keyboardVisible = true;
	}
	virtual void hideKeyboard() {
		keyboardVisible = false;
	}
	virtual bool isKeyboardVisible() {return keyboardVisible;}
protected:
	bool keyboardVisible;
};

#endif /*APPPLATFORM_H__*/
