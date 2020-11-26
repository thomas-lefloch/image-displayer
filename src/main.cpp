#include <GLFW/glfw3.h>

#include <time.h>
#include <helpers/RootDir.h>
#include <filesystem>

#include "current_dir.hpp"
#include "user_input.hpp"
#include "preferences.hpp"
#include "image_player.hpp"
#include "gui.hpp"

int main()
{
    GuiInformations gui_infos;
    if (!Gui::init(gui_infos)) return -1;

    ImagePlayer image_player;
    UserInput user_inputs;
    // Settings defaults
    user_inputs.session_path = current_directory() + "\\sessions";

    if (!std::filesystem::exists(user_inputs.session_path)) //
        std::filesystem::create_directory(user_inputs.session_path);

    Preferences::load(user_inputs);

    while (!glfwWindowShouldClose(gui_infos.window)) {
        double start_time = glfwGetTime();

        if (image_player.playing && image_player.time_left < 0) //
            image_player.next(user_inputs);

        glfwPollEvents();
        Gui::begin_new_imgui_frame();

        if (user_inputs.images.empty()) {
            switch (Gui::input_dialog(user_inputs)) {
            case Gui::INPUT_ACTION::NEW_SESSION:
                image_player.next(user_inputs);
                break;
            case Gui::INPUT_ACTION::REPLAY_SESSION:
                image_player.set_images(user_inputs.images);
                image_player.next(user_inputs);
                break;
            case Gui::INPUT_ACTION::SAVE_PREFERENCES:
                Preferences::save(user_inputs);
            case Gui::INPUT_ACTION::NO_ACTION:
            default:
                break;
            }
        } else {
            switch (Gui::control_panel((int)image_player.time_left, image_player.playing)) {
            case Gui::CP_ACTION::PLAY_PAUSE:
                image_player.toggle_play();
                break;
            case Gui::CP_ACTION::PREVIOUS:
                image_player.previous(user_inputs);
                break;
            case Gui::CP_ACTION::NEXT:
                image_player.next(user_inputs);
                break;
            case Gui::CP_ACTION::CLOSE: {
                user_inputs.clean();
                image_player.reset();
                break;
            }
            case Gui::CP_ACTION::NOOP:
            default:
                break;
            };
        }

        Gui::display_new_frame(gui_infos, user_inputs, image_player);

        glfwSwapBuffers(gui_infos.window);
        if (image_player.playing) image_player.time_left -= glfwGetTime() - start_time;
    }

    Gui::clean(gui_infos);
    return 0;
}
