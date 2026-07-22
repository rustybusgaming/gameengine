/**
 * @file UnrealImporter.cpp
 * @brief Complete Unreal Engine project importer implementation
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#include "GameImporter.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>

namespace fs = std::filesystem;

namespace Nexus {

// Unreal .uasset/.umap file parser (simplified - actual format is binary)
bool UnrealImporter::ParseLevelFile(const std::string& levelFile, std::vector<UnrealActor>& actors) {
    Logger::Info("Parsing Unreal level: " + levelFile);

    // Note: Actual .umap files are binary (Unreal's proprietary format)
    // This is a simplified text-based parser for demonstration
    // Real implementation would use Unreal's FBX export or custom binary parser

    std::ifstream file(levelFile);
    if (!file.is_open()) {
        Logger::Error("Failed to open Unreal level file: " + levelFile);
        return false;
    }

    try {
        std::string line;
        std::shared_ptr<UnrealActor> currentActor = nullptr;

        while (std::getline(file, line)) {
            // Parse actor declaration
            if (line.find("Begin Actor") != std::string::npos) {
                currentActor = std::make_shared<UnrealActor>();

                // Extract class name
                std::regex classRegex("Class=([^\\s]+)");
                std::smatch match;
                if (std::regex_search(line, match, classRegex) && match.size() > 1) {
                    currentActor->className = match[1].str();
                }

                // Extract name
                std::regex nameRegex("Name=([^\\s]+)");
                if (std::regex_search(line, match, nameRegex) && match.size() > 1) {
                    currentActor->name = match[1].str();
                }
            }

            // Parse actor properties
            if (currentActor) {
                // Location
                if (line.find("Location=") != std::string::npos) {
                    std::regex locRegex("Location=\\(X=([\\d.-]+),Y=([\\d.-]+),Z=([\\d.-]+)\\)");
                    std::smatch match;
                    if (std::regex_search(line, match, locRegex) && match.size() > 3) {
                        currentActor->location.x = std::stof(match[1].str());
                        currentActor->location.y = std::stof(match[2].str());
                        currentActor->location.z = std::stof(match[3].str());
                    }
                }

                // Rotation
                if (line.find("Rotation=") != std::string::npos) {
                    std::regex rotRegex("Rotation=\\(Pitch=([\\d.-]+),Yaw=([\\d.-]+),Roll=([\\d.-]+)\\)");
                    std::smatch match;
                    if (std::regex_search(line, match, rotRegex) && match.size() > 3) {
                        currentActor->rotation.x = std::stof(match[1].str()); // Pitch
                        currentActor->rotation.y = std::stof(match[2].str()); // Yaw
                        currentActor->rotation.z = std::stof(match[3].str()); // Roll
                    }
                }

                // Scale
                if (line.find("Scale3D=") != std::string::npos) {
                    std::regex scaleRegex("Scale3D=\\(X=([\\d.-]+),Y=([\\d.-]+),Z=([\\d.-]+)\\)");
                    std::smatch match;
                    if (std::regex_search(line, match, scaleRegex) && match.size() > 3) {
                        currentActor->scale.x = std::stof(match[1].str());
                        currentActor->scale.y = std::stof(match[2].str());
                        currentActor->scale.z = std::stof(match[3].str());
                    }
                }

                // Generic properties
                std::regex propRegex("(\\w+)=([^\\n]+)");
                std::smatch match;
                if (std::regex_search(line, match, propRegex) && match.size() > 2) {
                    currentActor->properties[match[1].str()] = match[2].str();
                }
            }

            // End actor
            if (line.find("End Actor") != std::string::npos && currentActor) {
                actors.push_back(*currentActor);
                currentActor = nullptr;
            }
        }

        Logger::Info("Parsed " + std::to_string(actors.size()) + " actors");
        return true;

    } catch (const std::exception& e) {
        Logger::Error("Exception parsing Unreal level: " + std::string(e.what()));
        return false;
    }
}

// Blueprint parser (simplified)
bool UnrealImporter::ParseBlueprintFile(const std::string& blueprintFile, UnrealBlueprint& blueprint) {
    Logger::Info("Parsing Unreal Blueprint: " + blueprintFile);

    std::ifstream file(blueprintFile);
    if (!file.is_open()) {
        Logger::Error("Failed to open Blueprint file: " + blueprintFile);
        return false;
    }

    try {
        std::string line;
        bool inVariableSection = false;
        bool inFunctionSection = false;

        while (std::getline(file, line)) {
            // Parse blueprint name
            if (line.find("BlueprintName=") != std::string::npos) {
                std::regex nameRegex("BlueprintName=([^\\s]+)");
                std::smatch match;
                if (std::regex_search(line, match, nameRegex) && match.size() > 1) {
                    blueprint.name = match[1].str();
                }
            }

            // Parse parent class
            if (line.find("ParentClass=") != std::string::npos) {
                std::regex parentRegex("ParentClass=([^\\s]+)");
                std::smatch match;
                if (std::regex_search(line, match, parentRegex) && match.size() > 1) {
                    blueprint.parentClass = match[1].str();
                }
            }

            // Parse variables
            if (line.find("Begin Variables") != std::string::npos) {
                inVariableSection = true;
            } else if (line.find("End Variables") != std::string::npos) {
                inVariableSection = false;
            } else if (inVariableSection) {
                std::regex varRegex("(\\w+)\\s+(\\w+)");
                std::smatch match;
                if (std::regex_search(line, match, varRegex) && match.size() > 2) {
                    blueprint.variables[match[2].str()] = match[1].str();
                }
            }

            // Parse functions
            if (line.find("Begin Functions") != std::string::npos) {
                inFunctionSection = true;
            } else if (line.find("End Functions") != std::string::npos) {
                inFunctionSection = false;
            } else if (inFunctionSection) {
                std::regex funcRegex("Function\\s+(\\w+)");
                std::smatch match;
                if (std::regex_search(line, match, funcRegex) && match.size() > 1) {
                    blueprint.functions.push_back(match[1].str());
                }
            }

            // Parse components
            if (line.find("Component=") != std::string::npos) {
                std::regex compRegex("Component=([^\\s]+)");
                std::smatch match;
                if (std::regex_search(line, match, compRegex) && match.size() > 1) {
                    blueprint.components.push_back(match[1].str());
                }
            }
        }

        return true;

    } catch (const std::exception& e) {
        Logger::Error("Failed to parse Blueprint: " + std::string(e.what()));
        return false;
    }
}

// Material parser
bool UnrealImporter::ParseMaterialFile(const std::string& materialFile, std::map<std::string, std::string>& materialData) {
    Logger::Info("Parsing Unreal Material: " + materialFile);

    std::ifstream file(materialFile);
    if (!file.is_open()) {
        return false;
    }

    try {
        std::string line;
        while (std::getline(file, line)) {
            // Parse material properties
            std::regex propRegex("(\\w+)=([^\\n]+)");
            std::smatch match;
            if (std::regex_search(line, match, propRegex) && match.size() > 2) {
                materialData[match[1].str()] = match[2].str();
            }
        }

        return !materialData.empty();

    } catch (const std::exception& e) {
        Logger::Error("Failed to parse material: " + std::string(e.what()));
        return false;
    }
}

// Blueprint to Lua conversion
std::string UnrealImporter::ConvertBlueprintToLua(const UnrealBlueprint& blueprint) {
    std::stringstream lua;

    lua << "-- Converted from Blueprint: " << blueprint.name << "\n";
    lua << "local " << blueprint.name << " = {}\n\n";

    // Variables
    lua << "-- Variables\n";
    for (const auto& [name, type] : blueprint.variables) {
        lua << blueprint.name << "." << name << " = nil  -- " << type << "\n";
    }
    lua << "\n";

    // Constructor
    lua << "function " << blueprint.name << ".new()\n";
    lua << "    local self = setmetatable({}, { __index = " << blueprint.name << " })\n";
    lua << "    return self\n";
    lua << "end\n\n";

    // Functions
    for (const auto& func : blueprint.functions) {
        lua << "function " << blueprint.name << ":" << func << "()\n";
        lua << "    -- TODO: Implement " << func << "\n";
        lua << "end\n\n";
    }

    // Event functions
    lua << "function " << blueprint.name << ":BeginPlay()\n";
    lua << "    -- Called when the game starts\n";
    lua << "end\n\n";

    lua << "function " << blueprint.name << ":Tick(deltaTime)\n";
    lua << "    -- Called every frame\n";
    lua << "end\n\n";

    lua << "return " << blueprint.name << "\n";

    return lua.str();
}

// Blueprint to C++ conversion
std::string UnrealImporter::ConvertBlueprintToCpp(const UnrealBlueprint& blueprint) {
    std::stringstream cpp;

    // Header
    cpp << "#pragma once\n\n";
    cpp << "#include \"GameFramework/Actor.h\"\n";
    cpp << "#include \"" << blueprint.name << ".generated.h\"\n\n";

    cpp << "UCLASS()\n";
    cpp << "class A" << blueprint.name << " : public ";
    cpp << (blueprint.parentClass.empty() ? "AActor" : blueprint.parentClass) << "\n";
    cpp << "{\n";
    cpp << "    GENERATED_BODY()\n\n";

    cpp << "public:\n";
    cpp << "    A" << blueprint.name << "();\n\n";

    // Variables
    cpp << "    // Variables\n";
    for (const auto& [name, type] : blueprint.variables) {
        cpp << "    UPROPERTY(EditAnywhere, BlueprintReadWrite)\n";
        cpp << "    " << type << " " << name << ";\n\n";
    }

    // Functions
    for (const auto& func : blueprint.functions) {
        cpp << "    UFUNCTION(BlueprintCallable)\n";
        cpp << "    void " << func << "();\n\n";
    }

    // Event functions
    cpp << "protected:\n";
    cpp << "    virtual void BeginPlay() override;\n";
    cpp << "    virtual void Tick(float DeltaTime) override;\n";

    cpp << "};\n";

    return cpp.str();
}

} // namespace Nexus
