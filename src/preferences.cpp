#include "preferences.hpp"
#include <fstream>
#include <string>
#include <filesystem>

static const inline std::string filename = "preferences.txt";
static const inline std::string project_name = "ImageDisplayer";

// TODO: support different OS
std::string get_preferences_dir_path()
{
    std::string appdata(getenv("APPDATA"));
    return appdata + "/" + project_name; // TODO: replace with project
}

bool Preferences::save(const UserInput& inputs)
{
    const auto dir_path = get_preferences_dir_path();
    std::cout << dir_path << std::endl;
    if (!std::filesystem::exists(dir_path)) { std::filesystem::create_directory(dir_path); }

    std::ofstream file;
    std::string file_path = dir_path + "/" + filename;
    std::cout << file_path << std::endl;

    file.open(file_path, std::ios::out | std::ios::trunc);
    file << "timer=" << std::to_string(inputs.timer) << "\n";
    file << "image_folder_path=" << inputs.image_folder_path << "\n";
    file << "session_path=" << inputs.session_path << "\n";
    file.close();
    return true;
}
