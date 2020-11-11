#ifndef image_player_h
#define image_player_h

#include "user_input.hpp"
#include "texture.hpp"
#include "gui.hpp"
#include <random>
#include <vector>

struct ImagePlayer {

    Texture current_texture;
    // TODO: replace those by double linked list
    std::vector<std::string> displayed_images;
    int current_image_idx = 0;
    std::string session_filepath;
    bool playing = false;
    double time_left;

    bool next(std::uniform_real_distribution<double>& dist, std::mt19937& gen, const UserInput& user_input);
    bool previous(const UserInput& user_input);
    void set_images(const std::vector<std::string>& images)
    {
        // TODO: convert to list
        // and set iterator
        displayed_images = images;
    }
    void reset()
    {
        displayed_images.clear();
        playing = false;
    }
};
#endif