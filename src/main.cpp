#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <filesystem>
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Texture.h"
#include "Light.h"

// Camera settings
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
float lastX = 400, lastY = 300;
bool firstMouse = true;

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

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

    Light light1 = {glm::vec3(1.2f, 1.0f, 2.0f), glm::vec3(5.0f, 5.0f, 5.0f)};
    Light light2 = {glm::vec3(-1.2f, -1.0f, -2.0f), glm::vec3(5.0f, 5.0f, 5.0f)};

    glEnable(GL_DEPTH_TEST);
    auto modelMat = glm::mat4(1.0f);
    glm::mat4 projectionMat = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    shader.use();
    shader.setMat4("model", modelMat);
    shader.setMat4("projection", projectionMat);
    shader.setVec3("lightPos1", light1.position);
    shader.setVec3("lightColor1", light1.color);
    shader.setVec3("lightPos2", light2.position);
    shader.setVec3("lightColor2", light2.color);
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        model.draw(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
