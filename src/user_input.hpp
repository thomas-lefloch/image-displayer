#ifndef user_input_h
#define user_input_h
#endif

#include <iostream>

struct UserInput {
    std::vector<std::string> images;
    int timer = 30;
    std::string session_path = "";
};
