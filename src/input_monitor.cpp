#include "input_monitor.h"

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <unordered_set>

WindowsInputMonitor* WindowsInputMonitor::instance_ = nullptr;

std::unique_ptr<InputMonitor> InputMonitor::create() {
    return std::make_unique<WindowsInputMonitor>();
}

bool WindowsInputMonitor::initialize() {
    instance_ = this;
    return true;
}

void WindowsInputMonitor::setMouseCallback(MouseCallback callback) {
    mouseCallback_ = callback;
}

void WindowsInputMonitor::setKeyboardCallback(KeyboardCallback callback) {
    keyboardCallback_ = callback;
}

void WindowsInputMonitor::setUpdateCallback(UpdateCallback callback) {
    updateCallback_ = callback;
}

void WindowsInputMonitor::clearCallbacks() {
    mouseCallback_ = nullptr;
    keyboardCallback_ = nullptr;
    updateCallback_ = nullptr;
}

void WindowsInputMonitor::startMonitoring() {
    running_ = true;
    
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, 
                                       GetModuleHandle(nullptr), 0);
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                                          GetModuleHandle(nullptr), 0);
    
    // Set up a timer for regular audio updates (every 10ms)
    const UINT_PTR TIMER_ID = 1;
    SetTimer(nullptr, TIMER_ID, 10, nullptr);
    
    MSG msg;
    while (running_ && GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_TIMER && msg.wParam == TIMER_ID) {
            // Call update callback for audio processing
            if (updateCallback_) {
                updateCallback_();
            }
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    KillTimer(nullptr, TIMER_ID);
    UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);
}

LRESULT CALLBACK WindowsInputMonitor::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance_ && instance_->mouseCallback_) {
        MouseButton button;
        MouseEvent event;
        bool validEvent = true;
        
        switch (wParam) {
            case WM_LBUTTONDOWN: button = MouseButton::LEFT; event = MouseEvent::BUTTON_DOWN; break;
            case WM_LBUTTONUP: button = MouseButton::LEFT; event = MouseEvent::BUTTON_UP; break;
            case WM_RBUTTONDOWN: button = MouseButton::RIGHT; event = MouseEvent::BUTTON_DOWN; break;
            case WM_RBUTTONUP: button = MouseButton::RIGHT; event = MouseEvent::BUTTON_UP; break;
            case WM_MBUTTONDOWN: button = MouseButton::MIDDLE; event = MouseEvent::BUTTON_DOWN; break;
            case WM_MBUTTONUP: button = MouseButton::MIDDLE; event = MouseEvent::BUTTON_UP; break;
            case WM_XBUTTONDOWN: {
                MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
                WORD xButton = HIWORD(mouseStruct->mouseData);
                button = (xButton == XBUTTON1) ? MouseButton::X1 : MouseButton::X2;
                event = MouseEvent::BUTTON_DOWN;
                break;
            }
            case WM_XBUTTONUP: {
                MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
                WORD xButton = HIWORD(mouseStruct->mouseData);
                button = (xButton == XBUTTON1) ? MouseButton::X1 : MouseButton::X2;
                event = MouseEvent::BUTTON_UP;
                break;
            }
            case WM_MOUSEWHEEL: {
                MSLLHOOKSTRUCT* mouseStruct = (MSLLHOOKSTRUCT*)lParam;
                short wheelDelta = GET_WHEEL_DELTA_WPARAM(mouseStruct->mouseData);
                button = MouseButton::MIDDLE; // Use middle for wheel events
                event = (wheelDelta > 0) ? MouseEvent::WHEEL_UP : MouseEvent::WHEEL_DOWN;
                break;
            }
            default: 
                validEvent = false;
                break;
        }
        
        if (validEvent) {
            instance_->mouseCallback_(button, event);
        }
    }
    
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

LRESULT CALLBACK WindowsInputMonitor::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance_ && instance_->keyboardCallback_) {
        KBDLLHOOKSTRUCT* kbd = (KBDLLHOOKSTRUCT*)lParam;
        KeyEvent event = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) ? 
                        KeyEvent::DOWN : KeyEvent::UP;
        
        instance_->keyboardCallback_(kbd->vkCode, event);
    }
    
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

void WindowsInputMonitor::stopMonitoring() {
    running_ = false;
    PostQuitMessage(0);
}
#endif