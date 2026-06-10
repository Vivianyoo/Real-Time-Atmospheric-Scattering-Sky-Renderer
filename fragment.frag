#version 450 core

in vec4 position;
in vec2 uv;
in vec3 normal;
in vec4 tangent;
in vec2 shadowUv;
in float shadowDepth;
out vec4 fragColor;

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

uniform sampler2D mBaseColor;
uniform sampler2D mNormalMap;
uniform sampler2D mEmissiveMap;
uniform sampler2D mMetallicRoughnessMap;
uniform sampler2D mShadowMap;

float DistributionGGX(vec3 N, vec3 H, float roughness){
	float a = roughness;
	float a2 = a * a;
	float NdotH = dot(N, H);
	float NdotH2 = NdotH * NdotH;
	float denom = NdotH2 * (a2 - 1.0) + 1.0;

	return a2 / (3.14159265 * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness){
	float r = roughness + 1.0;
	float k = r * r / 8.0;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness){
	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);
	return ggx1 * ggx2;
}

vec3 FresneSchlick(float cosTheta, vec3 F0){
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5);
}

vec3 BRDF(vec3 N, vec3 V, vec3 L, float roughness, vec3 F0, vec3 albedo, float metallic){
	vec3 H = normalize(V + L);

	float HdotV = max(dot(H, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float NdotV = max(dot(N, V), 0.0);

	float D = DistributionGGX(N, H, roughness);
	float G = GeometrySmith(N, V, L, roughness);
	vec3 F = FresneSchlick(HdotV, F0);

	vec3 specular = D * G * F / max(4.0 * NdotL * NdotV, 0.01);
	vec3 diffuse = (1.0 - F) * (1.0 - metallic) * albedo / 3.14159265;

	return (specular + diffuse) * NdotL;
}

void main(){
	vec3 wNormal = normalize(normal);
	vec3 wTangent = normalize(tangent.xyz);
	vec3 bitangent = cross(wNormal, wTangent) * tangent.w;
	mat3 TBN = mat3(wTangent, bitangent, wNormal);
	vec3 deltaNorm = TBN * (2 * texture(mNormalMap, uv).xyz - 1);
	vec3 N = normalize(wNormal + deltaNorm);
	vec3 V = normalize(cameraPos - position.xyz);

	float ambientIntensity = 3.0;
	float emissiveIntensity = 1.0;
	vec3 lightMain = vec3(1.0, 0.4, 0.2);
	vec3 lightAmbient = vec3(0.2, 0.1, 0.1);
	vec3 emissive = texture(mEmissiveMap, uv).xyz;
	float metallic = texture(mMetallicRoughnessMap, uv).x;
	float roughness = texture(mMetallicRoughnessMap, uv).y;
	vec3 albedo = texture(mBaseColor, uv).xyz;
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	float notInShadow = step(texture(mShadowMap, shadowUv).x - 0.001, shadowDepth);

	vec3 brdfColor = notInShadow * BRDF(N, V, lightDir, roughness, F0, albedo, metallic);
	vec3 color = ambientIntensity * lightAmbient * albedo
           + lightIntensity * lightMain * brdfColor
           + emissiveIntensity * emissive;
	fragColor = vec4(color, 1.0);
}