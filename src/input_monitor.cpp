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

void WindowsInputMonitor::startMonitoring() {
    running_ = true;
    
    HHOOK mouseHook = SetWindowsHookEx(WH_MOUSE_LL, LowLevelMouseProc, 
                                       GetModuleHandle(nullptr), 0);
    HHOOK keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc,
                                          GetModuleHandle(nullptr), 0);
    
    MSG msg;
    while (running_ && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    UnhookWindowsHookEx(mouseHook);
    UnhookWindowsHookEx(keyboardHook);
}

LRESULT CALLBACK WindowsInputMonitor::LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && instance_ && instance_->mouseCallback_) {
        MouseButton button;
        KeyEvent event;
        
        switch (wParam) {
            case WM_LBUTTONDOWN: button = MouseButton::LEFT; event = KeyEvent::DOWN; break;
            case WM_LBUTTONUP: button = MouseButton::LEFT; event = KeyEvent::UP; break;
            case WM_RBUTTONDOWN: button = MouseButton::RIGHT; event = KeyEvent::DOWN; break;
            case WM_RBUTTONUP: button = MouseButton::RIGHT; event = KeyEvent::UP; break;
            default: return CallNextHookEx(nullptr, nCode, wParam, lParam);
        }
        
        instance_->mouseCallback_(button, event);
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