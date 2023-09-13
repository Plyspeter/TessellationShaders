#version 410 core
// This is a very simple vertex vector to show effects without tessellation.
layout (location = 0) in vec3 pos;
layout (location = 1) in vec4 color;
layout (location = 2) in vec3 normal;

uniform mat4 viewProjection;
uniform mat4 model;

out vec4 color_FRAG;
out vec3 normal_FRAG;

void main()
{
   color_FRAG = color;
   normal_FRAG = normal;
   gl_Position = viewProjection * model * vec4(pos, 1.0f);
}