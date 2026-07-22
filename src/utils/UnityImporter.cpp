/**
 * @file UnityImporter.cpp
 * @brief Complete Unity project importer implementation
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#include "GameImporter.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>
#include <yaml-cpp/yaml.h> // For Unity YAML parsing

namespace fs = std::filesystem;

namespace Nexus {

// Unity scene file parser
bool UnityImporter::ParseSceneFile(const std::string& sceneFile, std::vector<UnityGameObject>& gameObjects) {
    Logger::Info("Parsing Unity scene: " + sceneFile);

    std::ifstream file(sceneFile);
    if (!file.is_open()) {
        Logger::Error("Failed to open Unity scene file: " + sceneFile);
        return false;
    }

    try {
        // Unity scenes are YAML-based
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();

        // Split by YAML documents (--- separator)
        std::regex docSeparator("^---\\s*!u!\\d+", std::regex::ECMAScript | std::regex::multiline);
        std::sregex_token_iterator iter(content.begin(), content.end(), docSeparator, -1);
        std::sregex_token_iterator end;

        for (; iter != end; ++iter) {
            std::string document = *iter;
            if (document.empty()) continue;

            // Check if this is a GameObject
            if (document.find("GameObject:") != std::string::npos) {
                auto gameObject = ParseUnityGameObject(document);
                if (gameObject) {
                    gameObjects.push_back(*gameObject);
                }
            }
        }

        Logger::Info("Parsed " + std::to_string(gameObjects.size()) + " GameObjects");
        return true;

    } catch (const std::exception& e) {
        Logger::Error("Exception parsing Unity scene: " + std::string(e.what()));
        return false;
    }
}

std::shared_ptr<UnityImporter::UnityGameObject> UnityImporter::ParseUnityGameObject(const std::string& yamlDoc) {
    auto obj = std::make_shared<UnityGameObject>();

    try {
        // Extract GameObject name
        std::regex nameRegex("m_Name:\\s*([^\\n]+)");
        std::smatch match;
        if (std::regex_search(yamlDoc, match, nameRegex) && match.size() > 1) {
            obj->name = match[1].str();
        }

        // Extract Transform component
        if (yamlDoc.find("Transform:") != std::string::npos ||
            yamlDoc.find("RectTransform:") != std::string::npos) {

            // Position
            std::regex posRegex("m_LocalPosition:\\s*\\{x:\\s*([\\d.-]+),\\s*y:\\s*([\\d.-]+),\\s*z:\\s*([\\d.-]+)\\}");
            if (std::regex_search(yamlDoc, match, posRegex) && match.size() > 3) {
                obj->position.x = std::stof(match[1].str());
                obj->position.y = std::stof(match[2].str());
                obj->position.z = std::stof(match[3].str());
            }

            // Rotation (Quaternion)
            std::regex rotRegex("m_LocalRotation:\\s*\\{x:\\s*([\\d.-]+),\\s*y:\\s*([\\d.-]+),\\s*z:\\s*([\\d.-]+),\\s*w:\\s*([\\d.-]+)\\}");
            if (std::regex_search(yamlDoc, match, rotRegex) && match.size() > 4) {
                // Convert quaternion to Euler angles (simplified)
                float qx = std::stof(match[1].str());
                float qy = std::stof(match[2].str());
                float qz = std::stof(match[3].str());
                float qw = std::stof(match[4].str());

                // Simplified quaternion to Euler conversion
                obj->rotation.x = std::atan2(2.0f * (qw * qx + qy * qz), 1.0f - 2.0f * (qx * qx + qy * qy));
                obj->rotation.y = std::asin(2.0f * (qw * qy - qz * qx));
                obj->rotation.z = std::atan2(2.0f * (qw * qz + qx * qy), 1.0f - 2.0f * (qy * qy + qz * qz));
            }

            // Scale
            std::regex scaleRegex("m_LocalScale:\\s*\\{x:\\s*([\\d.-]+),\\s*y:\\s*([\\d.-]+),\\s*z:\\s*([\\d.-]+)\\}");
            if (std::regex_search(yamlDoc, match, scaleRegex) && match.size() > 3) {
                obj->scale.x = std::stof(match[1].str());
                obj->scale.y = std::stof(match[2].str());
                obj->scale.z = std::stof(match[3].str());
            }
        }

        // Extract component list
        std::regex componentRegex("component:\\s*\\{fileID:\\s*(\\d+)");
        std::sregex_iterator compIter(yamlDoc.begin(), yamlDoc.end(), componentRegex);
        std::sregex_iterator compEnd;

        for (; compIter != compEnd; ++compIter) {
            obj->components.push_back((*compIter)[1].str());
        }

        return obj;

    } catch (const std::exception& e) {
        Logger::Warning("Failed to parse GameObject: " + std::string(e.what()));
        return nullptr;
    }
}

// Unity prefab parser
bool UnityImporter::ParsePrefabFile(const std::string& prefabFile, UnityGameObject& prefab) {
    Logger::Info("Parsing Unity prefab: " + prefabFile);

    std::ifstream file(prefabFile);
    if (!file.is_open()) {
        Logger::Error("Failed to open Unity prefab file: " + prefabFile);
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    auto obj = ParseUnityGameObject(content);
    if (obj) {
        prefab = *obj;
        return true;
    }

    return false;
}

// Unity meta file parser
bool UnityImporter::ParseMetaFile(const std::string& metaFile, UnityAsset& asset) {
    std::ifstream file(metaFile);
    if (!file.is_open()) {
        return false;
    }

    try {
        std::string line;
        while (std::getline(file, line)) {
            // Extract GUID
            if (line.find("guid:") != std::string::npos) {
                std::regex guidRegex("guid:\\s*([a-f0-9]+)");
                std::smatch match;
                if (std::regex_search(line, match, guidRegex) && match.size() > 1) {
                    asset.guid = match[1].str();
                }
            }

            // Extract fileID
            if (line.find("fileID:") != std::string::npos) {
                std::regex idRegex("fileID:\\s*(\\d+)");
                std::smatch match;
                if (std::regex_search(line, match, idRegex) && match.size() > 1) {
                    asset.fileID = match[1].str();
                }
            }
        }

        return !asset.guid.empty();

    } catch (const std::exception& e) {
        Logger::Error("Failed to parse Unity meta file: " + std::string(e.what()));
        return false;
    }
}

// C# to Lua conversion (basic)
std::string UnityImporter::ConvertCSharpToLua(const std::string& csharpCode) {
    std::string luaCode = csharpCode;

    // Basic conversions
    luaCode = std::regex_replace(luaCode, std::regex("public class (\\w+)"), "local $1 = {}");
    luaCode = std::regex_replace(luaCode, std::regex("private (\\w+) (\\w+);"), "-- $1 $2");
    luaCode = std::regex_replace(luaCode, std::regex("void (\\w+)\\("), "function $1.new(self)");
    luaCode = std::regex_replace(luaCode, std::regex("Update\\(\\)"), "update(self, dt)");
    luaCode = std::regex_replace(luaCode, std::regex("Start\\(\\)"), "start(self)");
    luaCode = std::regex_replace(luaCode, std::regex("//"), "--");

    return luaCode;
}

// C# to C++ conversion (advanced)
std::string UnityImporter::ConvertCSharpToCpp(const std::string& csharpCode) {
    std::stringstream cpp;

    cpp << "#include \"Component.h\"\n";
    cpp << "#include \"Transform.h\"\n";
    cpp << "#include \"Input.h\"\n\n";

    std::string converted = csharpCode;

    // Class conversion
    converted = std::regex_replace(converted, std::regex("public class (\\w+) : MonoBehaviour"),
                                   "class $1 : public Component");

    // Property conversions
    converted = std::regex_replace(converted, std::regex("public (\\w+) (\\w+);"),
                                   "$1 $2;");
    converted = std::regex_replace(converted, std::regex("private (\\w+) (\\w+);"),
                                   "$1 $2_;");

    // Method conversions
    converted = std::regex_replace(converted, std::regex("void Update\\(\\)"),
                                   "void Update(float deltaTime) override");
    converted = std::regex_replace(converted, std::regex("void Start\\(\\)"),
                                   "void Start() override");
    converted = std::regex_replace(converted, std::regex("void Awake\\(\\)"),
                                   "void Awake() override");

    // Unity API conversions
    converted = std::regex_replace(converted, std::regex("Input\\.GetKey\\("),
                                   "Input::GetKey(");
    converted = std::regex_replace(converted, std::regex("Input\\.GetAxis\\("),
                                   "Input::GetAxis(");
    converted = std::regex_replace(converted, std::regex("Time\\.deltaTime"),
                                   "deltaTime");
    converted = std::regex_replace(converted, std::regex("transform\\.position"),
                                   "transform->position");
    converted = std::regex_replace(converted, std::regex("transform\\.rotation"),
                                   "transform->rotation");

    // Type conversions
    converted = std::regex_replace(converted, std::regex("\\bfloat\\b"), "float");
    converted = std::regex_replace(converted, std::regex("\\bint\\b"), "int");
    converted = std::regex_replace(converted, std::regex("\\bbool\\b"), "bool");
    converted = std::regex_replace(converted, std::regex("\\bstring\\b"), "std::string");
    converted = std::regex_replace(converted, std::regex("Vector3"), "Vector3");

    cpp << converted;

    return cpp.str();
}

} // namespace Nexus
