#pragma once

#include <cstdint>
#include <string>

namespace Nexus {
namespace RHI {

enum class GraphicsAPI {
    DirectX11,
    Vulkan,
    OpenGL,
    Auto
};

enum class BufferUsage {
    Vertex,
    Index,
    Uniform,
    Storage
};

enum class BufferAccessMode {
    Static,
    Dynamic,
    Streaming
};

enum class PrimitiveTopology {
    TriangleList,
    TriangleStrip,
    LineList,
    LineStrip,
    PointList
};

enum class TextureFormat {
    R8_UNORM,
    RG8_UNORM,
    RGBA8_UNORM,
    RGBA8_SRGB,
    R16_FLOAT,
    RG16_FLOAT,
    RGBA16_FLOAT,
    R32_FLOAT,
    RG32_FLOAT,
    RGBA32_FLOAT,
    D24_UNORM_S8_UINT,
    D32_FLOAT,
    BC1_UNORM,
    BC3_UNORM,
    BC5_UNORM,
    BC7_UNORM
};

enum class TextureUsage {
    ShaderResource = 1 << 0,
    RenderTarget = 1 << 1,
    DepthStencil = 1 << 2,
    UnorderedAccess = 1 << 3
};

inline TextureUsage operator|(TextureUsage a, TextureUsage b) {
    return static_cast<TextureUsage>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(TextureUsage a, TextureUsage b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

enum class SamplerFilter {
    Point,
    Linear,
    Anisotropic
};

enum class SamplerAddressMode {
    Wrap,
    Clamp,
    Mirror,
    Border
};

enum class CompareFunc {
    Never,
    Less,
    Equal,
    LessEqual,
    Greater,
    NotEqual,
    GreaterEqual,
    Always
};

enum class BlendFactor {
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    SrcAlpha,
    InvSrcAlpha,
    DstColor,
    InvDstColor,
    DstAlpha,
    InvDstAlpha
};

enum class BlendOp {
    Add,
    Subtract,
    RevSubtract,
    Min,
    Max
};

enum class CullMode {
    None,
    Front,
    Back
};

enum class FillMode {
    Solid,
    Wireframe
};

enum class ShaderStage {
    Vertex = 1 << 0,
    Fragment = 1 << 1,
    Geometry = 1 << 2,
    Compute = 1 << 3,
    TessControl = 1 << 4,
    TessEvaluation = 1 << 5
};

inline ShaderStage operator|(ShaderStage a, ShaderStage b) {
    return static_cast<ShaderStage>(static_cast<int>(a) | static_cast<int>(b));
}

inline bool operator&(ShaderStage a, ShaderStage b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

struct Viewport {
    float x;
    float y;
    float width;
    float height;
    float minDepth;
    float maxDepth;
};

struct Scissor {
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
};

struct ClearColor {
    float r, g, b, a;
};

struct BufferDesc {
    uint64_t size;
    BufferUsage usage;
    BufferAccessMode accessMode;
};

struct TextureDesc {
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t mipLevels;
    uint32_t arraySize;
    TextureFormat format;
    TextureUsage usage;
    uint32_t sampleCount;
};

struct SamplerDesc {
    SamplerFilter minFilter;
    SamplerFilter magFilter;
    SamplerFilter mipFilter;
    SamplerAddressMode addressU;
    SamplerAddressMode addressV;
    SamplerAddressMode addressW;
    float mipLodBias;
    uint32_t maxAnisotropy;
    CompareFunc compareFunc;
    float borderColor[4];
    float minLod;
    float maxLod;
};

struct BlendStateDesc {
    bool blendEnable;
    BlendFactor srcBlend;
    BlendFactor dstBlend;
    BlendOp blendOp;
    BlendFactor srcBlendAlpha;
    BlendFactor dstBlendAlpha;
    BlendOp blendOpAlpha;
};

struct DepthStencilStateDesc {
    bool depthEnable;
    bool depthWriteEnable;
    CompareFunc depthFunc;
    bool stencilEnable;
};

struct RasterizerStateDesc {
    FillMode fillMode;
    CullMode cullMode;
    bool frontCounterClockwise;
    int32_t depthBias;
    float depthBiasClamp;
    float slopeScaledDepthBias;
    bool depthClipEnable;
    bool scissorEnable;
    bool multisampleEnable;
    bool antialiasedLineEnable;
};

} // namespace RHI
} // namespace Nexus
