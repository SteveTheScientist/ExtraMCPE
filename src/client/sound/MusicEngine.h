#ifndef NET_MINECRAFT_CLIENT_SOUND__MusicEngine_H__
#define NET_MINECRAFT_CLIENT_SOUND__MusicEngine_H__

#include <string>
#include <vector>
#include "../../util/Random.h"

#if defined(ANDROID) && !defined(PRE_ANDROID23)
#include "../../platform/audio/SoundSystemSL.h"
#elif (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
#include "../../platform/audio/SoundSystemAL.h"
#else
#include "../../platform/audio/SoundSystem.h"
#endif

struct stb_vorbis;
class Minecraft;
class Options;

class MusicEngine
{
public:
    MusicEngine();
    ~MusicEngine();

    void init(Minecraft* mc, Options* options);
    void tick();
    void updateVolume();
    void onAppPause();
    void onAppResume();

    bool playTrack(const std::string& trackNameOrPath);
    const std::vector<std::string>& getTracks() const { return tracks; }
    std::vector<std::string> getTrackNamesWithoutExtension() const;

private:
    void loadTrackList();
    void pickNextTrack();
    void startTrack(const std::string& trackName);
    void stopTrack();
    void updateStreamBuffers();

    Minecraft* mc;
    Options* options;
    Random random;

    std::vector<std::string> tracks;
    std::string currentTrack;
    std::string lastTrack;

    int delayTimer;
    bool isWorldActive;
    bool isAppPaused;

    stb_vorbis* vorbisDecoder;
    FILE* vorbisFile;
    unsigned char* trackBlobData;
    size_t trackBlobSize;
    void* memStreamPtr;

    int vorbisChannels;
    int vorbisSampleRate;
    bool isPlaying;
    bool isDisabled;

#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    ALuint streamingSource;
    ALuint streamingBuffers[3];

    bool fillBuffer(ALuint bufferID);
#endif
};

#endif /* NET_MINECRAFT_CLIENT_SOUND__MusicEngine_H__ */
