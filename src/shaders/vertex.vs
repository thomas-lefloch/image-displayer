R"(
#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 text_cord;

uniform vec2 img_ratio;

out vec2 texture_cord;

void main() {
  gl_Position = vec4(pos.xy * img_ratio, pos.z, 1.0);
  texture_cord = text_cord;
}
)";