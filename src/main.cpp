#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"

// Camera settings
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
float lastX = 400, lastY = 300;
bool firstMouse = true;

// Light colors
glm::vec3 lightColor1(5.0f, 5.0f, 5.0f);
glm::vec3 lightColor2(5.0f, 5.0f, 5.0f);

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void processInput(GLFWwindow* window)
{
    const float deltaTime = 0.01f; // Adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(0, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(1, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(2, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(3, deltaTime);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), (float)width / height, 0.1f, 100.0f);
    GLuint shaderProgram = *((GLuint*)glfwGetWindowUserPointer(window));
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMat));
}

void initializeImGui(GLFWwindow* window)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;      // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;    // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void renderImGui()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Example ImGui window
    ImGui::Begin("Settings");

    if (ImGui::CollapsingHeader("Light Colors"))
    {
        ImGui::ColorEdit3("Light 1 Color", (float*)&lightColor1);
        ImGui::ColorEdit3("Light 2 Color", (float*)&lightColor2);
    }

    ImGui::End();

    // Rendering
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}

int main()
{
    std::filesystem::current_path("../");
    std::cout << "Current working directory: " << std::filesystem::current_path() << std::endl;

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwSetErrorCallback([](int error, const char* description)
    {
        std::cerr << "Error: " << description << std::endl;
    });

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Raytraced Renderer", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Initialize ImGui
    initializeImGui(window);

    Shader shader("shaders/raytrace.vert", "shaders/raytrace.frag");
    GLuint shaderProgram = shader.ID;
    glfwSetWindowUserPointer(window, &shaderProgram);

    Model model("DamagedHelmet.glb");
    std::vector<std::string> faces = {
        "textures/cubemap/right.jpg",
        "textures/cubemap/left.jpg",
        "textures/cubemap/top.jpg",
        "textures/cubemap/bottom.jpg",
        "textures/cubemap/front.jpg",
        "textures/cubemap/back.jpg"
    };
    GLuint cubemapTexture = loadCubeMap(faces);

    Light light1 = {glm::vec3(1.2f, 1.0f, 2.0f), lightColor1};
    Light light2 = {glm::vec3(-1.2f, -1.0f, -2.0f), lightColor2};

    glEnable(GL_DEPTH_TEST);
    auto modelMat = glm::mat4(1.0f);
    glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    shader.use();
    shader.setMat4("model", modelMat);
    shader.setMat4("projection", projectionMat);
    shader.setLight("light1", light1);
    shader.setLight("light2", light2);
    shader.setInt("skybox", 1);

    bool diffuseTextureBound = false;
    bool normalTextureBound = false;
    for (const auto& texture : model.model.textures)
    {
        if (texture.source >= 0)
        {
            const tinygltf::Image& image = model.model.images[texture.source];
            if (!diffuseTextureBound)
            {
                GLuint textureID = createTexture(image);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, textureID);
                shader.setInt("texture_diffuse", 0);
                diffuseTextureBound = true;
            }
            if (!normalTextureBound)
            {
                GLuint textureID = createTexture(image);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, textureID);
                shader.setInt("texture_normal", 1);
                normalTextureBound = true;
            }
        }
    }

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glm::mat4 viewMat = camera.getViewMatrix();
        shader.use();
        shader.setMat4("view", viewMat);
        shader.setVec3("viewPos", camera.position);

        // Update light colors from ImGui
        light1.color = lightColor1;
        light2.color = lightColor2;
        shader.setLight("light1", light1);
        shader.setLight("light2", light2);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        model.draw(shaderProgram);

        // Render ImGui
        renderImGui();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Shutdown ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
