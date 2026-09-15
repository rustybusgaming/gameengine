#pragma once

// Platform-specific includes and definitions
#ifdef _WIN32
    // Define these before including Windows headers
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    
    #ifndef STRICT
    #define STRICT
    #endif
    
    // Include Windows headers first
    #include <windows.h>
    #include <objbase.h>
    #include <initguid.h>
    
    // DirectX 11 includes (order is important!)
    #include <dxgi.h>
    #include <d3d11.h>
    #include <d3dcompiler.h>
    #include <d3d11sdklayers.h>
    #include <d3d11_1.h>
    #include <dinput.h>
    // Include mmreg.h before dsound.h to avoid WAVEFORMATEX conflicts
    #include <mmreg.h>
    #include <dsound.h>
    #include <DirectXMath.h>
    
    // Link DirectX libraries
    #pragma comment(lib, "d3d11.lib")
    #pragma comment(lib, "dxgi.lib")
    #pragma comment(lib, "d3dcompiler.lib")
    #pragma comment(lib, "dinput8.lib")
    #pragma comment(lib, "dsound.lib")
    #pragma comment(lib, "dxguid.lib")
    
    using namespace DirectX;
    
    // Type aliases for compatibility with legacy DirectX 9 code
    typedef XMFLOAT2 D3DXVECTOR2;
    typedef XMFLOAT3 D3DXVECTOR3;
    typedef XMFLOAT4 D3DXVECTOR4;
    typedef XMMATRIX D3DXMATRIX;
    typedef XMFLOAT4 D3DXQUATERNION;
    
    // Windows-specific types
    typedef HWND WindowHandle;
    typedef HINSTANCE InstanceHandle;
#else
    // ---- Non-Windows -------------------------------------------------------
    // compat/win32 supplies a portable DirectXMath implementation and the Win32
    // typedefs the engine's headers are written against. Including them here,
    // in the same order and with the same `using` directive as the Windows
    // branch, means shared engine code sees an identical set of names on every
    // platform and needs no per-platform spelling.
    #include <Windows.h>
    #include <DirectXMath.h>

    // Engine headers declare Direct3D interface pointers as members and, on
    // Windows, picked these up transitively through this header. Mirroring that
    // include set here keeps those declarations parsing off Windows too - the
    // compat versions declare the interfaces without defining them, so only
    // declarations work and no real D3D call can slip through.
    #include <dxgi.h>
    #include <d3d11.h>
    #include <d3dcompiler.h>
    #include <dinput.h>
    #include <dsound.h>

    using namespace DirectX;

    // Legacy DirectX 9 spellings, kept in step with the Windows branch above.
    typedef XMFLOAT2 D3DXVECTOR2;
    typedef XMFLOAT3 D3DXVECTOR3;
    typedef XMFLOAT4 D3DXVECTOR4;
    typedef XMMATRIX D3DXMATRIX;
    typedef XMFLOAT4 D3DXQUATERNION;

    // Spelled as the Win32 handle types rather than void*, so a WindowHandle is
    // the same type on every platform and can be passed to the subsystem
    // Initialize() overloads that take an HWND without a per-platform cast.
    // compat/win32 declares these as distinct incomplete struct pointers, so
    // they still do not interchange with each other.
    typedef HWND WindowHandle;
    typedef HINSTANCE InstanceHandle;
#endif

// Standard library includes
#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <fstream>
#include <thread>
#include <mutex>

namespace Nexus {

/**
 * Platform abstraction layer with DirectX integration
 */
class Platform {
public:
    static bool Initialize();
    static void Shutdown();
    static std::string GetPlatformName();
    static bool IsConsoleSupported();
    static void SetConsoleMode(bool enabled);
    
    // Window management
    //
    // The window belongs to the platform layer, not to Engine. Engine used to
    // register its own window class and run its own message loop inline, which
    // is what kept the engine core Windows-only even though every other part of
    // it was portable.

    /// Creates a window whose *client area* is width x height, and shows it.
    /// Returns nullptr on failure, or on a platform with no window backend.
    static WindowHandle CreateGameWindow(const std::string& title, int width, int height);
    static void DestroyGameWindow(WindowHandle window);

    /// Drains the host message queue. Returns false once the application has
    /// been asked to quit.
    static bool ProcessMessages();

    /// Signature of a native-message observer: returns true if it consumed the
    /// message. Parameters are the native message triple, widened to integer
    /// types so this declaration needs no Win32 message types.
    using WindowMessageHook = bool (*)(WindowHandle window, unsigned int message,
                                       std::uintptr_t wParam, std::intptr_t lParam);

    /// Installs a hook that gets first look at every native window message.
    /// The UI layer uses this to feed ImGui's Win32 backend. Pass nullptr to
    /// remove it. No-op where there is no native message queue.
    static void SetWindowMessageHook(WindowMessageHook hook);
    
    // Timing
    static double GetTime();
    static void Sleep(int milliseconds);

    /// Percentage of physical RAM currently in use system-wide, 0-100.
    /// Returns -1 when the platform cannot report it, so callers can tell
    /// "unknown" apart from "not under pressure".
    static int GetSystemMemoryUsagePercent();
    
    // File system
    static bool FileExists(const std::string& path);
    static std::string GetExecutablePath();
    
private:
    static bool isInitialized_;
};

} // namespace Nexus
