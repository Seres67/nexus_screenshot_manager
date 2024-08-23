#ifndef SETTINGS_HPP
#define SETTINGS_HPP

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

extern bool is_addon_enabled;
extern std::filesystem::path screenshots_path;
extern float window_alpha;
typedef struct
{
    std::string name;
    std::string path;
    Vector2 position;
} Screenshot;
extern std::vector<Screenshot> screenshots;
void from_json(const nlohmann::json &j, Screenshot &s);
void to_json(nlohmann::json &j, const Screenshot &s);

extern const char *IS_ADDON_ENABLED;
extern const char *WINDOW_ALPHA;
extern const char *SCREENSHOTS;
} // namespace Settings

#endif // SETTINGS_HPP
