/**
 * @file RHIDefinitions.h
 * @brief Core definitions and types for the Render Hardware Interface (RHI)
 * @author Nexus Engine Team
 * @date 2026-07-22
 * @version 1.0.0
 *
 * This file contains all common types, enumerations, and structures used across
 * all graphics API backends (Vulkan, DirectX 11, OpenGL, etc.).
 *
 * The RHI provides a unified interface for graphics operations, allowing the engine
 * to support multiple graphics APIs without code duplication.
 *
 * @note All RHI types are platform-agnostic and map to native API types in backend implementations
 *
 * Example usage:
 * @code
 * using namespace Nexus::RHI;
 *
 * BufferDesc desc{};
 * desc.size = 1024;
 * desc.usage = BufferUsage::Vertex;
 * desc.accessMode = BufferAccessMode::Static;
 *
 * auto buffer = device->CreateBuffer(desc, vertexData);
 * @endcode
 */

#pragma once

#include <cstdint>
#include <string>

namespace Nexus {
namespace RHI {

/**
 * @enum GraphicsAPI
 * @brief Supported graphics API backends
 */
enum class GraphicsAPI {
    DirectX11,  ///< Microsoft DirectX 11 (Windows only)
    Vulkan,     ///< Khronos Vulkan 1.3 (Cross-platform)
    OpenGL,     ///< Khronos OpenGL 4.5+ (Cross-platform)
    Auto        ///< Automatically select best available API
};

/**
 * @enum BufferUsage
 * @brief Defines how a buffer will be used in the rendering pipeline
 */
enum class BufferUsage {
    Vertex,     ///< Vertex data buffer (positions, normals, UVs, etc.)
    Index,      ///< Index buffer for indexed drawing
    Uniform,    ///< Uniform/constant buffer for shader parameters
    Storage     ///< Storage buffer for compute shaders and general data
};

/**
 * @enum BufferAccessMode
 * @brief Defines how CPU will access buffer memory
 */
enum class BufferAccessMode {
    Static,     ///< Written once, read many times (best performance)
    Dynamic,    ///< Updated frequently from CPU
    Streaming   ///< Written once per frame, then discarded
};

/**
 * @enum PrimitiveTopology
 * @brief Defines how vertices are interpreted as primitives
 */
enum class PrimitiveTopology {
    TriangleList,   ///< Every 3 vertices form a triangle
    TriangleStrip,  ///< Each vertex after first 2 forms a triangle with previous 2
    LineList,       ///< Every 2 vertices form a line
    LineStrip,      ///< Each vertex after first forms a line with previous
    PointList       ///< Each vertex is rendered as a point
};

/**
 * @enum TextureFormat
 * @brief Pixel formats for textures and render targets
 */
enum class TextureFormat {
    // 8-bit formats
    R8_UNORM,           ///< 8-bit red channel, normalized [0,1]
    RG8_UNORM,          ///< 8-bit RG channels, normalized
    RGBA8_UNORM,        ///< 8-bit RGBA, normalized (standard format)
    RGBA8_SRGB,         ///< 8-bit RGBA with sRGB gamma correction

    // 16-bit float formats
    R16_FLOAT,          ///< 16-bit float red channel
    RG16_FLOAT,         ///< 16-bit float RG channels
    RGBA16_FLOAT,       ///< 16-bit float RGBA (HDR format)

    // 32-bit float formats
    R32_FLOAT,          ///< 32-bit float red channel
    RG32_FLOAT,         ///< 32-bit float RG channels
    RGBA32_FLOAT,       ///< 32-bit float RGBA (full HDR)

    // Depth/stencil formats
    D24_UNORM_S8_UINT,  ///< 24-bit depth, 8-bit stencil (common)
    D32_FLOAT,          ///< 32-bit float depth (high precision)

    // Compressed formats
    BC1_UNORM,          ///< Block compressed RGB (DXT1)
    BC3_UNORM,          ///< Block compressed RGBA (DXT5)
    BC5_UNORM,          ///< Block compressed normal maps
    BC7_UNORM           ///< High quality block compression
};

/**
 * @enum TextureUsage
 * @brief Bitflags defining how a texture will be used
 */
enum class TextureUsage {
    ShaderResource = 1 << 0,    ///< Texture can be read in shaders
    RenderTarget = 1 << 1,      ///< Texture can be rendered to
    DepthStencil = 1 << 2,      ///< Texture is depth/stencil buffer
    UnorderedAccess = 1 << 3    ///< Texture can be written in compute shaders
};

/// Bitwise OR operator for TextureUsage flags
inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<int>(a) | static_cast<int>(b));
}

/// Bitwise AND operator for TextureUsage flags
inline bool operator&(TextureUsage a, TextureUsage b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

/**
 * @enum SamplerFilter
 * @brief Texture filtering modes
 */
enum class SamplerFilter {
    Point,          ///< Nearest-neighbor filtering (pixelated)
    Linear,         ///< Bilinear filtering (smooth)
    Anisotropic     ///< Anisotropic filtering (highest quality)
};

/**
 * @enum SamplerAddressMode
 * @brief Defines texture coordinate behavior outside [0,1] range
 */
enum class SamplerAddressMode {
    Wrap,       ///< Repeat texture (tiling)
    Clamp,      ///< Clamp to edge color
    Mirror,     ///< Mirror texture on each repeat
    Border      ///< Use border color
};

/**
 * @enum CompareFunc
 * @brief Comparison functions for depth testing
 */
enum class CompareFunc {
    Never,          ///< Never pass
    Less,           ///< Pass if less than (standard depth test)
    Equal,          ///< Pass if equal
    LessEqual,      ///< Pass if less than or equal
    Greater,        ///< Pass if greater than
    NotEqual,       ///< Pass if not equal
    GreaterEqual,   ///< Pass if greater than or equal
    Always          ///< Always pass
};

/**
 * @enum BlendFactor
 * @brief Blend factors for color blending
 */
enum class BlendFactor {
    Zero,           ///< (0, 0, 0, 0)
    One,            ///< (1, 1, 1, 1)
    SrcColor,       ///< Source color
    InvSrcColor,    ///< 1 - source color
    SrcAlpha,       ///< Source alpha
    InvSrcAlpha,    ///< 1 - source alpha
    DstColor,       ///< Destination color
    InvDstColor,    ///< 1 - destination color
    DstAlpha,       ///< Destination alpha
    InvDstAlpha     ///< 1 - destination alpha
};

/**
 * @enum BlendOp
 * @brief Blend operations
 */
enum class BlendOp {
    Add,            ///< src + dst
    Subtract,       ///< src - dst
    RevSubtract,    ///< dst - src
    Min,            ///< min(src, dst)
    Max             ///< max(src, dst)
};

/**
 * @enum CullMode
 * @brief Face culling modes
 */
enum class CullMode {
    None,   ///< No culling (draw all faces)
    Front,  ///< Cull front faces
    Back    ///< Cull back faces (standard)
};

/**
 * @enum FillMode
 * @brief Polygon fill modes
 */
enum class FillMode {
    Solid,      ///< Fill polygons (standard)
    Wireframe   ///< Draw only edges (debugging)
};

/**
 * @enum ShaderStage
 * @brief Shader pipeline stages
 */
enum class ShaderStage {
    Vertex = 1 << 0,            ///< Vertex shader
    Fragment = 1 << 1,          ///< Fragment/pixel shader
    Geometry = 1 << 2,          ///< Geometry shader (optional)
    Compute = 1 << 3,           ///< Compute shader
    TessControl = 1 << 4,       ///< Tessellation control (optional)
    TessEvaluation = 1 << 5     ///< Tessellation evaluation (optional)
};

/// Bitwise OR operator for ShaderStage flags
inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
    return static_cast<ShaderStage>(static_cast<int>(a) | static_cast<int>(b));
}

/// Bitwise AND operator for ShaderStage flags
inline bool operator&(ShaderStage a, ShaderStage b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

/**
 * @struct Viewport
 * @brief Defines the viewport transformation
 */
struct Viewport {
    float x;            ///< Top-left X coordinate
    float y;            ///< Top-left Y coordinate
    float width;        ///< Viewport width in pixels
    float height;       ///< Viewport height in pixels
    float minDepth;     ///< Minimum depth value (typically 0.0)
    float maxDepth;     ///< Maximum depth value (typically 1.0)
};

/**
 * @struct Scissor
 * @brief Defines the scissor rectangle for clipping
 */
struct Scissor {
    int32_t x;          ///< Top-left X coordinate
    int32_t y;          ///< Top-left Y coordinate
    uint32_t width;     ///< Rectangle width in pixels
    uint32_t height;    ///< Rectangle height in pixels
};

/**
 * @struct ClearColor
 * @brief RGBA clear color
 */
struct ClearColor {
    float r, g, b, a;   ///< Red, green, blue, alpha [0,1]
};

/**
 * @struct BufferDesc
 * @brief Descriptor for buffer creation
 */
struct BufferDesc {
    uint64_t size;              ///< Buffer size in bytes
    BufferUsage usage;          ///< How buffer will be used
    BufferAccessMode accessMode;///< CPU access pattern
};

/**
 * @struct TextureDesc
 * @brief Descriptor for texture creation
 */
struct TextureDesc {
    uint32_t width;         ///< Texture width in pixels
    uint32_t height;        ///< Texture height in pixels
    uint32_t depth;         ///< Texture depth (1 for 2D textures, >1 for 3D)
    uint32_t mipLevels;     ///< Number of mipmap levels (1 = no mipmaps)
    uint32_t arraySize;     ///< Number of array layers (1 for non-array)
    TextureFormat format;   ///< Pixel format
    TextureUsage usage;     ///< Usage flags
    uint32_t sampleCount;   ///< MSAA sample count (1 = no MSAA)
};

/**
 * @struct SamplerDesc
 * @brief Descriptor for sampler state creation
 */
struct SamplerDesc {
    SamplerFilter minFilter;        ///< Minification filter
    SamplerFilter magFilter;        ///< Magnification filter
    SamplerFilter mipFilter;        ///< Mipmap filter
    SamplerAddressMode addressU;    ///< U coordinate addressing
    SamplerAddressMode addressV;    ///< V coordinate addressing
    SamplerAddressMode addressW;    ///< W coordinate addressing
    float mipLodBias;               ///< Mipmap LOD bias
    uint32_t maxAnisotropy;         ///< Max anisotropy (1-16)
    CompareFunc compareFunc;        ///< Comparison function for shadow sampling
    float borderColor[4];           ///< Border color for Border address mode
    float minLod;                   ///< Minimum LOD level
    float maxLod;                   ///< Maximum LOD level
};

/**
 * @struct BlendStateDesc
 * @brief Descriptor for color blending state
 */
struct BlendStateDesc {
    bool blendEnable;           ///< Enable blending
    BlendFactor srcBlend;       ///< Source color blend factor
    BlendFactor dstBlend;       ///< Destination color blend factor
    BlendOp blendOp;            ///< Color blend operation
    BlendFactor srcBlendAlpha;  ///< Source alpha blend factor
    BlendFactor dstBlendAlpha;  ///< Destination alpha blend factor
    BlendOp blendOpAlpha;       ///< Alpha blend operation
};

/**
 * @struct DepthStencilStateDesc
 * @brief Descriptor for depth-stencil state
 */
struct DepthStencilStateDesc {
    bool depthEnable;           ///< Enable depth testing
    bool depthWriteEnable;      ///< Enable writing to depth buffer
    CompareFunc depthFunc;      ///< Depth comparison function
    bool stencilEnable;         ///< Enable stencil testing
};

/**
 * @struct RasterizerStateDesc
 * @brief Descriptor for rasterization state
 */
struct RasterizerStateDesc {
    FillMode fillMode;                  ///< Fill mode (solid/wireframe)
    CullMode cullMode;                  ///< Face culling mode
    bool frontCounterClockwise;         ///< Front face winding order
    int32_t depthBias;                  ///< Depth bias for shadow mapping
    float depthBiasClamp;               ///< Maximum depth bias
    float slopeScaledDepthBias;         ///< Slope-scaled depth bias
    bool depthClipEnable;               ///< Enable depth clipping
    bool scissorEnable;                 ///< Enable scissor test
    bool multisampleEnable;             ///< Enable multisampling
    bool antialiasedLineEnable;         ///< Enable line antialiasing
};

} // namespace RHI
} // namespace Nexus
