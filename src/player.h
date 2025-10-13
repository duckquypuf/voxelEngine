#pragma once
#include "camera.h"
#include "voxelData.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

bool ENABLE_GRAVITY = true;
bool ENABLE_COLLISIONS = true;

enum Gamemode {
    SURVIVAL,
    CREATIVE,
    SPECTATOR
};

class Player{
    public:
        glm::vec3 feet = glm::vec3(0.0f, 18.2f, 0.0f);
        Camera camera = Camera(glm::vec3(0.0f, 20.0f, 0.0f), 90.0f);
        World* world;

        Gamemode gamemode = SPECTATOR;

        Player(World* _world = nullptr) {
            world = _world;

            if (gamemode == SURVIVAL) {
                ENABLE_GRAVITY = true;
                ENABLE_COLLISIONS = true;
            } else if(gamemode == CREATIVE) {
                ENABLE_GRAVITY = false;
                ENABLE_COLLISIONS = true;
            } else if(gamemode == SPECTATOR) {
                ENABLE_GRAVITY = false;
                ENABLE_COLLISIONS = false;
            }
        }

        void updateVelocity(GLFWwindow* window, float deltaTime) {
            glm::vec3 prevFeet = feet;
            glm::vec3 movement = processKeyboardInputs(window, deltaTime);

            if(ENABLE_GRAVITY)
                movement.y -= 9.81f * deltaTime;

            std::cout << "Pos: (" << feet.x << ", " << feet.y << ", " << feet.z << ")\n";

            feet.y = prevFeet.y + movement.y;
            if (world->isPlayerColliding(feet.x, feet.y, feet.z, 0.5f, 1.8f) && ENABLE_COLLISIONS)
                feet.y = prevFeet.y;

            feet.x = prevFeet.x + movement.x;
            if (world->isPlayerColliding(feet.x, feet.y, feet.z, 0.5f, 1.8f) && ENABLE_COLLISIONS)
                feet.x = prevFeet.x;

            feet.z = prevFeet.z + movement.z;
            if (world->isPlayerColliding(feet.x, feet.y, feet.z, 0.5f, 1.8f) && ENABLE_COLLISIONS)
                feet.z = prevFeet.z;

            camera.setPosition(feet + glm::vec3(0.0f, 1.8f, 0.0f));
        }

        glm::vec3 processKeyboardInputs(GLFWwindow* window, float deltaTime) {
            float cameraSpeed = camera.movementSpeed * deltaTime;

            glm::vec3 frontXZ = glm::normalize(glm::vec3(camera.cameraFront.x, 0.0f, camera.cameraFront.z));
            glm::vec3 rightXZ = glm::normalize(glm::vec3(camera.cameraRight.x, 0.0f, camera.cameraRight.z));

            glm::vec3 movement = glm::vec3(0.0f, 0.0f, 0.0f);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                movement += cameraSpeed * frontXZ;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                movement -= cameraSpeed * frontXZ;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                movement -= cameraSpeed * rightXZ;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                movement += cameraSpeed * rightXZ;
            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                movement += camera.worldUp * cameraSpeed * 2.0f;
            if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                movement -= camera.worldUp * cameraSpeed;

            return movement;
        }
};
