#include "gui.hpp"
#include "globals.hpp"
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
    if (ImGui::BeginChild("Filesystem##ScreenshotFS", {150, -FLT_MIN}, true)) {
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
    if (!selected_screenshot.empty() && ImGui::BeginChild("Editor##ScreenshotViewer", {0, 0}, true)) {
        ImGui::Text("%s", selected_screenshot.c_str());
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
                             ImVec2(textures[identifier]->Width / 2, textures[identifier]->Height / 2));
        }
        ImGui::EndChild();
    }
}