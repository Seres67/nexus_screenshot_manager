#include "globals.hpp"

#include <nlohmann/json.hpp>

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
NexusLinkData *nexus_link = nullptr;
Mumble::Data *mumble_link = nullptr;
char addon_name[] = "Screenshot Manager";
HWND game_handle = nullptr;
std::map<std::string, Texture *> textures;
UUIDv4::UUIDGenerator<std::mt19937_64> uuid_generator;

void from_json(const nlohmann::json &j, ImVec2 &v)
{
    v.x = j.at("x").get<float>();
    v.y = j.at("y").get<float>();
}

void to_json(nlohmann::json &j, const ImVec2 &v)
{
    j = nlohmann::json{
        {"x", v.x},
        {"y", v.y},
    };
}