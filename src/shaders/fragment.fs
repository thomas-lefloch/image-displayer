R"(
#version 330 core

in vec2 texture_cord;
uniform sampler2D image;
uniform bool black_white;
uniform float contrast;
uniform bool negative;

out vec4 out_color;

void main() { 
  vec3 color = texture(image, texture_cord).rgb;
  if(black_white == true) {
    float grey = (color.r + color.g + color.b) / 3.0;
    color = vec3(grey, grey, grey);
  }
  if(negative) {
    color = 1 - color;
  }
  float contrast_factor = (1+contrast) / (1-contrast);
  vec3 contrasted_color = contrast_factor*(color - vec3(0.5)) + vec3(0.5);
  // vec3 contrasted_color = contrast*(color - vec3(0.5)) + vec3(0.5);
 
  out_color = vec4(contrasted_color, 1.0);
}
)"