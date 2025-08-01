#include "config.h"
#include "audio_player.h"
#include "input_monitor.h"
#include "file_watcher.h"
#include <iostream>
#include <random>
#include <unordered_map>
#include <csignal>
#include <cstring>
#include <string>
#include <chrono>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

class ClickSoundsApp {
private:
    Config config_;
    std::unique_ptr<AudioPlayer> audioPlayer_;
    std::unique_ptr<InputMonitor> inputMonitor_;
    std::unique_ptr<FileWatcher> fileWatcher_;
    std::unordered_map<int, int> keySoundMap_;
    std::unordered_set<int> pressedKeys_;
    std::unordered_map<MouseButton, int> activeMouseSounds_; // Track active mouse sounds for fade-out
    std::unordered_map<int, int> activeKeySounds_; // Track active keyboard sounds for fade-out (vkCode -> soundId)
    int lastKeyPressed_ = -1; // Track the last key pressed for true randomization
    int lastScrollTime_ = 0; // Track last scroll event time for debouncing
    std::unordered_map<int, int> lastKeyPressTime_; // Track last press time for each key for debouncing
    std::mt19937 rng_;
    bool running_ = true;
    
public:
    ClickSoundsApp() : rng_(std::random_device{}()) {}
    
private:
    int getCurrentTimeMs() {
#ifdef PLATFORM_WINDOWS
        return GetTickCount();
#else
        auto now = std::chrono::steady_clock::now();
        auto duration = now.time_since_epoch();
        return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
#endif
    }
    
    void onConfigChanged(const std::string& filepath) {
        std::cout << "Config file changed, reloading..." << std::endl;
        
        if (config_.reload()) {
            // Apply new config to audio player
            audioPlayer_->setMaxConcurrentSounds(config_.audio.maxConcurrentSounds);
            audioPlayer_->setMasterVolume(config_.audio.masterVolume);
            audioPlayer_->setAudioEffects(config_.audio.effects);
            
            // Clear existing key sound mappings to force remapping with new sounds
            keySoundMap_.clear();
            
            // Re-setup callbacks in case mouse/keyboard enabled states changed
            setupCallbacks();
            
            std::cout << "Config hot reload completed successfully!" << std::endl;
        } else {
            std::cerr << "Failed to reload config file" << std::endl;
        }
    }
    
public:
    
    bool initialize() {
        config_ = Config::loadFromFile("config.json");
        
        audioPlayer_ = AudioPlayer::create();
        if (!audioPlayer_->initialize()) {
            std::cerr << "Failed to initialize audio player\n";
            return false;
        }
        
        audioPlayer_->setMaxConcurrentSounds(config_.audio.maxConcurrentSounds);
        audioPlayer_->setMasterVolume(config_.audio.masterVolume);
        audioPlayer_->setAudioEffects(config_.audio.effects);
        
        inputMonitor_ = InputMonitor::create();
        if (!inputMonitor_->initialize()) {
            std::cerr << "Failed to initialize input monitor\n";
            return false;
        }
        
        // Initialize file watcher for config hot reloading
        fileWatcher_ = std::make_unique<FileWatcher>();
        if (!fileWatcher_->watchFile(config_.getFilePath(), [this](const std::string& filepath) {
            onConfigChanged(filepath);
        })) {
            std::cerr << "Warning: Failed to start config file watcher. Hot reloading disabled.\n";
        }
        
        setupCallbacks();
        return true;
    }
    
    void setupCallbacks() {
        inputMonitor_->clearCallbacks();

        if (config_.mouse.enabled) {
            inputMonitor_->setMouseCallback([this](MouseButton button, MouseEvent event) {
                handleMouseEvent(button, event);
            });
        }
        
        if (config_.keyboard.enabled) {
            inputMonitor_->setKeyboardCallback([this](int vkCode, KeyEvent event) {
                handleKeyboardEvent(vkCode, event);
            });
        }
        
        // Set up regular audio updates for fade processing
        inputMonitor_->setUpdateCallback([this]() {
            audioPlayer_->update();
        });
    }
    
    void handleMouseEvent(MouseButton button, MouseEvent event) {
        std::string soundFile;
        bool shouldPlay = true;
        
        // Handle button events
        if (event == MouseEvent::BUTTON_DOWN || event == MouseEvent::BUTTON_UP) {
            // Check if fade-out is enabled and we have an active sound for this button
            if (config_.mouse.enableFadeOut && event == MouseEvent::BUTTON_UP) {
                auto it = activeMouseSounds_.find(button);
                if (it != activeMouseSounds_.end()) {
                    // Fade out the active sound instead of playing a new one
                    audioPlayer_->fadeOutSound(it->second, config_.mouse.fadeOutDurationMs);
                    activeMouseSounds_.erase(it);
                    return; // Don't play the up sound
                }
            }
            
            // Determine which sound file to play
            switch (button) {
                case MouseButton::LEFT:
                    soundFile = (event == MouseEvent::BUTTON_DOWN) ? config_.mouse.leftDown : config_.mouse.leftUp;
                    break;
                case MouseButton::RIGHT:
                    soundFile = (event == MouseEvent::BUTTON_DOWN) ? config_.mouse.rightDown : config_.mouse.rightUp;
                    break;
                case MouseButton::MIDDLE:
                    soundFile = (event == MouseEvent::BUTTON_DOWN) ? config_.mouse.middleDown : config_.mouse.middleUp;
                    break;
                case MouseButton::X1:
                    if (!config_.mouse.enableSideButtons) shouldPlay = false;
                    else soundFile = (event == MouseEvent::BUTTON_DOWN) ? config_.mouse.x1Down : config_.mouse.x1Up;
                    break;
                case MouseButton::X2:
                    if (!config_.mouse.enableSideButtons) shouldPlay = false;
                    else soundFile = (event == MouseEvent::BUTTON_DOWN) ? config_.mouse.x2Down : config_.mouse.x2Up;
                    break;
            }
        }
        // Handle scroll wheel events
        else if (event == MouseEvent::WHEEL_UP || event == MouseEvent::WHEEL_DOWN) {
            if (!config_.mouse.enableScrollWheel) {
                shouldPlay = false;
            } else {
                // Check debounce timing
                int currentTime = getCurrentTimeMs();
                if (currentTime - lastScrollTime_ < config_.mouse.scrollWheelDebounceMs) {
                    shouldPlay = false; // Too soon, skip this scroll event
                } else {
                    lastScrollTime_ = currentTime;
                    soundFile = (event == MouseEvent::WHEEL_UP) ? config_.mouse.wheelUp : config_.mouse.wheelDown;
                }
            }
        }
        
        // Play the sound if we have one and should play it
        if (shouldPlay && !soundFile.empty()) {
            int soundId = audioPlayer_->playSoundWithIdAndVolume(soundFile, config_.mouse.volume, config_.audio.asyncPlayback);
            
            // Track the sound ID for potential fade-out (only for button down events)
            if (config_.mouse.enableFadeOut && event == MouseEvent::BUTTON_DOWN && soundId > 0) {
                activeMouseSounds_[button] = soundId;
            }
        }
    }
    
    void handleKeyboardEvent(int vkCode, KeyEvent event) {
        // Skip excluded keys
        if (config_.keyboard.excludedKeys.count(vkCode)) return;
        
        if (event == KeyEvent::DOWN) {
            // Check debounce timing first
            int currentTime = getCurrentTimeMs();
            auto lastTimeIt = lastKeyPressTime_.find(vkCode);
            if (lastTimeIt != lastKeyPressTime_.end()) {
                if (currentTime - lastTimeIt->second < config_.keyboard.keyRepeatDebounceMs) {
                    return; // Too soon, skip this key press
                }
            }
            
            // Check if key repeat should be disabled (global or per-key)
            bool shouldDisableRepeat = config_.keyboard.disableRepeat || 
                                     config_.keyboard.noRepeatKeys.count(vkCode);
            
            if (shouldDisableRepeat && pressedKeys_.count(vkCode)) {
                return; // Key is already pressed, ignore repeat
            }
            
            // Update timing and pressed keys tracking
            lastKeyPressTime_[vkCode] = currentTime;
            pressedKeys_.insert(vkCode);
            
            if (!config_.keyboard.sounds.empty()) {
                int soundIndex;

                if (config_.keyboard.totallyRandomKeypresses) {
                    // True randomization: if switching to a different key, pick a new random sound
                    // If pressing the same key repeatedly, keep using the same sound
                    if (lastKeyPressed_ != vkCode) {
                        // Switching keys - pick a new random sound
                        std::uniform_int_distribution<int> dist(0, static_cast<int>(config_.keyboard.sounds.size()) - 1);
                        keySoundMap_[vkCode] = dist(rng_);
                        lastKeyPressed_ = vkCode;
                    }
                    soundIndex = keySoundMap_[vkCode];
                } else {
                    // Normal random: get or assign random sound for this key (consistent per key)
                    if (keySoundMap_.find(vkCode) == keySoundMap_.end()) {
                        std::uniform_int_distribution<int> dist(0, static_cast<int>(config_.keyboard.sounds.size()) - 1);
                        keySoundMap_[vkCode] = dist(rng_);
                    }
                    soundIndex = keySoundMap_[vkCode];
                }

                const std::string& soundFile = config_.keyboard.sounds[soundIndex];
                int soundId = audioPlayer_->playSoundWithIdAndVolume(soundFile, config_.keyboard.volume, config_.audio.asyncPlayback);
                
                // Track the sound ID for potential fade-out
                if (config_.keyboard.enableFadeOut && soundId > 0) {
                    activeKeySounds_[vkCode] = soundId;
                }
            }
        } else if (event == KeyEvent::UP) {
            pressedKeys_.erase(vkCode);
            
            // Clean up timing data for released key to prevent memory buildup
            lastKeyPressTime_.erase(vkCode);
            
            // Handle fade-out on key release
            if (config_.keyboard.enableFadeOut) {
                auto it = activeKeySounds_.find(vkCode);
                if (it != activeKeySounds_.end()) {
                    audioPlayer_->fadeOutSound(it->second, config_.keyboard.fadeOutDurationMs);
                    activeKeySounds_.erase(it);
                }
            }
        }
    }
    
    void run() {
        inputMonitor_->startMonitoring();
    }
    
    void stop() {
        running_ = false;
        if (fileWatcher_) {
            fileWatcher_->stopWatching();
        }
        inputMonitor_->stopMonitoring();
        audioPlayer_->cleanup();
    }
};

ClickSoundsApp* app = nullptr;

void signalHandler(int signal) {
    if (app) {
        app->stop();
    }
}

#ifdef PLATFORM_WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    bool showConsole = false;

    // Parse CLI arguments
    std::string cmdLine(lpCmdLine);
    if (cmdLine.find("--foreground") != std::string::npos || cmdLine.find("-f") != std::string::npos) {
        showConsole = true;
    } else if (cmdLine.find("--help") != std::string::npos || cmdLine.find("-h") != std::string::npos) {
        showConsole = true;
    }

    // Allocate console only if requested
    if (showConsole) {
        AllocConsole();
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
        freopen_s((FILE**)stderr, "CONOUT$", "w", stderr);
        freopen_s((FILE**)stdin, "CONIN$", "r", stdin);

        if (cmdLine.find("--help") != std::string::npos || cmdLine.find("-h") != std::string::npos) {
            std::cout << "ClickSounds - Keyboard and mouse sound effects\n";
            std::cout << "Usage: ClickSounds [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -f, --foreground    Run with console window (default: background)\n";
            std::cout << "  -h, --help          Show this help message\n";
            std::cout << "Press any key to exit...\n";
            std::cin.get();
            return 0;
        }

        std::cout << "ClickSounds started. Press Ctrl+C to exit.\n";
    }

    signal(SIGINT, signalHandler);

    app = new ClickSoundsApp();

    if (!app->initialize()) {
        if (showConsole) {
            std::cerr << "Failed to initialize application\n";
        }
        delete app;
        return 1;
    }

    app->run();

    delete app;
    return 0;
}
#else
int main(int argc, char* argv[]) {
    bool showConsole = true;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "ClickSounds - Keyboard and mouse sound effects\n";
            std::cout << "Usage: ClickSounds [options]\n";
            std::cout << "Options:\n";
            std::cout << "  -h, --help          Show this help message\n";
            return 0;
        }
    }

    std::cout << "ClickSounds started. Press Ctrl+C to exit.\n";

    signal(SIGINT, signalHandler);

    app = new ClickSoundsApp();

    if (!app->initialize()) {
        delete app;
        return 1;
    }

    app->run();

    delete app;
    return 0;
}
#endif