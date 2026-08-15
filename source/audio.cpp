#include "audio.h"
#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include <mutex>
#include <map>
#include <stdio.h> 

static SoLoud::Soloud gSoloud;

static std::map<int, SoLoud::AudioSource*> gAudioCache;
static std::mutex gAudioMutex;
static SoLoud::handle gMusicHandle = 0;

void audio_init() {
    gSoloud.init();
}

void audio_cleanup() {
    std::lock_guard<std::mutex> lock(gAudioMutex);

    gSoloud.stopAll();

    for (auto const& [id, source] : gAudioCache) {
        delete source;
    }
    gAudioCache.clear();

    gSoloud.deinit();
}

void audio_play_sound(int sndID, int vol, int isLoop) {
    
    std::lock_guard<std::mutex> lock(gAudioMutex);

    if (gAudioCache.find(sndID) == gAudioCache.end()) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), DATA_PATH "/res/raw/s%03d.wav", sndID);

        SoLoud::AudioSource* newSource = nullptr;

        if (sndID < 100) {
            SoLoud::Wav* sfx = new SoLoud::Wav();
            sfx->load(filepath);
            newSource = sfx;
        } else {
            SoLoud::WavStream* music = new SoLoud::WavStream();
            music->load(filepath);
            newSource = music;
        }

        gAudioCache[sndID] = newSource;
    }

    SoLoud::AudioSource* sourceToPlay = gAudioCache[sndID];
    if (sourceToPlay != nullptr) {
        if (sndID >= 100) {
            if (gMusicHandle != 0) {
                gSoloud.stop(gMusicHandle);
            }
            gMusicHandle = gSoloud.play(*sourceToPlay);
            gSoloud.setVolume(gMusicHandle, (float)vol / 100.0f);
            gSoloud.setLooping(gMusicHandle, isLoop != 0);
        } else {
            SoLoud::handle handle = gSoloud.play(*sourceToPlay);
            gSoloud.setVolume(handle, (float)vol / 100.0f);
            gSoloud.setLooping(handle, isLoop != 0);
        }

    }
}

void audio_stop_sound() {
    std::lock_guard<std::mutex> lock(gAudioMutex);
    gSoloud.stopAll();
}