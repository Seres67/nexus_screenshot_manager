#ifndef SETTINGS_HPP
#define SETTINGS_HPP

#include "imgui/imgui.h"
#include <mumble/Mumble.h>
#include <mutex>
#include <nlohmann/json.hpp>

namespace Settings
{

void load(const std::filesystem::path &path);
void save(const std::filesystem::path &path);

extern nlohmann::json json_settings;
extern std::filesystem::path settings_path;
extern std::mutex mutex;

extern std::filesystem::path screenshots_path;
extern float window_alpha;
extern ImVec2 image_scale;
extern bool bypass_screenshots_limit;
typedef struct
{
    std::string name;
    std::string path;
    Vector2 position;
} Screenshot;
extern std::vector<Screenshot> screenshots;
void from_json(const nlohmann::json &j, Screenshot &s);
void to_json(nlohmann::json &j, const Screenshot &s);

extern const char *WINDOW_ALPHA;
extern const char *SCREENSHOTS;
extern const char *IMAGE_SCALE;
extern const char *BYPASS_SCREENSHOTS_LIMIT;
} // namespace Settings

#endif // SETTINGS_HPP
