#include <string>
#include <fstream>
#include <iostream>

namespace Session {

/**
 * returns true if the file was parse succesfully
 * does not guarantee that data is valid
 */
bool parse_file(const std::string& path, int& out_timer, std::vector<std::string>& out_images_path)
{
    std::fstream file;
    file.open(path, std::ios::in);

    std::string first_line;
    std::getline(file, first_line);

    try {
        out_timer = std::stoi(first_line);
    } catch (const std::exception& e) {
        file.close();
        return false;
    }

    std::string image_path;
    while (std::getline(file, image_path)) {
        out_images_path.push_back(image_path);
    }
    file.close();
    return true;
}
}