#version 450 core

layout(location = 0) in vec3 pos;

layout(std140, binding = 5) uniform shadowBuffer{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	vec3 cameraPos;
};

layout(std140, binding = 1) uniform terrainData{
	mat4 world;
};

void main(){
	gl_Position = viewProj * world * vec4(pos, 1.0);
}