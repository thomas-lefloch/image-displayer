#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include <iostream>
#include <fstream>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <helpers/RootDir.h>
#include <filesystem>
#include <random>

#include "shader.hpp"
#include "user_input.hpp"
#include "texture.hpp"
#include "gui.hpp"

constexpr int window_width = 1600;
constexpr int window_height = 900;

ImVec4 clear_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Image displayer", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 130";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glfwMakeContextCurrent(window);
    glViewport(0, 0, window_width, window_height);

    float vertices[] = {
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f, //
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, //
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f //
    };
    unsigned int indices[] = { 1, 2, 3, 3, 0, 1 };
    unsigned int vao, vbo, ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    int shader_id = create_shader_program(ROOT_DIR "shaders/vertex.vs", ROOT_DIR "shaders/fragment.fs");
    if (shader_id == -1) std::cout << "Error while parsing/compiling shaders" << std::endl;

    int img_ratio_uniform = glGetUniformLocation(shader_id, "img_ratio");

    std::random_device device;
    std::mt19937 generator(device());
    std::uniform_real_distribution<double> distribution(0, 1);

    struct State {
        Texture current_texture;
        UserInput user_input;
        std::vector<std::string> selected_files;
        std::vector<std::string> displayed_images;
        int current_image_idx = 0;
        std::string session_filepath;
        bool playing = false;
        double time_left;
    };

    State state;

    auto next_image = [](std::uniform_real_distribution<double> * dist, std::mt19937 * gen, State * s) -> auto
    {
        // FIXME: throws execption if clicking close (clearing the file list) at the same moment as picking a filename
        glDeleteTextures(1, &s->current_texture.id);

        std::string selected_filepath;
        bool new_image = false;
        int max_index = s->displayed_images.size() - 1; // ¯\_(ツ)_/¯ bugged when inlining into if statement
        if (s->current_image_idx < max_index) {
            selected_filepath = s->displayed_images[++s->current_image_idx];
        } else {
            selected_filepath = s->selected_files.at((int)((*dist)(*gen) * s->selected_files.size()));
            new_image = true;
        }
        bool image_loaded = Texture::load_from_file(selected_filepath.c_str(), &s->current_texture);

        if (!image_loaded) {
            return false;
        } else if (new_image) {
            s->displayed_images.push_back(selected_filepath);
            s->current_image_idx = s->displayed_images.size() - 1;
            // saving filename into session
            std::ofstream save_file;
            while (s->session_filepath.empty()) { // apparently windows automatically translate "/" to "\\"
                const auto timestamp = std::chrono::system_clock::now().time_since_epoch().count();
                const auto filepath = s->user_input.session_folder + "/" + std::to_string(timestamp) + ".txt";
                if (!std::filesystem::exists(filepath)) s->session_filepath = filepath;
                // opening save_file two times  beacause i don"t know how to check if file is empty with ofstream
                save_file.open(s->session_filepath, std::ios::app);
                save_file << std::to_string(s->user_input.timer) + "\n"; // first line of the session file is the timer
                save_file.close();
            }
            save_file.open(s->session_filepath, std::ios::app);
            save_file << selected_filepath << "\n";
            save_file.close();
            return true;
        } else
            return true; // nothing to do anymore image already loaded
    };

    while (!glfwWindowShouldClose(window)) {
        double start_time = glfwGetTime();
        if (state.playing && state.time_left < 0) {
            next_image(&distribution, &generator, &state);
            state.time_left = state.user_input.timer;
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (state.selected_files.empty()) {
            if (Gui::input_dialog(&state.user_input)) {
                state.time_left = state.user_input.timer;
                for (const auto& file : std::filesystem::directory_iterator(state.user_input.images_folder))
                    state.selected_files.push_back(file.path().string());
                next_image(&distribution, &generator, &state);
            }
        } else {
            switch (Gui::control_panel(state.time_left, state.playing)) {
            case Gui::CP_ACTION::PLAY_PAUSE:
                state.playing = !state.playing;
                break;
            case Gui::CP_ACTION::PREVIOUS: // TODO: implement
                if (state.current_image_idx > 0) {
                    state.time_left = state.user_input.timer;
                    glDeleteTextures(1, &state.current_texture.id);
                    Texture::load_from_file(
                        state.displayed_images[--state.current_image_idx].c_str(), &state.current_texture);
                }
                break;
            case Gui::CP_ACTION::NEXT:
                next_image(&distribution, &generator, &state);
                state.time_left = state.user_input.timer;
                break;
            case Gui::CP_ACTION::CLOSE: {
                state.selected_files.clear();
                state.displayed_images.clear();
                state.playing = false;
                break;
            }
            case Gui::CP_ACTION::NOOP:
                break;
            default:
                break;
            };
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        if (state.current_texture.id) {
            glBindTexture(GL_TEXTURE_2D, state.current_texture.id);
            glUseProgram(shader_id);
            glBindVertexArray(vao);
            // probably exist a better solution to make the image fit the window
            float img_ratio[2] = { //
                (float)state.current_texture.width / (float)window_width,
                (float)state.current_texture.height / (float)window_height
            };
            if (img_ratio[0] < img_ratio[1]) {
                img_ratio[0] = img_ratio[0] / img_ratio[1];
                img_ratio[1] = 1;
            } else {
                img_ratio[1] = img_ratio[1] / img_ratio[0];
                img_ratio[0] = 1;
            }
            glUniform2fv(img_ratio_uniform, 1, img_ratio);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        if (state.playing) state.time_left -= glfwGetTime() - start_time;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
