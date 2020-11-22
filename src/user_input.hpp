#ifndef user_input_h
#define user_input_h

#include <iostream>
#include <vector>

struct UserInput {
    std::vector<std::string> images;
    std::string image_folder_path = "";
    std::string session_path = "";
    int timer = 30;
    void clean() { images.clear(); }
};
#endif
