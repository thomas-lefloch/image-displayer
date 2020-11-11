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
#include <filesystem>
#include <stdio.h>
#include <helpers/RootDir.h>
#include <random>

#include "shader.hpp"
#include "user_input.hpp"
#include "gui.hpp"
#include "image_player.hpp"

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

    ImagePlayer image_player;
    UserInput user_input;

    while (!glfwWindowShouldClose(window)) {
        double start_time = glfwGetTime();

        if (image_player.playing && image_player.time_left < 0) //
            image_player.next(distribution, generator, user_input);

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (user_input.images.empty()) {
            switch (Gui::input_dialog(user_input)) {
            case Gui::INPUT_ACTION::NEW_SESSION:
                image_player.next(distribution, generator, user_input);
                break;
            case Gui::INPUT_ACTION::REPLAY_SESSION:
                image_player.set_images(user_input.images);
                image_player.next(distribution, generator, user_input);
                break;
            case Gui::INPUT_ACTION::NO_ACTION:
                break;
            }
        } else {
            switch (Gui::control_panel(image_player.time_left, image_player.playing)) {
            case Gui::CP_ACTION::PLAY_PAUSE:
                image_player.playing = !image_player.playing;
                break;
            case Gui::CP_ACTION::PREVIOUS:
                image_player.previous(user_input);
                break;
            case Gui::CP_ACTION::NEXT:
                image_player.next(distribution, generator, user_input);
                break;
            case Gui::CP_ACTION::CLOSE: {
                user_input.clear_images();
                image_player.reset();
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

        if (image_player.current_texture.id) {
            glBindTexture(GL_TEXTURE_2D, image_player.current_texture.id);
            glUseProgram(shader_id);
            glBindVertexArray(vao);
            // probably exist a better solution to make the image fit the window
            float img_ratio[2] = { //
                (float)image_player.current_texture.width / (float)window_width,
                (float)image_player.current_texture.height / (float)window_height
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

        if (image_player.playing) image_player.time_left -= glfwGetTime() - start_time;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
