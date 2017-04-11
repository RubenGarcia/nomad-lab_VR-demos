#version 330 core
layout (location = 0) in vec3 pos; 
layout (location = 1) in vec3 colin;
out vec3 col;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(pos, 1.0);
    col=colin;
}  
