#include "config.h"
#include "key_mapping.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

std::vector<std::string> Config::loadSoundsFromDirectory(const std::string& dir) {
    std::vector<std::string> sounds;
    
    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        std::cerr << "Directory not found: " << dir << std::endl;
        return sounds;
    }
    
    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            auto ext = entry.path().extension().string();
            if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || 
                ext == ".flac" || ext == ".m4a") {
                sounds.push_back(entry.path().string());
            }
        }
    }
    
    return sounds;
}

Config Config::loadFromFile(const std::string& filepath) {
    Config config;
    config.filepath_ = filepath; // Store the file path for reloading
    
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Could not open config file: " << filepath << std::endl;
            return config;
        }
        
        json j;
        file >> j;
        
        config.parseFromJson(j);
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
    }
    
    return config;
}

bool Config::reload() {
    if (filepath_.empty()) {
        std::cerr << "Cannot reload config: no file path stored" << std::endl;
        return false;
    }
    
    try {
        std::ifstream file(filepath_);
        if (!file.is_open()) {
            std::cerr << "Could not open config file for reload: " << filepath_ << std::endl;
            return false;
        }
        
        json j;
        file >> j;
        
        // Clear existing data before parsing
        mouse = MouseConfig{};
        keyboard = KeyboardConfig{};
        audio = AudioConfig{};
        
        parseFromJson(j);
        
        std::cout << "Config reloaded successfully from: " << filepath_ << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Error reloading config: " << e.what() << std::endl;
        return false;
    }
}

void Config::parseFromJson(const nlohmann::json& j) {
    // Mouse config
    if (j.contains("mouse")) {
        auto& mouse_json = j["mouse"];
        mouse.enabled = mouse_json.value("enabled", true);
        mouse.soundsDir = mouse_json.value("sounds_dir", "sounds");
        mouse.enableScrollWheel = mouse_json.value("enable_scroll_wheel", false);
        mouse.enableSideButtons = mouse_json.value("enable_side_buttons", false);
        mouse.enableFadeOut = mouse_json.value("enable_fade_out", false);
        mouse.fadeOutDurationMs = mouse_json.value("fade_out_duration_ms", 50);
        mouse.scrollWheelDebounceMs = mouse_json.value("scroll_wheel_debounce_ms", 50);
        mouse.volume = mouse_json.value("volume", 1.0f);
        
        if (mouse_json.contains("sounds")) {
            auto& sounds = mouse_json["sounds"];
            mouse.leftDown = mouse.soundsDir + "/" + sounds.value("left_down", "");
            mouse.leftUp = mouse.soundsDir + "/" + sounds.value("left_up", "");
            mouse.rightDown = mouse.soundsDir + "/" + sounds.value("right_down", "");
            mouse.rightUp = mouse.soundsDir + "/" + sounds.value("right_up", "");
            mouse.middleDown = mouse.soundsDir + "/" + sounds.value("middle_down", "");
            mouse.middleUp = mouse.soundsDir + "/" + sounds.value("middle_up", "");
            mouse.x1Down = mouse.soundsDir + "/" + sounds.value("x1_down", "");
            mouse.x1Up = mouse.soundsDir + "/" + sounds.value("x1_up", "");
            mouse.x2Down = mouse.soundsDir + "/" + sounds.value("x2_down", "");
            mouse.x2Up = mouse.soundsDir + "/" + sounds.value("x2_up", "");
            mouse.wheelUp = mouse.soundsDir + "/" + sounds.value("wheel_up", "");
            mouse.wheelDown = mouse.soundsDir + "/" + sounds.value("wheel_down", "");
        }
    }
    
    // Keyboard config
    if (j.contains("keyboard")) {
        auto& keyboard_json = j["keyboard"];
        keyboard.enabled = keyboard_json.value("enabled", true);
        keyboard.soundsDir = keyboard_json.value("sounds_dir", "sounds");
        keyboard.randomSounds = keyboard_json.value("random_sounds", true);
        keyboard.totallyRandomKeypresses = keyboard_json.value("totally_random_keypresses", false);
        keyboard.disableRepeat = keyboard_json.value("disable_repeat", false);
        keyboard.enableFadeOut = keyboard_json.value("enable_fade_out", false);
        keyboard.fadeOutDurationMs = keyboard_json.value("fade_out_duration_ms", 50);
        keyboard.keyRepeatDebounceMs = keyboard_json.value("key_repeat_debounce_ms", 50);
        keyboard.volume = keyboard_json.value("volume", 1.0f);
        
        if (keyboard.randomSounds) {
            keyboard.sounds = loadSoundsFromDirectory(keyboard.soundsDir);
        }
        
        if (keyboard_json.contains("no_repeat_keys")) {
            for (const auto& key : keyboard_json["no_repeat_keys"]) {
                if (key.is_string()) {
                    int keyCode = KeyMapping::getKeyCode(key.get<std::string>());
                    if (keyCode != -1) {
                        keyboard.noRepeatKeys.insert(keyCode);
                    }
                } else if (key.is_number()) {
                    keyboard.noRepeatKeys.insert(key.get<int>());
                }
            }
        }
        
        if (keyboard_json.contains("excluded_keys")) {
            for (const auto& key : keyboard_json["excluded_keys"]) {
                if (key.is_string()) {
                    int keyCode = KeyMapping::getKeyCode(key.get<std::string>());
                    if (keyCode != -1) {
                        keyboard.excludedKeys.insert(keyCode);
                    }
                } else if (key.is_number()) {
                    keyboard.excludedKeys.insert(key.get<int>());
                }
            }
        }
    }
    
    // Audio config
    if (j.contains("audio")) {
        auto& audio_json = j["audio"];
        audio.asyncPlayback = audio_json.value("async_playback", true);
        audio.maxConcurrentSounds = audio_json.value("max_concurrent_sounds", 32);
        audio.masterVolume = audio_json.value("master_volume", 1.0f);
        
        // Audio effects config
        if (audio_json.contains("effects")) {
            auto& effects = audio_json["effects"];
            audio.effects.enableReverb = effects.value("enable_reverb", false);
            audio.effects.reverbWetness = effects.value("reverb_wetness", 0.3f);
            audio.effects.reverbRoomSize = effects.value("reverb_room_size", 0.5f);
            audio.effects.reverbDecayTime = effects.value("reverb_decay_time", 1.0f);
            audio.effects.reverbDamping = effects.value("reverb_damping", 0.5f);
            audio.effects.reverbWidth = effects.value("reverb_width", 1.0f);
            
            audio.effects.enableEcho = effects.value("enable_echo", false);
            audio.effects.echoDelay = effects.value("echo_delay", 0.3f);
            audio.effects.echoDecay = effects.value("echo_decay", 0.4f);
            audio.effects.echoTaps = effects.value("echo_taps", 3);
            
            audio.effects.enableSpatializer = effects.value("enable_spatializer", false);
            audio.effects.randomSpatialPosition = effects.value("random_spatial_position", true);
            audio.effects.spatialSpread = effects.value("spatial_spread", 2.0f);
            audio.effects.listenerDistance = effects.value("listener_distance", 1.0f);
        }
    }
}
