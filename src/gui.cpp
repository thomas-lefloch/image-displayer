#include "gui.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include <vector>
#include <filesystem>

bool input_dialog(std::string* selected_path, int* timer)
{
    static std::vector<std::string> error_messages;
    bool form_validated = false;
    ImGui::Begin("files");
    // TODO: add multiple folder input
    // TODO: make enter key press "Ok" button
    ImGui::InputText("Folder path", selected_path);
    ImGui::InputInt("Timer (sec)", timer);
    if (ImGui::Button("Ok")) {
        form_validated = true;
        error_messages.clear();
        if (selected_path->empty())
            error_messages.push_back("please insert a path to a directory");
        else if (!std::filesystem::exists(*selected_path))
            error_messages.push_back((*selected_path) + " not found");
        if (*timer < 1) error_messages.push_back("timer must be > 1 sec");
    }
    for (const auto& err_msg : error_messages)
        ImGui::Text(err_msg.c_str()); // TODO: red text

    ImGui::End();
    return form_validated && error_messages.empty();
}

void control_panel(int time_left)
{
    ImGui::Begin("Control panel");
    // TODO: keyboad shortcuts
    // TODO: change icon color to white, get rid of background
    // TODO: bigger font size
    ImGui::Text(std::to_string(time_left).c_str()); // better way ???
    // TODO: reimplement buttons. require some sort of data management
    // ImGui::SameLine();
    // TODO: refactor. (with lambda ??)
    // if (ImGui::ImageButton((ImTextureID)prev_texture.id, ImVec2(prev_texture.width, prev_texture.height))) {
    //     // TODO: remember previous images
    // }
    // ImGui::SameLine();
    // if (!playing) {
    //     if (ImGui::ImageButton((ImTextureID)play_texture.id, ImVec2(play_texture.width, play_texture.height))) {
    //         playing = true;
    //     }
    // } else {
    //     if (ImGui::ImageButton(
    //             (ImTextureID)pause_texture.id, ImVec2(pause_texture.width, pause_texture.height))) {
    //         playing = false;
    //     }
    // }
    // ImGui::SameLine();
    // if (ImGui::ImageButton((ImTextureID)next_texture.id, ImVec2(next_texture.width, next_texture.height))) {
    //     pick_image(&distribution, &generator, &current_texture, &filelist);
    //     clock = base_timer;
    // }
    // ImGui::SameLine();
    // if (ImGui::ImageButton((ImTextureID)close_texture.id, ImVec2(close_texture.width, close_texture.height))) {
    //     filelist.clear();
    //     playing = false;
    // }
    ImGui::End();
}
