#include <vector>
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include "gui.hpp"

bool Gui::input_dialog(std::string* selected_path, int* timer)
{
    static std::vector<std::string> error_messages;
    bool form_validated = false;
    ImGui::Begin("files");
    ImGui::InputText("Folder path", selected_path);
    ImGui::InputInt("Timer (sec)", timer);
    if (ImGui::Button("Ok")) { // TODO: make enter key press "Ok" button
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

Gui::CP_ACTION Gui::control_panel(const int time_left, const bool playing)
{
    Gui::CP_ACTION action = Gui::CP_ACTION::NOOP;
    ImGui::Begin("Control panel");

    ImGui::Text(std::to_string(time_left).c_str()); // better way ??? // TODO: bigger font size
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
