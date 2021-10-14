#ifndef image_player_h
#define image_player_h

#include "user_input.hpp"
#include "texture.hpp"
#include <random>
#include <vector>
#include <list>

struct ImagePlayer {

    Texture current_texture;
    // TODO: replace those by double linked list
    std::list<std::string> displayed_images;
    std::list<std::string>::iterator it;
    std::string session_filepath;
    bool playing = false;
    double time_left;
    bool black_white = false;

    // should be pointers
    std::random_device device;
    std::mt19937 generator;
    std::uniform_real_distribution<double> distribution;

    ImagePlayer()
    {
        it = displayed_images.begin();
        generator = std::mt19937(device());
        distribution = std::uniform_real_distribution<double>(0, 1);
    }

    bool next(const UserInput& user_input);
    bool previous(const UserInput& user_input);
    void set_images(const std::vector<std::string>& images)
    {
        std::copy(images.begin(), images.end(), std::back_inserter(displayed_images));
        it = displayed_images.begin();
    }
    void reset()
    {
        displayed_images.clear();
        playing = false;
    }
    void toggle_play() { playing = !playing; }
};
#endif