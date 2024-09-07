#include "gui.hpp"
#include "globals.hpp"
#include "stbi/stbi.hpp"
#include <filesystem>
#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>
#include <settings.hpp>

void texture_callback(const char *identifier, Texture *texture)
{
    if (identifier == nullptr || texture == nullptr)
        return;
    textures[identifier] = texture;
}

int screenshot_index = -1;
void render_file_browser()
{
    if (ImGui::BeginChild("##ScreenshotFS", {150, -FLT_MIN}, true)) {
        for (int i = 0; i < Settings::screenshots.size(); i++) {
            if (ImGui::Selectable(Settings::screenshots[i].name.c_str(), i == screenshot_index)) {
                screenshot_index = i;
            }
        }
        ImGui::EndChild();
    }
}

bool renaming = false;
std::string new_name;
void render_screenshot()
{
    Settings::Screenshot selected_screenshot;
    if (screenshot_index == -1)
        selected_screenshot = {};
    else
        selected_screenshot = Settings::screenshots[screenshot_index];
    if (ImGui::BeginChild("##ScreenshotViewer", {0, 0}, true)) {
        ImGui::Text("%s", selected_screenshot.name.c_str());
        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
        if (selected_screenshot.name.find(".png") == std::string::npos &&
            ImGui::SmallButton("Convert to PNG##ScreenshotConvert")) {
            if (selected_screenshot.name.empty())
                return;
            const std::string path = (Settings::screenshots_path / selected_screenshot.name).string();
            std::string path_png = (Settings::screenshots_path / selected_screenshot.name).string();
            path_png.replace(path_png.find_last_of('.'), 4, ".png");
            auto name_png = std::string(selected_screenshot.name);
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
            Settings::screenshots.emplace_back(name_png, path_png, selected_screenshot.position);
            std::filesystem::remove(path.c_str());
            Settings::screenshots.erase(Settings::screenshots.begin() + screenshot_index);
            screenshot_index = -1;
            Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
            Settings::save(Settings::settings_path);
            ImGui::EndChild();
            return;
        }
        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
        if (ImGui::SmallButton("Rename##ScreenshotRename")) {
            if (selected_screenshot.name.empty())
                return;
            new_name = selected_screenshot.name;
            renaming = true;
        }
        ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
        if (ImGui::SmallButton("Delete##ScreenshotDelete")) {
            if (selected_screenshot.name.empty())
                return;
            std::filesystem::remove(Settings::screenshots_path / selected_screenshot.name);
            Settings::screenshots.erase(Settings::screenshots.begin() + screenshot_index);
            Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
            Settings::save(Settings::settings_path);
            screenshot_index = -1;
            ImGui::EndChild();
            return;
        }
        if (renaming) {
            ImGui::InputText("New Name##ScreenshotRenameInput", &new_name);
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::SmallButton("Confirm##ScreenshotConfirmRename")) {
                if (selected_screenshot.name.find(".jpg") != std::string::npos) {
                    if (new_name.find(".jpg") == std::string::npos) {
                        new_name.append(".jpg");
                    }
                } else if (selected_screenshot.name.find(".png") != std::string::npos) {
                    if (new_name.find(".png") == std::string::npos) {
                        new_name.append(".png");
                    }
                }
                std::filesystem::rename(Settings::screenshots_path / selected_screenshot.name,
                                        Settings::screenshots_path / new_name);
                Settings::screenshots.emplace_back(new_name, (Settings::screenshots_path / new_name).string(),
                                                   selected_screenshot.position);
                Settings::screenshots.erase(Settings::screenshots.begin() + screenshot_index);
                selected_screenshot = Settings::screenshots.back();
                screenshot_index = Settings::screenshots.size() - 1;
                Settings::json_settings[Settings::SCREENSHOTS] = Settings::screenshots;
                Settings::save(Settings::settings_path);

                new_name.clear();
                renaming = false;
            }
            ImGui::SameLine(0, 1 * ImGui::GetStyle().ItemSpacing.x);
            if (ImGui::SmallButton("Cancel##ScreenshotCancelRename")) {
                new_name.clear();
                renaming = false;
            }
        }

        ImGui::Text("Taken at: %f %f", selected_screenshot.position.X, selected_screenshot.position.Y);
        if (const auto pos = selected_screenshot.name.find('.'); pos != std::string::npos) {
            const std::string identifier =
                std::string("SCREENSHOTS_IMAGE_").append(selected_screenshot.name.substr(0, pos));
            if (!textures.contains(identifier)) {
                const std::filesystem::path file_path = Settings::screenshots_path / selected_screenshot.name;
                api->Textures.LoadFromFile(identifier.c_str(), file_path.string().c_str(), texture_callback);
            }
            if (textures[identifier] != nullptr)
                ImGui::Image(textures[identifier]->Resource,
                             ImVec2(static_cast<float>(textures[identifier]->Width) * Settings::image_scale.x,
                                    static_cast<float>(textures[identifier]->Height) * Settings::image_scale.y));
        }
        ImGui::EndChild();
    }
}
