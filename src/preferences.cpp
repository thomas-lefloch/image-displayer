#include "preferences.hpp"
#include <fstream>
#include <string>
#include <filesystem>
#include "current_dir.hpp"

// TODO: set create preferences.txt where exe is (like imgui.ini)
static const inline std::string filename = "preferences.txt";
static const inline std::string delimiter = "=";
static const inline std::string timer = "timer";
static const inline std::string image_folder_path = "image_folder_path";
static const inline std::string session_path = "session_path";

bool Preferences::save(const UserInput& inputs)
{
    std::ofstream file;
    std::string file_path = current_directory() + "/" + filename;

    file.open(file_path, std::ios::out | std::ios::trunc);
    file << timer << delimiter << std::to_string(inputs.timer) << "\n";
    file << image_folder_path << delimiter << inputs.image_folder_path << "\n";
    file << session_path << delimiter << inputs.session_path << "\n";
    file.close();
    return true;
}

bool Preferences::load(UserInput& out)
{
    const auto file_path = current_directory() + "/" + filename;
    if (!std::filesystem::exists(file_path)) return false;

    std::fstream file;
    file.open(file_path, std::ios::in);
    std::string line;
    while (std::getline(file, line)) {
        const auto delim_index = line.find_first_of(delimiter);
        const std::string member = line.substr(0, delim_index);
        const std::string value = line.substr(delim_index + 1);
        if (member == timer && value.length() != 0) {
            try {
                out.timer = stoi(value); // should not throw an error but we never know
            } catch (std::exception e) {
                std::cout << e.what() << std::endl;
                file.close();
                return false;
            }
        } else if (member == image_folder_path && value.length() != 0)
            out.image_folder_path = value;
        else if (member == session_path && value.length() != 0)
            out.session_path = value;
        else
            break;
    }

    file.close();
    return true;
}