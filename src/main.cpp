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

#include "shader.hpp"

constexpr int window_width = 1600;
constexpr int window_height = 900;

ImVec4 clear_color = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

struct Texture {
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

// Simple helper function to load an image into a OpenGL texture with common settings
bool load_texture(const char* filename, Texture* out_texture)
{
    // Load from file
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL) return false;

    // Create a OpenGL texture identifier
    GLuint image_texture;
    glGenTextures(1, &image_texture);
    glBindTexture(GL_TEXTURE_2D, image_texture);

    // Setup filtering parameters for display
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Upload pixels into texture
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
    stbi_image_free(image_data);

    out_texture->id = image_texture;
    out_texture->width = image_width;
    out_texture->height = image_height;

    return true;
}

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

    Texture current_texture, play_texture, pause_texture, prev_texture, next_texture, close_texture;
    load_texture(ROOT_DIR "res/play.png", &play_texture);
    load_texture(ROOT_DIR "res/pause.png", &pause_texture);
    load_texture(ROOT_DIR "res/prev.png", &prev_texture);
    load_texture(ROOT_DIR "res/next.png", &next_texture);
    load_texture(ROOT_DIR "res/close.png", &close_texture);

    std::vector<std::string> filelist;
    bool playing = false;
    static std::string input_path;
    static int base_timer = 30;
    double clock = base_timer;

    while (!glfwWindowShouldClose(window)) {

        // TODO: refactor timer management
        double start_time = glfwGetTime();
        if (playing && clock < 0) {
            // change image
            // take one random from filelist and load it, display, delete it from the list
            std::cout << "timer" << std::endl;
            clock = base_timer;
        }

        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (filelist.empty()) {
            static std::vector<std::string> error_messages;
            ImGui::Begin("files");

            // TODO: add multiple folder input
            // TODO: make enter key press "Ok" button
            ImGui::InputText("Folder path", &input_path);
            // TODO: minimum 1
            ImGui::InputInt("Timer (sec)", &base_timer);
            if (ImGui::Button("Ok")) {
                error_messages.clear();
                filelist.clear();
                bool has_error = false;
                if (!std::filesystem::exists(input_path)) {
                    error_messages.push_back(input_path + " not found");
                    has_error = true;
                }
                if (base_timer < 1) {
                    error_messages.push_back("timer must be > 1 sec");
                    has_error = true;
                }
                if (!has_error) {
                    clock = base_timer;
                    for (const auto& file : std::filesystem::directory_iterator(input_path))
                        filelist.push_back(file.path().string());
                }
            }

            for (const auto& filepath : filelist) {
                if (ImGui::Button(filepath.c_str())) {
                    // TODO: memory leak ???
                    glDeleteTextures(1, &current_texture.id);
                    // TODO: check for images
                    // -> black texture if trying to display something other than an image
                    // = display palceholder image (image not recognized, please open something
                    // else)
                    load_texture(filepath.c_str(), &current_texture);
                }
            }
            if (current_texture.width && current_texture.height) {
                ImGui::Text("%d, %d", current_texture.width, current_texture.height);
            }

            for (const auto& err_msg : error_messages) {
                // TODO: red text
                ImGui::Text(err_msg.c_str());
            }
            ImGui::End();
        } else {
            ImGui::Begin("Control panel");
            ImGui::Text(std::to_string((int)clock).c_str()); // better way ???
            ImGui::SameLine();
            // TODO: refactor. (with lambda ??)
            if (ImGui::ImageButton((ImTextureID)prev_texture.id, ImVec2(prev_texture.width, prev_texture.height))) {
                // previous image
            }
            ImGui::SameLine();
            if (!playing) {
                if (ImGui::ImageButton((ImTextureID)play_texture.id, ImVec2(play_texture.width, play_texture.height))) {
                    playing = true;
                }
            } else {
                if (ImGui::ImageButton(
                        (ImTextureID)pause_texture.id, ImVec2(pause_texture.width, pause_texture.height))) {
                    playing = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::ImageButton((ImTextureID)next_texture.id, ImVec2(next_texture.width, next_texture.height))) {
                // next image
            }
            ImGui::SameLine();
            if (ImGui::ImageButton((ImTextureID)close_texture.id, ImVec2(close_texture.width, close_texture.height))) {
                // back to selecting folders
                filelist.clear();
            }

            ImGui::End();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        if (current_texture.id) {
            glBindTexture(GL_TEXTURE_2D, current_texture.id);
            glUseProgram(shader_id);
            glBindVertexArray(vao);
            // probably exist a better solution to make the image fit the window
            float img_ratio[2] = { //
                (float)current_texture.width / (float)window_width, (float)current_texture.height / (float)window_height
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
        // TODO: refactor timer management
        if (playing) clock -= glfwGetTime() - start_time;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
