#include "globals.hpp"

HMODULE self_module = nullptr;
AddonDefinition addon_def{};
AddonAPI *api = nullptr;
char addon_name[] = "Screenshot Manager";
HWND game_handle = nullptr;
std::map<std::string, Texture *> textures;
