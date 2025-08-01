#pragma once
#include <string>
#include <vector>
#include <unordered_set>
#include "nlohmann/json.hpp"

struct MouseConfig {
    bool enabled = true;
    std::string soundsDir;
    std::string leftDown, leftUp, rightDown, rightUp;
    std::string middleDown, middleUp;
    std::string x1Down, x1Up, x2Down, x2Up;
    std::string wheelUp, wheelDown;
    bool enableScrollWheel = false;
    bool enableSideButtons = false;
    bool enableFadeOut = false;
    int fadeOutDurationMs = 50;
    int scrollWheelDebounceMs = 50;
    float volume = 1.0f; // 0.0 to 1.0
};

struct KeyboardConfig {
    bool enabled = true;
    std::string soundsDir;
    bool randomSounds = true;
    bool totallyRandomKeypresses = false;
    bool disableRepeat = false;
    bool enableFadeOut = false;
    int fadeOutDurationMs = 50;
    int keyRepeatDebounceMs = 50;
    float volume = 1.0f; // 0.0 to 1.0
    std::unordered_set<int> noRepeatKeys;
    std::vector<std::string> sounds;
    std::unordered_set<int> excludedKeys;
};

struct AudioEffectsConfig {
    bool enableReverb = false;
    float reverbWetness = 0.3f;        // 0.0 = dry, 1.0 = fully wet
    float reverbRoomSize = 0.5f;       // 0.0 = small room, 1.0 = large hall
    float reverbDecayTime = 1.0f;      // Decay time in seconds
    float reverbDamping = 0.5f;        // High frequency damping
    float reverbWidth = 1.0f;          // Stereo width
    
    bool enableEcho = false;
    float echoDelay = 0.3f;            // Echo delay in seconds
    float echoDecay = 0.4f;            // Echo volume decay (0.0-1.0)
    int echoTaps = 3;                  // Number of echo repetitions
    
    bool enableSpatializer = false;
    bool randomSpatialPosition = true; // Randomize 3D position for each sound
    float spatialSpread = 2.0f;        // How wide the spatial field is
    float listenerDistance = 1.0f;    // Distance from listener to sound field
};

struct AudioConfig {
    bool asyncPlayback = true;
    int maxConcurrentSounds = 32;
    float masterVolume = 1.0f; // 0.0 to 1.0 - overall volume control
    AudioEffectsConfig effects;
};

struct Config {
    MouseConfig mouse;
    KeyboardConfig keyboard;
    AudioConfig audio;
    
    static Config loadFromFile(const std::string& filepath);
    
    // Reload config from the same file path
    bool reload();
    
    // Get the file path used to load this config
    const std::string& getFilePath() const { return filepath_; }
    
private:
    static std::vector<std::string> loadSoundsFromDirectory(const std::string& dir);
    void parseFromJson(const nlohmann::json& j);
    std::string filepath_; // Store the file path for reloading
};
