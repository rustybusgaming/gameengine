/**
 * @file SourceEngineImporter.h
 * @brief Valve Source Engine asset importer (Half-Life 2, Portal, CS:GO, etc.)
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <DirectXMath.h>

using namespace DirectX;

namespace Nexus {

/**
 * Source Engine specific asset parsers and converters
 * Supports Valve games: Half-Life 2, Portal, Counter-Strike, TF2, L4D, etc.
 */
class SourceEngineImporter {
public:
    // Source Engine file format structures

    struct VMFEntity {
        std::string className;
        int entityId;
        XMFLOAT3 origin;
        XMFLOAT3 angles;
        std::map<std::string, std::string> properties;
        std::vector<std::shared_ptr<VMFEntity>> children;
    };

    struct VMFBrush {
        int brushId;
        std::vector<XMFLOAT3> vertices;
        std::vector<std::string> textures;
        std::map<std::string, std::string> properties;
    };

    struct VMFSolid {
        int solidId;
        std::vector<VMFBrush> brushes;
    };

    struct VMTMaterial {
        std::string shaderName;
        std::string baseTexture;
        std::string normalMap;
        std::string bumpMap;
        std::string specularMap;
        std::string envMap;
        float opacity;
        float reflectivity;
        bool translucent;
        bool additive;
        std::map<std::string, std::string> parameters;
    };

    struct VTFTexture {
        uint32_t width;
        uint32_t height;
        uint32_t mipLevels;
        uint32_t frames;
        uint32_t format;
        std::vector<uint8_t> imageData;
    };

    struct MDLModel {
        std::string name;
        uint32_t version;
        std::vector<std::string> meshes;
        std::vector<std::string> materials;
        std::vector<std::string> textures;
        std::vector<std::string> bones;
        XMFLOAT3 eyePosition;
        XMFLOAT3 hullMin;
        XMFLOAT3 hullMax;
    };

    struct BSPMap {
        uint32_t version;
        std::vector<VMFEntity> entities;
        std::vector<XMFLOAT3> vertices;
        std::vector<std::string> textures;
        std::map<std::string, std::string> worldspawn;
    };

public:
    SourceEngineImporter();
    ~SourceEngineImporter();

    // Project detection
    bool ValidateSourceGame(const std::string& gamePath);
    std::string DetectSourceGameType(const std::string& gamePath);

    // VMF (Valve Map Format) - Hammer Editor files
    bool ParseVMFFile(const std::string& vmfFile, std::vector<VMFEntity>& entities, std::vector<VMFSolid>& solids);
    bool ConvertVMFToNexusScene(const std::string& vmfFile, const std::string& outputPath);

    // VMT (Valve Material Type)
    bool ParseVMTFile(const std::string& vmtFile, VMTMaterial& material);
    bool ConvertVMTToNexusMaterial(const std::string& vmtFile, const std::string& outputPath);
    std::string MapSourceShaderToNexus(const std::string& sourceShader);

    // VTF (Valve Texture Format)
    bool ParseVTFFile(const std::string& vtfFile, VTFTexture& texture);
    bool ConvertVTFToNexusTexture(const std::string& vtfFile, const std::string& outputPath);

    // MDL (Source Model Format)
    bool ParseMDLFile(const std::string& mdlFile, MDLModel& model);
    bool ConvertMDLToNexusMesh(const std::string& mdlFile, const std::string& outputPath);

    // BSP (Binary Space Partition - compiled maps)
    bool ParseBSPFile(const std::string& bspFile, BSPMap& map);
    bool ConvertBSPToNexusScene(const std::string& bspFile, const std::string& outputPath);
    bool ExtractBSPEntities(const std::string& bspFile, std::vector<VMFEntity>& entities);

    // VPK (Valve Package - archive format)
    bool ExtractVPKArchive(const std::string& vpkFile, const std::string& outputDir);
    std::vector<std::string> ListVPKContents(const std::string& vpkFile);

    // Game-specific conversions
    bool ImportHalfLife2Content(const std::string& hl2Path, const std::string& outputPath);
    bool ImportPortalContent(const std::string& portalPath, const std::string& outputPath);
    bool ImportCSGOContent(const std::string& csgoPath, const std::string& outputPath);
    bool ImportTF2Content(const std::string& tf2Path, const std::string& outputPath);

    // Utility functions
    std::string FindSourceGamePath(const std::string& steamPath, const std::string& gameName);
    std::vector<std::string> ScanSourceAssets(const std::string& gamePath);
    std::string ResolveSourceMaterialPath(const std::string& materialName, const std::string& gamePath);
    std::string ResolveSourceModelPath(const std::string& modelName, const std::string& gamePath);

private:
    // VMF parsing helpers
    std::map<std::string, std::string> ParseVMFBlock(const std::string& blockContent);
    void ParseVMFEntity(const std::string& entityBlock, VMFEntity& entity);
    void ParseVMFSolid(const std::string& solidBlock, VMFSolid& solid);

    // VMT parsing helpers
    std::map<std::string, std::string> ParseVMTParameters(const std::string& vmtContent);

    // VTF parsing helpers
    bool ReadVTFHeader(std::ifstream& file, VTFTexture& texture);
    bool DecompressVTFData(const std::vector<uint8_t>& compressed, std::vector<uint8_t>& decompressed, uint32_t format);

    // MDL parsing helpers
    bool ReadMDLHeader(std::ifstream& file, MDLModel& model);
    bool ParseMDLStudioHeader(const std::vector<uint8_t>& data, MDLModel& model);

    // BSP parsing helpers
    bool ReadBSPHeader(std::ifstream& file, BSPMap& map);
    bool ParseBSPLump(std::ifstream& file, int lumpIndex, BSPMap& map);

    // Coordinate system conversion (Source uses Z-up, Nexus uses Y-up)
    XMFLOAT3 ConvertSourceToNexusCoordinates(const XMFLOAT3& sourceCoords);
    XMFLOAT3 ConvertSourceToNexusAngles(const XMFLOAT3& sourceAngles);

    // Path resolution
    std::string NormalizeSourcePath(const std::string& path);
    std::string GetGameDirectory(const std::string& gamePath);

private:
    std::string gameBasePath_;
    std::string materialsPath_;
    std::string modelsPath_;
    std::string mapsPath_;
    std::map<std::string, std::string> textureCache_;
    std::map<std::string, std::string> materialCache_;
};

} // namespace Nexus
