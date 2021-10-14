#include <vector>
#include <filesystem>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#include <regex>

#include "session.hpp"
#include "shader.hpp"
#include "gui.hpp"

Gui::INPUT_ACTION Gui::input_dialog(UserInput& inputs)
{
    static std::vector<std::string> error_messages;
    Gui::INPUT_ACTION user_action = Gui::INPUT_ACTION::NO_ACTION;

    ImGui::Begin("files");
    ImGui::InputText("Images path", &inputs.image_folder_path);
    ImGui::InputInt("Timer (sec)", &inputs.timer);
    ImGui::InputText("Session path", &inputs.session_path);
    if (ImGui::Button("Begin New Session")) {
        error_messages.clear();
        user_action = Gui::INPUT_ACTION::NEW_SESSION;

        // image_folder_path errors
        if (inputs.image_folder_path.empty())
            error_messages.push_back("Images path :: please insert a path to a directory");
        else if (!std::filesystem::is_directory(inputs.image_folder_path))
            error_messages.push_back(inputs.image_folder_path + " not found or is not a directory");
        // timer errors
        if (inputs.timer < 1) error_messages.push_back("timer must be > 1 sec");
        // session_path errors
        if (inputs.session_path.empty())
            error_messages.push_back("Session path :: please insert a path to a directory");
        else if (!std::filesystem::is_directory(inputs.session_path))
            error_messages.push_back(inputs.session_path + " not found or is not a directory");

        if (error_messages.empty()) {
            for (const auto& file : std::filesystem::directory_iterator(inputs.image_folder_path))
                inputs.images.push_back(file.path().string());
        }
    }
    ImGui::SameLine();

    if (ImGui::Button("Save Preferences")) { user_action = Gui::INPUT_ACTION::SAVE_PREFERENCES; }

    ImGui::NewLine();

    static std::string session_file_path;
    ImGui::InputText("Session File", &session_file_path);
    if (ImGui::Button("Replay session")) {
        error_messages.clear();
        user_action = Gui::INPUT_ACTION::REPLAY_SESSION;
        if (session_file_path == "")
            error_messages.push_back("Replay Session :: please insert a path to a file");
        else if (!std::filesystem::exists(session_file_path))
            error_messages.push_back("Replay Session :: file does not exists");

        if (!Session::parse_file(session_file_path, inputs.timer, inputs.images))
            error_messages.push_back("Replay Session :: Session file not parsable");
        if (inputs.timer < 1) error_messages.push_back("Replay Session :: timer must be > 1 sec");
        if (inputs.images.empty()) error_messages.push_back("Replay Session :: Invalid number of images");
    }

    for (const auto& err_msg : error_messages)
        ImGui::Text(err_msg.c_str()); // TODO: red text
    ImGui::End();

    if (user_action == Gui::INPUT_ACTION::SAVE_PREFERENCES) return user_action; // kinda hacky
    return error_messages.empty() ? user_action : Gui::INPUT_ACTION::NO_ACTION;
}

Gui::CP_ACTION Gui::control_panel(const int time_left, const bool playing, bool& black_white, float& contrast, bool& negative)
{
    Gui::CP_ACTION action = Gui::CP_ACTION::NOOP;
    ImGui::Begin("Control panel");

    ImGui::Text(std::to_string(time_left).c_str()); // better way ???
    if (ImGui::Button("Previous")) action = Gui::CP_ACTION::PREVIOUS;
    if (!playing) {
        if (ImGui::Button("Play")) action = Gui::CP_ACTION::PLAY_PAUSE;
    } else {
        if (ImGui::Button("Pause")) action = Gui::CP_ACTION::PLAY_PAUSE;
    }
    if (ImGui::Button("Next")) action = Gui::CP_ACTION::NEXT;
    if (ImGui::Button("Close")) action = Gui::CP_ACTION::CLOSE;
    ImGui::Checkbox("Black & White", &black_white);
    ImGui::Checkbox("Negative", &negative);
    ImGui::SliderFloat("Contrast", &contrast, -1.0f, 1.0f);

    ImGui::End();
    return action;
}

bool Gui::init(GuiInformations& infos)
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    infos.window = glfwCreateWindow(infos.window_width, infos.window_height, "Image displayer", NULL, NULL);
    if (infos.window == NULL) {
        std::cout << "Failed to create window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(infos.window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    const char* glsl_version = "#version 130";
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(infos.window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glfwMakeContextCurrent(infos.window);
    glViewport(0, 0, infos.window_width, infos.window_height);

    float quad[] = {
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f, //
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, //
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, //
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f //
    };
    unsigned int indices[] = { 1, 2, 3, 3, 0, 1 };
    unsigned int vbo, ebo;

    glGenVertexArrays(1, &infos.vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(infos.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    infos.shader_id = create_shader_program();
    if (infos.shader_id == -1) std::cout << "Error while parsing/compiling shaders" << std::endl;

    infos.img_ratio_uniform = glGetUniformLocation(infos.shader_id, "img_ratio");
    infos.black_white_uniform = glGetUniformLocation(infos.shader_id, "black_white");
    infos.contrast_uniform = glGetUniformLocation(infos.shader_id, "contrast");
    infos.negative_uniform = glGetUniformLocation(infos.shader_id, "negative");
    return true;
}

void Gui::clean(GuiInformations& infos)
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(infos.window);
    glfwTerminate();
}

void Gui::begin_new_imgui_frame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void Gui::display_new_frame(GuiInformations& gui_infos, const UserInput& user_inputs, const ImagePlayer& image_player)
{
    // TODO: check integrity,

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(gui_infos.window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    gui_infos.window_width = display_w;
    gui_infos.window_height = display_h;
    glClearColor(0.10f, 0.10f, 0.10f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (image_player.current_texture.id) {
        glBindTexture(GL_TEXTURE_2D, image_player.current_texture.id);
        glUseProgram(gui_infos.shader_id);
        glBindVertexArray(gui_infos.vao);
        // probably exist a better solution to make the image fit the window
        float img_ratio[2] = { //
            (float)image_player.current_texture.width / (float)gui_infos.window_width,
            (float)image_player.current_texture.height / (float)gui_infos.window_height
        };
        if (img_ratio[0] < img_ratio[1]) {
            img_ratio[0] = img_ratio[0] / img_ratio[1];
            img_ratio[1] = 1;
        } else {
            img_ratio[1] = img_ratio[1] / img_ratio[0];
            img_ratio[0] = 1;
        }
        glUniform2fv(gui_infos.img_ratio_uniform, 1, img_ratio);
        glUniform1i(gui_infos.black_white_uniform, image_player.black_white);
        glUniform1f(gui_infos.contrast_uniform, image_player.contrast);
        glUniform1i(gui_infos.negative_uniform, image_player.negative);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}