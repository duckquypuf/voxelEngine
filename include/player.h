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
        glm::vec3 feet;
        glm::vec3 pos;
        Camera camera = Camera(glm::vec3(0.0f, 0.0f, 0.0f), 90.0f);
        World* world;

        Gamemode gamemode = SURVIVAL;

        float movementSpeed = 8.0f;

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

            feet = glm::vec3(worldCentreOffset, CHUNK_HEIGHT+5.0f, worldCentreOffset);
        }

        void updateVelocity(GLFWwindow* window, float deltaTime) {
            glm::vec3 prevFeet = feet;
            glm::vec3 dir = processKeyboardInputs(window, deltaTime);

            glm::vec3 diff = dir * movementSpeed - glm::vec3(velocity.x, 0.0f, velocity.z);
            velocity.x += diff.x * deltaTime;
            velocity.z += diff.z * deltaTime;

            if(ENABLE_GRAVITY && !isGrounded)
                velocity.y -= 30.0f * deltaTime;

            if (isGrounded)
                coyoteTimer = coyoteTimeMax;
            else
                coyoteTimer -= deltaTime;

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && coyoteTimer > 0.0f && gamemode == SURVIVAL) {
                velocity.y = jumpStrength;
                coyoteTimer = 0.0f;
            }

            if(gamemode != SURVIVAL) {
                velocity.y = dir.y;
            }

            feet += velocity * deltaTime;

            if (world->isPlayerColliding(feet.x, feet.y, feet.z, 0.5f, 1.8f) && ENABLE_COLLISIONS) {
                isGrounded = true;
                velocity.y = 0;
                feet.y = prevFeet.y;
            } else {
                isGrounded = false;
            }

            if (world->isPlayerColliding(feet.x, feet.y, feet.z, 0.5f, 1.8f) && ENABLE_COLLISIONS) {
                velocity.x = 0;
                feet.x = prevFeet.x;
            }

            if (world->isPlayerColliding(feet.x, feet.y, feet.z, 0.5f, 1.8f) && ENABLE_COLLISIONS) {
                velocity.z = 0;
                feet.z = prevFeet.z;
            }

            velocity.x *= (1.0f - decel * deltaTime);
            velocity.z *= (1.0f - decel * deltaTime);

            camera.setPosition(feet + glm::vec3(0.0f, 1.8f, 0.0f));

            pos = glm::vec3(feet.x-worldCentreOffset, feet.y, feet.z-worldCentreOffset);
            std::cout << "Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")\n";
            std::cout << "Grounded = " << isGrounded << "\n";
        }

        glm::vec3 processKeyboardInputs(GLFWwindow* window, float deltaTime) {
            float speed = movementSpeed;

            glm::vec3 frontXZ = glm::normalize(glm::vec3(camera.cameraFront.x, 0.0f, camera.cameraFront.z));
            glm::vec3 rightXZ = glm::normalize(glm::vec3(camera.cameraRight.x, 0.0f, camera.cameraRight.z));

            glm::vec3 dir = glm::vec3(0.0f);

            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                dir += frontXZ;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                dir -= frontXZ;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                dir -= rightXZ;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                dir += rightXZ;

            if (dir != glm::vec3(0.0f))
                dir = glm::normalize(dir) * speed;

            if (gamemode != SURVIVAL) {
                if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
                    dir += camera.worldUp * speed * 2.0f;
                }
                if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
                    dir -= camera.worldUp * speed;
            }
            return dir;
        }
    private:
        glm::vec3 velocity;
        float decel = 10.0f;
        float maxSpeed = 10.0f;
        float jumpStrength = 9.0f;

        float worldCentreOffset = (CHUNK_WIDTH * WORLD_WIDTH) / 2.0f;
        bool isGrounded = false;

        float coyoteTimer = 0.0f;
        float coyoteTimeMax = 0.1f;
};