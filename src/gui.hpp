#include <iostream>

#ifndef user_input_h
#include "user_input.hpp"
#endif

namespace Gui {

enum INPUT_ACTION { NO_ACTION, REPLAY_SESSION, NEW_SESSION };
INPUT_ACTION input_dialog(UserInput* inputs);
enum CP_ACTION { NOOP, PREVIOUS, NEXT, CLOSE, PLAY_PAUSE };
CP_ACTION control_panel(int time_left, const bool playing);

};
