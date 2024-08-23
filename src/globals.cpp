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
