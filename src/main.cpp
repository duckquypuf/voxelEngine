#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader.h"
#include "player.h"
#include "voxelData.h"
#include "settings.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void processInput(GLFWwindow *window);

GLFWwindow *setupWindow(const char *title);
unsigned int setupMesh();
unsigned int loadTexture(const char* filePath);

World world = World();

Player player = Player(&world);

float deltaTime, lastFrame = 0.0f;
bool firstMouse = true;
bool mouseLocked = true;
static bool lKeyPressedLastFrame = false;
float lastX, lastY;

int main()
{
    GLFWwindow *window = setupWindow("Voxel Engine");
    Shader shader = Shader("../src/1.shader.vs", "../src/1.shader.fs");

    lastX = SCR_WIDTH / 2;
    lastY = SCR_HEIGHT / 2;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    stbi_set_flip_vertically_on_load(true);

    unsigned int VAO = setupMesh();

    BlockRegistry::initialize();
    unsigned int grassTopTex = loadTexture("../assets/textures/grass_top.png");
    unsigned int grassSideTex = loadTexture("../assets/textures/grass_side.png");
    unsigned int dirtTex = loadTexture("../assets/textures/dirt.png");
    unsigned int stoneTex = loadTexture("../assets/textures/stone.png");
    BlockRegistry::setBlockTexture(DIRT, dirtTex);
    BlockRegistry::setBlockTexture(STONE, stoneTex);
    BlockRegistry::setBlockTextures(GRASS, grassSideTex, grassTopTex, dirtTex);

    std::vector<unsigned int> textureIDs = {stoneTex, grassTopTex, grassSideTex, dirtTex};

    shader.use();

    if(ENABLE_WIREFRAME)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else{
        glEnable(GL_CULL_FACE);
        glFrontFace(GL_CW);
        glCullFace(GL_BACK);
    }

    for (unsigned int cx = 0; cx < WORLD_WIDTH; cx++) {
        for (unsigned int cz = 0; cz < WORLD_WIDTH; cz++) {
            world.chunks[cx][cz].generateMesh(world, cx, cz);
        }
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        player.updateVelocity(window, deltaTime);

        shader.setMat4("view", player.camera.getViewMatrix());
        shader.setMat4("projection", glm::perspective(glm::radians(player.camera.fov), SCR_WIDTH / SCR_HEIGHT, 0.01f, 1000.0f));
        shader.setInt("textureSampler", 0);

        shader.use();
        for (int i = 0; i < 4; i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
            shader.setInt(("textures[" + std::to_string(i) + "]").c_str(), i);
        }

        for (unsigned int cx = 0; cx < WORLD_WIDTH; cx++) {
            for (unsigned int cz = 0; cz < WORLD_WIDTH; cz++) {
                world.chunks[cx][cz].renderMesh(shader, cx, cz);
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello GUI");
        ImGui::Text("Player pos: %.2f %.2f %.2f", player.pos.x, player.pos.y, player.pos.z);
        ImGui::SliderFloat("FOV", &player.camera.fov, 30.0f, 120.0f);
        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (!lKeyPressedLastFrame) {
            mouseLocked = !mouseLocked;
            glfwSetInputMode(window, GLFW_CURSOR, mouseLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        }

        lKeyPressedLastFrame = true;
    }
    else
        lKeyPressedLastFrame = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    player.camera.processMouseMovement(xoffset, yoffset);
}

GLFWwindow* setupWindow(const char* title) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, title, NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialise GLAD" << std::endl;
        return nullptr;
    }

    return window;
}

unsigned int setupMesh() {
    float vertices[] = {
        // Front face (z = -0.5) - CCW from outside
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        // Back face (z = 0.5) - CCW from outside (REVERSED)
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        // Left face (x = -0.5) - CCW from outside
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        // Right face (x = 0.5) - CCW from outside
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

        // Bottom face (y = -0.5) - CCW from outside
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

        // Top face (y = 0.5) - CCW from outside
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f};

    unsigned int VBO, VAO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    return VAO;
}

unsigned int loadTexture(const char* filePath) {
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;

    unsigned char *data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "Texture loaded: " << filePath << " - " << width << "x" << height
                  << " channels: " << nrChannels << std::endl;

        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "FAILED TO LOAD TEXTURE: " << filePath << std::endl;
    }
    stbi_image_free(data);

    return texture;
}
