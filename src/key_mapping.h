#pragma once
#include <string>
#include <unordered_map>

class KeyMapping {
public:
    static int getKeyCode(const std::string& keyName);
    static std::string getKeyName(int keyCode);
    
private:
    static const std::unordered_map<std::string, int> keyNameToCode_;
    static const std::unordered_map<int, std::string> keyCodeToName_;
};