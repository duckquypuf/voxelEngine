#pragma once
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include "shader.h"

enum DrawType {
    PANEL = 0,
};

class GuiDrawing {
    public:
        DrawType draw;
        glm::vec2 size;
        float orientation;
        glm::vec3 colour;

        GuiDrawing(DrawType _draw, glm::vec2 _size, float _orientation, glm::vec3 _colour) {
            draw = _draw;
            size = _size;
            orientation = _orientation;
            colour = _colour;
        }
};

class GuiElement {
    public:
        glm::vec2 pos;
        std::vector<GuiDrawing> drawings;

        GuiElement(int x, int y, std::vector<GuiDrawing> _drawings) {
            pos.x = x;
            pos.y = y;
            drawings = _drawings;
        }

};

class GuiManager {
    public:
        unsigned int VAO, VBO;
        Shader* hudShader;
        glm::mat4 projection;

        int screenWidth, screenHeight;

        std::vector<GuiElement> elements;

        GuiManager(int width, int height, Shader* shader) {
            screenWidth = width;
            screenHeight = height;
            hudShader = shader;

            projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);

            float vertices[] = {
                -0.5f, -0.5f, 0.0f, 0.0f,
                 0.5f, -0.5f, 1.0f, 0.0f,
                 0.5f,  0.5f, 1.0f, 1.0f,

                 0.5f,  0.5f, 1.0f, 1.0f,
                -0.5f,  0.5f, 0.0f, 1.0f,
                -0.5f, -0.5f, 0.0f, 0.0f
            };

            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            glEnableVertexAttribArray(1);
        }

        ~GuiManager() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
        }

        void AddElement(GuiElement element) {
            elements.push_back(element);
        }

        void RenderDrawing(GuiDrawing& drawing, glm::vec2 pos) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(pos.x, pos.y, 0.0f));
            model = glm::rotate(model, glm::radians(drawing.orientation), glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::scale(model, glm::vec3(drawing.size.x, drawing.size.y, 1.0f));

            hudShader->setMat4("model", model);
            hudShader->setVec3("colour", drawing.colour);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        void RenderElement(GuiElement& element) {
            for(auto& drawing : element.drawings) {
                RenderDrawing(drawing, element.pos);
            }
        }

        void Render() {
            glDisable(GL_DEPTH_TEST);

            hudShader->use();
            hudShader->setMat4("projection", projection);

            for(auto& element : elements) {
                RenderElement(element);
            }
        }
};
