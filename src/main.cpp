#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include <iostream>
#include <math.h>
#include <stdio.h>
#include <helpers/RootDir.h>
#include <filesystem>
#include <random>

#include "shader.hpp"
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

    Gui::init_texture();

    Texture current_image;
    std::vector<std::string> filelist;
    bool playing = false;
    static std::string input_path = "";
    static int base_timer = 30;
    double time_left = base_timer;

    // FIXME: way too dirty, wait for global refactoring ??
    auto pick_image = [](std::uniform_real_distribution<double> * dist, std::mt19937 * gen, Texture * texture,
        std::vector<std::string> * files) -> auto
    {
        // TODO: check for images
        // -> black texture if trying to display something other than an image
        // = display palceholder image (image not recognized, please open something else)
        // FIXME: throws execption if clicking close (clearing the vector) at the same moment as picking a filename
        glDeleteTextures(1, &texture->id);
        return Texture::load_from_file(files->at((int)((*dist)(*gen) * files->size())).c_str(), texture);
    };

    while (!glfwWindowShouldClose(window)) {

        // FIXME: Display does not update when image is moving but time is
        double start_time = glfwGetTime();
        if (playing && time_left < 0) {
            pick_image(&distribution, &generator, &current_image, &filelist);
            time_left = base_timer;
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (filelist.empty()) {
            // TODO: make input_dialog return valid path and timer instead of passing them by ref
            if (Gui::input_dialog(&input_path, &base_timer)) {
                time_left = base_timer;
                for (const auto& file : std::filesystem::directory_iterator(input_path))
                    filelist.push_back(file.path().string());
                pick_image(&distribution, &generator, &current_image, &filelist);
            }
        } else {
            switch (Gui::control_panel(time_left, playing)) {
            case Gui::CP_ACTION::PLAY_PAUSE:
                playing = !playing;
                break;
            case Gui::CP_ACTION::PREVIOUS:
                break; // TODO: implement that
            case Gui::CP_ACTION::NEXT:
                pick_image(&distribution, &generator, &current_image, &filelist);
                time_left = base_timer;
                break;
            case Gui::CP_ACTION::ABORT:
                filelist.clear();
                playing = false;
                break;
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

        if (current_image.id) {
            glBindTexture(GL_TEXTURE_2D, current_image.id);
            glUseProgram(shader_id);
            glBindVertexArray(vao);
            // probably exist a better solution to make the image fit the window
            float img_ratio[2] = { //
                (float)current_image.width / (float)window_width, (float)current_image.height / (float)window_height
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

        if (playing) time_left -= glfwGetTime() - start_time;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
