#include "Platform.h"
#include "Logger.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

#if defined(_WIN32)
    #include <windows.h>
#elif defined(__APPLE__)
    #include <mach-o/dyld.h>
    #include <unistd.h>
#else
    #include <unistd.h>
#endif

namespace Nexus {

// Most of this class was previously declared but never defined, so any call to
// Platform::Sleep, Platform::GetTime, Platform::FileExists or
// Platform::GetExecutablePath was an unresolved symbol at link time. Engine code
// worked around that by calling Win32 directly, which is what tied the core to
// Windows in the first place. These implementations give that code somewhere
// portable to go.

bool Platform::isInitialized_ = false;

bool Platform::Initialize() {
    if (isInitialized_) {
        return true;
    }

    isInitialized_ = true;
    Logger::Info("Platform layer initialized for " + GetPlatformName());
    return true;
}

void Platform::Shutdown() {
    if (!isInitialized_) {
        return;
    }

    isInitialized_ = false;
    Logger::Info("Platform layer shutdown");
}

std::string Platform::GetPlatformName() {
    // Previously hard-coded to "Windows", which made every log line and any
    // platform-conditional logic wrong everywhere else.
#if defined(_WIN32)
    return "Windows";
#elif defined(__APPLE__)
    return "macOS";
#elif defined(__linux__)
    return "Linux";
#elif defined(__FreeBSD__)
    return "FreeBSD";
#else
    return "Unknown";
#endif
}

bool Platform::IsConsoleSupported() {
    // Console backends are selected at build time via ENABLE_CONSOLE_PLATFORMS
    // and none are wired up yet.
    return false;
}

void Platform::SetConsoleMode(bool enabled) {
    (void)enabled;
}

// --- Timing -----------------------------------------------------------------

double Platform::GetTime() {
    // steady_clock rather than system_clock: this feeds frame pacing, which must
    // not jump if the wall clock is adjusted.
    using Clock = std::chrono::steady_clock;
    static const Clock::time_point origin = Clock::now();

    return std::chrono::duration<double>(Clock::now() - origin).count();
}

void Platform::Sleep(int milliseconds) {
    if (milliseconds <= 0) {
        return;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

int Platform::GetSystemMemoryUsagePercent() {
#if defined(_WIN32)
    MEMORYSTATUSEX memStatus = {};
    memStatus.dwLength = sizeof(memStatus);
    if (!GlobalMemoryStatusEx(&memStatus)) {
        return -1;
    }
    return static_cast<int>(memStatus.dwMemoryLoad);

#elif defined(__linux__)
    // MemAvailable is the right field rather than MemFree: the kernel counts
    // reclaimable page cache and slab as available, so MemFree alone reports
    // near-exhaustion on any machine that has been up for a while.
    std::ifstream meminfo("/proc/meminfo");
    if (!meminfo) {
        return -1;
    }

    long long totalKb = 0;
    long long availableKb = -1;
    std::string key;
    long long value = 0;
    std::string unit;

    while (meminfo >> key >> value >> unit) {
        if (key == "MemTotal:") {
            totalKb = value;
        } else if (key == "MemAvailable:") {
            availableKb = value;
            break;      // MemAvailable follows MemTotal; nothing else is needed.
        }
    }

    if (totalKb <= 0 || availableKb < 0) {
        return -1;
    }

    const long long usedKb = totalKb - availableKb;
    return static_cast<int>((usedKb * 100) / totalKb);

#else
    // No portable equivalent on this platform yet.
    return -1;
#endif
}

// --- File system ------------------------------------------------------------

bool Platform::FileExists(const std::string& path) {
    std::error_code ec;
    // The non-throwing overload matters here: callers use this to probe for
    // optional assets, where a permission error should read as "absent" rather
    // than terminate the process.
    return std::filesystem::is_regular_file(path, ec) && !ec;
}

std::string Platform::GetExecutablePath() {
#if defined(_WIN32)
    char buffer[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length == MAX_PATH) {
        return {};
    }
    return std::string(buffer, length);

#elif defined(__APPLE__)
    uint32_t size = 0;
    _NSGetExecutablePath(nullptr, &size);   // First call reports the size needed.

    std::string buffer(size, '\0');
    if (_NSGetExecutablePath(buffer.data(), &size) != 0) {
        return {};
    }
    buffer.resize(std::char_traits<char>::length(buffer.c_str()));

    std::error_code ec;
    const std::filesystem::path resolved = std::filesystem::canonical(buffer, ec);
    return ec ? buffer : resolved.string();

#else
    std::error_code ec;
    const std::filesystem::path resolved = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (ec) {
        Logger::Warning("Could not resolve executable path: " + ec.message());
        return {};
    }
    return resolved.string();
#endif
}

// --- Window management ------------------------------------------------------
//
// Window ownership lives here rather than in Engine. Engine previously
// registered its own window class, ran its own WindowProc and called
// DestroyWindow/UnregisterClass inline, which is the only reason the engine
// core could not be compiled off Windows - every other line in it is portable.

namespace {

/// First-look observer for native messages; see Platform::SetWindowMessageHook.
Platform::WindowMessageHook g_messageHook = nullptr;

#if defined(_WIN32)

constexpr const char* kWindowClassName = "NexusEngineWindow";

/// Live windows created through CreateGameWindow. The class is registered on
/// the first and unregistered after the last, so repeated
/// Initialize/Shutdown cycles in one process do not leak a class registration
/// or fail on a duplicate one.
int g_windowCount = 0;
bool g_classRegistered = false;

LRESULT CALLBACK NexusWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (g_messageHook &&
        g_messageHook(hwnd, static_cast<unsigned int>(message),
                      static_cast<std::uintptr_t>(wParam),
                      static_cast<std::intptr_t>(lParam))) {
        return 0;
    }

    switch (message) {
        case WM_CLOSE:
            // Ask the loop to stop rather than destroying the window from
            // under the renderer; teardown runs in Engine::Shutdown.
            PostQuitMessage(0);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }

    return DefWindowProc(hwnd, message, wParam, lParam);
}

bool EnsureWindowClass() {
    if (g_classRegistered) {
        return true;
    }

    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = NexusWindowProc;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.hIcon         = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kWindowClassName;

    if (RegisterClassExA(&wc) == 0) {
        Logger::Error("Failed to register window class '" + std::string(kWindowClassName) + "'");
        return false;
    }

    g_classRegistered = true;
    return true;
}

#endif // _WIN32

} // namespace

void Platform::SetWindowMessageHook(WindowMessageHook hook) {
    g_messageHook = hook;
}

WindowHandle Platform::CreateGameWindow(const std::string& title, int width, int height) {
    if (width <= 0 || height <= 0) {
        Logger::Error("CreateGameWindow: invalid size " + std::to_string(width) + "x" +
                      std::to_string(height));
        return nullptr;
    }

#if defined(_WIN32)
    if (!EnsureWindowClass()) {
        return nullptr;
    }

    // width/height describe the client area - the region the swap chain is
    // sized against. Passing them straight to CreateWindowEx would size the
    // whole window instead, leaving a client area smaller than the back buffer
    // and a permanently rescaled image.
    const DWORD style = WS_OVERLAPPEDWINDOW;
    RECT rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    if (!AdjustWindowRect(&rect, style, FALSE)) {
        Logger::Warning("AdjustWindowRect failed; falling back to an unadjusted window size");
        rect = {0, 0, static_cast<LONG>(width), static_cast<LONG>(height)};
    }

    HWND hwnd = CreateWindowExA(
        0,
        kWindowClassName,
        title.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr, nullptr,
        GetModuleHandleA(nullptr),
        nullptr);

    if (hwnd == nullptr) {
        Logger::Error("Failed to create window '" + title + "'");
        if (g_windowCount == 0 && g_classRegistered) {
            UnregisterClassA(kWindowClassName, GetModuleHandleA(nullptr));
            g_classRegistered = false;
        }
        return nullptr;
    }

    ++g_windowCount;
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return hwnd;

#else
    // There is no cross-platform windowing backend wired up yet. Reporting
    // failure clearly beats handing back a placeholder handle that the
    // renderer would only fail on later, further from the cause.
    (void)title;
    Logger::Error("Platform::CreateGameWindow is not implemented on " + GetPlatformName() +
                  " - no windowing backend is compiled in");
    return nullptr;
#endif
}

void Platform::DestroyGameWindow(WindowHandle window) {
    if (window == nullptr) {
        return;
    }

#if defined(_WIN32)
    DestroyWindow(window);

    if (g_windowCount > 0) {
        --g_windowCount;
    }

    // Unregister only once the last window is gone: the class is process-wide
    // and unregistering it while another window still uses it would fail.
    if (g_windowCount == 0 && g_classRegistered) {
        UnregisterClassA(kWindowClassName, GetModuleHandleA(nullptr));
        g_classRegistered = false;
    }
#endif
}

bool Platform::ProcessMessages() {
    // Drains whatever the host windowing system has queued and reports whether
    // the application should keep running. Owning the pump here is what lets
    // Engine::Run be the same code on every platform.
#if defined(_WIN32)
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
#else
    // No window means no message queue to drain; report "keep running" so a
    // headless host loop is not terminated by the absence of a window.
    return true;
#endif
}

} // namespace Nexus
