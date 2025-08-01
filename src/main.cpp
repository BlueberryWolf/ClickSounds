#include "config.h"
#include "audio_player.h"
#include "input_monitor.h"
#include <iostream>
#include <random>
#include <unordered_map>
#include <csignal>
#include <cstring>
#include <string>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

class ClickSoundsApp {
private:
    Config config_;
    std::unique_ptr<AudioPlayer> audioPlayer_;
    std::unique_ptr<InputMonitor> inputMonitor_;
    std::unordered_map<int, int> keySoundMap_;
    std::unordered_set<int> pressedKeys_;
    std::mt19937 rng_;
    bool running_ = true;
    
public:
    ClickSoundsApp() : rng_(std::random_device{}()) {}
    
    bool initialize() {
        config_ = Config::loadFromFile("config.json");
        
        audioPlayer_ = AudioPlayer::create();
        if (!audioPlayer_->initialize()) {
            std::cerr << "Failed to initialize audio player\n";
            return false;
        }
        
        audioPlayer_->setMaxConcurrentSounds(config_.audio.maxConcurrentSounds);
        
        inputMonitor_ = InputMonitor::create();
        if (!inputMonitor_->initialize()) {
            std::cerr << "Failed to initialize input monitor\n";
            return false;
        }
        
        setupCallbacks();
        return true;
    }
    
    void setupCallbacks() {
        if (config_.mouse.enabled) {
            inputMonitor_->setMouseCallback([this](MouseButton button, KeyEvent event) {
                handleMouseEvent(button, event);
            });
        }
        
        if (config_.keyboard.enabled) {
            inputMonitor_->setKeyboardCallback([this](int vkCode, KeyEvent event) {
                handleKeyboardEvent(vkCode, event);
            });
        }
    }
    
    void handleMouseEvent(MouseButton button, KeyEvent event) {
        std::string soundFile;
        
        if (button == MouseButton::LEFT) {
            soundFile = (event == KeyEvent::DOWN) ? config_.mouse.leftDown : config_.mouse.leftUp;
        } else if (button == MouseButton::RIGHT) {
            soundFile = (event == KeyEvent::DOWN) ? config_.mouse.rightDown : config_.mouse.rightUp;
        }
        
        if (!soundFile.empty()) {
            audioPlayer_->playSound(soundFile, config_.audio.asyncPlayback);
        }
    }
    
    void handleKeyboardEvent(int vkCode, KeyEvent event) {
        // Skip excluded keys
        if (config_.keyboard.excludedKeys.count(vkCode)) return;
        
        if (event == KeyEvent::DOWN) {
            // Check if key repeat should be disabled (global or per-key)
            bool shouldDisableRepeat = config_.keyboard.disableRepeat || 
                                     config_.keyboard.noRepeatKeys.count(vkCode);
            
            if (shouldDisableRepeat && pressedKeys_.count(vkCode)) {
                return; // Key is already pressed, ignore repeat
            }
            
            pressedKeys_.insert(vkCode);
            
            if (!config_.keyboard.sounds.empty()) {
                int soundIndex;

                if (config_.keyboard.totallyRandomKeypresses) {
                    // Totally random: pick a completely random sound for every keypress
                    std::uniform_int_distribution<int> dist(0, static_cast<int>(config_.keyboard.sounds.size()) - 1);
                    soundIndex = dist(rng_);
                } else {
                    // Normal random: get or assign random sound for this key (consistent per key)
                    if (keySoundMap_.find(vkCode) == keySoundMap_.end()) {
                        std::uniform_int_distribution<int> dist(0, static_cast<int>(config_.keyboard.sounds.size()) - 1);
                        keySoundMap_[vkCode] = dist(rng_);
                    }
                    soundIndex = keySoundMap_[vkCode];
                }

                const std::string& soundFile = config_.keyboard.sounds[soundIndex];
                audioPlayer_->playSound(soundFile, config_.audio.asyncPlayback);
            }
        } else if (event == KeyEvent::UP) {
            pressedKeys_.erase(vkCode);
        }
    }
    
    void run() {
        inputMonitor_->startMonitoring();
    }
    
    void stop() {
        running_ = false;
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