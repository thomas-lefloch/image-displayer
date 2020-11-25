#ifndef preferences_h
#define preferences_h

#include "user_input.hpp"
struct Preferences {
    static bool save(const UserInput& inputs);
    static bool load(UserInput& out);
};

#endif