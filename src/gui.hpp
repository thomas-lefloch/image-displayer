#include <iostream>

bool input_dialog(std::string* filepath, int* timer);

// control panel action
enum CP_ACTION { NOOP, PREVIOUS, NEXT, ABORT, PLAY_PAUSE };

CP_ACTION control_panel(int time_left, const bool playing);