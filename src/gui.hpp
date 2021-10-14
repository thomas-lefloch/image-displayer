#ifndef gui_h
#define gui_h

#include <GLFW/glfw3.h>
#include <iostream>

#include "user_input.hpp"
#include "image_player.hpp"

struct GuiInformations {
    GLFWwindow* window;
    int shader_id = 0;
    int window_width = 1600;
    int window_height = 900;
    GLuint vao = 0;
    int img_ratio_uniform = 0;
    int black_white_uniform = 0;
};

struct Gui {
    // Not Good looks like we are passing options for the gui to init but it is init's return value
    static bool init(GuiInformations& infos);
    static void clean(GuiInformations& infos);
    static void begin_new_imgui_frame();

    static void display_new_frame( GuiInformations& gui_infos, const UserInput& user_inputs, const ImagePlayer& image_player);

    enum class INPUT_ACTION { NO_ACTION, REPLAY_SESSION, NEW_SESSION, SAVE_PREFERENCES };
    enum class CP_ACTION { NOOP, PREVIOUS, NEXT, CLOSE, PLAY_PAUSE, TOGGLE_BW };
    static INPUT_ACTION input_dialog(UserInput& inputs);
    static CP_ACTION control_panel(int time_left, const bool playing, bool black_white);
};

#endif
