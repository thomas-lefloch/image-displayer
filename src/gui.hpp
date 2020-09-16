#include <iostream>
#include "texture.hpp"

namespace Gui {

bool input_dialog(std::string* filepath, int* timer);
enum CP_ACTION { NOOP, PREVIOUS, NEXT, CLOSE, PLAY_PAUSE };
CP_ACTION control_panel(int time_left, const bool playing);

}
