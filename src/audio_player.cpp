#include "audio_player.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <algorithm>

std::unique_ptr<AudioPlayer> AudioPlayer::create() {
    return std::make_unique<MiniaudioPlayer>();
}

bool MiniaudioPlayer::initialize() {
    engine_ = new ma_engine();
    ma_result result = ma_engine_init(nullptr, static_cast<ma_engine*>(engine_));
    return result == MA_SUCCESS;
}

void MiniaudioPlayer::setMaxConcurrentSounds(int maxSounds) {
    std::lock_guard<std::mutex> lock(soundsMutex_);
    maxConcurrentSounds_ = maxSounds;
}

void MiniaudioPlayer::cleanupFinishedSounds() {
    activeSounds_.erase(
        std::remove_if(activeSounds_.begin(), activeSounds_.end(),
            [](void* soundPtr) {
                ma_sound* sound = static_cast<ma_sound*>(soundPtr);
                if (!ma_sound_is_playing(sound)) {
                    ma_sound_uninit(sound);
                    delete sound;
                    return true;
                }
                return false;
            }),
        activeSounds_.end());
}

void MiniaudioPlayer::playSound(const std::string& filepath, bool async) {
    if (!engine_) return;
    
    std::lock_guard<std::mutex> lock(soundsMutex_);
    cleanupFinishedSounds();
    
    if (static_cast<int>(activeSounds_.size()) >= maxConcurrentSounds_) {
        return; // Skip if at limit
    }
    
    if (async) {
        ma_sound* sound = new ma_sound();
        ma_result result = ma_sound_init_from_file(static_cast<ma_engine*>(engine_), 
                                                   filepath.c_str(), 0, nullptr, nullptr, sound);
        if (result == MA_SUCCESS) {
            ma_sound_start(sound);
            activeSounds_.push_back(sound);
        } else {
            delete sound;
        }
    } else {
        // Synchronous - don't track, just play and wait
        ma_sound sound;
        ma_result result = ma_sound_init_from_file(static_cast<ma_engine*>(engine_), 
                                                   filepath.c_str(), 0, nullptr, nullptr, &sound);
        if (result == MA_SUCCESS) {
            ma_sound_start(&sound);
            while (ma_sound_is_playing(&sound)) {
                ma_sleep(1);
            }
            ma_sound_uninit(&sound);
        }
    }
}

void MiniaudioPlayer::cleanup() {
    if (engine_) {
        std::lock_guard<std::mutex> lock(soundsMutex_);
        for (void* soundPtr : activeSounds_) {
            ma_sound* sound = static_cast<ma_sound*>(soundPtr);
            ma_sound_uninit(sound);
            delete sound;
        }
        activeSounds_.clear();
        
        ma_engine_uninit(static_cast<ma_engine*>(engine_));
        delete static_cast<ma_engine*>(engine_);
        engine_ = nullptr;
    }
}

MiniaudioPlayer::~MiniaudioPlayer() {
    cleanup();
}



