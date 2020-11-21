#ifndef user_input_h
#define user_input_h

#include <iostream>
#include <vector>

struct UserInput {
    std::vector<std::string> images;
    int timer = 30;
    std::string session_path = "";

    void clean() { images.clear(); }
};
#endif
