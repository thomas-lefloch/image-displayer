#include <vector>
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <regex>

#include "gui.hpp"

Gui::INPUT_ACTION Gui::input_dialog(UserInput* inputs)
{
    static std::vector<std::string> error_messages;
    Gui::INPUT_ACTION user_action = Gui::INPUT_ACTION::NO_ACTION;

    ImGui::Begin("files");
    ImGui::InputText("Images folder", &inputs->images_folder);
    ImGui::InputInt("Timer (sec)", &inputs->timer);
    ImGui::InputText("Session folder", &inputs->session_folder); // TODO: support filename
    if (ImGui::Button("Ok")) {
        error_messages.clear();
        user_action = Gui::INPUT_ACTION::NEW_SESSION;
        // images_folder errors
        if (inputs->images_folder.empty())
            error_messages.push_back("Images folder :: please insert a path to a directory");
        else if (!std::filesystem::is_directory(inputs->images_folder))
            error_messages.push_back(inputs->images_folder + " not found or is not a directory");
        // timer errors
        if (inputs->timer < 1) error_messages.push_back("timer must be > 1 sec");
        // session_folder errors
        if (inputs->session_folder.empty())
            error_messages.push_back("Session Folder :: please insert a path to a directory");
        else if (!std::filesystem::is_directory(inputs->session_folder))
            error_messages.push_back(inputs->session_folder + " not found or is not a directory");
    }

    ImGui::InputText("Session File", &inputs->session_replay_file);
    if (ImGui::Button("Replay session")) {
        error_messages.clear();
        user_action = Gui::INPUT_ACTION::REPLAY_SESSION;
        if (inputs->session_replay_file.empty())
            error_messages.push_back("Replay Session :: please insert a path to a file");
        else if (!std::filesystem::exists(inputs->session_replay_file))
            error_messages.push_back("Replay Session :: file does not exists");
        // TODO: check file format
    }

    for (const auto& err_msg : error_messages)
        ImGui::Text(err_msg.c_str()); // TODO: red text
    ImGui::End();

    return error_messages.empty() ? user_action : Gui::INPUT_ACTION::NO_ACTION;
}

Gui::CP_ACTION Gui::control_panel(const int time_left, const bool playing)
{
    Gui::CP_ACTION action = Gui::CP_ACTION::NOOP;
    ImGui::Begin("Control panel");

    ImGui::Text(std::to_string(time_left).c_str()); // better way ???
    if (ImGui::Button("Previous")) action = Gui::CP_ACTION::PREVIOUS;
    if (!playing) {
        if (ImGui::Button("Play")) action = Gui::CP_ACTION::PLAY_PAUSE;
    } else {
        if (ImGui::Button("Pause")) action = Gui::CP_ACTION::PLAY_PAUSE;
    }
    if (ImGui::Button("Next")) action = Gui::CP_ACTION::NEXT;
    if (ImGui::Button("Close")) action = Gui::CP_ACTION::CLOSE;
    ImGui::End();
    return action;
}
