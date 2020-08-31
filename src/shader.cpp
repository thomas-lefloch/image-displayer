#include "shader.hpp"

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

std::string read_file(const char *path) {
  // ensure ifstream objects can throw exceptions:
  std::ifstream file;
  file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
  try {
    file.open(path);
    std::stringstream content;
    content << file.rdbuf();
    file.close();
    return content.str();
  } catch (std::ifstream::failure &e) {
    std::cout << "ERROR while reading " << path << "\n"
              << e.code() << " >> " << e.what() << std::endl;
    return "";
  }
}

void checkCompileErrors(GLuint shader, std::string type) {
  GLint success;
  GLchar infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                << infoLog << "\n -- ---------------\n"
                << std::endl;
    }
  } else {
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(shader, 1024, NULL, infoLog);
      std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                << infoLog << "\n -- ---------------\n"
                << std::endl;
    }
  }
}

int create_shader_program(const char *vertex_path, const char *fragment_path) {
  std::string v_code = read_file(vertex_path);
  if (v_code == "") return -1;
  std::cout << vertex_path << " SUCCESSFULLY READ" << std::endl;

  std::string f_code = read_file(fragment_path);
  if (f_code == "") return -1;
  std::cout << fragment_path << " SUCCESSFULLY READ" << std::endl;

  const char *v_shader_code = v_code.c_str();
  const char *f_shader_code = f_code.c_str();

  unsigned int vertex, fragment;
  // vertex shader
  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, &v_shader_code, NULL);
  glCompileShader(vertex);
  checkCompileErrors(vertex, "VERTEX");
  // fragment Shader
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, &f_shader_code, NULL);
  glCompileShader(fragment);
  checkCompileErrors(fragment, "FRAGMENT");

  // shader Program
  unsigned int program_id = glCreateProgram();
  glAttachShader(program_id, vertex);
  glAttachShader(program_id, fragment);
  glLinkProgram(program_id);
  checkCompileErrors(program_id, "PROGRAM");
  // delete the shaders as they're linked into our program now and no longer
  // necessery
  glDeleteShader(vertex);
  glDeleteShader(fragment);

  return program_id;
}
