#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>

class Camera {
    public:
        glm::vec3 cameraPos;
        glm::vec3 cameraFront;
        glm::vec3 cameraUp;
        glm::vec3 worldUp;
        glm::vec3 cameraRight;

        float yaw;
        float pitch;

        float fov = 90.0f;

        float sensitivity = 0.1f;
        float movementSpeed = 10.0f;

        Camera(glm::vec3 pos, float _fov) {
            cameraPos = pos;
            fov = _fov;
            cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
            worldUp = cameraUp;

            yaw = 90.0f;
            pitch = 0.0f;

            updateCameraVectors();
        }

        void processMouseMovement(float xOffset, float yOffset) {
            xOffset *= sensitivity;
            yOffset *= sensitivity;

            yaw += xOffset;
            pitch += yOffset;

            if(pitch > 89.0f) 
                pitch = 89.0f;
            else if(pitch < -89.0f)
                pitch = -89.0f;
                
            updateCameraVectors();
        }

        glm::mat4 getViewMatrix() {
            return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        }

        void setPosition(glm::vec3 pos) {
            cameraPos = pos;
        }

    private:
        void updateCameraVectors() {
            glm::vec3 front;
            front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
            front.y = sin(glm::radians(pitch));
            front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
            cameraFront = glm::normalize(front);

            cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
            cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
        }
};
