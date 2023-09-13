#version 410 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;

uniform mat4 model;

out vec4 color_TCS;
out vec3 normal_TCS;
out vec3 pos_TCS;

void main()
{
   color_TCS = color;
   normal_TCS = normal;
   pos_TCS = (model * vec4(pos, 1.0f)).xyz;
   gl_Position = vec4(pos, 1.0f);
}