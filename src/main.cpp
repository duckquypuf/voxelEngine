#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "voxelData.h"
#include "world.h"
#include "shader.h"
#include "camera.h"
#include "player.h"

class Engine
{
public:
    Shader* shader;
    World *world;
    Player* player;

    GLFWwindow* window;

    Engine()
    {
        initOpenGL();

        glFrontFace(GL_CCW);
        glCullFace(GL_BACK);
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        atlasID = loadTexture("../assets/textures/atlas_256x256.png");
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlasID);

        shader = new Shader("../src/vertex.glsl", "../src/fragment.glsl");
        shader->use();
        shader->setInt("atlasSampler", 0);

        world = new World();
        player = new Player(world, new Camera(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)), SURVIVAL);

        lastFrame = 0.0f;
        dt = 0.0f;

        int worldCentre = floor(worldWidth / 2);
        world->updateRenderDistance(ChunkCoord(worldCentre, worldCentre));
    }

    ~Engine()
    {
        cleanUp();
    }

    void run()
    {
        while (!glfwWindowShouldClose(window) && shouldRun)
        {
            glClearColor(0.6f, 0.95f, 1.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            calculateDeltaTime();

            processInput(window, dt);

            if(!paused)
            {
                player->processInput(window, dt);
                player->updatePhysics(dt);
                player->updateCoord();

                if(player->gamemode == CREATIVE || player->gamemode == SURVIVAL) player->checkBlock();

                if((!(player->lastCoord == player->coord) || player->checkRD()) && useRD) world->updateRenderDistance(player->coord);

                int chunksPerFrame = 2;
                for (int i = 0; i < chunksPerFrame && !world->chunksToGenerate.empty(); i++)
                {
                    ChunkCoord next = world->chunksToGenerate.front();
                    world->chunksToGenerate.pop();
                    world->chunksInQueue.erase(next);

                    auto it = world->chunks.find(next);
                    if (it != world->chunks.end() && it->second != nullptr && it->second->shouldRegen)
                    {
                        it->second->generateMesh();
                    }
                }
            }

            for (int x = player->coord.x - renderDistance; x < player->coord.x + renderDistance; x++)
            {
                for (int z = player->coord.z - renderDistance; z < player->coord.z + renderDistance; z++)
                {
                    ChunkCoord coord(x, z);
                    auto it = world->chunks.find(coord);
                    if (it != world->chunks.end() && it->second != nullptr)
                        it->second->renderChunk(shader, player->camera->GetViewMatrix(), player->camera->GetProjectionMatrix());
                }
            }

            player->lastCoord = player->coord;

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            drawCrosshair();
            drawUI();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
private:
    unsigned int VAO, VBO;
    unsigned int atlasID;

    float lastFrame;
    float dt;

    bool shouldRun = true;
    bool pauseClicked = false;
    bool paused = false;

    void cleanUp()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        delete shader;
        delete player;
        glDeleteBuffers(1, &VBO);
        glDeleteVertexArrays(1, &VAO);
        glfwTerminate();
    }

    void initOpenGL()
    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(1440, 900, "Minecraft", NULL, NULL);

        if (window == NULL)
        {
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 410");

        ImGui::StyleColorsDark();

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            glfwTerminate();
            return;
        }
    }

    void drawUI()
    {
        ImGui::Begin("Debug Info (F3)");

        ImGui::SliderInt("Render Distance: ", &renderDistance, 1, 20);

        ImGui::Text("FPS: %.0f", 1.0f / dt);
        ImGui::Text("Delta Time: %.4f ms", dt * 1000.0f);

        ImGui::Separator();

        glm::vec3 pos = player->camera->pos;
        ImGui::Text("Position: X:%.2f Y:%.2f Z:%.2f", pos.x, pos.y, pos.z);

        ImGui::Text("Chunk Coords: X:%d Z:%d", player->coord.x, player->coord.z);

        ImGui::Separator();

        const char *gamemode_str;
        switch (player->gamemode)
        {
        case SPECTATOR:
            gamemode_str = "SPECTATOR";
            break;
        case CREATIVE:
            gamemode_str = "CREATIVE";
            break;
        case SURVIVAL:
            gamemode_str = "SURVIVAL";
            break;
        default:
            gamemode_str = "UNKNOWN";
        }
        ImGui::Text("Gamemode: %s", gamemode_str);

        ImGui::Text("Chunks Loaded: %zu", world->chunks.size());
        ImGui::Text("Chunks Queued: %zu", world->chunksToGenerate.size());

        ImGui::End();

        ImGui::Begin("Lodes", NULL, ImGuiWindowFlags_AlwaysVerticalScrollbar);

        for (int i = 0; i < lodeCount; i++)
        {
            Lode& lode = lodes[i];

            if (ImGui::CollapsingHeader(lode.name.c_str()))
            {
                int minH = lode.minHeight;
                int maxH = lode.maxHeight;
                float offs = lode.offset;
                float sc = lode.scale;
                float thresh = lode.threshold;

                ImGui::SliderInt("Min Height", &minH, 1, chunkHeight-1);
                ImGui::SliderInt("Max Height", &maxH, 1, chunkHeight-1);
                ImGui::InputFloat("Noise Offset", &offs, 1.0f, 100000.0f);
                ImGui::InputFloat("Noise Scale", &sc, 0.001f, 1.0f);
                ImGui::InputFloat("Threshold", &thresh, 0.0f, 1.0f);

                if (ImGui::IsItemDeactivatedAfterEdit())
                {
                    lode.minHeight = (uint8_t)minH;
                    lode.maxHeight = (uint8_t)maxH;
                    lode.offset = offs;
                    lode.scale = sc;
                    lode.threshold = thresh;

                    world->updateRenderDistance(player->coord, true);
                }
            }
        }

        ImGui::End();
    }

    void drawCrosshair()
    {
        int w, h;
        glfwGetWindowSize(window, &w, &h);

        float cx = w * 0.5f;
        float cy = h * 0.5f;

        auto dl = ImGui::GetForegroundDrawList();

        float t = 2.0f;    // thickness
        float len = 10.0f; // length

        // vertical
        dl->AddLine(
            ImVec2(cx, cy - len),
            ImVec2(cx, cy + len),
            IM_COL32(255, 255, 255, 255),
            t);

        // horizontal
        dl->AddLine(
            ImVec2(cx - len, cy),
            ImVec2(cx + len, cy),
            IM_COL32(255, 255, 255, 255),
            t);
    }

    void calculateDeltaTime()
    {
        float currentFrame = glfwGetTime();
        dt = currentFrame - lastFrame;
        lastFrame = currentFrame;
    }

    unsigned int loadTexture(const char *path)
    {
        if (!std::filesystem::exists(path))
        {
            std::cout << "ERROR: File does not exist at path: " << path << std::endl;
            return 0;
        }

        unsigned int id;
        glGenTextures(1, &id);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);

        if (data)
        {
            std::cout << "Texture loaded successfully: " << width << "x" << height << " with " << nrComponents << " components" << std::endl;
            GLenum format;

            if (nrComponents == 1) format = GL_RED;
            else if (nrComponents == 3) format = GL_RGB;
            else if (nrComponents == 4) format = GL_RGBA;

            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "STB Image failed to load. Reason: " << stbi_failure_reason() << std::endl;
            std::cout << "Path attempted: " << path << std::endl;
            stbi_image_free(data);
            return 0;
        }
        return id;
    }

    void processInput(GLFWwindow *window, float dt)
    {
        if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) shouldRun = false;

        if(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) 
        {
            if(!pauseClicked)
                paused = !paused;
            pauseClicked = true;
        } else
        {
            pauseClicked = false;
        }

        glfwSetInputMode(window, GLFW_CURSOR, paused ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
    }
};

int main()
{
    Engine engine = Engine();
    engine.run();

    return 0;
}
