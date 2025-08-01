#pragma once
#include <string>
#include <memory>
#include <vector>
#include <mutex>

class AudioPlayer {
public:
    static std::unique_ptr<AudioPlayer> create();
    virtual ~AudioPlayer() = default;
    
    virtual bool initialize() = 0;
    virtual void playSound(const std::string& filepath, bool async = true) = 0;
    virtual void cleanup() = 0;
    virtual void setMaxConcurrentSounds(int maxSounds) = 0;
};

class MiniaudioPlayer : public AudioPlayer {
private:
    void* engine_; // ma_engine*
    std::vector<void*> activeSounds_; // ma_sound*
    std::mutex soundsMutex_;
    int maxConcurrentSounds_ = 32;
    
    void cleanupFinishedSounds();
    
public:
    bool initialize() override;
    void playSound(const std::string& filepath, bool async = true) override;
    void cleanup() override;
    void setMaxConcurrentSounds(int maxSounds) override;
    ~MiniaudioPlayer();
};


