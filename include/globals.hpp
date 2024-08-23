#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <nexus/Nexus.h>
#include <string>
#include <vector>
#include <map>
#include <imgui/imgui.h>
#include <mumble/Mumble.h>

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

#endif // GLOBALS_HPP
