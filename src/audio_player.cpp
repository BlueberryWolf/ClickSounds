#include "audio_player.h"
#include "config.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio/miniaudio.h"
#include "miniaudio/ma_reverb_node.h"
#include <algorithm>
#include <chrono>
#include <random>
#include <iostream>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

std::unique_ptr<AudioPlayer> AudioPlayer::create() {
    return std::make_unique<MiniaudioPlayer>();
}

bool MiniaudioPlayer::initialize() {
    engine_ = new ma_engine();
    delayNode_ = nullptr;
    reverbNode_ = nullptr;
    
    ma_result result = ma_engine_init(nullptr, static_cast<ma_engine*>(engine_));
    if (result != MA_SUCCESS) {
        delete static_cast<ma_engine*>(engine_);
        engine_ = nullptr;
        return false;
    }
    
    return true;
}

void MiniaudioPlayer::setMaxConcurrentSounds(int maxSounds) {
    std::lock_guard<std::mutex> lock(soundsMutex_);
    maxConcurrentSounds_ = maxSounds;
}

void MiniaudioPlayer::setMasterVolume(float volume) {
    masterVolume_ = std::max(0.0f, std::min(1.0f, volume));
}

int MiniaudioPlayer::getCurrentTimeMs() {
#ifdef PLATFORM_WINDOWS
    return GetTickCount();
#else
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
#endif
}

void MiniaudioPlayer::cleanupFinishedSounds() {
    activeSounds_.erase(
        std::remove_if(activeSounds_.begin(), activeSounds_.end(),
            [](const SoundInstance& instance) {
                ma_sound* sound = static_cast<ma_sound*>(instance.sound);
                if (!ma_sound_is_playing(sound)) {
                    ma_sound_uninit(sound);
                    delete sound;
                    return true;
                }
                return false;
            }),
        activeSounds_.end());
}

void MiniaudioPlayer::updateFadingSounds() {
    int currentTime = getCurrentTimeMs();
    
    for (auto& instance : activeSounds_) {
        if (instance.fadingOut) {
            ma_sound* sound = static_cast<ma_sound*>(instance.sound);
            int elapsed = currentTime - instance.fadeStartTime;
            
            if (elapsed >= instance.fadeDuration) {
                // Fade complete, stop the sound
                ma_sound_stop(sound);
                instance.fadingOut = false;
            } else {
                // Calculate fade progress (0.0 to 1.0)
                float progress = static_cast<float>(elapsed) / instance.fadeDuration;
                float volume = instance.originalVolume * (1.0f - progress);
                ma_sound_set_volume(sound, volume);
            }
        }
    }
}

void MiniaudioPlayer::playSound(const std::string& filepath, bool async) {
    playSoundWithId(filepath, async); // Just call the ID version and ignore the ID
}

int MiniaudioPlayer::playSoundWithId(const std::string& filepath, bool async) {
    return playSoundWithIdAndVolume(filepath, 1.0f, async);
}

int MiniaudioPlayer::playSoundWithIdAndVolume(const std::string& filepath, float volume, bool async) {
    if (!engine_) return -1;
    
    std::lock_guard<std::mutex> lock(soundsMutex_);
    cleanupFinishedSounds();
    updateFadingSounds();
    
    if (static_cast<int>(activeSounds_.size()) >= maxConcurrentSounds_) {
        return -1; // Skip if at limit
    }
    
    // Calculate final volume (individual * master)
    float finalVolume = volume * masterVolume_;
    
    if (async) {
        ma_sound* sound = new ma_sound();
        ma_result result = ma_sound_init_from_file(static_cast<ma_engine*>(engine_), 
                                                   filepath.c_str(), 0, nullptr, nullptr, sound);
        if (result == MA_SUCCESS) {
            SoundInstance instance;
            instance.sound = sound;
            instance.id = nextSoundId_++;
            instance.originalVolume = finalVolume;
            
            // Set the volume
            ma_sound_set_volume(sound, finalVolume);
            
            // Apply spatial effects
            applySpatialEffects(sound, instance);
            
            // Route sound through effects chain if available
            if (effectsInitialized_) {
                ma_node* soundNode = sound;
                ma_node* endpoint = ma_engine_get_endpoint(static_cast<ma_engine*>(engine_));
                
                // Disconnect sound from default routing
                ma_node_detach_output_bus(soundNode, 0);
                
                // Route through effects chain: sound -> reverb -> delay -> endpoint
                ma_node* currentOutput = soundNode;
                
                if (reverbNode_) {
                    ma_node_attach_output_bus(currentOutput, 0, static_cast<ma_reverb_node*>(reverbNode_), 0);
                    currentOutput = static_cast<ma_reverb_node*>(reverbNode_);
                }
                
                if (delayNode_) {
                    ma_node_attach_output_bus(currentOutput, 0, static_cast<ma_delay_node*>(delayNode_), 0);
                    currentOutput = static_cast<ma_delay_node*>(delayNode_);
                }
                
                // Connect final output to endpoint
                ma_node_attach_output_bus(currentOutput, 0, endpoint, 0);
            }
            
            ma_sound_start(sound);
            activeSounds_.push_back(instance);
            
            return instance.id;
        } else {
            delete sound;
            return -1;
        }
    } else {
        // Synchronous - don't track, just play and wait
        ma_sound sound;
        ma_result result = ma_sound_init_from_file(static_cast<ma_engine*>(engine_), 
                                                   filepath.c_str(), 0, nullptr, nullptr, &sound);
        if (result == MA_SUCCESS) {
            ma_sound_set_volume(&sound, finalVolume);
            ma_sound_start(&sound);
            while (ma_sound_is_playing(&sound)) {
                ma_sleep(1);
            }
            ma_sound_uninit(&sound);
            return 0; // Synchronous sounds don't need tracking
        }
        return -1;
    }
}

void MiniaudioPlayer::fadeOutSound(int soundId, int durationMs) {
    if (!engine_ || soundId <= 0) return;
    
    std::lock_guard<std::mutex> lock(soundsMutex_);
    
    for (auto& instance : activeSounds_) {
        if (instance.id == soundId && !instance.fadingOut) {
            instance.fadingOut = true;
            instance.fadeStartTime = getCurrentTimeMs();
            instance.fadeDuration = durationMs;
            
            ma_sound* sound = static_cast<ma_sound*>(instance.sound);
            instance.originalVolume = ma_sound_get_volume(sound);
            break;
        }
    }
}

void MiniaudioPlayer::stopSound(int soundId) {
    if (!engine_ || soundId <= 0) return;
    
    std::lock_guard<std::mutex> lock(soundsMutex_);
    
    for (auto& instance : activeSounds_) {
        if (instance.id == soundId) {
            ma_sound* sound = static_cast<ma_sound*>(instance.sound);
            ma_sound_stop(sound);
            break;
        }
    }
}

void MiniaudioPlayer::update() {
    if (!engine_) return;
    
    std::lock_guard<std::mutex> lock(soundsMutex_);
    cleanupFinishedSounds();
    updateFadingSounds();
}

void MiniaudioPlayer::setAudioEffects(const AudioEffectsConfig& effects) {
    effectsConfig_ = const_cast<AudioEffectsConfig*>(&effects);
    
    // Always cleanup and reinitialize effects chain to handle config changes
    cleanupEffectsChain();
    
    // Initialize effects chain if any effects are enabled
    if (effects.enableEcho || effects.enableReverb) {
        if (initializeEffectsChain()) {
            std::cout << "Audio effects applied successfully" << std::endl;
        } else {
            std::cerr << "Failed to initialize audio effects" << std::endl;
        }
    } else {
        std::cout << "Audio effects disabled" << std::endl;
    }
}

bool MiniaudioPlayer::initializeEffectsChain() {
    if (!engine_ || effectsInitialized_) return effectsInitialized_;
    
    ma_engine* engine = static_cast<ma_engine*>(engine_);
    ma_node_graph* nodeGraph = ma_engine_get_node_graph(engine);
    ma_node* endpoint = ma_engine_get_endpoint(engine);
    
    // Clean up any existing effects first
    cleanupEffectsChain();
    
    ma_node* currentInput = endpoint;  // Start from the endpoint and work backwards
    
    // Initialize delay node for echo effects (last in chain, closest to output)
    if (effectsConfig_ && effectsConfig_->enableEcho) {
        delayNode_ = new ma_delay_node();
        ma_delay_node* delay = static_cast<ma_delay_node*>(delayNode_);
        
        ma_delay_node_config delayConfig = ma_delay_node_config_init(
            ma_engine_get_channels(engine),
            ma_engine_get_sample_rate(engine),
            (ma_uint32)(effectsConfig_->echoDelay * ma_engine_get_sample_rate(engine)), // delay in samples
            effectsConfig_->echoDecay
        );
        
        ma_result result = ma_delay_node_init(nodeGraph, &delayConfig, nullptr, delay);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize delay node: " << result << std::endl;
            delete delay;
            delayNode_ = nullptr;
            return false;
        }
        
        // Connect delay output to the current input (endpoint or next effect)
        ma_node_attach_output_bus(delay, 0, currentInput, 0);
        currentInput = delay;  // Now delay becomes the input for the next effect
        
        std::cout << "Echo effect initialized successfully" << std::endl;
    }
    
    // Initialize reverb node (first in chain, closest to input)
    if (effectsConfig_ && effectsConfig_->enableReverb) {
        reverbNode_ = new ma_reverb_node();
        ma_reverb_node* reverb = static_cast<ma_reverb_node*>(reverbNode_);
        
        ma_reverb_node_config reverbConfig = ma_reverb_node_config_init(
            ma_engine_get_channels(engine),
            ma_engine_get_sample_rate(engine)
        );
        
        // Configure reverb parameters based on the actual verblib API
        // Compensate for verblib's internal scaling (wet=3x, dry=2x)
        float wetness = effectsConfig_->reverbWetness;
        float wetLevel = wetness * (2.0f / 3.0f);  // Compensate for stronger wet scaling
        float dryLevel = 1.0f - wetness;           // Keep dry proportional to wetness
        
        reverbConfig.roomSize = effectsConfig_->reverbRoomSize;     // 0.0 to 1.0
        reverbConfig.damping = effectsConfig_->reverbDamping;       // 0.0 to 1.0
        reverbConfig.wetVolume = wetLevel;                          // Balanced wet signal level
        reverbConfig.dryVolume = dryLevel;                          // Balanced dry signal level
        reverbConfig.width = effectsConfig_->reverbWidth;           // Stereo width
        reverbConfig.mode = 0.0f;                                   // Normal mode (not frozen)
        
        ma_result result = ma_reverb_node_init(nodeGraph, &reverbConfig, nullptr, reverb);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize reverb node: " << result << std::endl;
            delete reverb;
            reverbNode_ = nullptr;
            return false;
        }
        
        // Connect reverb output to the current input (delay or endpoint)
        ma_node_attach_output_bus(reverb, 0, currentInput, 0);
        currentInput = reverb;  // Now reverb becomes the input for sounds
        
        std::cout << "Reverb effect initialized successfully" << std::endl;
    }
    
    // The effects chain is now set up and ready
    // Individual sounds will be routed through it in playSoundWithId
    if (currentInput != endpoint) {
        std::cout << "Effects chain established: sounds -> ";
        if (reverbNode_) std::cout << "reverb -> ";
        if (delayNode_) std::cout << "delay -> ";
        std::cout << "output" << std::endl;
    }
    
    effectsInitialized_ = true;
    return true;
}

void MiniaudioPlayer::cleanupEffectsChain() {
    if (delayNode_) {
        ma_delay_node_uninit(static_cast<ma_delay_node*>(delayNode_), nullptr);
        delete static_cast<ma_delay_node*>(delayNode_);
        delayNode_ = nullptr;
    }
    
    if (reverbNode_) {
        ma_reverb_node_uninit(static_cast<ma_reverb_node*>(reverbNode_), nullptr);
        delete static_cast<ma_reverb_node*>(reverbNode_);
        reverbNode_ = nullptr;
    }
    
    effectsInitialized_ = false;
}

void MiniaudioPlayer::updateReverbSettings() {
    if (!reverbNode_ || !effectsConfig_) return;
    
    ma_reverb_node* reverb = static_cast<ma_reverb_node*>(reverbNode_);
    verblib* verb = &reverb->reverb;
    
    // Compensate for verblib's internal scaling:
    // verblib_scalewet = 3.0f, verblib_scaledry = 2.0f
    // To get equal loudness at 0.5 wetness, we need to account for this difference
    float wetness = effectsConfig_->reverbWetness;
    float dryness = 1.0f - wetness;
    
    // Scale compensation: wet gets 3x boost, dry gets 2x boost
    // To balance them, we need wet_actual * 3.0 ≈ dry_actual * 2.0
    // So wet_actual = dry_actual * (2.0/3.0) = dry_actual * 0.667
    float wetLevel = wetness * (2.0f / 3.0f);  // Compensate for stronger wet scaling
    float dryLevel = dryness;                  // Keep dry as-is
    
    // Update verblib parameters directly
    verblib_set_room_size(verb, effectsConfig_->reverbRoomSize);
    verblib_set_damping(verb, effectsConfig_->reverbDamping);
    verblib_set_wet(verb, wetLevel);
    verblib_set_dry(verb, dryLevel);
    verblib_set_width(verb, 1.0f); // Keep stereo width at 1.0
}

void MiniaudioPlayer::applySpatialEffects(void* sound, SoundInstance& instance) {
    if (!effectsConfig_ || !effectsConfig_->enableSpatializer) return;
    
    ma_sound* maSound = static_cast<ma_sound*>(sound);
    
    if (effectsConfig_->randomSpatialPosition) {
        // Generate random 3D position within the spatial field
        std::uniform_real_distribution<float> dist(-effectsConfig_->spatialSpread, effectsConfig_->spatialSpread);
        instance.spatialX = dist(spatialRng_);
        instance.spatialY = dist(spatialRng_) * 0.5f; // Less vertical spread
        instance.spatialZ = effectsConfig_->listenerDistance + dist(spatialRng_) * 0.5f;
    }
    
    // Apply 3D positioning
    ma_sound_set_position(maSound, instance.spatialX, instance.spatialY, instance.spatialZ);
    ma_sound_set_spatialization_enabled(maSound, MA_TRUE);
}

void MiniaudioPlayer::cleanup() {
    if (engine_) {
        std::lock_guard<std::mutex> lock(soundsMutex_);
        
        // Stop and cleanup all active sounds
        for (const auto& instance : activeSounds_) {
            ma_sound* sound = static_cast<ma_sound*>(instance.sound);
            ma_sound_uninit(sound);
            delete sound;
        }
        activeSounds_.clear();
        
        // Cleanup effects chain
        cleanupEffectsChain();
        
        // Cleanup engine
        ma_engine_uninit(static_cast<ma_engine*>(engine_));
        delete static_cast<ma_engine*>(engine_);
        engine_ = nullptr;
    }
}

MiniaudioPlayer::~MiniaudioPlayer() {
    cleanup();
}
