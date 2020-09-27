#include <string>
#include <fstream>
#include <iostream>

namespace Session {

bool is_file_format_valid(std::string path)
{
    std::fstream file;
    file.open(path, std::ios::in);

    std::string first_line;
    std::getline(file, first_line);

    const unsigned int timer = std::stoi(first_line);
    if (timer < 1) return false;

        return false;
}
}