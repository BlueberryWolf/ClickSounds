#pragma once
#include <string>
#include <vector>
#include <unordered_set>

struct MouseConfig {
    bool enabled = true;
    std::string soundsDir;
    std::string leftDown, leftUp, rightDown, rightUp;
};

struct KeyboardConfig {
    bool enabled = true;
    std::string soundsDir;
    bool randomSounds = true;
    bool totallyRandomKeypresses = false;
    bool disableRepeat = false;
    std::unordered_set<int> noRepeatKeys;
    std::vector<std::string> sounds;
    std::unordered_set<int> excludedKeys;
};

struct AudioConfig {
    bool asyncPlayback = true;
    int maxConcurrentSounds = 32;
};

struct Config {
    MouseConfig mouse;
    KeyboardConfig keyboard;
    AudioConfig audio;
    
    static Config loadFromFile(const std::string& filepath);
private:
    static std::vector<std::string> loadSoundsFromDirectory(const std::string& dir);
};


