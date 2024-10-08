#include "globals.hpp"
#include "mumble/Mumble.h"
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
void keybind_handler(const char *identifier, bool is_release);

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
    addon_def.Version.Minor = 2;
    addon_def.Version.Build = 0;
    addon_def.Version.Revision = 3;
    addon_def.Author = "Seres67";
    addon_def.Description = "A Nexus addon manage screenshots in game.";
    addon_def.Load = addon_load;
    addon_def.Unload = addon_unload;
    addon_def.Flags = EAddonFlags_None;
    addon_def.Provider = EUpdateProvider_GitHub;
    addon_def.UpdateLink = "https://github.com/Seres67/nexus_screenshot_manager";

    return &addon_def;
}

void add_screenshot(const std::filesystem::directory_entry &entry, const Vector2 &location = {0, 0})
{
    if (Settings::bypass_screenshots_limit) {
        std::string path = entry.path().string();
        path.erase(path.find_last_of("/\\") + 1);
        const std::chrono::time_point now =
            std::chrono::zoned_time{std::chrono::current_zone(), std::chrono::system_clock::now()}.get_local_time();
        auto dp = std::chrono::floor<std::chrono::days>(now);
        const std::chrono::year_month_day ymd{dp};
        const std::chrono::hh_mm_ss<std::chrono::milliseconds> hms{
            std::chrono::floor<std::chrono::milliseconds>(now - dp)};
        std::string date = std::to_string(static_cast<int>(ymd.year())) + "-" +
                           std::to_string(static_cast<unsigned int>(ymd.month())) + "-" +
                           std::to_string(static_cast<unsigned int>(ymd.day())) + "_" +
                           std::to_string(hms.hours().count()) + "-" + std::to_string(hms.minutes().count()) + "-" +
                           std::to_string(hms.seconds().count()) + "-" + std::to_string(hms.subseconds().count());
        std::string uuid = uuid_generator.getUUID().str();
        std::filesystem::rename(entry.path(), path + date + "-" + uuid + ".jpg");
        Settings::screenshots.emplace_back(date + "-" + uuid + ".jpg", path + date + "-" + uuid + ".jpg", location);
    } else {
        Settings::screenshots.emplace_back(entry.path().filename().string(), entry.path().string(), location);
    }
}

std::mutex api_mutex;
std::thread textures_loader_thread;
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
    auto userprofile = static_cast<char *>(malloc(10240));
    size_t userprofile_size = 10240;
    _dupenv_s(&userprofile, &userprofile_size, "USERPROFILE");
    Settings::screenshots_path = userprofile + std::string("\\Documents\\Guild Wars 2\\Screens");
    if (std::filesystem::exists(Settings::settings_path)) {
        Settings::load(Settings::settings_path);
        if (std::filesystem::exists(Settings::screenshots_path))
            Settings::screenshots.erase(std::remove_if(Settings::screenshots.begin(), Settings::screenshots.end(),
                                                       [](const Settings::Screenshot &sc) -> bool
                                                       { return !std::filesystem::exists(sc.path); }),
                                        Settings::screenshots.end());
        for (const auto &entry : std::filesystem::directory_iterator(Settings::screenshots_path)) {
            if (entry.is_regular_file()) {
                if (std::ranges::find(Settings::screenshots, entry.path().filename().string(),
                                      &Settings::Screenshot::name) == Settings::screenshots.end()) {
                    if (entry.path().filename().string().find("gw") != std::string::npos)
                        add_screenshot(entry);
                    else
                        Settings::screenshots.emplace_back(entry.path().filename().string(), entry.path().string(),
                                                           Vector2{0, 0});
                }
            }
        }
        textures_loader_thread = std::thread(
            []()
            {
                for (auto &[name, path, position] : Settings::screenshots) {
                    const auto pos = name.find('.');
                    const auto identifier = std::string("SCREENSHOTS_IMAGE_").append(name.substr(0, pos));
                    {
                        std::lock_guard lock(api_mutex);
                        if (!api)
                            return;
                        api->Textures.LoadFromFile(identifier.c_str(), path.c_str(), texture_callback);
                    }
                }
            });
        textures_loader_thread.detach();
    } else {
        Settings::save(Settings::settings_path);
    }
    api->QuickAccess.Add("QA_SCREENSHOTS", "ICON_SCREENSHOTS", "ICON_SCREENSHOTS_HOVER", "KB_SCREENSHOTS_TOGGLE",
                         "Screenshots");
    api->InputBinds.RegisterWithString("KB_SCREENSHOTS_TOGGLE", keybind_handler, "((null))");

    api->Log(ELogLevel_INFO, addon_name, "addon loaded!");
}

void addon_unload()
{
    api->Log(ELogLevel_INFO, addon_name, "unloading addon...");
    api->QuickAccess.Remove("QA_SCREENSHOTS");
    api->Renderer.Deregister(addon_render);
    api->Renderer.Deregister(addon_options);
    api->Log(ELogLevel_INFO, addon_name, "addon unloaded!");
    {
        std::lock_guard lock(api_mutex);
        api = nullptr;
    }
    if (textures_loader_thread.joinable())
        textures_loader_thread.join();
}

void reload_screenshots()
{
    for (const auto &entry : std::filesystem::directory_iterator(Settings::screenshots_path)) {
        std::string name = entry.path().filename().string();
        if (entry.is_regular_file() && std::ranges::find(Settings::screenshots, name, &Settings::Screenshot::name) ==
                                           Settings::screenshots.end()) {
            if (name.find("gw") != std::string::npos) {
                add_screenshot(entry, mumble_link->Context.Compass.PlayerPosition);
                Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
                Settings::save(Settings::settings_path);
            } else {
                // TODO: wtf is this? needs to change.
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

bool window_open = false;
void addon_render()
{
    reload_screenshots();
    ImGui::SetNextWindowPos(ImVec2(400, 600), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(Settings::window_alpha);
    if (window_open && ImGui::Begin("Screenshots###ScreenshotsMainWindow", &window_open, ImGuiWindowFlags_NoCollapse)) {
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
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "WARNING: when ticked, screenshots will be automatically renamed to \"date-time-uuid.jpg\".");
    if (ImGui::Checkbox("Bypass Screenshots Limit##ScreenshotsLimit", &Settings::bypass_screenshots_limit)) {
        Settings::json_settings[Settings::BYPASS_SCREENSHOTS_LIMIT] = Settings::bypass_screenshots_limit;
        Settings::save(Settings::settings_path);
    }
}

void keybind_handler(const char *identifier, bool is_release)
{
    if (!strcmp(identifier, "KB_SCREENSHOTS_TOGGLE")) {
        window_open = !window_open;
    }
}
