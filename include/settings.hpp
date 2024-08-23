#ifndef SETTINGS_HPP
#define SETTINGS_HPP

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

extern const char *IS_ADDON_ENABLED;
extern const char *IMAGES_PATH;
extern const char *WINDOW_ALPHA;
} // namespace Settings

#endif // SETTINGS_HPP
