#include "globals.hpp"
#include "nexus/Nexus.h"
#include <filesystem>
#include <gui.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <settings.hpp>
#include <string>
#include <vector>
#include <windows.h>

void addon_load(AddonAPI *api_p);
void addon_unload();
void addon_render();
void addon_options();

BOOL APIENTRY dll_main(const HMODULE hModule, const DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        self_module = hModule;
        break;
    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    default:
        break;
    }
    return TRUE;
}

// NOLINTNEXTLINE(readability-identifier-naming)
extern "C" __declspec(dllexport) AddonDefinition *GetAddonDef()
{
    addon_def.Signature = -912653496;
    addon_def.APIVersion = NEXUS_API_VERSION;
    addon_def.Name = addon_name;
    addon_def.Version.Major = 0;
    addon_def.Version.Minor = 1;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 4;
    addon_def.Author = "Seres67";
    addon_def.Description = "A Nexus addon manage screenshots in game.";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = EAddonFlags_None;
    addon_def.Provider = EUpdateProvider_GitHub;
    addon_def.UpdateLink = "https://github.com/Seres67/nexus_screenshot_manager";

    return &addon_def;
}

void addon_load(AddonAPI *api_p)
{
    api = api_p;

    ImGui::SetCurrentContext(static_cast<ImGuiContext *>(api->ImguiContext));
    ImGui::SetAllocatorFunctions(static_cast<void *(*)(size_t, void *)>(api->ImguiMalloc),
                                 static_cast<void (*)(void *, void *)>(api->ImguiFree)); // on imgui 1.80+
    api->Renderer.Register(ERenderType_Render, addon_render);
    api->Renderer.Register(ERenderType_OptionsRender, addon_options);

    nexus_link = static_cast<NexusLinkData *>(api->DataLink.Get("DL_NEXUS_LINK"));
    mumble_link = static_cast<Mumble::Data *>(api->DataLink.Get("DL_MUMBLE_LINK"));

    Settings::settings_path = api->Paths.GetAddonDirectory("screenshots\\settings.json");
    // C:\Users\seres\Documents\Guild Wars 2\Screens
    auto userprofile = static_cast<char *>(malloc(10240));
    size_t userprofile_size = 10240;
    _dupenv_s(&userprofile, &userprofile_size, "USERPROFILE");
    Settings::screenshots_path = userprofile + std::string("\\Documents\\Guild Wars 2\\Screens");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
        if (std::filesystem::exists(Settings::screenshots_path)) {
            for (const auto &entry : std::filesystem::directory_iterator(Settings::screenshots_path)) {
                if (entry.is_regular_file() &&
                    std::ranges::find(Settings::screenshots, entry.path().filename().string(),
                                      &Settings::Screenshot::name) == Settings::screenshots.end()) {
                    if (entry.path().filename().string().find("gw") != std::string::npos) {
                        std::string path = entry.path().string();
                        path.erase(path.find_last_of("/\\") + 1);
                        std::string uuid = uuid_generator.getUUID().str();
                        std::filesystem::rename(entry.path(), path + uuid + ".jpg");
                        Settings::screenshots.emplace_back(uuid + ".jpg", path + uuid + ".jpg", Vector2{0, 0});
                    } else {
                        Settings::screenshots.emplace_back(entry.path().filename().string(), entry.path().string(),
                                                           Vector2{0, 0});
                    }
                }
            }
            for (auto &[name, path, position] : Settings::screenshots) {
                const auto pos = name.find('.');
                const auto identifier = std::string("SCREENSHOTS_IMAGE_").append(name.substr(0, pos));
                api->Textures.LoadFromFile(identifier.c_str(), path.c_str(), texture_callback);
            }
        }
    } else {
        Settings::save(Settings::settings_path);
    }
    api->QuickAccess.Add("QA_SCREENSHOTS", "ICON_SCREENSHOTS", "ICON_SCREENSHOTS_HOVER", "KB_SCREENSHOTS_TOGGLE",
                         "Screenshots");
    api->Log(ELogLevel_INFO, addon_name, "addon loaded!");
}

void addon_unload()
{
    api->Log(ELogLevel_INFO, addon_name, "unloading addon...");
    api->QuickAccess.Remove("QA_SCREENSHOTS");
    api->Renderer.Deregister(addon_render);
    api->Renderer.Deregister(addon_options);
    api->Log(ELogLevel_INFO, addon_name, "addon unloaded!");
    api = nullptr;
}

void reload_screenshots()
{
    for (const auto &entry : std::filesystem::directory_iterator(Settings::screenshots_path)) {
        if (entry.is_regular_file() && std::ranges::find(Settings::screenshots, entry.path().filename().string(),
                                                         &Settings::Screenshot::name) == Settings::screenshots.end()) {
            if (entry.path().filename().string().find("gw") != std::string::npos) {
                std::string path = entry.path().string();
                path.erase(path.find_last_of("/\\") + 1);
                std::string uuid = uuid_generator.getUUID().str();
                std::filesystem::rename(entry.path(), path + uuid + ".jpg");
                Settings::screenshots.emplace_back(uuid + ".jpg", path + uuid + ".jpg",
                                                   mumble_link->Context.Compass.PlayerPosition);
                Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
                Settings::save(Settings::settings_path);
            } else {
                if (!std::filesystem::exists(entry.path().string())) {
                    auto pos =
                        std::ranges::find(Settings::screenshots, entry.path().string(), &Settings::Screenshot::path);
                    Settings::screenshots.erase(pos);
                    Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
                    Settings::save(Settings::settings_path);
                }
            }
        }
    }
}

bool tmp_open = true;
void addon_render()
{
    ImGui::SetNextWindowPos(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(Settings::window_alpha);
    if (tmp_open && ImGui::Begin("Screenshots###ScreenshotsMainWindow", &tmp_open, ImGuiWindowFlags_NoCollapse)) {
        reload_screenshots();
        render_file_browser();
        ImGui::SameLine(0, 0 * ImGui::GetStyle().ItemSpacing.x);
        render_screenshot();
        ImGui::End();
    }
}

void addon_options()
{
    if (ImGui::SliderFloat("Window Opacity##ScreenshotsOpacity", &Settings::window_alpha, 0.f, 1.f)) {
        Settings::json_settings[Settings::WINDOW_ALPHA] = Settings::window_alpha;
        Settings::save(Settings::settings_path);
    }
    if (ImGui::SliderFloat2("Image Scale##ScreenshotsScale", &Settings::image_scale.x, 0.f, 2.f)) {
        Settings::json_settings[Settings::IMAGE_SCALE] = Settings::image_scale;
        Settings::save(Settings::settings_path);
    }
}
