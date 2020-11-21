#ifndef image_player_h
#define image_player_h

#include "user_input.hpp"
#include "texture.hpp"
#include "gui.hpp"
#include <random>
#include <vector>
#include <list>
#include <iterator>

struct ImagePlayer {

    Texture current_texture;
    // TODO: replace those by double linked list
    std::list<std::string> displayed_images;
    std::list<std::string>::iterator it;
    std::string session_filepath;
    bool playing = false;
    double time_left;

    ImagePlayer() { it = displayed_images.begin(); }

    bool next(std::uniform_real_distribution<double>& dist, std::mt19937& gen, const UserInput& user_input);
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