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
    
    try {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "Could not open config file: " << filepath << std::endl;
            return config;
        }
        
        json j;
        file >> j;
        
        // Mouse config
        if (j.contains("mouse")) {
            auto& mouse = j["mouse"];
            config.mouse.enabled = mouse.value("enabled", true);
            config.mouse.soundsDir = mouse.value("sounds_dir", "sounds");
            
            if (mouse.contains("sounds")) {
                auto& sounds = mouse["sounds"];
                config.mouse.leftDown = config.mouse.soundsDir + "/" + sounds.value("left_down", "");
                config.mouse.leftUp = config.mouse.soundsDir + "/" + sounds.value("left_up", "");
                config.mouse.rightDown = config.mouse.soundsDir + "/" + sounds.value("right_down", "");
                config.mouse.rightUp = config.mouse.soundsDir + "/" + sounds.value("right_up", "");
            }
        }
        
        // Keyboard config
        if (j.contains("keyboard")) {
            auto& keyboard = j["keyboard"];
            config.keyboard.enabled = keyboard.value("enabled", true);
            config.keyboard.soundsDir = keyboard.value("sounds_dir", "sounds");
            config.keyboard.randomSounds = keyboard.value("random_sounds", true);
            config.keyboard.totallyRandomKeypresses = keyboard.value("totally_random_keypresses", false);
            config.keyboard.disableRepeat = keyboard.value("disable_repeat", false);
            
            if (config.keyboard.randomSounds) {
                config.keyboard.sounds = loadSoundsFromDirectory(config.keyboard.soundsDir);
            }
            
            if (keyboard.contains("no_repeat_keys")) {
                for (const auto& key : keyboard["no_repeat_keys"]) {
                    if (key.is_string()) {
                        int keyCode = KeyMapping::getKeyCode(key.get<std::string>());
                        if (keyCode != -1) {
                            config.keyboard.noRepeatKeys.insert(keyCode);
                        }
                    } else if (key.is_number()) {
                        config.keyboard.noRepeatKeys.insert(key.get<int>());
                    }
                }
            }
            
            if (keyboard.contains("excluded_keys")) {
                for (const auto& key : keyboard["excluded_keys"]) {
                    if (key.is_string()) {
                        int keyCode = KeyMapping::getKeyCode(key.get<std::string>());
                        if (keyCode != -1) {
                            config.keyboard.excludedKeys.insert(keyCode);
                        }
                    } else if (key.is_number()) {
                        config.keyboard.excludedKeys.insert(key.get<int>());
                    }
                }
            }
        }
        
        // Audio config
        if (j.contains("audio")) {
            auto& audio = j["audio"];
            config.audio.asyncPlayback = audio.value("async_playback", true);
            config.audio.maxConcurrentSounds = audio.value("max_concurrent_sounds", 32);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error loading config: " << e.what() << std::endl;
    }
    
    return config;
}









