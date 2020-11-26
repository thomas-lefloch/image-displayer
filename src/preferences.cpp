#include "preferences.hpp"
#include <fstream>
#include <string>
#include <filesystem>

// TODO: set create preferences.txt where exe is (like imgui.ini)
static const inline std::string filename = "preferences.txt";
static const inline std::string project_name = "ImageDisplayer";
static const inline std::string delimiter = "=";
static const inline std::string timer = "timer";
static const inline std::string image_folder_path = "image_folder_path";
static const inline std::string session_path = "session_path";

// TODO: support different OS
std::string get_preferences_dir_path()
{
    std::string appdata(getenv("APPDATA"));
    return appdata + "/" + project_name; // TODO: replace with project
}

bool Preferences::save(const UserInput& inputs)
{
    const auto dir_path = get_preferences_dir_path();
    if (!std::filesystem::exists(dir_path)) { std::filesystem::create_directory(dir_path); }

    std::ofstream file;
    std::string file_path = dir_path + "/" + filename;

    file.open(file_path, std::ios::out | std::ios::trunc);
    file << timer << delimiter << std::to_string(inputs.timer) << "\n";
    file << image_folder_path << delimiter << inputs.image_folder_path << "\n";
    file << session_path << delimiter << inputs.session_path << "\n";
    file.close();
    return true;
}

bool Preferences::load(UserInput& out)
{
    const auto file_path = get_preferences_dir_path() + "/" + filename;
    if (!std::filesystem::exists(file_path)) return false;

    std::fstream file;
    file.open(file_path, std::ios::in);
    std::string line;
    while (std::getline(file, line)) {
        const auto delim_index = line.find_first_of(delimiter);
        const std::string member = line.substr(0, delim_index);
        const std::string value = line.substr(delim_index + 1);
        if (member == timer) {
            try {
                out.timer = stoi(value); // should not throw an error but we never know
            } catch (std::exception e) {
                std::cout << e.what() << std::endl;
                file.close();
                return false;
            }
        } else if (member == image_folder_path)
            out.image_folder_path = value;
        else if (member == session_path)
            out.session_path = value;
        else
            break;
    }

    file.close();
    return true;
}