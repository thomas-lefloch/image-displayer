#include "image_player.hpp"
#include <math.h>
#include <time.h>
#include <filesystem>
#include <fstream>
#include <stdio.h>
#include <string>

// FIXME: throws execption if clicking close (clearing the file list) at the same moment as picking a
// filename
bool ImagePlayer::next(const UserInput& user_input)
{
    time_left = user_input.timer;
    glDeleteTextures(1, &current_texture.id);

    if (displayed_images.size() > 0 && it != --displayed_images.end()) {
        return Texture::load_from_file((++it)->c_str(), current_texture);
    } else {
        std::string next_image = user_input.images.at((int)(distribution(generator) * user_input.images.size()));
        displayed_images.push_back(next_image);
        it = --displayed_images.end();

        bool image_loaded = Texture::load_from_file(it->c_str(), current_texture);

        if (!image_loaded) { // error while loading image, cleaning & not saving it in session
            displayed_images.pop_back();
            it--;
            return false;
        }

        std::ofstream save_file;
        while (session_filepath.empty()) { // we never know if a file with the same name already exists
            const auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            // windows automatically translate "/" to "\\"
            const auto filepath = user_input.session_path + "/" + std::to_string(timestamp) + ".txt";
            if (std::filesystem::exists(filepath)) continue;

            session_filepath = filepath;
            save_file.open(session_filepath, std::ios::app);
            save_file << std::to_string(user_input.timer) + "\n"; // first line of the file is the timer
        }
        if (!save_file.is_open()) save_file.open(session_filepath, std::ios::app);
        save_file << *it << "\n";
        save_file.close();

        return image_loaded;
    }
}

bool ImagePlayer::previous(const UserInput& user_input)
{
    if (it == displayed_images.begin()) return false;

    time_left = user_input.timer;
    glDeleteTextures(1, &current_texture.id);
    return Texture::load_from_file((--it)->c_str(), current_texture);
}
