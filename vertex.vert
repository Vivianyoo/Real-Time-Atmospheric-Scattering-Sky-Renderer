#version 450 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex;
layout(location = 2) in vec3 norm;
layout(location = 3) in vec4 tang;
out vec4 position;
out vec2 uv;
out vec3 normal;
out vec4 tangent;
out vec2 shadowUv;
out float shadowDepth;

layout(std140, binding = 0) uniform cameraData{
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	vec3 cameraPos;
};

layout(std140, binding = 1) uniform terrainData{
	mat4 world;
};

layout(std140, binding = 5) uniform shadowBuffer{
	mat4 sView;
	mat4 sProj;
	mat4 sViewProj;
	vec3 sCameraPos;
};

void main(){
	mat3 normalMatrix = inverse(transpose(mat3(world)));
    position = world * vec4(pos, 1.0);
	gl_Position = viewProj * position;
	uv = tex;
	normal = normalMatrix * norm;
	tangent = vec4(normalMatrix * tang.xyz, tang.w);
	vec4 shadowPos = sViewProj * world * vec4(pos, 1.0);
	shadowUv = 0.5 * shadowPos.xy / shadowPos.w + 0.5;
	shadowDepth = 0.5 * shadowPos.z / shadowPos.w + 0.5;
}