#pragma once
#include <functional>
#include <memory>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

enum class MouseButton { LEFT, RIGHT, MIDDLE };
enum class KeyEvent { DOWN, UP };

class InputMonitor {
public:
    using MouseCallback = std::function<void(MouseButton, KeyEvent)>;
    using KeyboardCallback = std::function<void(int, KeyEvent)>;
    
    static std::unique_ptr<InputMonitor> create();
    virtual ~InputMonitor() = default;
    
    virtual bool initialize() = 0;
    virtual void setMouseCallback(MouseCallback callback) = 0;
    virtual void setKeyboardCallback(KeyboardCallback callback) = 0;
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
};

#ifdef PLATFORM_WINDOWS
class WindowsInputMonitor : public InputMonitor {
private:
    MouseCallback mouseCallback_;
    KeyboardCallback keyboardCallback_;
    bool running_ = false;
    static WindowsInputMonitor* instance_;
    
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    
public:
    bool initialize() override;
    void setMouseCallback(MouseCallback callback) override;
    void setKeyboardCallback(KeyboardCallback callback) override;
    void startMonitoring() override;
    void stopMonitoring() override;
};
#endif
