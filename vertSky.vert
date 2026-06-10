#version 450 core

layout(location = 0) in vec3 pos;
out vec3 position;
out vec3 sunDir;

layout(std140, binding = 0) uniform cameraData{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	vec3 cameraPos;
};

layout(std140, binding = 2) uniform renderData{
	vec3 lightDir;
	float lightIntensity;
};

layout(std140, binding = 3) uniform skyboxData{
	mat4 world;
};

void main(){
	position = pos;
	sunDir = lightDir;
	vec4 cachePos = proj * view * world * vec4(position, 1.0);
	gl_Position = cachePos.xyww;
}