#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <chrono>

namespace Nexus {

// Asset types supported by the converter
enum class AssetType {
    Model,
    Texture,
    Material,
    Sound,
    Animation,
    Map,
    Scene,
    Particle,
    Script,
    Package,
    Unknown
};

// Quality settings for conversion
enum class ConversionQuality {
    Low,
    Medium,
    High
};

// Settings for asset conversion
struct ConversionSettings {
    ConversionQuality quality = ConversionQuality::High;
    bool compress = false;
    bool optimize = true;
    bool preserveOriginal = false;
    std::string outputFormat;
    float scaleFactor = 1.0f;
    std::map<std::string, std::string> customSettings;
};

// Conversion statistics
struct ConversionStats {
    size_t inputSize = 0;
    size_t outputSize = 0;
    int conversionTime = 0;
    std::string inputFormat;
    std::string outputFormat;
    int warnings = 0;
    int errors = 0;
};

// Source engine specific formats
enum class SourceFormat {
    VMF,    // Valve Map Format
    VMT,    // Valve Material Type
    VTF,    // Valve Texture Format
    MDL,    // Model
    VVD,    // Vertex data
    PHY,    // Physics model
    ANI,    // Animation
    PCF,    // Precomputed lighting
    BSP,    // Map/level format
    VCD,    // Choreography/scene
    VCO,    // Compiled object
    VPK,    // Valve Pak
    VDF     // Valve Data Format
};

// Source engine material parameters
struct SourceMaterialParams {
    std::string shader;
    std::string baseTexture;
    std::string bumpMap;
    float bumpScale = 1.0f;
    bool translucent = false;
    float opacity = 1.0f;
    float specularLevel = 0.5f;
    float glossiness = 0.5f;
    std::map<std::string, std::string> customParams;
};

// Source engine model info
struct SourceModelInfo {
    std::vector<std::string> materialPaths;
    std::string physicsModel;
    std::vector<std::string> animations;
    int lodCount = 0;
    bool hasCollision = false;
};

/**
 * Universal Asset Converter for Nexus Engine
 * Supports conversion from many formats including Source engine formats
 */
class AssetConverter {
public:
    AssetConverter();
    ~AssetConverter();
    
    // Asset detection
    AssetType DetectAssetType(const std::string& filePath);
    SourceFormat DetectSourceFormat(const std::string& filePath);
    
    // Generic conversion
    bool ConvertAsset(const std::string& inputPath, const std::string& outputPath, 
                     AssetType type, const ConversionSettings& settings = ConversionSettings());
    
    // Source engine specific conversions
    bool ConvertVMF(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings = ConversionSettings());
    bool ConvertVMT(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings = ConversionSettings());
    bool ConvertVTF(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings = ConversionSettings());
    bool ConvertMDL(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings = ConversionSettings());
    bool ConvertBSP(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings = ConversionSettings());
    bool ConvertVPK(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings = ConversionSettings());
                   
    // Source engine specific utilities
    bool ExtractVPK(const std::string& vpkPath, const std::string& outputDir);
    SourceMaterialParams ParseVMT(const std::string& vmtPath);
    SourceModelInfo ParseMDL(const std::string& mdlPath);
    
    // Batch operations
    bool BatchConvertDirectory(const std::string& inputDir, const std::string& outputDir, 
                              const ConversionSettings& settings = ConversionSettings());
    bool BatchConvertSourceGame(const std::string& gamePath, const std::string& outputDir,
                              const ConversionSettings& settings = ConversionSettings());
    
    // Statistics
    ConversionStats GetLastConversionStats() const;
    
private:
    // Utility functions
    bool CheckSourceGameValid(const std::string& gamePath);
    std::string DetectFileFormat(const std::string& filePath);
    bool ValidateSourceFormat(const std::string& filePath, SourceFormat format);
    
    // Internal conversion helpers
    bool ConvertTexture(const std::string& inputPath, const std::string& outputPath, 
                       const ConversionSettings& settings);
    bool ConvertModel(const std::string& inputPath, const std::string& outputPath, 
                     const ConversionSettings& settings);
    bool ConvertMap(const std::string& inputPath, const std::string& outputPath, 
                   const ConversionSettings& settings);
    
    // Tracking
    ConversionStats lastStats_;
    std::vector<std::string> supportedFormats_;
    std::chrono::high_resolution_clock::time_point startTime_;
};

} // namespace Nexus