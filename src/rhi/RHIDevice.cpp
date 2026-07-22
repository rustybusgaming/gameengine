#include "RHI/RHIDevice.h"

#ifdef NEXUS_VULKAN_ENABLED
#include "RHI/Vulkan/VulkanDevice.h"
#endif

#ifdef NEXUS_OPENGL_ENABLED
// #include "RHI/OpenGL/OpenGLDevice.h"
#endif

#ifdef NEXUS_DIRECTX11_ENABLED
#include "RHI/D3D11/D3D11Device.h"
#endif

#include "Logger.h"
#include "EngineConfig.h"

namespace Nexus {
namespace RHI {

std::unique_ptr<RHIDevice> RHIDevice::Create(GraphicsAPI api) {
    if (api == GraphicsAPI::Auto) {
        #ifdef NEXUS_VULKAN_ENABLED
        api = GraphicsAPI::Vulkan;
        #elif defined(NEXUS_DIRECTX11_ENABLED)
        api = GraphicsAPI::DirectX11;
        #elif defined(NEXUS_OPENGL_ENABLED)
        api = GraphicsAPI::OpenGL;
        #else
        Logger::Error("No graphics API available!");
        return nullptr;
        #endif
    }

    switch (api) {
        case GraphicsAPI::Vulkan:
            #ifdef NEXUS_VULKAN_ENABLED
            Logger::Info("Creating Vulkan RHI Device");
            return std::make_unique<VulkanDevice>();
            #else
            Logger::Error("Vulkan support not compiled in");
            return nullptr;
            #endif

        case GraphicsAPI::OpenGL:
            #ifdef NEXUS_OPENGL_ENABLED
            Logger::Info("Creating OpenGL RHI Device");
            // return std::make_unique<OpenGLDevice>();
            Logger::Error("OpenGL backend not yet implemented");
            return nullptr;
            #else
            Logger::Error("OpenGL support not compiled in");
            return nullptr;
            #endif

        case GraphicsAPI::DirectX11:
            #ifdef NEXUS_DIRECTX11_ENABLED
            Logger::Info("Creating DirectX 11 RHI Device");
            return std::make_unique<D3D11Device>();
            #else
            Logger::Error("DirectX 11 support not compiled in");
            return nullptr;
            #endif

        default:
            Logger::Error("Unknown graphics API");
            return nullptr;
    }
}

} // namespace RHI
} // namespace Nexus
