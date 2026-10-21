#ifndef APPPLATFORM_ANDROID_H__
#define APPPLATFORM_ANDROID_H__

#include "AppPlatform.h"
#include "client/renderer/gles.h"
#include "platform/log.h"
#include "platform/time.h"
#include "platform/PngLoader.h"
#include <jni.h>
#include <cmath>
#include <exception>
#include <fstream>
#include <android/native_activity.h>

class JVMAttacher {
public:
    JVMAttacher(JavaVM* vm)
    :   _vm(vm),
        _isAttached(false),
        _env(NULL)
    {
        if( _vm->GetEnv((void**)&_env, JNI_VERSION_1_4 ) != JNI_OK ) {
            _vm->AttachCurrentThread(&_env, NULL);
            _isAttached = _env != NULL;
        }
    }

    ~JVMAttacher() {
        forceDetach();
    }

    JNIEnv* getEnv() {
        return _env;
    }

    void forceDetach() {
        if (!_isAttached) return;
        _vm->DetachCurrentThread();
        _isAttached = false;
    }

private:
    JavaVM* _vm;
    JNIEnv* _env;
    bool    _isAttached;
};

class AppPlatform_android: public AppPlatform
{
    typedef AppPlatform super;
public:
    AppPlatform_android()
    :   _vm(NULL),
        _isInited(false),

        instance(0),
        _activityClass(0),

        _methodSaveScreenshot(0),
        _postScreenshotToFacebook(0),
        _getImageData(0),
        _getImageDataFromBytes(0),
		_readAssetFile(0),
        _methodPlaySound(0),
        _showDialog(0),
        _methodTick(0),
        _methodFinish(0),

        _methodUserInputInitiate(0),
        _methodUserInputStatus(0),
        _methodUserInputString(0),

        _methodGetDateString(0),

        _methodVibrate(0),
        _methodSupportsTouchscreen(0),
		_methodSetIsPowerVR(0),
		_methodIsNetworkEnabled(0),

        _getScreenWidth(0),
        _getScreenHeight(0),
        _methodGetPixelsPerMillimeter(0),
		_methodGetPlatformStringVar(0),

		_classWindow(0),
		_classContext(0),
		_fieldINPUT_METHOD_SERVICE(0),
		_classInputManager(0),
		_methodGetSystemService(0),
		_methodGetWindow(0),
		_methodGetDecorView(0),
		_methodShowSoftInput(0),
		_classView(0),
		_methodGetWindowToken(0),
		_methodHideSoftInput(0),

        _screenWidth(854),
        _screenHeight(480),
		_hasSetPowerVR(false),
		_nativeActivity(0),

		_methodGetKeyFromKeyCode(0),
		_methodOpenURL(0),
		_methodImportSkinOrCape(0)
    {
    }

    int getScreenWidth() { return _screenWidth; }
    int getScreenHeight() { return _screenHeight; }
    void setScreenDimensions(int w, int h) { _screenWidth = w; _screenHeight = h; }

    static jmethodID getMethodSafe(JNIEnv* env, jclass clazz, const char* name, const char* sig, bool isStatic = false) {
        if (!env || !clazz) return NULL;
        jmethodID mid = isStatic ? env->GetStaticMethodID(clazz, name, sig) : env->GetMethodID(clazz, name, sig);
        if (env->ExceptionOccurred()) {
            LOGW("Failed to find JNI method: %s %s (static=%d)", name, sig, isStatic);
            env->ExceptionClear();
            return NULL;
        }
        return mid;
    }

    int init(JavaVM* vm) {
        if (_isInited)
            return -1;

        JVMAttacher ta(vm);
        JNIEnv* env = ta.getEnv();

        if(!env) {
            LOGI("%s - Failed to get the environment using JVMAttacher::getEnv()", __FUNCTION__);
            return -2;
        }

        const char* interface_path = "com/mojang/minecraftpe/MainActivity";
        jclass clazz = NULL;
        if( (clazz = env->FindClass( interface_path )) == 0 ) {
            if (env->ExceptionOccurred()) env->ExceptionClear();
            return -3;
        }
        _activityClass = (jclass)env->NewGlobalRef(clazz);

        _methodSaveScreenshot = getMethodSafe(env, _activityClass, "saveScreenshot", "(Ljava/lang/String;II[I)V", true);
        _postScreenshotToFacebook = getMethodSafe(env, _activityClass, "postScreenshotToFacebook", "(Ljava/lang/String;II[I)V");
        _getImageData = getMethodSafe(env, _activityClass, "getImageData", "(Ljava/lang/String;)[I");
        _getImageDataFromBytes = getMethodSafe(env, _activityClass, "getImageDataFromBytes", "([B)[I");
		_readAssetFile = getMethodSafe(env, _activityClass, "getFileDataBytes", "(Ljava/lang/String;)[B");
        #if defined(PRE_ANDROID23)
            _methodPlaySound = getMethodSafe(env, _activityClass, "playSound", "(Ljava/lang/String;FF)V");
        #endif
        _showDialog = getMethodSafe(env, _activityClass, "displayDialog", "(I)V");
        _methodTick = getMethodSafe(env, _activityClass, "tick", "()V");
        _methodFinish = getMethodSafe(env, _activityClass, "quit", "()V");

        _methodUserInputInitiate = getMethodSafe(env, _activityClass, "initiateUserInput", "(I)V");
        _methodUserInputStatus = getMethodSafe(env, _activityClass, "getUserInputStatus", "()I");
        _methodUserInputString = getMethodSafe(env, _activityClass, "getUserInputString", "()[Ljava/lang/String;");

        _methodGetDateString = getMethodSafe(env, _activityClass, "getDateString", "(I)Ljava/lang/String;");

        _methodVibrate = getMethodSafe(env, _activityClass, "vibrate", "(I)V");
        _methodSupportsTouchscreen = getMethodSafe(env, _activityClass, "supportsTouchscreen", "()Z");
		_methodSetIsPowerVR  = getMethodSafe(env, _activityClass, "setIsPowerVR", "(Z)V");
		_methodIsNetworkEnabled = getMethodSafe(env, _activityClass, "isNetworkEnabled", "(Z)Z");

		_methodGetPixelsPerMillimeter = getMethodSafe(env, _activityClass, "getPixelsPerMillimeter", "()F");
		_methodGetPlatformStringVar = getMethodSafe(env, _activityClass, "getPlatformStringVar", "(I)Ljava/lang/String;");
		_methodOpenURL = getMethodSafe(env, _activityClass, "openURL", "(Ljava/lang/String;)V");
		_methodImportSkinOrCape = getMethodSafe(env, _activityClass, "importSkinOrCape", "(I)V");

		jclass winCls = env->FindClass("android/view/Window");
		if (winCls) {
			_classWindow = (jclass)env->NewGlobalRef(winCls);
			env->DeleteLocalRef(winCls);
		}
		jclass ctxCls = env->FindClass("android/content/Context");
		if (ctxCls) {
			_classContext = (jclass)env->NewGlobalRef(ctxCls);
			env->DeleteLocalRef(ctxCls);
		}
		jclass viewCls = env->FindClass("android/view/View");
		if (viewCls) {
			_classView = (jclass)env->NewGlobalRef(viewCls);
			env->DeleteLocalRef(viewCls);
		}
		jclass immCls = env->FindClass("android/view/inputmethod/InputMethodManager");
		if (immCls) {
			_classInputManager = (jclass)env->NewGlobalRef(immCls);
			env->DeleteLocalRef(immCls);
		}

		if (_classContext) {
			_fieldINPUT_METHOD_SERVICE = env->GetStaticFieldID(_classContext, "INPUT_METHOD_SERVICE", "Ljava/lang/String;");
			if (env->ExceptionOccurred()) env->ExceptionClear();
		}
		_methodGetSystemService = getMethodSafe(env, _activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
		_methodGetWindow = getMethodSafe(env, _activityClass, "getWindow", "()Landroid/view/Window;");
		if (_classWindow) {
			_methodGetDecorView = getMethodSafe(env, _classWindow, "getDecorView", "()Landroid/view/View;");
		}
		if (_classInputManager) {
			_methodShowSoftInput = getMethodSafe(env, _classInputManager, "showSoftInput", "(Landroid/view/View;I)Z");
			_methodHideSoftInput = getMethodSafe(env, _classInputManager, "hideSoftInputFromWindow", "(Landroid/os/IBinder;I)Z");
		}
		if (_classView) {
			_methodGetWindowToken = getMethodSafe(env, _classView, "getWindowToken", "()Landroid/os/IBinder;");
		}

		_methodGetKeyFromKeyCode = getMethodSafe(env, _activityClass, "getKeyFromKeyCode", "(III)I");

        if (env->ExceptionOccurred()) {
            env->ExceptionClear();
        }

        _vm = vm;
        _isInited = true;

        return JNI_VERSION_1_4;
    }

    int initConsts() {
        if (!instance) return 0;
        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return 0;

        jmethodID fWidth = getMethodSafe(env, _activityClass, "getScreenWidth", "()I");
        jmethodID fHeight = getMethodSafe(env, _activityClass, "getScreenHeight", "()I");

        if (fWidth) _screenWidth = env->CallIntMethod(instance, fWidth);
        if (fHeight) _screenHeight = env->CallIntMethod(instance, fHeight);

        return 1;
    }

    void tick() {
        if (!_isInited || !instance || !_methodTick) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        env->CallVoidMethod(instance, _methodTick);
    }

    void showDialog(int dialogId) {
        if (!_isInited || !instance || !_showDialog) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        env->CallVoidMethod(instance, _showDialog, dialogId);
    }

    void createUserInput() {
        if (!_isInited || !instance || !_methodUserInputInitiate) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        env->CallVoidMethod(instance, _methodUserInputInitiate, 1);
    }

    int getUserInputStatus() {
        if (!_isInited || !instance || !_methodUserInputStatus) return -2;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return -2;

        return env->CallIntMethod(instance, _methodUserInputStatus);
    }

    float getPixelsPerMillimeter() {
        if (!_isInited || !instance || !_methodGetPixelsPerMillimeter) return 10.0f;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return 10.0f;

        return env->CallFloatMethod(instance, _methodGetPixelsPerMillimeter);
    }

    StringVector getUserInput() {
        if (!_isInited || !instance || !_methodUserInputString) return std::vector<std::string>();

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return std::vector<std::string>();

        jobjectArray sa = (jobjectArray) env->CallObjectMethod(instance, _methodUserInputString);
        if (!sa) return std::vector<std::string>();
        jsize len = env->GetArrayLength(sa);

        StringVector out;
        for (int i = 0; i < len; ++i) {

            jstring s = (jstring)env->GetObjectArrayElement(sa, i);
            if (!s) continue;
            int length = env->GetStringLength (s);
            if (length > 1023)
                length = 1023;
            static char buf[1024];

            const char* str = env->GetStringUTFChars(s, 0);
            if (str) {
                strncpy(buf, str, 1023);
                buf[1023] = 0;
                env->ReleaseStringUTFChars(s, str);
                out.push_back(std::string(buf));
            }
            env->DeleteLocalRef(s);
        }
        env->DeleteLocalRef(sa);
        return out;
    }

    void saveScreenshot(const std::string& filename, int glWidth, int glHeight) {
        if (!_isInited || !_methodSaveScreenshot) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        const int numPixels = glWidth * glHeight;
        unsigned int* pixels = new unsigned int[numPixels];

        if (!pixels)
            return;

        glReadPixels(0, 0, glWidth, glHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        convertOpenGlToAndroidPixels(glWidth, glHeight, pixels);

        jintArray jPixels = env->NewIntArray(numPixels);
        if (jPixels != NULL)
        {
            jobject gpixRef = env->NewGlobalRef(jPixels);
            env->SetIntArrayRegion(jPixels, 0, numPixels, (int*)pixels);
            jstring fnStr = env->NewStringUTF(filename.c_str());
            env->CallStaticVoidMethod(  _activityClass,
                                        _methodSaveScreenshot,
                                        fnStr,
                                        glWidth, glHeight, jPixels);
            if (fnStr) env->DeleteLocalRef(fnStr);
            env->DeleteGlobalRef(gpixRef);
            delete[] pixels;
        }
    }

    __inline unsigned int rgbToBgr(unsigned int p) {
        return (p & 0xff00ff00) | ((p >> 16) & 0xff) | ((p << 16) & 0xff0000);
    }

    void convertOpenGlToAndroidPixels(int w, int h, unsigned int* pixels) {
        for (int y = 0; y < h/2; ++y)
        for (int x = 0; x < w;  ++x) {
            const int i0 = y * w + x;
            const int i1 = (h - y - 1) * w + x;
            const unsigned int p1 = pixels[i1];
            const unsigned int p0 = pixels[i0];
            pixels[i0] = rgbToBgr(p1);
            pixels[i1] = rgbToBgr(p0);
        }
        if (h & 1) {
            const int start = w * h/2;
            const int stop = start + w;
            for (int i = start; i < stop; ++i) {
                const unsigned int p = pixels[i];
                pixels[i] = rgbToBgr(p);
            }
        }
    }

    virtual void playSound(const std::string& filename, float volume, float pitch) override {
        if (!_isInited || !instance || !_methodPlaySound) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        jstring fnStr = env->NewStringUTF(filename.c_str());
        env->CallVoidMethod(instance, _methodPlaySound, fnStr, volume, pitch);
        if (fnStr) env->DeleteLocalRef(fnStr);
    }

    virtual void uploadPlatformDependentData(int id, void* data) override {
        if (!_isInited || !instance) return;
    }

    TextureData loadTextureFromMemory(const unsigned char* data, size_t size) override {
        if (!_isInited || !instance || !_getImageDataFromBytes || !data || size == 0) return TextureData();

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return TextureData();

        jbyteArray jbytes = env->NewByteArray(size);
        if (!jbytes) return TextureData();

        env->SetByteArrayRegion(jbytes, 0, size, (const jbyte*)data);

        jintArray arr = (jintArray)env->CallObjectMethod(instance, _getImageDataFromBytes, jbytes);
        env->DeleteLocalRef(jbytes);

        if (!arr) return TextureData();

        jsize len = env->GetArrayLength(arr);
        if (len < 2) {
            env->DeleteLocalRef(arr);
            return TextureData();
        }

        int numPixels = len - 2;
        int* pixels = new int[numPixels];
        jint* body = env->GetIntArrayElements(arr, 0);

        int w = body[0];
        int h = body[1];
        if (numPixels > 0 && body) {
            memcpy(pixels, &body[2], numPixels * sizeof(int));
        }

        for (int i = 0; i < numPixels; ++i)
            pixels[i] = rgbToBgr(pixels[i]);

        env->ReleaseIntArrayElements(arr, body, 0);
        env->DeleteLocalRef(arr);

        TextureData out;
        out.w = w;
        out.h = h;
        out.data = (unsigned char*)pixels;
        out.memoryHandledExternally = false;
        return out;
    }

    TextureData loadTexture(const std::string& filename, bool textureFolder) override {
        if (!_isInited) return TextureData();

        // First try reading directly from files directory if it exists on disk (e.g. data/images/skins/...)
        std::string diskPath = textureFolder ? "data/images/" + filename : filename;
        std::ifstream source(diskPath.c_str(), std::ios::binary);
        if (source) {
            std::vector<unsigned char> fileData((std::istreambuf_iterator<char>(source)), std::istreambuf_iterator<char>());
            source.close();
            if (!fileData.empty()) {
                return loadTextureFromMemory(fileData.data(), fileData.size());
            }
        }

        if (!instance || !_getImageData) return TextureData();

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return TextureData();

        std::string path = textureFolder ? "images/" + filename : filename;
        jstring pathStr = env->NewStringUTF(path.c_str());
        jintArray arr = (jintArray)env->CallObjectMethod(instance, _getImageData, pathStr);
        if (pathStr) env->DeleteLocalRef(pathStr);

        if (!arr)
            return TextureData();

        jsize len = env->GetArrayLength(arr);
        if (len < 2) {
            env->DeleteLocalRef(arr);
            return TextureData();
        }

        int numPixels = len - 2;

        int* pixels = new int[numPixels];
        jint* body = env->GetIntArrayElements(arr, 0);

        int w = body[0];
        int h = body[1];
        if (numPixels > 0 && body) {
            memcpy(pixels, &body[2], numPixels * sizeof(int));
        }

        for (int i = 0; i < numPixels; ++i)
            pixels[i] = rgbToBgr(pixels[i]);

        env->ReleaseIntArrayElements(arr, body, 0);
        env->DeleteLocalRef(arr);

        TextureData out;
        out.w = w;
        out.h = h;
        out.data = (unsigned char*)pixels;
        out.memoryHandledExternally = false;
        return out;
    }

	BinaryBlob readAssetFile(const std::string& filename) override {
		if (!_isInited || !instance || !_readAssetFile) return BinaryBlob();

		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return BinaryBlob();

		jstring fnStr = env->NewStringUTF(filename.c_str());
		jbyteArray arr = (jbyteArray)env->CallObjectMethod(instance, _readAssetFile, fnStr);
		if (fnStr) env->DeleteLocalRef(fnStr);

		if (!arr)
			return BinaryBlob();

		jsize len = env->GetArrayLength(arr);
		BinaryBlob blob(new unsigned char[len], len);
		jbyte* body = env->GetByteArrayElements(arr, 0);
		if (body) {
			memcpy(blob.data, body, len);
			env->ReleaseByteArrayElements(arr, body, 0);
		}
		env->DeleteLocalRef(arr);
		return blob;
	}

	std::string getDateString(int s) override {
        if (!_isInited || !instance || !_methodGetDateString) return "";

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return "";

        std::string out;
        jstring dateString = (jstring)env->CallObjectMethod(instance, _methodGetDateString, s);
        if (dateString) {
            const char* str = env->GetStringUTFChars(dateString, NULL);
            if (str) {
                out = str;
                env->ReleaseStringUTFChars(dateString, str);
            }
            env->DeleteLocalRef(dateString);
        }
        return out;
    }

	std::string getPlatformStringVar(int s) override {
		if (!_isInited || !instance || !_methodGetPlatformStringVar) return "";

		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return "";

		std::string out;
		jstring stringVar = (jstring)env->CallObjectMethod(instance, _methodGetPlatformStringVar, s);
		if (stringVar) {
			const char* str = env->GetStringUTFChars(stringVar, NULL);
			if (str) {
				out = str;
				env->ReleaseStringUTFChars(stringVar, str);
			}
			env->DeleteLocalRef(stringVar);
		}
		return out;
	}

	virtual void openURL(const std::string& url) override {
		if (!_isInited || !instance || !_methodOpenURL) return;
		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return;

		jstring jurl = env->NewStringUTF(url.c_str());
		env->CallVoidMethod(instance, _methodOpenURL, jurl);
		if (jurl) env->DeleteLocalRef(jurl);
	}

	virtual void importSkinOrCape(int type) override {
		if (!_isInited || !instance || !_methodImportSkinOrCape) return;
		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return;

		env->CallVoidMethod(instance, _methodImportSkinOrCape, type);
	}

    virtual void finish() override {
        if (!_isInited) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        if (instance && _methodFinish) {
            env->CallVoidMethod(instance, _methodFinish);
        }
        if (_activityClass) env->DeleteGlobalRef( _activityClass );
        if (_classWindow) env->DeleteGlobalRef( _classWindow );
        if (_classContext) env->DeleteGlobalRef( _classContext );
        if (_classView) env->DeleteGlobalRef( _classView );
        if (_classInputManager) env->DeleteGlobalRef( _classInputManager );
    }

    virtual bool supportsTouchscreen() override {
        if (!_isInited || !instance || !_methodSupportsTouchscreen) return true;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return true;

        return env->CallBooleanMethod(instance, _methodSupportsTouchscreen);
    }

	virtual void vibrate(int milliSeconds) override {
        if (!_isInited || !instance || !_methodVibrate) return;

        JVMAttacher ta(_vm);
        JNIEnv* env = ta.getEnv();
        if (!env) return;

        env->CallVoidMethod(instance, _methodVibrate, milliSeconds);
    }

	virtual bool isPowerVR() override {
	    bool is = super::isPowerVR();

		if (_methodSetIsPowerVR && _isInited && instance && !_hasSetPowerVR ) {
			JVMAttacher ta(_vm);
			JNIEnv* env = ta.getEnv();
			if (env) {
				_hasSetPowerVR = true;
				env->CallVoidMethod(instance, _methodSetIsPowerVR, is);
			}
		}
		return is;
	}

	virtual bool isNetworkEnabled(bool onlyWifiAllowed) override {
		if (!_isInited || !instance || !_methodIsNetworkEnabled) return false;

		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return false;

		return env->CallBooleanMethod(instance, _methodIsNetworkEnabled, onlyWifiAllowed);
	}

    static __inline bool isSquare(int n) {
        int L = n & 0xf;
        if (((1 << L) & 0x213) == 0) return false;

        int t = (int) sqrt((double) n) + 0.5;
        return t*t == n;
    }

	virtual void showKeyboard() override {
		showKeyboard(true);
		super::showKeyboard();
	}
	virtual void hideKeyboard() override {
		showKeyboard(false);
		super::hideKeyboard();
	}
	virtual void showKeyboard(bool bShow) {
		if (!_isInited || !instance || !_classContext || !_classInputManager || !_classWindow || !_classView) return;

		jint lFlags = 0;

		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return;

		jobject INPUT_METHOD_SERVICE =
			env->GetStaticObjectField(_classContext,
			_fieldINPUT_METHOD_SERVICE);

		if (!INPUT_METHOD_SERVICE) return;

		jobject lInputMethodManager = env->CallObjectMethod(
			instance, _methodGetSystemService,
			INPUT_METHOD_SERVICE);

		if (!lInputMethodManager) return;

		jobject lWindow = env->CallObjectMethod(instance,
			_methodGetWindow);

		if (!lWindow) return;

		jobject lDecorView = env->CallObjectMethod(lWindow,
			_methodGetDecorView);

		if (!lDecorView) return;

		if (bShow) {
			if (_methodShowSoftInput) {
				env->CallBooleanMethod(
					lInputMethodManager, _methodShowSoftInput,
					lDecorView, lFlags);
			}
		} else {
			if (_methodGetWindowToken && _methodHideSoftInput) {
				jobject lBinder = env->CallObjectMethod(lDecorView,
					_methodGetWindowToken);
				if (lBinder) {
					env->CallBooleanMethod(
						lInputMethodManager, _methodHideSoftInput,
						lBinder, lFlags);
				}
			}
		}
	}
	virtual int getKeyFromKeyCode(int keyCode, int metaState, int deviceId) override {
		if (!_isInited || !instance || !_methodGetKeyFromKeyCode) return 0;
		JVMAttacher ta(_vm);
		JNIEnv* env = ta.getEnv();
		if (!env) return 0;

		return (int) env->CallIntMethod(instance, _methodGetKeyFromKeyCode, keyCode, metaState, deviceId);
	}
public:
    jobject instance;

private:
    bool _isInited;
    JavaVM* _vm;

    jclass _activityClass;

    jmethodID _methodSaveScreenshot;
    jmethodID _postScreenshotToFacebook;
    jmethodID _getImageData;
    jmethodID _getImageDataFromBytes;
	jmethodID _readAssetFile;
    jmethodID _methodPlaySound;
    jmethodID _showDialog;
    jmethodID _methodTick;
    jmethodID _methodFinish;

    jmethodID _methodUserInputInitiate;
    jmethodID _methodUserInputStatus;
    jmethodID _methodUserInputString;

    jmethodID _methodGetDateString;

    jmethodID _getScreenWidth;
    jmethodID _getScreenHeight;
    jmethodID _methodGetPixelsPerMillimeter;
    jmethodID _methodVibrate;
    jmethodID _methodSupportsTouchscreen;
    jmethodID _methodSetIsPowerVR;
	jmethodID _methodIsNetworkEnabled;

	jmethodID _methodGetPlatformStringVar;
	jmethodID _methodOpenURL;
	jmethodID _methodImportSkinOrCape;

	jclass _classWindow;
	jclass _classContext;
	jfieldID _fieldINPUT_METHOD_SERVICE;
	jclass _classInputManager;
	jmethodID _methodGetSystemService;
	jmethodID _methodGetWindow;
	jmethodID _methodGetDecorView;
	jmethodID _methodShowSoftInput;
	jclass _classView;
	jmethodID _methodGetWindowToken;
	jmethodID _methodHideSoftInput;

	jmethodID _methodGetKeyFromKeyCode;

    int _screenWidth;
    int _screenHeight;
	bool _hasSetPowerVR;
public:
	ANativeActivity* _nativeActivity;
};

#endif /*APPPLATFORM_ANDROID_H__*/
