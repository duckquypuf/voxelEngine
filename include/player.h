#pragma once

#include <GLFW/glfw3.h>

#include "camera.h"
#include "world.h"

enum Gamemode
{
    CREATIVE,
    SURVIVAL,
    SPECTATOR,
    ADVENTURE
};

class Player
{
public:
    Camera *camera;

    glm::vec3 velocity;
    glm::vec3 position;

    bool isGrounded = false;

    Gamemode gamemode;

    float reach = 5.0f;
    float stepIncrement = 0.01f;

    bool mouseDown = false;
    bool click = false;
    bool rightMouseDown = false;
    bool rightClick = false;

    bool jump = false;

    ChunkCoord coord;
    ChunkCoord lastCoord;

    Player(World *world, Camera *camera, Gamemode gamemode)
    {
        this->world = world;
        this->camera = camera;
        this->gamemode = gamemode;

        position = glm::vec3(worldWidth / 2.0f * chunkWidth, 500.0f, worldWidth / 2.0f * chunkWidth);
        velocity = glm::vec3(0.0f);

        coord = ChunkCoord(floor(worldWidth/2), floor(worldWidth/2));
        lastCoord = coord;
    }

    ~Player()
    {
        delete camera;
    }

    void updateCoord()
    {
        coord = world->getChunkCoordFromVec3(position);
    }

    void checkBlock()
    {
        glm::vec3 rayPos = camera->pos;
        glm::vec3 prevPos = rayPos;
        glm::vec3 rayDir = glm::normalize(camera->forward) * stepIncrement;

        while(glm::distance(camera->pos, rayPos) < reach)
        {
            int x = round(rayPos.x);
            int y = round(rayPos.y);
            int z = round(rayPos.z);

            if (world->checkForVoxel(x, y, z))
            {
                if (click)
                {
                    world->setVoxel(x, y, z, 0); // Air
                    break;
                }
                else if (rightClick)
                {
                    int placeX = round(prevPos.x);
                    int placeY = round(prevPos.y);
                    int placeZ = round(prevPos.z);

                    if (!world->checkForVoxel(placeX, placeY, placeZ))
                    {
                        glm::vec3 blockPos = glm::vec3(placeX, placeY, placeZ);
                        float distToPlayer = glm::distance(blockPos, position);

                        if (distToPlayer > playerWidth)
                        {
                            world->setVoxel(placeX, placeY, placeZ, 3); // Stone
                        }
                    }
                    break;
                }
            }

            prevPos = rayPos;
            rayPos += rayDir;
        }
    }

    bool checkRD()
    {
        if(_rd != renderDistance)
        {
            _rd = renderDistance;
            return true;
        }

        return false;
    }

    void processInput(GLFWwindow *window, float dt)
    {
        if (gamemode == SPECTATOR)
        {
            processSpectatorMovement(window, dt);
        }
        else
        {
            processNormalMovement(window, dt);
        }

        if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if(mouseDown)
            {
                click = false;
            } else
            {
                click = true;
            }

            mouseDown = true;
        } else
        {
            mouseDown = false;
        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
        {
            if (rightMouseDown)
            {
                rightClick = false;
            }
            else
            {
                rightClick = true;
            }

            rightMouseDown = true;
        }
        else
        {
            rightMouseDown = false;
        }

        jump = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        camera->processInput(window, dt);
        camera->setPos(position + glm::vec3(0.0f, playerHeight*0.9f, 0.0f));
    }

    void updatePhysics(float dt)
    {
        // Skip physics in spectator mode
        if (gamemode == SPECTATOR)
        {
            position += velocity * dt;
            velocity *= spectatorAirResistance;
            return;
        }

        // Apply gravity
        velocity.y -= gravity * dt;

        // Clamp velocities
        velocity.y = glm::clamp(velocity.y, -maxFallSpeed, maxFallSpeed);
        float horizontalSpeed = glm::length(glm::vec2(velocity.x, velocity.z));
        if (horizontalSpeed > maxMovementSpeed)
        {
            glm::vec2 normalized = glm::normalize(glm::vec2(velocity.x, velocity.z));
            velocity.x = normalized.x * maxMovementSpeed;
            velocity.z = normalized.y * maxMovementSpeed;
        }

        moveWithCollision(dt);

        camera->setPos(position + glm::vec3(0.0f, playerHeight * 0.9f, 0.0f));
    }

private:
    World *world;

    int _rd = renderDistance;

    float playerHeight = 1.8f;
    float playerWidth = 0.6f;

    float spectatorSpeed = 50.0f;
    float spectatorAirResistance = 0.98f;

    float speed = 4.5f;
    float sprintSpeed = 6.5f;
    float friction = 0.1f;

    bool isSprinting = false;

    float groundAcceleration = 20.0f;
    float airAcceleration = 5.0f;

    float gravity = 24.0f;
    float jumpForce = 8.0f;
    float maxFallSpeed = 50.0f;
    float maxMovementSpeed = 8.0f;

    void processSpectatorMovement(GLFWwindow *window, float dt)
    {
        glm::vec3 inputDir = glm::vec3(0.0f);

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            inputDir += camera->forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            inputDir -= camera->forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            inputDir -= camera->right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            inputDir += camera->right;
        if (jump)
            inputDir += glm::vec3(0.0f, 1.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            inputDir -= glm::vec3(0.0f, 1.0f, 0.0f);

        if (glm::length(inputDir) > 0.0f)
            inputDir = glm::normalize(inputDir);

        velocity += inputDir * spectatorSpeed * dt;
    }

    void processNormalMovement(GLFWwindow *window, float dt)
    {
        glm::vec3 inputDir = glm::vec3(0.0f);

        glm::vec3 forward = glm::normalize(glm::vec3(camera->forward.x, 0.0f, camera->forward.z));
        glm::vec3 right = glm::normalize(glm::vec3(camera->right.x, 0.0f, camera->right.z));

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            inputDir += forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            inputDir -= forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            inputDir -= right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            inputDir += right;

        if (glm::length(inputDir) > 0.0f)
            inputDir = glm::normalize(inputDir);

        isSprinting = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;

        float targetSpeed = isSprinting? sprintSpeed : speed;

        camera->updateFOV(isSprinting);

        glm::vec3 targetVelocity = inputDir * targetSpeed;

        float accel = isGrounded ? groundAcceleration : airAcceleration;
        velocity.x = glm::mix(velocity.x, targetVelocity.x, accel * dt);
        velocity.z = glm::mix(velocity.z, targetVelocity.z, accel * dt);

        if (isGrounded && glm::length(inputDir) < 0.01f)
        {
            velocity.x *= pow(friction, dt);
            velocity.z *= pow(friction, dt);

            if (abs(velocity.x) < 0.01f)
                velocity.x = 0;
            if (abs(velocity.z) < 0.01f)
                velocity.z = 0;
        }

        // Jump
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if (jump && isGrounded)
            {
                velocity.y = jumpForce;
                isGrounded = false;
            }
        }
    }

    void moveWithCollision(float dt)
    {
        isGrounded = false;

        // Move X axis
        glm::vec3 newPos = position;
        newPos.x += velocity.x * dt;
        if (!checkCollision(newPos))
        {
            position.x = newPos.x;
        }
        else
        {
            velocity.x = 0;
        }

        // Move Z axis
        newPos = position;
        newPos.z += velocity.z * dt;
        if (!checkCollision(newPos))
        {
            position.z = newPos.z;
        }
        else
        {
            velocity.z = 0;
        }

        // Move Y axis last (vertical)
        newPos = position;
        newPos.y += velocity.y * dt;

        if (!checkCollision(newPos))
        {
            position.y = newPos.y;
        }
        else
        {
            if (velocity.y < 0) // Falling down
            {
                isGrounded = true;
            }
            velocity.y = 0;
        }
    }

    bool checkCollision(glm::vec3 pos)
    {
        float halfWidth = playerWidth / 2.0f;

        // Check corners of player bounding box
        glm::vec3 checkPoints[] = {
            // Bottom corners
            pos + glm::vec3(-halfWidth, 0.0f, -halfWidth),
            pos + glm::vec3(halfWidth, 0.0f, -halfWidth),
            pos + glm::vec3(-halfWidth, 0.0f, halfWidth),
            pos + glm::vec3(halfWidth, 0.0f, halfWidth),

            // Top corners
            pos + glm::vec3(-halfWidth, playerHeight, -halfWidth),
            pos + glm::vec3(halfWidth, playerHeight, -halfWidth),
            pos + glm::vec3(-halfWidth, playerHeight, halfWidth),
            pos + glm::vec3(halfWidth, playerHeight, halfWidth),

            // Middle corners
            pos + glm::vec3(-halfWidth, playerHeight * 0.5f, -halfWidth),
            pos + glm::vec3(halfWidth, playerHeight * 0.5f, -halfWidth),
            pos + glm::vec3(-halfWidth, playerHeight * 0.5f, halfWidth),
            pos + glm::vec3(halfWidth, playerHeight * 0.5f, halfWidth),
        };

        for (const auto &point : checkPoints)
        {
            if (world->isVoxelSolid(round(point.x), round(point.y), round(point.z)))
            {
                return true;
            }
        }

        return false;
    }
};
