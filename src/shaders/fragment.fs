R"(
#version 330 core

in vec2 texture_cord;
uniform sampler2D image;
uniform bool black_white;

out vec4 out_color;

void main() { 
  out_color = texture(image, texture_cord);
  if(black_white == true) {
    float grey = (out_color.r + out_color.g + out_color.b) / 3.0;
    out_color = vec4(grey, grey, grey, 1.0);
  }
}
)"