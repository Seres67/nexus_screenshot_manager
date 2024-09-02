#include <filesystem>
#include <fstream>
#include <globals.hpp>
#include <imgui/imgui.h>
#include <nlohmann/json.hpp>
#include <settings.hpp>

using json = nlohmann::json;
namespace Settings
{
const char *WINDOW_ALPHA = "WindowAlpha";
const char *IMAGE_SCALE = "ImageScale";
const char *SCREENSHOTS = "Screenshots";

json json_settings;
std::mutex mutex;
std::filesystem::path settings_path;

std::filesystem::path screenshots_path;
float window_alpha = 1.f;
ImVec2 image_scale = {.33f, .33f};
std::vector<Screenshot> screenshots;

void from_json(const nlohmann::json &j, Screenshot &s)
{
    s.path = j.at("path").get<std::string>();
    s.name = j.at("name").get<std::string>();
    s.position.X = j.at("x").get<float>();
    s.position.Y = j.at("y").get<float>();
}

void to_json(nlohmann::json &j, const Screenshot &s)
{
    j = json{
        {"path", s.path},
        {"name", s.name},
        {"x", s.position.X},
        {"y", s.position.Y},
    };
}

void load(const std::filesystem::path &path)
{
    json_settings = json::object();
    if (!std::filesystem::exists(path)) {
        return;
    }

    {
        std::lock_guard lock(mutex);
        try {
            if (std::ifstream file(path); file.is_open()) {
                json_settings = json::parse(file);
                file.close();
            }
        } catch (json::parse_error &ex) {
            api->Log(ELogLevel_WARNING, addon_name, "settings.json could not be parsed.");
            api->Log(ELogLevel_WARNING, addon_name, ex.what());
        }
    }
    if (!json_settings[WINDOW_ALPHA].is_null()) {
        json_settings[WINDOW_ALPHA].get_to(window_alpha);
    }
    if (!json_settings[SCREENSHOTS].is_null()) {
        json_settings[SCREENSHOTS].get_to(screenshots);
    }
    if (!json_settings[IMAGE_SCALE].is_null()) {
        json_settings[IMAGE_SCALE].get_to(image_scale);
    }
    api->Log(ELogLevel_INFO, addon_name, "settings loaded!");
}

void save(const std::filesystem::path &path)
{
    if (json_settings.is_null()) {
        api->Log(ELogLevel_WARNING, addon_name, "settings.json is null, cannot save.");
        return;
    }
    if (!std::filesystem::exists(path.parent_path())) {
        std::filesystem::create_directories(path.parent_path());
    }
    {
        std::lock_guard lock(mutex);
        if (std::ofstream file(path); file.is_open()) {
            file << json_settings.dump(1, '\t') << std::endl;
            file.close();
        }
        api->Log(ELogLevel_INFO, addon_name, "settings saved!");
    }
}
} // namespace Settings