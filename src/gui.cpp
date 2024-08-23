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
        for (std::filesystem::directory_iterator dir(Settings::screenshots_path); const auto &entry : dir) {
            if (entry.is_regular_file()) {
                if (ImGui::Selectable(entry.path().filename().string().c_str(),
                                      selected_screenshot == entry.path().filename().string())) {
                    selected_screenshot = entry.path().filename().string();
                }
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
            std::filesystem::remove(Settings::screenshots_path.string() + "\\" + selected_screenshot);
        }
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