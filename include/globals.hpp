#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <imgui/imgui.h>
#include <map>
#include <mumble/Mumble.h>
#include <nexus/Nexus.h>
#include <nlohmann/json.hpp>
#include <string>
#include <uuid_v4/uuid_v4.h>

// handle to self hmodule
extern HMODULE self_module;
// addon definition
extern AddonDefinition addon_def;
// addon api
extern AddonAPI *api;

extern NexusLinkData *nexus_link;
extern Mumble::Data *mumble_link;

extern char addon_name[];

extern HWND game_handle;

extern std::map<std::string, Texture *> textures;

extern UUIDv4::UUIDGenerator<std::mt19937_64> uuid_generator;

void from_json(const nlohmann::json &j, ImVec2 &v);
void to_json(nlohmann::json &j, const ImVec2 &v);

#endif // GLOBALS_HPP
