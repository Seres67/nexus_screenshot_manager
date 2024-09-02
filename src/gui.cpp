#include "gui.hpp"
#include "globals.hpp"
#include "stbi/stbi.hpp"
#include <imgui/imgui.h>
#include <settings.hpp>

void texture_callback(const char *identifier, Texture *texture)
{
    if (identifier == nullptr || texture == nullptr)
        return;
    textures[identifier] = texture;
}

std::string selected_screenshot;
void render_file_browser()
{
    if (ImGui::BeginChild("##ScreenshotFS", {150, -FLT_MIN}, true)) {
        for (const auto &[name, path, position] : Settings::screenshots) {
            if (ImGui::Selectable(name.c_str(), selected_screenshot == name)) {
                selected_screenshot = name;
            }
        }
        ImGui::EndChild();
    }
}

bool editing_text = false;
void render_screenshot()
{
    if (!selected_screenshot.empty() && ImGui::BeginChild("##ScreenshotViewer", {0, 0}, true)) {
        ImGui::Text("%s", selected_screenshot.c_str());
        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
        if (selected_screenshot.find(".png") == std::string::npos &&
            ImGui::SmallButton("Convert to PNG##ScreenshotConvert")) {
            if (selected_screenshot.empty())
                return;
            const std::string path = Settings::screenshots_path.string() + "\\" + selected_screenshot;
            std::string path_png = Settings::screenshots_path.string() + "\\" + selected_screenshot;
            path_png.replace(path_png.find_last_of('.'), 4, ".png");
            auto name_png = std::string(selected_screenshot);
            name_png.replace(name_png.find_last_of('.'), 4, ".png");
            int x, y, n;
            const auto img = stbi_load(path.c_str(), &x, &y, &n, 0);
            if (img == nullptr) {
                api->Log(ELogLevel_WARNING, addon_name, "Failed to load image");
                ImGui::EndChild();
                return;
            }
            stbi_write_png(path_png.c_str(), x, y, n, img, x * n);
            stbi_image_free(img);
            const auto out = std::ranges::find(Settings::screenshots, selected_screenshot, &Settings::Screenshot::name);
            Settings::screenshots.emplace_back(name_png, path_png, out->position);
            std::filesystem::remove(path.c_str());
            Settings::screenshots.erase(
                std::ranges::find(Settings::screenshots, selected_screenshot, &Settings::Screenshot::name));
            selected_screenshot = name_png;
            Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
            Settings::save(Settings::settings_path);
            ImGui::EndChild();
            return;
        }
        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
        if (ImGui::SmallButton("Delete##ScreenshotDelete")) {
            if (selected_screenshot.empty())
                return;
            std::filesystem::remove(Settings::screenshots_path.string() + "\\" + selected_screenshot);
            Settings::screenshots.erase(
                std::ranges::find(Settings::screenshots, selected_screenshot, &Settings::Screenshot::name));
            Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
            Settings::save(Settings::settings_path);
            selected_screenshot.clear();
            ImGui::EndChild();
            return;
        }
        const auto screenshot =
            std::ranges::find(Settings::screenshots, selected_screenshot, &Settings::Screenshot::name);
        ImGui::Text("Taken at: %f %f", screenshot->position.X, screenshot->position.Y);
        if (const auto pos = selected_screenshot.find('.'); pos != std::string::npos) {
            const std::string identifier = std::string("SCREENSHOTS_IMAGE_").append(selected_screenshot.substr(0, pos));
            if (!textures.contains(identifier)) {
                const std::string file_path = Settings::screenshots_path.string() + "\\" + selected_screenshot;
                api->Textures.LoadFromFile(identifier.c_str(), file_path.c_str(), texture_callback);
            }
            if (textures[identifier] != nullptr)
                ImGui::Image(textures[identifier]->Resource,
                             ImVec2(static_cast<float>(textures[identifier]->Width) * Settings::image_scale.x,
                                    static_cast<float>(textures[identifier]->Height) * Settings::image_scale.y));
        }
        ImGui::EndChild();
    }
}