/**
 * @file LuaScripting.h
 * @brief Complete Lua 5.4 scripting system with engine bindings
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#ifdef NEXUS_LUA_ENABLED

extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <memory>

namespace Nexus {

class Engine;

/**
 * Lua scripting engine with full engine bindings
 * Designed for Source Engine mod scripting
 */
class LuaScripting {
public:
    LuaScripting();
    ~LuaScripting();

    // Core initialization
    bool Initialize();
    void Shutdown();

    // Script execution
    bool ExecuteFile(const std::string& filepath);
    bool ExecuteString(const std::string& code);

    // Function calls
    bool CallFunction(const std::string& functionName);
    bool CallFunction(const std::string& functionName, int numArgs, int numResults = 0);

    // Global variable access
    void SetGlobal(const std::string& name, int value);
    void SetGlobal(const std::string& name, float value);
    void SetGlobal(const std::string& name, bool value);
    void SetGlobal(const std::string& name, const std::string& value);
    void SetGlobal(const std::string& name, void* ptr);

    int GetGlobalInt(const std::string& name, int defaultValue = 0);
    float GetGlobalFloat(const std::string& name, float defaultValue = 0.0f);
    bool GetGlobalBool(const std::string& name, bool defaultValue = false);
    std::string GetGlobalString(const std::string& name, const std::string& defaultValue = "");

    // Table operations
    void CreateTable(const std::string& name);
    void SetTableValue(const std::string& tableName, const std::string& key, int value);
    void SetTableValue(const std::string& tableName, const std::string& key, float value);
    void SetTableValue(const std::string& tableName, const std::string& key, const std::string& value);

    // Module management
    bool LoadModule(const std::string& moduleName);
    bool ReloadModule(const std::string& moduleName);

    // Hot reload
    void EnableHotReload(bool enable);
    void CheckForChanges();

    // Error handling
    std::string GetLastError();
    void ClearError();
    bool HasError();

    // Engine integration
    void SetEngine(Engine* engine) { engine_ = engine; }
    Engine* GetEngine() { return engine_; }

    // Script lifecycle
    void CallStartCallbacks();
    void CallUpdateCallbacks(float deltaTime);
    void CallShutdownCallbacks();

    // Direct Lua state access (advanced)
    lua_State* GetState() { return L_; }

    // Stack helpers
    void PushInt(int value);
    void PushFloat(float value);
    void PushBool(bool value);
    void PushString(const std::string& value);
    void PushNil();

    int PopInt();
    float PopFloat();
    bool PopBool();
    std::string PopString();

private:
    // Internal initialization
    bool InitializeLua();
    bool RegisterEngineBindings();
    void RegisterStandardLibraries();

    // Error handling
    void HandleError(int status);
    static int LuaErrorHandler(lua_State* L);

    // Module tracking
    std::map<std::string, time_t> moduleTimestamps_;

    // State
    lua_State* L_;
    Engine* engine_;
    bool hotReloadEnabled_;
    bool initialized_;
    std::string lastError_;
};

// Lua C API bindings for Nexus Engine
namespace LuaBindings {
    // Engine functions
    int lua_log(lua_State* L);
    int lua_get_delta_time(lua_State* L);
    int lua_quit(lua_State* L);

    // Input functions
    int lua_get_key(lua_State* L);
    int lua_get_mouse_position(lua_State* L);
    int lua_get_mouse_button(lua_State* L);

    // GameObject functions
    int lua_create_entity(lua_State* L);
    int lua_destroy_entity(lua_State* L);
    int lua_find_entity(lua_State* L);
    int lua_get_entity_position(lua_State* L);
    int lua_set_entity_position(lua_State* L);

    // Physics functions
    int lua_apply_force(lua_State* L);
    int lua_set_velocity(lua_State* L);
    int lua_raycast(lua_State* L);

    // Audio functions
    int lua_play_sound(lua_State* L);
    int lua_stop_sound(lua_State* L);

    // Resource loading
    int lua_load_texture(lua_State* L);
    int lua_load_model(lua_State* L);
    int lua_load_sound(lua_State* L);

    // Source Engine specific functions
    int lua_load_vmf_map(lua_State* L);
    int lua_spawn_source_entity(lua_State* L);
    int lua_get_source_entity_by_name(lua_State* L);
    int lua_trigger_source_output(lua_State* L);

    // Valve mod support
    int lua_register_convar(lua_State* L);
    int lua_register_concommand(lua_State* L);
    int lua_print_console(lua_State* L);

    // Helper functions
    void RegisterEngineFunctions(lua_State* L);
    void RegisterSourceEngineFunctions(lua_State* L);
}

} // namespace Nexus

#endif // NEXUS_LUA_ENABLED
