/**
 * @file PythonScripting.h
 * @brief Complete Python 3.9+ scripting system with hot-reload
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#ifdef NEXUS_PYTHON_ENABLED

#include <Python.h>
#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

namespace Nexus {

class Engine;
class GameObject;
class Component;

/**
 * Python scripting engine with full engine bindings
 */
class PythonScripting {
public:
    PythonScripting();
    ~PythonScripting();

    // Core initialization
    bool Initialize();
    void Shutdown();

    // Script execution
    bool ExecuteFile(const std::string& filepath);
    bool ExecuteString(const std::string& code);
    PyObject* EvaluateExpression(const std::string& expression);

    // Module management
    bool ImportModule(const std::string& moduleName);
    bool ReloadModule(const std::string& moduleName);
    PyObject* GetModule(const std::string& moduleName);

    // Hot reload
    void EnableHotReload(bool enable);
    void CheckForChanges();
    bool ReloadScript(const std::string& filepath);

    // Function calls
    PyObject* CallFunction(const std::string& moduleName, const std::string& functionName, PyObject* args = nullptr);
    PyObject* CallMethod(PyObject* obj, const std::string& methodName, PyObject* args = nullptr);

    // Object creation
    PyObject* CreateObject(const std::string& className, PyObject* args = nullptr);

    // Variable access
    PyObject* GetVariable(const std::string& varName);
    bool SetVariable(const std::string& varName, PyObject* value);

    // Error handling
    std::string GetLastError();
    void ClearError();
    bool HasError();

    // Engine integration
    void SetEngine(Engine* engine) { engine_ = engine; }
    Engine* GetEngine() { return engine_; }

    // Script lifecycle callbacks
    void RegisterStartCallback(const std::string& scriptName, std::function<void()> callback);
    void RegisterUpdateCallback(const std::string& scriptName, std::function<void(float)> callback);
    void RegisterShutdownCallback(const std::string& scriptName, std::function<void()> callback);

    // Trigger callbacks
    void CallStartCallbacks();
    void CallUpdateCallbacks(float deltaTime);
    void CallShutdownCallbacks();

    // Type conversion helpers
    static PyObject* ToPython(int value);
    static PyObject* ToPython(float value);
    static PyObject* ToPython(bool value);
    static PyObject* ToPython(const std::string& value);
    static int FromPython(PyObject* obj, int defaultValue);
    static float FromPython(PyObject* obj, float defaultValue);
    static bool FromPython(PyObject* obj, bool defaultValue);
    static std::string FromPython(PyObject* obj, const std::string& defaultValue);

    // Python path management
    void AddModulePath(const std::string& path);
    std::vector<std::string> GetModulePaths();

private:
    // Internal initialization
    bool InitializePython();
    bool RegisterEngineBindings();
    bool SetupModulePaths();

    // Module tracking
    std::map<std::string, PyObject*> loadedModules_;
    std::map<std::string, time_t> moduleTimestamps_;

    // Callbacks
    std::map<std::string, std::function<void()>> startCallbacks_;
    std::map<std::string, std::function<void(float)>> updateCallbacks_;
    std::map<std::string, std::function<void()>> shutdownCallbacks_;

    // State
    Engine* engine_;
    bool hotReloadEnabled_;
    bool initialized_;
    std::string lastError_;

    // Python state
    PyObject* mainModule_;
    PyObject* mainDict_;
};

// Python C API bindings for Nexus Engine
namespace PythonBindings {
    // Module initialization
    PyObject* InitNexusModule();

    // Engine functions
    PyObject* nexus_log(PyObject* self, PyObject* args);
    PyObject* nexus_get_delta_time(PyObject* self, PyObject* args);
    PyObject* nexus_quit(PyObject* self, PyObject* args);

    // Input functions
    PyObject* nexus_get_key(PyObject* self, PyObject* args);
    PyObject* nexus_get_mouse_position(PyObject* self, PyObject* args);
    PyObject* nexus_get_mouse_button(PyObject* self, PyObject* args);

    // GameObject functions
    PyObject* nexus_create_gameobject(PyObject* self, PyObject* args);
    PyObject* nexus_destroy_gameobject(PyObject* self, PyObject* args);
    PyObject* nexus_find_gameobject(PyObject* self, PyObject* args);

    // Transform functions
    PyObject* nexus_get_position(PyObject* self, PyObject* args);
    PyObject* nexus_set_position(PyObject* self, PyObject* args);
    PyObject* nexus_get_rotation(PyObject* self, PyObject* args);
    PyObject* nexus_set_rotation(PyObject* self, PyObject* args);

    // Physics functions
    PyObject* nexus_apply_force(PyObject* self, PyObject* args);
    PyObject* nexus_set_velocity(PyObject* self, PyObject* args);
    PyObject* nexus_raycast(PyObject* self, PyObject* args);

    // Audio functions
    PyObject* nexus_play_sound(PyObject* self, PyObject* args);
    PyObject* nexus_stop_sound(PyObject* self, PyObject* args);

    // Resource loading
    PyObject* nexus_load_texture(PyObject* self, PyObject* args);
    PyObject* nexus_load_model(PyObject* self, PyObject* args);
    PyObject* nexus_load_sound(PyObject* self, PyObject* args);

    // Source Engine specific
    PyObject* nexus_load_vmf_map(PyObject* self, PyObject* args);
    PyObject* nexus_spawn_source_entity(PyObject* self, PyObject* args);
}

} // namespace Nexus

#endif // NEXUS_PYTHON_ENABLED
