#version 330 core
layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vColor;
out vec3 passColor;

uniform mat4 ModelTransform;
uniform mat4 ViewTransform;
uniform mat4 ProjectionTransform;
void main ()
{
gl_Position = ProjectionTransform * ViewTransform * ModelTransform * vec4(vPos,1.0);
passColor = vColor;
}