#pragma once

#include "Platform.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace Nexus {

using Microsoft::WRL::ComPtr;

// Post-Processing Effects Manager
class PostProcessing {
public:
    struct BloomSettings {
        bool enabled = true;
        float threshold = 1.0f;      // Brightness threshold
        float intensity = 0.8f;      // Bloom intensity
        float softThreshold = 0.5f;  // Soft threshold for smooth falloff
        int blurPasses = 5;          // Number of blur iterations
        float blurRadius = 1.0f;     // Blur radius multiplier
    };

    struct TonemappingSettings {
        enum class Mode {
            None,
            Reinhard,
            ReinhardLuminance,
            ACES,
            Uncharted2
        };

        bool enabled = true;
        Mode mode = Mode::ACES;
        float exposure = 1.0f;
        float whitePoint = 11.2f;
    };

    struct ColorGradingSettings {
        bool enabled = false;
        DirectX::XMFLOAT3 colorFilter = {1.0f, 1.0f, 1.0f};
        float saturation = 1.0f;
        float contrast = 1.0f;
        float brightness = 0.0f;
        float gamma = 2.2f;
    };

    struct VignetteSettings {
        bool enabled = false;
        float intensity = 0.5f;
        float smoothness = 0.5f;
        DirectX::XMFLOAT3 color = {0.0f, 0.0f, 0.0f};
    };

    struct ChromaticAberrationSettings {
        bool enabled = false;
        float intensity = 0.02f;
    };

    struct FilmGrainSettings {
        bool enabled = false;
        float intensity = 0.1f;
        float size = 1.0f;
    };

    PostProcessing();
    ~PostProcessing();

    bool Initialize(ID3D11Device* device, int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    // Apply all enabled post-processing effects
    void Apply(ID3D11DeviceContext* context,
               ID3D11ShaderResourceView* inputSRV,
               ID3D11RenderTargetView* outputRTV);

    // Individual effect controls
    void SetBloomSettings(const BloomSettings& settings) { bloom_ = settings; }
    void SetTonemappingSettings(const TonemappingSettings& settings) { tonemapping_ = settings; }
    void SetColorGradingSettings(const ColorGradingSettings& settings) { colorGrading_ = settings; }
    void SetVignetteSettings(const VignetteSettings& settings) { vignette_ = settings; }
    void SetChromaticAberrationSettings(const ChromaticAberrationSettings& settings) { chromatic_ = settings; }
    void SetFilmGrainSettings(const FilmGrainSettings& settings) { filmGrain_ = settings; }

    BloomSettings& GetBloomSettings() { return bloom_; }
    TonemappingSettings& GetTonemappingSettings() { return tonemapping_; }
    ColorGradingSettings& GetColorGradingSettings() { return colorGrading_; }
    VignetteSettings& GetVignetteSettings() { return vignette_; }
    ChromaticAberrationSettings& GetChromaticAberrationSettings() { return chromatic_; }
    FilmGrainSettings& GetFilmGrainSettings() { return filmGrain_; }

    // UI for tweaking effects
    void RenderUI();

private:
    struct RenderTarget {
        ComPtr<ID3D11Texture2D> texture;
        ComPtr<ID3D11RenderTargetView> rtv;
        ComPtr<ID3D11ShaderResourceView> srv;
        int width;
        int height;
    };

    bool CreateRenderTarget(int width, int height, DXGI_FORMAT format, RenderTarget& rt);
    void ApplyBloom(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
    void ApplyTonemapping(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
    void ApplyColorGrading(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
    void ApplyVignette(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
    void ApplyChromaticAberration(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
    void ApplyFilmGrain(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);

    void ExtractBrightPixels(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);
    void GaussianBlur(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output, bool horizontal);
    void Combine(ID3D11DeviceContext* context, ID3D11ShaderResourceView* scene, ID3D11ShaderResourceView* bloom, ID3D11RenderTargetView* output);
    void Blit(ID3D11DeviceContext* context, ID3D11ShaderResourceView* input, ID3D11RenderTargetView* output);

    ID3D11Device* device_;
    int width_;
    int height_;

    // Settings
    BloomSettings bloom_;
    TonemappingSettings tonemapping_;
    ColorGradingSettings colorGrading_;
    VignetteSettings vignette_;
    ChromaticAberrationSettings chromatic_;
    FilmGrainSettings filmGrain_;

    // Render targets for bloom
    RenderTarget brightPass_;
    RenderTarget bloomDownsample1_;
    RenderTarget bloomDownsample2_;
    RenderTarget bloomDownsample3_;
    RenderTarget bloomBlurH_;
    RenderTarget bloomBlurV_;
    RenderTarget tempRT_;

    // Shaders (would be loaded from files)
    ComPtr<ID3D11VertexShader> fullscreenVS_;
    ComPtr<ID3D11PixelShader> brightPassPS_;
    ComPtr<ID3D11PixelShader> blurPS_;
    ComPtr<ID3D11PixelShader> combinePS_;
    ComPtr<ID3D11PixelShader> tonemapPS_;
    ComPtr<ID3D11PixelShader> colorGradePS_;
    ComPtr<ID3D11PixelShader> vignettePS_;
    ComPtr<ID3D11PixelShader> chromaticPS_;
    ComPtr<ID3D11PixelShader> filmGrainPS_;
    ComPtr<ID3D11PixelShader> blitPS_;

    // Constant buffers
    struct BloomConstants {
        float threshold;
        float intensity;
        float softThreshold;
        float padding;
    };

    struct TonemapConstants {
        float exposure;
        float whitePoint;
        int mode;
        float padding;
    };

    struct ColorGradeConstants {
        DirectX::XMFLOAT3 colorFilter;
        float saturation;
        float contrast;
        float brightness;
        float gamma;
        float padding;
    };

    struct VignetteConstants {
        float intensity;
        float smoothness;
        DirectX::XMFLOAT2 padding;
        DirectX::XMFLOAT3 color;
        float padding2;
    };

    struct ChromaticConstants {
        float intensity;
        DirectX::XMFLOAT3 padding;
    };

    struct FilmGrainConstants {
        float intensity;
        float size;
        float time;
        float padding;
    };

    ComPtr<ID3D11Buffer> bloomCB_;
    ComPtr<ID3D11Buffer> tonemapCB_;
    ComPtr<ID3D11Buffer> colorGradeCB_;
    ComPtr<ID3D11Buffer> vignetteCB_;
    ComPtr<ID3D11Buffer> chromaticCB_;
    ComPtr<ID3D11Buffer> filmGrainCB_;

    // Samplers
    ComPtr<ID3D11SamplerState> linearSampler_;
    ComPtr<ID3D11SamplerState> pointSampler_;

    bool showUI_ = false;
};

// Simple Bloom Implementation (inline for quick setup)
inline void PostProcessing::ApplyBloom(ID3D11DeviceContext* context,
                                       ID3D11ShaderResourceView* input,
                                       ID3D11RenderTargetView* output) {
    if (!bloom_.enabled) {
        Blit(context, input, output);
        return;
    }

    // 1. Extract bright pixels
    ExtractBrightPixels(context, input, brightPass_.rtv.Get());

    // 2. Downsample and blur
    Blit(context, brightPass_.srv.Get(), bloomDownsample1_.rtv.Get());

    for (int i = 0; i < bloom_.blurPasses; i++) {
        // Horizontal blur
        GaussianBlur(context, bloomDownsample1_.srv.Get(), bloomBlurH_.rtv.Get(), true);
        // Vertical blur
        GaussianBlur(context, bloomBlurH_.srv.Get(), bloomBlurV_.rtv.Get(), false);
        // Copy back for next iteration
        if (i < bloom_.blurPasses - 1) {
            Blit(context, bloomBlurV_.srv.Get(), bloomDownsample1_.rtv.Get());
        }
    }

    // 3. Combine with original scene
    Combine(context, input, bloomBlurV_.srv.Get(), output);
}

} // namespace Nexus
