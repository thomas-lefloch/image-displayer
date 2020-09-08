#include <iostream>
#include "texture.hpp"

namespace Gui {

void init_texture();
bool input_dialog(std::string* filepath, int* timer);
enum CP_ACTION { NOOP, PREVIOUS, NEXT, ABORT, PLAY_PAUSE };
CP_ACTION control_panel(int time_left, const bool playing);
static Texture play_texture, pause_texture, prev_texture, next_texture, close_texture;

}
