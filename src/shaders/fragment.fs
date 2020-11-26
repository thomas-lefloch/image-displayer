R"(
#version 330 core

in vec2 texture_cord;
uniform sampler2D image;

out vec4 out_color;

void main() { out_color = texture(image, texture_cord); }
)"