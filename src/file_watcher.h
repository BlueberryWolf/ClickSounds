#pragma once
#include <string>
#include <functional>
#include <memory>
#include <filesystem>
#include "ThomasMonkman/FileWatch.hpp"

class FileWatcher {
public:
    using FileChangedCallback = std::function<void(const std::string& filepath)>;
    
    FileWatcher();
    ~FileWatcher();
    
    // Start watching a file for changes
    bool watchFile(const std::string& filepath, FileChangedCallback callback);
    
    // Stop watching the file
    void stopWatching();
    
    // Check if currently watching a file
    bool isWatching() const;
    
private:
    std::unique_ptr<filewatch::FileWatch<std::string>> watcher_;
    FileChangedCallback callback_;
    std::string filepath_;
    bool watching_;
};