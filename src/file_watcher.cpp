#include "file_watcher.h"
#include <iostream>

FileWatcher::FileWatcher() : watching_(false) {
}

FileWatcher::~FileWatcher() {
    stopWatching();
}

bool FileWatcher::watchFile(const std::string& filepath, FileChangedCallback callback) {
    if (watching_) {
        stopWatching();
    }
    
    try {
        filepath_ = filepath;
        callback_ = callback;
        
        // Create the file watcher for the specific file
        watcher_ = std::make_unique<filewatch::FileWatch<std::string>>(
            filepath,
            [this](const std::string& path, const filewatch::Event change_type) {
                // Only trigger on file modifications
                if (change_type == filewatch::Event::modified) {
                    if (callback_) {
                        callback_(path);
                    }
                }
            }
        );
        
        watching_ = true;
        std::cout << "Started watching config file: " << filepath << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "Failed to start file watcher: " << e.what() << std::endl;
        return false;
    }
}

void FileWatcher::stopWatching() {
    if (watching_) {
        watcher_.reset();
        watching_ = false;
        std::cout << "Stopped watching config file" << std::endl;
    }
}

bool FileWatcher::isWatching() const {
    return watching_;
}