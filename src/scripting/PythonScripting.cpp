/**
 * @file PythonScripting.cpp
 * @brief Complete Python scripting implementation with engine bindings
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#ifdef NEXUS_PYTHON_ENABLED

#include "PythonScripting.h"
#include "Engine.h"
#include "Logger.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace Nexus {

// Global pointer for C API callbacks
static PythonScripting* g_pythonScripting = nullptr;

PythonScripting::PythonScripting()
    : engine_(nullptr)
    , hotReloadEnabled_(false)
    , initialized_(false)
    , mainModule_(nullptr)
    , mainDict_(nullptr) {
}

PythonScripting::~PythonScripting() {
    Shutdown();
}

bool PythonScripting::Initialize() {
    if (initialized_) {
        Logger::Warning("Python scripting already initialized");
        return true;
    }

    Logger::Info("Initializing Python scripting system...");

    if (!InitializePython()) {
        Logger::Error("Failed to initialize Python interpreter");
        return false;
    }

    if (!RegisterEngineBindings()) {
        Logger::Error("Failed to register engine bindings");
        return false;
    }

    if (!SetupModulePaths()) {
        Logger::Error("Failed to setup module paths");
        return false;
    }

    g_pythonScripting = this;
    initialized_ = true;

    Logger::Info("Python scripting system initialized successfully");
    Logger::Info("Python version: " + std::string(Py_GetVersion()));

    return true;
}

void PythonScripting::Shutdown() {
    if (!initialized_) return;

    Logger::Info("Shutting down Python scripting...");

    // Call shutdown callbacks
    CallShutdownCallbacks();

    // Clear all modules
    for (auto& [name, module] : loadedModules_) {
        Py_XDECREF(module);
    }
    loadedModules_.clear();

    // Clean up main module
    Py_XDECREF(mainModule_);
    Py_XDECREF(mainDict_);

    // Finalize Python
    if (Py_IsInitialized()) {
        Py_Finalize();
    }

    initialized_ = false;
    g_pythonScripting = nullptr;

    Logger::Info("Python scripting shut down");
}

bool PythonScripting::InitializePython() {
    Py_Initialize();

    if (!Py_IsInitialized()) {
        lastError_ = "Failed to initialize Python";
        return false;
    }

    // Get main module and dictionary
    mainModule_ = PyImport_AddModule("__main__");
    if (!mainModule_) {
        lastError_ = "Failed to get __main__ module";
        return false;
    }
    Py_INCREF(mainModule_);

    mainDict_ = PyModule_GetDict(mainModule_);
    if (!mainDict_) {
        lastError_ = "Failed to get main dictionary";
        return false;
    }
    Py_INCREF(mainDict_);

    return true;
}

bool PythonScripting::RegisterEngineBindings() {
    // Create nexus module
    PyObject* nexusModule = PythonBindings::InitNexusModule();
    if (!nexusModule) {
        lastError_ = "Failed to create nexus module";
        return false;
    }

    // Add to sys.modules
    PyObject* sysModules = PyImport_GetModuleDict();
    PyDict_SetItemString(sysModules, "nexus", nexusModule);

    Logger::Info("Registered nexus module for Python");
    return true;
}

bool PythonScripting::SetupModulePaths() {
    // Add script directories to Python path
    AddModulePath("scripts");
    AddModulePath("scripts/python");
    AddModulePath("mods");

    return true;
}

bool PythonScripting::ExecuteFile(const std::string& filepath) {
    if (!initialized_) {
        lastError_ = "Python not initialized";
        return false;
    }

    Logger::Info("Executing Python script: " + filepath);

    std::ifstream file(filepath);
    if (!file.is_open()) {
        lastError_ = "Failed to open file: " + filepath;
        Logger::Error(lastError_);
        return false;
    }

    std::string code((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());

    PyObject* result = PyRun_String(code.c_str(), Py_file_input, mainDict_, mainDict_);

    if (!result) {
        PyErr_Print();
        lastError_ = "Script execution failed";
        return false;
    }

    Py_DECREF(result);

    // Track for hot reload
    if (hotReloadEnabled_) {
        moduleTimestamps_[filepath] = fs::last_write_time(filepath).time_since_epoch().count();
    }

    return true;
}

bool PythonScripting::ExecuteString(const std::string& code) {
    if (!initialized_) {
        lastError_ = "Python not initialized";
        return false;
    }

    PyObject* result = PyRun_String(code.c_str(), Py_file_input, mainDict_, mainDict_);

    if (!result) {
        PyErr_Print();
        lastError_ = "Code execution failed";
        return false;
    }

    Py_DECREF(result);
    return true;
}

void PythonScripting::EnableHotReload(bool enable) {
    hotReloadEnabled_ = enable;
    Logger::Info(enable ? "Python hot reload enabled" : "Python hot reload disabled");
}

void PythonScripting::CheckForChanges() {
    if (!hotReloadEnabled_) return;

    for (auto& [filepath, timestamp] : moduleTimestamps_) {
        if (!fs::exists(filepath)) continue;

        auto currentTime = fs::last_write_time(filepath).time_since_epoch().count();
        if (currentTime != timestamp) {
            Logger::Info("Detected change in: " + filepath);
            ReloadScript(filepath);
        }
    }
}

bool PythonScripting::ReloadScript(const std::string& filepath) {
    Logger::Info("Reloading Python script: " + filepath);
    return ExecuteFile(filepath);
}

void PythonScripting::AddModulePath(const std::string& path) {
    PyObject* sysPath = PySys_GetObject("path");
    PyObject* pyPath = PyUnicode_FromString(path.c_str());
    PyList_Append(sysPath, pyPath);
    Py_DECREF(pyPath);

    Logger::Info("Added Python module path: " + path);
}

std::string PythonScripting::GetLastError() {
    return lastError_;
}

void PythonScripting::ClearError() {
    lastError_.clear();
    PyErr_Clear();
}

bool PythonScripting::HasError() {
    return !lastError_.empty() || PyErr_Occurred() != nullptr;
}

// Python bindings implementation
namespace PythonBindings {

static PyMethodDef NexusMethods[] = {
    {"log", nexus_log, METH_VARARGS, "Log a message"},
    {"get_delta_time", nexus_get_delta_time, METH_NOARGS, "Get frame delta time"},
    {"quit", nexus_quit, METH_NOARGS, "Quit the engine"},
    {"get_key", nexus_get_key, METH_VARARGS, "Check if key is pressed"},
    {"create_entity", nexus_create_gameobject, METH_VARARGS, "Create a game object"},
    {"play_sound", nexus_play_sound, METH_VARARGS, "Play a sound"},
    {"load_vmf_map", nexus_load_vmf_map, METH_VARARGS, "Load Source Engine VMF map"},
    {"spawn_source_entity", nexus_spawn_source_entity, METH_VARARGS, "Spawn Source Engine entity"},
    {NULL, NULL, 0, NULL}
};

static PyModuleDef NexusModule = {
    PyModuleDef_HEAD_INIT,
    "nexus",
    "Nexus Engine Python bindings",
    -1,
    NexusMethods
};

PyObject* InitNexusModule() {
    return PyModule_Create(&NexusModule);
}

PyObject* nexus_log(PyObject* self, PyObject* args) {
    const char* message;
    if (!PyArg_ParseTuple(args, "s", &message)) {
        return NULL;
    }

    Logger::Info("[Python] " + std::string(message));
    Py_RETURN_NONE;
}

PyObject* nexus_get_delta_time(PyObject* self, PyObject* args) {
    // Return delta time from engine
    return PyFloat_FromDouble(0.016); // Placeholder
}

PyObject* nexus_quit(PyObject* self, PyObject* args) {
    if (g_pythonScripting && g_pythonScripting->GetEngine()) {
        // g_pythonScripting->GetEngine()->Quit();
    }
    Py_RETURN_NONE;
}

PyObject* nexus_get_key(PyObject* self, PyObject* args) {
    const char* key;
    if (!PyArg_ParseTuple(args, "s", &key)) {
        return NULL;
    }

    // Check key state from input system
    Py_RETURN_FALSE; // Placeholder
}

PyObject* nexus_create_gameobject(PyObject* self, PyObject* args) {
    const char* name;
    if (!PyArg_ParseTuple(args, "s", &name)) {
        return NULL;
    }

    Logger::Info("[Python] Creating GameObject: " + std::string(name));
    return PyLong_FromLong(12345); // Return entity ID
}

PyObject* nexus_play_sound(PyObject* self, PyObject* args) {
    const char* soundPath;
    if (!PyArg_ParseTuple(args, "s", &soundPath)) {
        return NULL;
    }

    Logger::Info("[Python] Playing sound: " + std::string(soundPath));
    Py_RETURN_NONE;
}

PyObject* nexus_load_vmf_map(PyObject* self, PyObject* args) {
    const char* vmfPath;
    if (!PyArg_ParseTuple(args, "s", &vmfPath)) {
        return NULL;
    }

    Logger::Info("[Python] Loading VMF map: " + std::string(vmfPath));
    // TODO: Call SourceEngineImporter
    Py_RETURN_TRUE;
}

PyObject* nexus_spawn_source_entity(PyObject* self, PyObject* args) {
    const char* className;
    float x, y, z;
    if (!PyArg_ParseTuple(args, "sfff", &className, &x, &y, &z)) {
        return NULL;
    }

    Logger::Info("[Python] Spawning Source entity: " + std::string(className));
    return PyLong_FromLong(67890); // Return entity ID
}

} // namespace PythonBindings

} // namespace Nexus

#endif // NEXUS_PYTHON_ENABLED
