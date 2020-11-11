#ifndef gui_h
#define gui_h

#include <iostream>
#include "user_input.hpp"

namespace Gui {

enum INPUT_ACTION { NO_ACTION, REPLAY_SESSION, NEW_SESSION };
INPUT_ACTION input_dialog(UserInput& inputs);
enum CP_ACTION { NOOP, PREVIOUS, NEXT, CLOSE, PLAY_PAUSE };
CP_ACTION control_panel(int time_left, const bool playing);

};

#endif
