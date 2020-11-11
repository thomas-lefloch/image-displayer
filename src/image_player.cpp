#include "image_player.hpp"
#include <math.h>
#include <time.h>
#include <filesystem>
#include <fstream>
#include <stdio.h>
#include <string>

bool ImagePlayer::next(std::uniform_real_distribution<double>& dist, std::mt19937& gen, const UserInput& user_input)
{
    // FIXME: throws execption if clicking close (clearing the file list) at the same moment as picking a
    // filename
    glDeleteTextures(1, &current_texture.id);

    std::string next_image;
    bool chose_new_image = false;
    int max_index = displayed_images.size() - 1; // ¯\_(ツ)_/¯ bugged when inlining into if statement
    if (current_image_idx < max_index) {
        next_image = displayed_images[++current_image_idx];
    } else {
        next_image = user_input.images.at((int)(dist(gen) * user_input.images.size()));
        chose_new_image = true;
    }
    bool image_loaded = Texture::load_from_file(next_image.c_str(), &current_texture);

    if (image_loaded && chose_new_image) {
        displayed_images.push_back(next_image);
        current_image_idx = displayed_images.size() - 1;
        // saving filename into session
        std::ofstream save_file;
        while (session_filepath.empty()) { // we never know if a file with the same name already exists
            const auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
            // windows automatically translate "/" to "\\"
            const auto filepath = user_input.session_path + "/" + std::to_string(timestamp) + ".txt";
            if (std::filesystem::exists(filepath)) continue;

            session_filepath = filepath;
            save_file.open(session_filepath, std::ios::app);
            save_file << std::to_string(user_input.timer) + "\n"; // first line of the file is the timer
            save_file.close();
        }
        // opening save_file two times beacause i don't know how to check if file is empty with ofstream
        save_file.open(session_filepath, std::ios::app);
        save_file << next_image << "\n";
        save_file.close();
    }

    time_left = user_input.timer;
    return image_loaded;
}

bool ImagePlayer::previous(const UserInput& user_input)
{
    if (current_image_idx > 0) {
        time_left = user_input.timer;
        glDeleteTextures(1, &current_texture.id);
        return Texture::load_from_file(displayed_images[--current_image_idx].c_str(), &current_texture);
    }
    return false;
}
