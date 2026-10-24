#include "MusicEngine.h"
#include "../Minecraft.h"
#include "../Options.h"
#include "../../platform/log.h"
#include "../../world/level/storage/LevelData.h"
#include "../../world/level/Level.h"
#include "../../AppPlatform.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "stb_vorbis.c"

#define BUFFER_SAMPLES 4096

// Custom stb_vorbis callbacks for reading from memory buffer
typedef struct {
    const unsigned char* data;
    size_t size;
    size_t offset;
} MemStream;

static size_t mem_read(void* ptr, size_t byteSize, size_t sizeToRead, void* datasource) {
    MemStream* ms = (MemStream*)datasource;
    size_t bytesRequested = byteSize * sizeToRead;
    size_t bytesLeft = ms->size - ms->offset;
    size_t bytesToCopy = (bytesRequested < bytesLeft) ? bytesRequested : bytesLeft;
    if (bytesToCopy > 0) {
        memcpy(ptr, ms->data + ms->offset, bytesToCopy);
        ms->offset += bytesToCopy;
    }
    return bytesToCopy / byteSize;
}

static void mem_seek(void* datasource, int offset, int whence) {
    MemStream* ms = (MemStream*)datasource;
    if (whence == SEEK_SET) {
        ms->offset = (size_t)offset;
    } else if (whence == SEEK_CUR) {
        ms->offset = (size_t)((int)ms->offset + offset);
    } else if (whence == SEEK_END) {
        ms->offset = (size_t)((int)ms->size + offset);
    }
    if (ms->offset > ms->size) {
        ms->offset = ms->size;
    }
}

static void mem_close(void* datasource) {
    // Nothing needed
}

static int mem_tell(void* datasource) {
    MemStream* ms = (MemStream*)datasource;
    return (int)ms->offset;
}

static stb_vorbis_alloc mem_alloc_struct = { NULL, 0 };

MusicEngine::MusicEngine()
: mc(NULL),
  options(NULL),
  delayTimer(0),
  isWorldActive(false),
  isAppPaused(false),
  vorbisDecoder(NULL),
  vorbisFile(NULL),
  trackBlobData(NULL),
  trackBlobSize(0),
  memStreamPtr(NULL),
  vorbisChannels(0),
  vorbisSampleRate(0),
  isPlaying(false)
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
  , streamingSource(0)
#endif
{
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    streamingBuffers[0] = 0;
    streamingBuffers[1] = 0;
    streamingBuffers[2] = 0;
#endif
}

MusicEngine::~MusicEngine() {
    stopTrack();
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    if (streamingSource != 0) {
        alDeleteSources(1, &streamingSource);
        streamingSource = 0;
    }
    if (streamingBuffers[0] != 0) {
        alDeleteBuffers(3, streamingBuffers);
        streamingBuffers[0] = streamingBuffers[1] = streamingBuffers[2] = 0;
    }
#endif
}

void MusicEngine::init(Minecraft* mc, Options* options) {
    this->mc = mc;
    this->options = options;

#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    alGenSources(1, &streamingSource);
    alGenBuffers(3, streamingBuffers);
#endif

    loadTrackList();
}

void MusicEngine::loadTrackList() {
    tracks.clear();

    std::string trackPath = "data/sounds/background/tracks.txt";
    BinaryBlob blob = mc->platform()->readAssetFile(trackPath);
    if (blob.size <= 0 || !blob.data) {
        trackPath = "data/sound/background/tracks.txt";
        blob = mc->platform()->readAssetFile(trackPath);
    }

    if (blob.size > 0 && blob.data) {
        std::string content((char*)blob.data, blob.size);
        std::istringstream iss(content);
        std::string line;
        while (std::getline(iss, line)) {
            while (!line.empty() && (line[line.size() - 1] == '\r' || line[line.size() - 1] == '\n' || line[line.size() - 1] == ' ')) {
                line.erase(line.size() - 1);
            }
            if (!line.empty()) {
                tracks.push_back(line);
            }
        }
        delete[] blob.data;
    }

    if (tracks.empty()) {
        tracks.push_back("aria_math_hardstyle.ogg");
        tracks.push_back("featherfall.ogg");
        tracks.push_back("komorebi.ogg");
        tracks.push_back("Precipice.ogg");
        tracks.push_back("Puzzlebox [ray1Svv6Sl8].ogg");
    }

    LOGI("MusicEngine loaded %d tracks.\n", (int)tracks.size());
}

void MusicEngine::tick() {
    if (!mc || !options) return;

    bool worldInGame = (mc->level != NULL);

    if (!worldInGame) {
        if (isWorldActive) {
            stopTrack();
            isWorldActive = false;
            delayTimer = 0;
        }
        return;
    }

    if (!isWorldActive) {
        isWorldActive = true;
        delayTimer = 5 * 20; // 5 seconds (100 ticks) after joining a world
    }

    if (isAppPaused) return;

    float musicVol = options->getProgressValue(OPTIONS_MUSIC_VOLUME);
    if (musicVol <= 0.0f) {
        if (isPlaying) {
            stopTrack();
        }
        return;
    }

    updateVolume();

    if (isPlaying) {
        updateStreamBuffers();
    } else {
        if (delayTimer > 0) {
            delayTimer--;
        } else {
            pickNextTrack();
        }
    }
}

void MusicEngine::updateVolume() {
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    if (streamingSource != 0 && options != NULL) {
        float musicVol = options->getProgressValue(OPTIONS_MUSIC_VOLUME);
        alSourcef(streamingSource, AL_GAIN, musicVol);
    }
#endif
}

void MusicEngine::onAppPause() {
    isAppPaused = true;
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    if (isPlaying && streamingSource != 0) {
        alSourcePause(streamingSource);
    }
#endif
}

void MusicEngine::onAppResume() {
    isAppPaused = false;
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    if (isPlaying && streamingSource != 0) {
        ALint state;
        alGetSourcei(streamingSource, AL_SOURCE_STATE, &state);
        if (state == AL_PAUSED) {
            alSourcePlay(streamingSource);
        }
    }
#endif
}

void MusicEngine::pickNextTrack() {
    if (tracks.empty()) return;

    std::string candidate;
    if (tracks.size() == 1) {
        candidate = tracks[0];
    } else {
        do {
            int idx = random.nextInt(tracks.size());
            candidate = tracks[idx];
        } while (candidate == lastTrack);
    }

    startTrack(candidate);
}

void MusicEngine::startTrack(const std::string& trackName) {
    stopTrack();

    currentTrack = trackName;
    lastTrack = trackName;

    int error = 0;

    // First try opening directly via stdio FILE*
    std::string fullPath = "data/sounds/background/" + trackName;
    vorbisFile = fopen(fullPath.c_str(), "rb");
    if (!vorbisFile) {
        fullPath = "data/sound/background/" + trackName;
        vorbisFile = fopen(fullPath.c_str(), "rb");
    }

    if (vorbisFile) {
        vorbisDecoder = stb_vorbis_open_file(vorbisFile, FALSE, &error, NULL);
    } else {
        // Fall back to platform readAssetFile for packaged assets (e.g. Android APK or iOS bundle)
        std::string assetPath = "data/sounds/background/" + trackName;
        BinaryBlob blob = mc->platform()->readAssetFile(assetPath);
        if (blob.size <= 0 || !blob.data) {
            assetPath = "data/sound/background/" + trackName;
            blob = mc->platform()->readAssetFile(assetPath);
        }

        if (blob.size > 0 && blob.data) {
            trackBlobData = blob.data;
            trackBlobSize = blob.size;

            MemStream* ms = new MemStream();
            ms->data = trackBlobData;
            ms->size = trackBlobSize;
            ms->offset = 0;
            memStreamPtr = ms;

            stb_vorbis_alloc alloc;
            alloc.alloc_buffer = NULL;
            alloc.alloc_buffer_length_in_bytes = 0;

            vorbisDecoder = stb_vorbis_open_memory(trackBlobData, (int)trackBlobSize, &error, NULL);
        }
    }

    if (!vorbisDecoder) {
        LOGE("MusicEngine: Failed to decode OGG Vorbis file %s (error %d). Skipping.\n", trackName.c_str(), error);
        stopTrack();
        delayTimer = 200; // Retry next track in 10 seconds
        return;
    }

    stb_vorbis_info info = stb_vorbis_get_info(vorbisDecoder);
    vorbisChannels = info.channels;
    vorbisSampleRate = info.sample_rate;

#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    alSourcei(streamingSource, AL_BUFFER, 0);

    int queuedBuffers = 0;
    for (int i = 0; i < 3; ++i) {
        if (fillBuffer(streamingBuffers[i])) {
            queuedBuffers++;
        }
    }

    if (queuedBuffers == 0) {
        LOGE("MusicEngine: Could not read sample data from %s. Skipping.\n", trackName.c_str());
        stopTrack();
        delayTimer = 200;
        return;
    }

    alSourceQueueBuffers(streamingSource, queuedBuffers, streamingBuffers);
    updateVolume();
    alSourcePlay(streamingSource);

    isPlaying = true;
    LOGI("MusicEngine: Started playing %s\n", trackName.c_str());
#else
    // On non-OpenAL platforms (such as Android OpenSL or Web), decoding is verified without crash
    isPlaying = true;
    LOGI("MusicEngine: Started streaming track %s\n", trackName.c_str());
#endif
}

#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
bool MusicEngine::fillBuffer(ALuint bufferID) {
    if (!vorbisDecoder) return false;

    short pcmBuffer[BUFFER_SAMPLES];
    int samplesRead = stb_vorbis_get_samples_short_interleaved(vorbisDecoder, vorbisChannels, pcmBuffer, BUFFER_SAMPLES);

    if (samplesRead <= 0) {
        return false;
    }

    ALenum format = (vorbisChannels == 2) ? AL_FORMAT_STEREO16 : AL_FORMAT_MONO16;
    alBufferData(bufferID, format, pcmBuffer, samplesRead * vorbisChannels * sizeof(short), vorbisSampleRate);
    return true;
}

void MusicEngine::updateStreamBuffers() {
    if (!isPlaying) return;

    ALint processed = 0;
    alGetSourcei(streamingSource, AL_BUFFERS_PROCESSED, &processed);

    while (processed > 0) {
        ALuint bufferID = 0;
        alSourceUnqueueBuffers(streamingSource, 1, &bufferID);
        processed--;

        if (fillBuffer(bufferID)) {
            alSourceQueueBuffers(streamingSource, 1, &bufferID);
        }
    }

    ALint state = 0;
    alGetSourcei(streamingSource, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING && state != AL_PAUSED) {
        ALint queued = 0;
        alGetSourcei(streamingSource, AL_BUFFERS_QUEUED, &queued);
        if (queued > 0) {
            alSourcePlay(streamingSource);
        } else {
            // Track finished!
            stopTrack();

            bool isCreative = (mc->level && mc->level->getLevelData()) ? (mc->level->getLevelData()->getGameType() == GameType::Creative) : mc->isCreativeMode();

            if (isCreative) {
                delayTimer = 1200 + random.nextInt(2401); // 1200 to 3600 ticks (1 to 3 minutes)
            } else {
                delayTimer = 12000 + random.nextInt(12001); // 12000 to 24000 ticks (10 to 20 minutes)
            }

            LOGI("MusicEngine: Track finished. Next track in %d ticks.\n", delayTimer);
        }
    }
}
#else
void MusicEngine::updateStreamBuffers() {}
#endif

void MusicEngine::stopTrack() {
#if (defined(__APPLE__) || defined(PLATFORM_DESKTOP)) && !defined(NO_SOUND)
    if (streamingSource != 0) {
        alSourceStop(streamingSource);
        alSourcei(streamingSource, AL_BUFFER, 0);
    }
#endif
    if (vorbisDecoder) {
        stb_vorbis_close(vorbisDecoder);
        vorbisDecoder = NULL;
    }
    if (vorbisFile) {
        fclose(vorbisFile);
        vorbisFile = NULL;
    }
    if (memStreamPtr) {
        delete (MemStream*)memStreamPtr;
        memStreamPtr = NULL;
    }
    if (trackBlobData) {
        delete[] trackBlobData;
        trackBlobData = NULL;
    }
    trackBlobSize = 0;

    isPlaying = false;
    currentTrack = "";
}
