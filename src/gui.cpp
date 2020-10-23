#include <vector>
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"
#include <regex>

#include "gui.hpp"
#include "session.hpp"

Gui::INPUT_ACTION Gui::input_dialog(UserInput& inputs)
{
    static std::vector<std::string> error_messages;
    Gui::INPUT_ACTION user_action = Gui::INPUT_ACTION::NO_ACTION;

    ImGui::Begin("files");
    static std::string image_folder_path = "";
    ImGui::InputText("Images path", &image_folder_path);
    ImGui::InputInt("Timer (sec)", &inputs.timer);
    ImGui::InputText("Session path", &inputs.session_path); // TODO: support filename
    if (ImGui::Button("Ok")) {
        error_messages.clear();
        user_action = Gui::INPUT_ACTION::NEW_SESSION;

        // image_folder_path errors
        if (image_folder_path.empty())
            error_messages.push_back("Images path :: please insert a path to a directory");
        else if (!std::filesystem::is_directory(image_folder_path))
            error_messages.push_back(image_folder_path + " not found or is not a directory");
        // timer errors
        if (inputs.timer < 1) error_messages.push_back("timer must be > 1 sec");
        // session_path errors
        if (inputs.session_path.empty())
            error_messages.push_back("Session path :: please insert a path to a directory");
        else if (!std::filesystem::is_directory(inputs.session_path))
            error_messages.push_back(inputs.session_path + " not found or is not a directory");

        if (error_messages.empty()) {
            for (const auto& file : std::filesystem::directory_iterator(image_folder_path))
                inputs.images.push_back(file.path().string());
        }
    }

    static std::string session_file_path;
    ImGui::InputText("Session File", &session_file_path);
    if (ImGui::Button("Replay session")) {
        error_messages.clear();
        user_action = Gui::INPUT_ACTION::REPLAY_SESSION;
        if (session_file_path == "")
            error_messages.push_back("Replay Session :: please insert a path to a file");
        else if (!std::filesystem::exists(session_file_path))
            error_messages.push_back("Replay Session :: file does not exists");

        if (!Session::parse_file(session_file_path, inputs.timer, inputs.images))
            error_messages.push_back("Replay Session :: Session file not parsable");
        if (inputs.timer < 1) error_messages.push_back("Replay Session :: timer must be > 1 sec");
        if (inputs.images.empty()) error_messages.push_back("Replay Session :: Invalid number of images");
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
