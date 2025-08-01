#include "key_mapping.h"
#include <algorithm>

#ifdef PLATFORM_WINDOWS
#include <windows.h>

const std::unordered_map<std::string, int> KeyMapping::keyNameToCode_ = {
    // Letters
    {"a", 0x41}, {"b", 0x42}, {"c", 0x43}, {"d", 0x44}, {"e", 0x45},
    {"f", 0x46}, {"g", 0x47}, {"h", 0x48}, {"i", 0x49}, {"j", 0x4A},
    {"k", 0x4B}, {"l", 0x4C}, {"m", 0x4D}, {"n", 0x4E}, {"o", 0x4F},
    {"p", 0x50}, {"q", 0x51}, {"r", 0x52}, {"s", 0x53}, {"t", 0x54},
    {"u", 0x55}, {"v", 0x56}, {"w", 0x57}, {"x", 0x58}, {"y", 0x59}, {"z", 0x5A},
    
    // Numbers
    {"0", 0x30}, {"1", 0x31}, {"2", 0x32}, {"3", 0x33}, {"4", 0x34},
    {"5", 0x35}, {"6", 0x36}, {"7", 0x37}, {"8", 0x38}, {"9", 0x39},
    
    // Special keys - using specific left/right variants
    {"space", VK_SPACE}, {"enter", VK_RETURN}, {"tab", VK_TAB},
    {"lshift", VK_LSHIFT}, {"rshift", VK_RSHIFT}, {"shift", VK_LSHIFT},
    {"lctrl", VK_LCONTROL}, {"rctrl", VK_RCONTROL}, {"ctrl", VK_LCONTROL},
    {"lalt", VK_LMENU}, {"ralt", VK_RMENU}, {"alt", VK_LMENU},
    {"escape", VK_ESCAPE}, {"backspace", VK_BACK}, {"delete", VK_DELETE},
    {"home", VK_HOME}, {"end", VK_END}, {"pageup", VK_PRIOR}, {"pagedown", VK_NEXT},
    {"insert", VK_INSERT}, {"capslock", VK_CAPITAL},
    
    // Arrow keys
    {"left", VK_LEFT}, {"right", VK_RIGHT}, {"up", VK_UP}, {"down", VK_DOWN},
    
    // Function keys
    {"f1", VK_F1}, {"f2", VK_F2}, {"f3", VK_F3}, {"f4", VK_F4},
    {"f5", VK_F5}, {"f6", VK_F6}, {"f7", VK_F7}, {"f8", VK_F8},
    {"f9", VK_F9}, {"f10", VK_F10}, {"f11", VK_F11}, {"f12", VK_F12},

    // Numpad keys
    {"numpad0", VK_NUMPAD0}, {"numpad1", VK_NUMPAD1}, {"numpad2", VK_NUMPAD2}, {"numpad3", VK_NUMPAD3},
    {"numpad4", VK_NUMPAD4}, {"numpad5", VK_NUMPAD5}, {"numpad6", VK_NUMPAD6}, {"numpad7", VK_NUMPAD7},
    {"numpad8", VK_NUMPAD8}, {"numpad9", VK_NUMPAD9}, {"numpadenter", VK_RETURN},
    {"numpadplus", VK_ADD}, {"numpadminus", VK_SUBTRACT}, {"numpadmultiply", VK_MULTIPLY},
    {"numpaddivide", VK_DIVIDE}, {"numpaddot", VK_DECIMAL},

    // Additional special keys
    {"printscreen", VK_SNAPSHOT}, {"scrolllock", VK_SCROLL}, {"pause", VK_PAUSE},
    {"menu", VK_APPS}, {"lwin", VK_LWIN}, {"rwin", VK_RWIN}, {"win", VK_LWIN},

    // Punctuation and symbols (common ones)
    {"semicolon", VK_OEM_1}, {"apostrophe", VK_OEM_7}, {"grave", VK_OEM_3},
    {"backslash", VK_OEM_5}, {"comma", VK_OEM_COMMA}, {"dot", VK_OEM_PERIOD}, {"slash", VK_OEM_2},
    {"leftbracket", VK_OEM_4}, {"rightbracket", VK_OEM_6}, {"equal", VK_OEM_PLUS}, {"minus", VK_OEM_MINUS}
};

#else
// Linux key codes (using Linux input event codes)
const std::unordered_map<std::string, int> KeyMapping::keyNameToCode_ = {
    // Letters (KEY_A = 30, KEY_B = 48, etc.)
    {"a", 30}, {"b", 48}, {"c", 46}, {"d", 32}, {"e", 18},
    {"f", 33}, {"g", 34}, {"h", 35}, {"i", 23}, {"j", 36},
    {"k", 37}, {"l", 38}, {"m", 50}, {"n", 49}, {"o", 24},
    {"p", 25}, {"q", 16}, {"r", 19}, {"s", 31}, {"t", 20},
    {"u", 22}, {"v", 47}, {"w", 17}, {"x", 45}, {"y", 21}, {"z", 44},

    // Numbers (KEY_1 = 2, KEY_2 = 3, etc.)
    {"0", 11}, {"1", 2}, {"2", 3}, {"3", 4}, {"4", 5},
    {"5", 6}, {"6", 7}, {"7", 8}, {"8", 9}, {"9", 10},

    // Special keys
    {"space", 57}, {"enter", 28}, {"tab", 15},
    {"lshift", 42}, {"rshift", 54}, {"shift", 42},
    {"lctrl", 29}, {"rctrl", 97}, {"ctrl", 29},
    {"lalt", 56}, {"ralt", 100}, {"alt", 56},
    {"escape", 1}, {"backspace", 14}, {"delete", 111},
    {"home", 102}, {"end", 107}, {"pageup", 104}, {"pagedown", 109},
    {"insert", 110}, {"capslock", 58},

    // Arrow keys
    {"left", 105}, {"right", 106}, {"up", 103}, {"down", 108},

    // Function keys (KEY_F1 = 59, KEY_F2 = 60, etc.)
    {"f1", 59}, {"f2", 60}, {"f3", 61}, {"f4", 62},
    {"f5", 63}, {"f6", 64}, {"f7", 65}, {"f8", 66},
    {"f9", 67}, {"f10", 68}, {"f11", 87}, {"f12", 88},

    // Numpad keys
    {"numpad0", 82}, {"numpad1", 79}, {"numpad2", 80}, {"numpad3", 81},
    {"numpad4", 75}, {"numpad5", 76}, {"numpad6", 77}, {"numpad7", 71},
    {"numpad8", 72}, {"numpad9", 73}, {"numpadenter", 96},
    {"numpadplus", 78}, {"numpadminus", 74}, {"numpadmultiply", 55},
    {"numpaddivide", 98}, {"numpaddot", 83},

    // Additional special keys
    {"printscreen", 99}, {"scrolllock", 70}, {"pause", 119},
    {"menu", 139}, {"lwin", 125}, {"rwin", 126}, {"win", 125},

    // Punctuation and symbols (common ones)
    {"semicolon", 39}, {"apostrophe", 40}, {"grave", 41},
    {"backslash", 43}, {"comma", 51}, {"dot", 52}, {"slash", 53},
    {"leftbracket", 26}, {"rightbracket", 27}, {"equal", 13}, {"minus", 12}
};
#endif

const std::unordered_map<int, std::string> KeyMapping::keyCodeToName_ = []() {
    std::unordered_map<int, std::string> reverse;
    for (const auto& pair : keyNameToCode_) {
        reverse[pair.second] = pair.first;
    }
    return reverse;
}();

int KeyMapping::getKeyCode(const std::string& keyName) {
    std::string lowerKey = keyName;
    std::transform(lowerKey.begin(), lowerKey.end(), lowerKey.begin(), ::tolower);
    
    auto it = keyNameToCode_.find(lowerKey);
    return (it != keyNameToCode_.end()) ? it->second : -1;
}

std::string KeyMapping::getKeyName(int keyCode) {
    auto it = keyCodeToName_.find(keyCode);
    return (it != keyCodeToName_.end()) ? it->second : std::to_string(keyCode);
}