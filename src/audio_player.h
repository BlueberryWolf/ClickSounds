#pragma once
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <random>

// Forward declaration
struct AudioEffectsConfig;

class AudioPlayer {
public:
    static std::unique_ptr<AudioPlayer> create();
    virtual ~AudioPlayer() = default;
    
    virtual bool initialize() = 0;
    virtual void playSound(const std::string& filepath, bool async = true) = 0;
    virtual int playSoundWithId(const std::string& filepath, bool async = true) = 0;
    virtual int playSoundWithIdAndVolume(const std::string& filepath, float volume, bool async = true) = 0;
    virtual void fadeOutSound(int soundId, int durationMs) = 0;
    virtual void stopSound(int soundId) = 0;
    virtual void update() = 0; // Regular update for fade processing
    virtual void cleanup() = 0;
    virtual void setMaxConcurrentSounds(int maxSounds) = 0;
    virtual void setAudioEffects(const AudioEffectsConfig& effects) = 0;
    virtual void setMasterVolume(float volume) = 0;
};

struct SoundInstance {
    void* sound; // ma_sound*
    int id;
    bool fadingOut = false;
    float originalVolume = 1.0f;
    int fadeStartTime = 0;
    int fadeDuration = 0;
    float spatialX = 0.0f;
    float spatialY = 0.0f;
    float spatialZ = 0.0f;
};

class MiniaudioPlayer : public AudioPlayer {
private:
    void* engine_; // ma_engine*
    void* delayNode_; // ma_delay_node*
    void* reverbNode_; // ma_reverb_node*
    std::vector<SoundInstance> activeSounds_;
    std::mutex soundsMutex_;
    int maxConcurrentSounds_ = 32;
    int nextSoundId_ = 1;
    AudioEffectsConfig* effectsConfig_ = nullptr;
    std::mt19937 spatialRng_;
    bool effectsInitialized_ = false;
    float masterVolume_ = 1.0f;
    
    void cleanupFinishedSounds();
    void updateFadingSounds();
    int getCurrentTimeMs();
    void applySpatialEffects(void* sound, SoundInstance& instance);
    bool initializeEffectsChain();
    void cleanupEffectsChain();
    void updateReverbSettings();
    
public:
    MiniaudioPlayer() : spatialRng_(std::random_device{}()) {}
    bool initialize() override;
    void playSound(const std::string& filepath, bool async = true) override;
    int playSoundWithId(const std::string& filepath, bool async = true) override;
    int playSoundWithIdAndVolume(const std::string& filepath, float volume, bool async = true) override;
    void fadeOutSound(int soundId, int durationMs) override;
    void stopSound(int soundId) override;
    void update() override;
    void cleanup() override;
    void setMaxConcurrentSounds(int maxSounds) override;
    void setAudioEffects(const AudioEffectsConfig& effects) override;
    void setMasterVolume(float volume) override;
    ~MiniaudioPlayer();
};
