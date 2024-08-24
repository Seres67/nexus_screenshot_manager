#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#include <map>
#include <mumble/Mumble.h>
#include <string>
#include <nexus/Nexus.h>

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
