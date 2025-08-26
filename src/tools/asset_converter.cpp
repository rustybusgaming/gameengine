#include "AssetConverter.h"
#include "Logger.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    std::cout << "=== NEXUS ENGINE - UNIVERSAL ASSET CONVERTER ===" << std::endl;
    
    if (argc < 3) {
        std::cout << "Usage: NexusAssetConverter <input_file> <output_file> [options]" << std::endl;
        std::cout << std::endl;
        std::cout << "Supported formats:" << std::endl;
        std::cout << "  Models: .fbx, .obj, .dae, .3ds, .blend, .gltf, .uasset, .mdl (Source)" << std::endl;
        std::cout << "  Textures: .png, .jpg, .tga, .dds, .hdr, .exr, .vtf (Source)" << std::endl;
        std::cout << "  Materials: .vmt (Source)" << std::endl;
        std::cout << "  Audio: .wav, .mp3, .ogg, .flac" << std::endl;
        std::cout << "  Animations: .fbx, .bvh, .anim, .ani (Source)" << std::endl;
        std::cout << "  Maps: .vmf, .bsp (Source)" << std::endl;
        std::cout << "  Packages: .vpk (Source)" << std::endl;
        std::cout << std::endl;
        std::cout << "Options:" << std::endl;
        std::cout << "  --quality <high|medium|low>  Set conversion quality" << std::endl;
        std::cout << "  --compress                   Enable compression" << std::endl;
        std::cout << "  --optimize                   Optimize for Nexus Engine" << std::endl;
        std::cout << "  --source-game <path>         Process an entire Source engine game directory" << std::endl;
        return 1;
    }
    
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    
    // Parse options
    Nexus::ConversionSettings settings;
    settings.quality = Nexus::ConversionQuality::High;
    settings.compress = false;
    settings.optimize = true;
    
    bool sourceGameMode = false;
    std::string sourceGamePath;
    
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--quality" && i + 1 < argc) {
            std::string quality = argv[++i];
            if (quality == "high") settings.quality = Nexus::ConversionQuality::High;
            else if (quality == "medium") settings.quality = Nexus::ConversionQuality::Medium;
            else if (quality == "low") settings.quality = Nexus::ConversionQuality::Low;
        } else if (arg == "--compress") {
            settings.compress = true;
        } else if (arg == "--optimize") {
            settings.optimize = true;
        } else if (arg == "--source-game" && i + 1 < argc) {
            sourceGameMode = true;
            sourceGamePath = argv[++i];
        }
    }
    
    try {
        Nexus::AssetConverter converter;
        bool success = false;
        
        // Handle Source engine game conversion mode
        if (sourceGameMode) {
            Nexus::Logger::Info("Starting Source engine game conversion...");
            Nexus::Logger::Info("Source Game Path: " + sourceGamePath);
            Nexus::Logger::Info("Output Directory: " + outputFile);
            
            if (!std::filesystem::exists(sourceGamePath)) {
                Nexus::Logger::Error("Source game directory does not exist: " + sourceGamePath);
                return 1;
            }
            
            // Create output directory if it doesn't exist
            std::filesystem::create_directories(outputFile);
            
            // Convert the entire Source game
            success = converter.BatchConvertSourceGame(sourceGamePath, outputFile, settings);
        } else {
            // Normal single file conversion mode
            Nexus::Logger::Info("Starting asset conversion...");
            Nexus::Logger::Info("Input: " + inputFile);
            Nexus::Logger::Info("Output: " + outputFile);
            
            if (!std::filesystem::exists(inputFile)) {
                Nexus::Logger::Error("Input file does not exist: " + inputFile);
                return 1;
            }
            
            // Check if it's a Source engine format
            Nexus::SourceFormat sourceFormat = converter.DetectSourceFormat(inputFile);
            if (sourceFormat != static_cast<Nexus::SourceFormat>(-1)) {
                Nexus::Logger::Info("Detected Source engine format");
                
                // Use appropriate conversion based on format
                switch (sourceFormat) {
                case Nexus::SourceFormat::VMF:
                    success = converter.ConvertVMF(inputFile, outputFile, settings);
                    break;
                case Nexus::SourceFormat::VMT:
                    success = converter.ConvertVMT(inputFile, outputFile, settings);
                    break;
                case Nexus::SourceFormat::VTF:
                    success = converter.ConvertVTF(inputFile, outputFile, settings);
                    break;
                case Nexus::SourceFormat::MDL:
                    success = converter.ConvertMDL(inputFile, outputFile, settings);
                    break;
                case Nexus::SourceFormat::BSP:
                    success = converter.ConvertBSP(inputFile, outputFile, settings);
                    break;
                case Nexus::SourceFormat::VPK:
                    success = converter.ConvertVPK(inputFile, outputFile, settings);
                    break;
                default:
                    Nexus::Logger::Warning("Source format detected but converter not implemented yet");
                    success = false;
                }
            } else {
                // Generic asset conversion
                Nexus::AssetType assetType = converter.DetectAssetType(inputFile);
                if (assetType == Nexus::AssetType::Unknown) {
                    Nexus::Logger::Error("Unsupported file format: " + inputFile);
                    return 1;
                }
                
                // Convert the asset
                success = converter.ConvertAsset(inputFile, outputFile, assetType, settings);
            }
        }
        
        if (success) {
            std::cout << std::endl;
            std::cout << "✅ Asset converted successfully!" << std::endl;
            std::cout << "📁 Output: " << outputFile << std::endl;
            
            // Show conversion stats
            auto stats = converter.GetLastConversionStats();
            std::cout << "📊 Conversion Stats:" << std::endl;
            std::cout << "   Input size: " << stats.inputSize << " bytes" << std::endl;
            std::cout << "   Output size: " << stats.outputSize << " bytes" << std::endl;
            std::cout << "   Compression: " << (100.0f * (1.0f - float(stats.outputSize) / float(stats.inputSize))) << "%" << std::endl;
            std::cout << "   Time: " << stats.conversionTime << "ms" << std::endl;
            
            return 0;
        } else {
            std::cout << std::endl;
            std::cout << "❌ Failed to convert asset" << std::endl;
            std::cout << "📄 Check the log for details" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        Nexus::Logger::Error("Exception during conversion: " + std::string(e.what()));
        std::cout << "❌ Exception occurred: " << e.what() << std::endl;
        return 1;
    }
}