#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
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
glm::vec3 lightColor1(1.0f, 1.0f, 1.0f);
glm::vec3 lightColor2(1.0f, 1.0f, 1.0f);

// Framebuffer
GLuint framebuffer, textureColorbuffer, rbo;
bool mouseInViewport = false;
bool rightMousePressed = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

void processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
    if (mouseInViewport && rightMousePressed)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.processKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.processKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.processKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.processKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            camera.processKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            camera.processKeyboard(DOWN, deltaTime);

        // Handle mouse input
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        float xoffset = xpos - lastX;
        float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

        lastX = xpos;
        lastY = ypos;

        camera.processMouseMovement(xoffset, yoffset);
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void renderToFramebuffer(Shader& shader, Model& model, GLuint cubemapTexture, int framebufferWidth, int framebufferHeight)
{
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f); // Light blue background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    glm::mat4 viewMat = camera.getViewMatrix();
    shader.setMat4("view", viewMat);
    shader.setVec3("viewPos", camera.position);

    Light light1 = { glm::vec3(1.2f, 1.0f, 2.0f), lightColor1 };
    Light light2 = { glm::vec3(-1.2f, -1.0f, -2.0f), lightColor2 };
    shader.setLight("light1", light1);
    shader.setLight("light2", light2);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    model.draw(shader.ID);

    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Unbind framebuffer
}

void renderImGui(GLFWwindow* window, Shader& shader, Model& model, GLuint cubemapTexture, int& framebufferWidth, int& framebufferHeight, float deltaTime)
{
    // Start the ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Create docking space
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();

    // Light Control Tab
    ImGui::Begin("Light Control");
    ImGui::ColorEdit3("Light 1 Color", glm::value_ptr(lightColor1));
    ImGui::ColorEdit3("Light 2 Color", glm::value_ptr(lightColor2));
    ImGui::End();

    // 3D Viewport Tab
    ImGui::Begin("3D Viewport");

    // Get the position of the cursor at the start of the viewport
    ImVec2 viewportPos = ImGui::GetCursorScreenPos();
    // Get the size of the available space in the viewport
    ImVec2 viewportSize = ImGui::GetContentRegionAvail();

    if (framebufferWidth != (int)viewportSize.x || framebufferHeight != (int)viewportSize.y)
    {
        framebufferWidth = (int)viewportSize.x;
        framebufferHeight = (int)viewportSize.y;

        // Resize framebuffer attachments
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, framebufferWidth, framebufferHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, framebufferWidth, framebufferHeight);
    }

    // Render to framebuffer with the new size
    renderToFramebuffer(shader, model, cubemapTexture, framebufferWidth, framebufferHeight);

    // Display the framebuffer texture in the ImGui window
    ImGui::Image((void*)(intptr_t)textureColorbuffer, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

    // Check if the mouse is in the viewport
    mouseInViewport = ImGui::IsItemHovered();

    // Handle right mouse button behavior
    if (mouseInViewport && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        rightMousePressed = true;
        firstMouse = true;  // Reset the initial mouse state
        // Set the initial mouse position to where the right-click happened
        ImVec2 mousePos = ImGui::GetMousePos();
        lastX = mousePos.x;
        lastY = mousePos.y;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if (rightMousePressed && ImGui::IsMouseReleased(ImGuiMouseButton_Right))
    {
        rightMousePressed = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    ImGui::End();

    // Process input
    processInput(window, camera, deltaTime);

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable; // Enable Docking

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    Light light1 = { glm::vec3(1.2f, 1.0f, 2.0f), lightColor1 };
    Light light2 = { glm::vec3(-1.2f, -1.0f, -2.0f), lightColor2 };

    // Create framebuffer for offscreen rendering
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Create a renderbuffer object for depth and stencil attachment
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);

    glm::mat4 modelMat = glm::mat4(1.0f);
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

    int framebufferWidth = 800, framebufferHeight = 600;

    while (!glfwWindowShouldClose(window))
    {
        float deltaTime = 0.01f; // Adjust as needed

        glfwPollEvents();

        // Start the ImGui frame and render everything
        renderImGui(window, shader, model, cubemapTexture, framebufferWidth, framebufferHeight, deltaTime);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
