#define _CRT_SECURE_NO_WARNINGS

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define PI 3.14159265358979323846

static float g_sunElevationDeg = 25.0f;
static float g_betaA = 0.000012f;

static bool g_stepDownPressed = false;
static bool g_stepUpPressed = false;
static bool g_resetPressed = false;

static double g_prevMouseX = 400.0;
static double g_prevMouseY = 400.0;
static bool g_mouseJustEntered = true;

static float g_yawDeg = 90.0f;
static float g_pitchDeg = 90.0f;
static float g_mouseSensitivity = 0.1f;

static float g_windowWidth = 800.0f;
static float g_windowHeight = 600.0f;

static glm::vec3 g_eye = glm::vec3(-6.0f, 3.5f, 1.0f);
static glm::vec3 g_forward = glm::vec3(1.0f, 0.0f, 0.0f);
static glm::vec3 g_up = glm::vec3(0.0f, 1.0f, 0.0f);

static std::vector<glm::vec3> g_cubeVerts;
static std::vector<uint32_t> g_cubeIndices;

static float g_currentFPS = 0.0f;

struct CameraBlock
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProj;
    alignas(16) glm::vec3 eye;
    alignas(4) float pad0;
};

struct LightingBlock
{
    alignas(16) glm::vec3 sunDir;
    alignas(4) float sunPower;
};

struct SkyTransformBlock
{
    alignas(16) glm::mat4 model;
};

struct AtmosphereBlock
{
    alignas(16) glm::vec3 sunColor;
    alignas(4) float thickness;
    alignas(4) float density;
    alignas(4) float sunSize;
    alignas(4) float steps;
    alignas(4) float intensity;
    alignas(4) float betaA;
    alignas(16) float pad1[3];
};

static CameraBlock g_cameraUBO{};
static LightingBlock g_lightUBO{};
static SkyTransformBlock g_skyUBO{};
static AtmosphereBlock g_atmosphereUBO{};

static void RefreshViewMatrix()
{
    g_cameraUBO.view = glm::lookAt(g_eye, g_eye + g_forward, glm::vec3(0.0f, 1.0f, 0.0f));
}

static void UpdateProjection(int width, int height)
{
    g_windowWidth = static_cast<float>(width);
    g_windowHeight = static_cast<float>(height);

    glViewport(0, 0, width, height);
    g_cameraUBO.proj = glm::perspective(
        glm::radians(60.0f),
        static_cast<float>(width) / static_cast<float>(height),
        0.1f,
        100.0f
    );
}

static void FramebufferSizeCallback(GLFWwindow* /*window*/, int width, int height)
{
    UpdateProjection(width, height);
}

static void UpdateCaption(GLFWwindow* wnd)
{
    std::ostringstream text;
    text << std::fixed << std::setprecision(3)
        << "RTR Project"
        << " | FPS: " << g_currentFPS
        << " | SunAngle: " << g_sunElevationDeg
        << " | Density: " << g_atmosphereUBO.density
        << " | Steps: " << g_atmosphereUBO.steps
        << " | Intensity: " << g_atmosphereUBO.intensity
        << " | SunSize: " << g_atmosphereUBO.sunSize
        << " | betaA: " << g_betaA;

    glfwSetWindowTitle(wnd, text.str().c_str());
}

static float ClampFloat(float v, float minV, float maxV)
{
    if (v < minV) return minV;
    if (v > maxV) return maxV;
    return v;
}

static void HandleKeyboard(GLFWwindow* wnd, float dt)
{
    const float moveSpeed = 5.0f * dt;

    glm::vec3 flatForward = glm::normalize(glm::vec3(g_forward.x, 0.0f, g_forward.z));
    glm::vec3 flatRight = glm::normalize(glm::cross(flatForward, glm::vec3(0.0f, 1.0f, 0.0f)));

    if (glfwGetKey(wnd, GLFW_KEY_W) == GLFW_PRESS)
    {
        g_eye += flatForward * moveSpeed;
    }
    if (glfwGetKey(wnd, GLFW_KEY_S) == GLFW_PRESS)
    {
        g_eye -= flatForward * moveSpeed;
    }
    if (glfwGetKey(wnd, GLFW_KEY_A) == GLFW_PRESS)
    {
        g_eye -= flatRight * moveSpeed;
    }
    if (glfwGetKey(wnd, GLFW_KEY_D) == GLFW_PRESS)
    {
        g_eye += flatRight * moveSpeed;
    }

    if (glfwGetKey(wnd, GLFW_KEY_UP) == GLFW_PRESS)
    {
        g_sunElevationDeg += 30.0f * dt;
    }
    if (glfwGetKey(wnd, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        g_sunElevationDeg -= 30.0f * dt;
    }

    g_sunElevationDeg = ClampFloat(g_sunElevationDeg, -5.0f, 89.0f);

    if (glfwGetKey(wnd, GLFW_KEY_1) == GLFW_PRESS)
    {
        g_atmosphereUBO.density -= 0.5f * dt;
    }
    if (glfwGetKey(wnd, GLFW_KEY_2) == GLFW_PRESS)
    {
        g_atmosphereUBO.density += 0.5f * dt;
    }

    g_atmosphereUBO.density = ClampFloat(g_atmosphereUBO.density, 0.05f, 5.0f);

    if (glfwGetKey(wnd, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (!g_stepDownPressed)
        {
            g_atmosphereUBO.steps -= 1.0f;
            if (g_atmosphereUBO.steps < 0.0f)
            {
                g_atmosphereUBO.steps = 0.0f;
            }
            g_stepDownPressed = true;
        }
    }
    else
    {
        g_stepDownPressed = false;
    }

    if (glfwGetKey(wnd, GLFW_KEY_4) == GLFW_PRESS)
    {
        if (!g_stepUpPressed)
        {
            g_atmosphereUBO.steps += 1.0f;
            if (g_atmosphereUBO.steps > 64.0f)
            {
                g_atmosphereUBO.steps = 64.0f;
            }
            g_stepUpPressed = true;
        }
    }
    else
    {
        g_stepUpPressed = false;
    }

    if (glfwGetKey(wnd, GLFW_KEY_7) == GLFW_PRESS)
    {
        g_atmosphereUBO.sunSize -= 0.01f * dt;
    }
    if (glfwGetKey(wnd, GLFW_KEY_8) == GLFW_PRESS)
    {
        g_atmosphereUBO.sunSize += 0.01f * dt;
    }

    g_atmosphereUBO.sunSize = ClampFloat(g_atmosphereUBO.sunSize, 0.001f, 0.05f);

    if (glfwGetKey(wnd, GLFW_KEY_9) == GLFW_PRESS)
    {
        g_betaA -= 0.00001f * dt;
    }
    if (glfwGetKey(wnd, GLFW_KEY_0) == GLFW_PRESS)
    {
        g_betaA += 0.00001f * dt;
    }

    g_betaA = ClampFloat(g_betaA, 0.0f, 0.0001f);

    if (glfwGetKey(wnd, GLFW_KEY_R) == GLFW_PRESS)
    {
        if (!g_resetPressed)
        {
            g_sunElevationDeg = 25.0f;
            g_atmosphereUBO.thickness = 80000.0f;
            g_atmosphereUBO.sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
            g_atmosphereUBO.density = 1.0f;
            g_atmosphereUBO.sunSize = 0.0093f;
            g_atmosphereUBO.steps = 24.0f;
            g_atmosphereUBO.intensity = 9.0f;
            g_betaA = 0.000012f;
            g_resetPressed = true;
        }
    }
    else
    {
        g_resetPressed = false;
    }
}

static void CursorCallback(GLFWwindow* /*window*/, double mouseX, double mouseY)
{
    if (g_mouseJustEntered)
    {
        g_prevMouseX = mouseX;
        g_prevMouseY = mouseY;
        g_mouseJustEntered = false;
    }

    float dx = static_cast<float>(mouseX - g_prevMouseX);
    float dy = static_cast<float>(mouseY - g_prevMouseY);

    g_prevMouseX = mouseX;
    g_prevMouseY = mouseY;

    dx *= g_mouseSensitivity;
    dy *= g_mouseSensitivity;

    g_yawDeg -= dx;
    g_pitchDeg += dy;

    if (g_pitchDeg > 179.0f)
    {
        g_pitchDeg = 179.0f;
    }
    else if (g_pitchDeg < 1.0f)
    {
        g_pitchDeg = 1.0f;
    }

    glm::vec3 dir;
    dir.x = sin(glm::radians(g_yawDeg)) * sin(glm::radians(g_pitchDeg));
    dir.y = cos(glm::radians(g_pitchDeg));
    dir.z = cos(glm::radians(g_yawDeg)) * sin(glm::radians(g_pitchDeg));

    g_forward = glm::normalize(dir);
    g_up = glm::vec3(0.0f, cos(glm::radians(g_pitchDeg)), 0.0f);
}

static std::string LoadTextFile(const std::string& path)
{
    std::ifstream in(path);
    if (!in.is_open())
    {
        std::cout << "Failed to open file: " << path << std::endl;
        return "";
    }

    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

static void ReportShaderStatus(GLuint shader, const std::string& name)
{
    GLint ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);

    if (!ok)
    {
        GLint length = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

        std::string log(length, '\0');
        glGetShaderInfoLog(shader, length, nullptr, log.data());

        std::cout << "Shader " << name << " Compile failed:\n" << log << std::endl;
    }
    else
    {
        std::cout << "Shader compile succeed." << std::endl;
    }
}

static void ReportProgramStatus(GLuint program, const std::string& name)
{
    GLint ok = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);

    if (!ok)
    {
        GLint length = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

        std::string log(length, '\0');
        glGetProgramInfoLog(program, length, nullptr, log.data());

        std::cout << "Program " << name << " link failed:\n" << log << std::endl;
    }
    else
    {
        std::cout << "Program link succeed." << std::endl;
    }
}

static void BuildSkyCube()
{
    g_cubeVerts.clear();
    g_cubeIndices.clear();

    for (int i = 0; i < 8; ++i)
    {
        g_cubeVerts.emplace_back(
            (i & 1) ? 1.0f : -1.0f,
            (i & 2) ? 1.0f : -1.0f,
            (i & 4) ? 1.0f : -1.0f
        );
    }

    const int quads[6][4] =
    {
        {0, 1, 3, 2},
        {5, 4, 6, 7},
        {1, 5, 7, 3},
        {4, 0, 2, 6},
        {4, 5, 1, 0},
        {6, 7, 3, 2}
    };

    for (int f = 0; f < 6; ++f)
    {
        const int i0 = quads[f][0];
        const int i1 = quads[f][1];
        const int i2 = quads[f][2];
        const int i3 = quads[f][3];

        g_cubeIndices.push_back(i0);
        g_cubeIndices.push_back(i1);
        g_cubeIndices.push_back(i2);

        g_cubeIndices.push_back(i0);
        g_cubeIndices.push_back(i2);
        g_cubeIndices.push_back(i3);
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Graphics Project - Pheonix", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD." << std::endl;
        glfwTerminate();
        return -1;
    }

    GLuint uboCamera = 0;
    GLuint uboLighting = 0;
    GLuint uboAtmosphere = 0;
    GLuint uboSkyTransform = 0;

    GLuint vaoSky = 0;
    GLuint vboSky = 0;
    GLuint eboSky = 0;

    glCreateVertexArrays(1, &vaoSky);
    BuildSkyCube();

    glCreateBuffers(1, &vboSky);
    glNamedBufferStorage(
        vboSky,
        g_cubeVerts.size() * sizeof(glm::vec3),
        g_cubeVerts.data(),
        0
    );

    glVertexArrayVertexBuffer(vaoSky, 0, vboSky, 0, sizeof(glm::vec3));
    glEnableVertexArrayAttrib(vaoSky, 0);
    glVertexArrayAttribFormat(vaoSky, 0, 3, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(vaoSky, 0, 0);

    glCreateBuffers(1, &eboSky);
    glNamedBufferStorage(
        eboSky,
        g_cubeIndices.size() * sizeof(uint32_t),
        g_cubeIndices.data(),
        0
    );
    glVertexArrayElementBuffer(vaoSky, eboSky);

    glCreateBuffers(1, &uboCamera);
    glNamedBufferStorage(uboCamera, sizeof(CameraBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboCamera);

    glCreateBuffers(1, &uboLighting);
    glNamedBufferStorage(uboLighting, sizeof(LightingBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, uboLighting);

    glCreateBuffers(1, &uboSkyTransform);
    glNamedBufferStorage(uboSkyTransform, sizeof(SkyTransformBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, uboSkyTransform);

    glCreateBuffers(1, &uboAtmosphere);
    glNamedBufferStorage(uboAtmosphere, sizeof(AtmosphereBlock), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, uboAtmosphere);

    GLuint programSky = glCreateProgram();

    {
        GLuint vs = glCreateShader(GL_VERTEX_SHADER);
        std::string vsCode = LoadTextFile("vertSky.vert");
        const char* vsPtr = vsCode.c_str();
        glShaderSource(vs, 1, &vsPtr, 0);
        glCompileShader(vs);
        ReportShaderStatus(vs, "vertSky.vert");
        glAttachShader(programSky, vs);
        glDeleteShader(vs);
    }

    {
        GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
        std::string fsCode = LoadTextFile("fragSky.frag");
        const char* fsPtr = fsCode.c_str();
        glShaderSource(fs, 1, &fsPtr, 0);
        glCompileShader(fs);
        ReportShaderStatus(fs, "fragSky.frag");
        glAttachShader(programSky, fs);
        glDeleteShader(fs);
    }

    glLinkProgram(programSky);
    ReportProgramStatus(programSky, "Skybox Program");

    UpdateProjection(800, 600);

    glfwSetCursorPosCallback(window, CursorCallback);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    g_atmosphereUBO.thickness = 80000.0f;
    g_atmosphereUBO.sunColor = glm::vec3(1.0f, 1.0f, 1.0f);
    g_atmosphereUBO.density = 1.0f;
    g_atmosphereUBO.sunSize = 0.0093f;
    g_atmosphereUBO.steps = 24.0f;
    g_atmosphereUBO.intensity = 9.0f;
    g_atmosphereUBO.betaA = g_betaA;
    glNamedBufferSubData(uboAtmosphere, 0, sizeof(AtmosphereBlock), &g_atmosphereUBO);

    float dt = 0.0f;
    float prevTime = 0.0f;

    float fpsTimer = 0.0f;
    int fpsFrameCount = 0;

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        float now = static_cast<float>(glfwGetTime());
        dt = now - prevTime;
        prevTime = now;

        fpsTimer += dt;
        fpsFrameCount++;

        if (fpsTimer >= 1.0f)
        {
            g_currentFPS = static_cast<float>(fpsFrameCount) / fpsTimer;
            std::cout << "FPS: " << g_currentFPS << std::endl;
            fpsTimer = 0.0f;
            fpsFrameCount = 0;
        }

        HandleKeyboard(window, dt);

        float solarFactor = sin(glm::radians(g_sunElevationDeg));
        if (solarFactor < 0.0f)
        {
            solarFactor = 0.0f;
        }

        g_atmosphereUBO.intensity = 1.5f + solarFactor * 8.5f;
        g_atmosphereUBO.betaA = g_betaA;
        glNamedBufferSubData(uboAtmosphere, 0, sizeof(AtmosphereBlock), &g_atmosphereUBO);

        float sunAngleRad = glm::radians(g_sunElevationDeg);
        g_lightUBO.sunDir = glm::normalize(glm::vec3(
            cos(sunAngleRad),
            sin(sunAngleRad),
            0.0f
        ));
        g_lightUBO.sunPower = 1.0f;
        glNamedBufferSubData(uboLighting, 0, sizeof(LightingBlock), &g_lightUBO);

        RefreshViewMatrix();
        g_cameraUBO.viewProj = g_cameraUBO.proj * g_cameraUBO.view;
        g_cameraUBO.eye = g_eye;
        glNamedBufferSubData(uboCamera, 0, sizeof(CameraBlock), &g_cameraUBO);

        g_skyUBO.model = glm::translate(glm::mat4(1.0f), g_eye);
        glNamedBufferSubData(uboSkyTransform, 0, sizeof(SkyTransformBlock), &g_skyUBO);

        UpdateCaption(window);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, static_cast<GLsizei>(g_windowWidth), static_cast<GLsizei>(g_windowHeight));

        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(vaoSky);
        glUseProgram(programSky);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(g_cubeIndices.size()), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}