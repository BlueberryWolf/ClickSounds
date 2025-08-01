#pragma once
#include <functional>
#include <memory>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#endif

enum class MouseButton { LEFT, RIGHT, MIDDLE, X1, X2 };
enum class MouseEvent { BUTTON_DOWN, BUTTON_UP, WHEEL_UP, WHEEL_DOWN };
enum class KeyEvent { DOWN, UP };

class InputMonitor {
public:
    using MouseCallback = std::function<void(MouseButton, MouseEvent)>;
    using KeyboardCallback = std::function<void(int, KeyEvent)>;
    using UpdateCallback = std::function<void()>;
    
    static std::unique_ptr<InputMonitor> create();
    virtual ~InputMonitor() = default;
    
    virtual bool initialize() = 0;
    virtual void setMouseCallback(MouseCallback callback) = 0;
    virtual void setKeyboardCallback(KeyboardCallback callback) = 0;
    virtual void setUpdateCallback(UpdateCallback callback) = 0;
    virtual void clearCallbacks() = 0;
    virtual void startMonitoring() = 0;
    virtual void stopMonitoring() = 0;
};

#ifdef PLATFORM_WINDOWS
class WindowsInputMonitor : public InputMonitor {
private:
    MouseCallback mouseCallback_;
    KeyboardCallback keyboardCallback_;
    UpdateCallback updateCallback_;
    bool running_ = false;
    static WindowsInputMonitor* instance_;
    
    static LRESULT CALLBACK LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
    
public:
    bool initialize() override;
    void setMouseCallback(MouseCallback callback) override;
    void setKeyboardCallback(KeyboardCallback callback) override;
    void setUpdateCallback(UpdateCallback callback) override;
    void clearCallbacks() override;
    void startMonitoring() override;
    void stopMonitoring() override;
};
#endif
