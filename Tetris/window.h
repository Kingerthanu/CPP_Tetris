#pragma once

#include "shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <string>
#include <cstring>
#include "board.h"
#include <fstream>



class Window
{

public:
    enum class GameState
    {
        MAIN_MENU,
        PLAYING,
        GAME_OVER
    };
    bool needsRedraw = true;

private:

    Board gameBoard;

    Shader backgroundShader;
    Shader textShader;

    GLFWwindow* window;
    unsigned int width, height;
    std::string title;

    // Background Rendering
    GLuint backgroundVAO, backgroundVBO, backgroundEBO;

    // Text Rendering with Texture Atlas
    GLuint textVAO, textVBO, textEBO;
    GLuint fontTextureID;
    std::vector<float> textVertices;
    std::vector<unsigned int> textIndices;

    // Cached values for change detection
    unsigned int lastScore = UINT32_MAX;
    unsigned int lastLevel = UINT32_MAX;
    unsigned int lastLines = UINT32_MAX;
    bool textNeedsUpdate = true;

    // UI/Game State
    unsigned int score;
    unsigned int level;
    unsigned int linesCleared;
    bool isInit, gameOverFlag = false;;

    float currentR = 0.1f, currentG = 0.1f, currentB = 0.2f;
    float targetR = 0.3f, targetG = 0.3f, targetB = 0.3f;
    float colorTransitionSpeed = 0.01f;
    int colorChangeTimer = 0;

    // Character mapping constants
    static constexpr float CHAR_SIZE = 1.0f / 16.0f;  // 16x16 grid
    static constexpr int ATLAS_SIZE = 128;  // 128x128 texture
    static constexpr int CHAR_PIXEL_SIZE = 8;  // 8x8 pixels per character

    static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win) win->handleKeyInput(key, scancode, action, mods);
    }

    void updateBackgroundVertexColor(float r, float g, float b)
    {

        float vertices[] = {
            -1.0f, -1.0f, 0.0f, r, g, b,
            1.0f, -1.0f, 0.0f, r, g, b,
            1.0f, 1.0f, 0.0f, r, g, b,
            -1.0f, 1.0f, 0.0f, r, g, b
        };

		// Set Our Current

        glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    }

    void updateBackgroundVertexColorRandom()
    {

        float vertices[] = {
            -1.0f, -1.0f, 0.0f, ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f),
            1.0f, -1.0f, 0.0f, ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f),
            1.0f, 1.0f, 0.0f, ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f),
            -1.0f, 1.0f, 0.0f, ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f), ((rand() % 256) / 255.0f)
        };

        // Set Our Current

        glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    }

    static void glfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            // Handle left mouse button click
        }
    }

    static void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if (win) {
            win->handleResize(width, height);
        }
    }

    void handleResize(int newWidth, int newHeight)
    {
		glViewport(0, 0, newWidth, newHeight);
        this->needsRedraw = true;
    }

    void handleKeyInput(int key, int scancode, int action, int mods)
    {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        {

            // Handle Escape Differently For Current State
            switch(currentState)
            {
                case GameState::PLAYING:
                case GameState::GAME_OVER:
                    // Resume the game
                    currentState = GameState::MAIN_MENU;
                    break;
                case GameState::MAIN_MENU:
                    // Exit the game
                    glfwSetWindowShouldClose(window, true);
					break;
			}

            glfwSetWindowShouldClose(window, true);
        }

        // Test controls for demo
        if (action == GLFW_PRESS) {
            switch (currentState) {
            case GameState::MAIN_MENU:
                switch (key) {
                    case GLFW_KEY_ENTER:
                        currentState = GameState::PLAYING;
                        break;
                }
                break;

            case GameState::PLAYING:
                // Game controls
                switch (key) {
                    case GLFW_KEY_SPACE:
                        addScore(100);
                        break;
                    case GLFW_KEY_LEFT:
                        gameBoard.shiftActivePiece('L');
                        gameBoard.ghostPiece();
                        this->needsRedraw = true;
                        break;
                    case GLFW_KEY_RIGHT:
                        gameBoard.shiftActivePiece('R');
                        gameBoard.ghostPiece();
                        this->needsRedraw = true;
                        break;
                    case GLFW_KEY_DOWN:
                        gameBoard.step();
                        this->needsRedraw = true;
                        break;
                    case GLFW_KEY_UP:
                        gameBoard.rotateActivePiece();
                        gameBoard.ghostPiece();
						this->needsRedraw = true;
                        break;
                    case GLFW_KEY_ENTER:
                        gameBoard.shiftActivePiece('D');
                        this->needsRedraw = true;
                        break;
                }
                break;

            case GameState::GAME_OVER:
                switch (key) {
                    case GLFW_KEY_ENTER:
                        currentState = GameState::MAIN_MENU;
						// Reset the game state
						this->resetGame();
                        this->setupMainMenuText();
                        break;
                    case GLFW_KEY_R:
                        currentState = GameState::PLAYING;
						this->resetGame();
                        break;
                }
                break;
            }
        }

    }

    void setupGameBackground()
    {
        float vertices[] = {
            -1.0f, -1.0f, 0.0f, 1.0f, 0.5f, 0.0f,   // Bottom Left
            1.0f, -1.0f, 0.0f,  0.0f, 0.5f, 0.0f,   // Bottom Right
            1.0f, 1.0f, 0.0f,   0.0f, 0.85f, 0.6f,  // Top Right
            -1.0f, 1.0f, 0.0f,  0.4f, 0.232f, 0.0f  // Top Left
        };

        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        glGenVertexArrays(1, &backgroundVAO);
        glGenBuffers(1, &backgroundVBO);
        glGenBuffers(1, &backgroundEBO);

        glBindVertexArray(backgroundVAO);

        glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void createFontTexture()
    {
        // Create a simple procedural font texture (8x8 pixels per character)
        unsigned char fontData[ATLAS_SIZE * ATLAS_SIZE];
        memset(fontData, 0, sizeof(fontData));

        // Define simple bitmap patterns for digits 0-9
        unsigned char digitPatterns[10][8] = {
            // 0
            {0b01111110, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b01111110},
            // 1
            {0b00011000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111110},
            // 2
            {0b01111110, 0b11000011, 0b00000011, 0b00000110, 0b00001100, 0b00110000, 0b01100000, 0b11111111},
            // 3
            {0b01111110, 0b11000011, 0b00000011, 0b00111110, 0b00000011, 0b00000011, 0b11000011, 0b01111110},
            // 4
            {0b11000011, 0b11000011, 0b11000011, 0b11111111, 0b00000011, 0b00000011, 0b00000011, 0b00000011},
            // 5
            {0b11111111, 0b11000000, 0b11000000, 0b11111110, 0b00000011, 0b00000011, 0b11000011, 0b01111110},
            // 6
            {0b01111110, 0b11000011, 0b11000000, 0b11111110, 0b11000011, 0b11000011, 0b11000011, 0b01111110},
            // 7
            {0b11111111, 0b00000011, 0b00000110, 0b00001100, 0b00011000, 0b00110000, 0b01100000, 0b11000000},
            // 8
            {0b01111110, 0b11000011, 0b11000011, 0b01111110, 0b11000011, 0b11000011, 0b11000011, 0b01111110},
            // 9
            {0b01111110, 0b11000011, 0b11000011, 0b11000011, 0b01111111, 0b00000011, 0b11000011, 0b01111110}
        };

        // Letter patterns for our UI text
        unsigned char letterPatterns[28][8] = {
            // S (10)
            {0b01111110, 0b11000000, 0b11000000, 0b01111110, 0b00000011, 0b00000011, 0b00000011, 0b11111110},
            // c (11)
            {0b00000000, 0b00000000, 0b01111110, 0b11000000, 0b11000000, 0b11000000, 0b01111110, 0b00000000},
            // o (12)
            {0b00000000, 0b00000000, 0b01111110, 0b11000011, 0b11000011, 0b11000011, 0b01111110, 0b00000000},
            // r (13)
            {0b00000000, 0b00000000, 0b11111110, 0b11000011, 0b11000000, 0b11000000, 0b11000000, 0b00000000},
            // e (14)
            {0b00000000, 0b00000000, 0b01111110, 0b11000011, 0b11111110, 0b11000000, 0b01111110, 0b00000000},
            // : (15)
            {0b00000000, 0b00011000, 0b00011000, 0b00000000, 0b00000000, 0b00011000, 0b00011000, 0b00000000},
            // space (16)
            {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000},
            // L (17)
            {0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11000000, 0b11111111, 0b00000000},
            // v (18)
            {0b00000000, 0b00000000, 0b11000011, 0b11000011, 0b11000011, 0b01100110, 0b00111100, 0b00011000},
            // l (19)
            {0b01100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b00100000, 0b01110000, 0b00000000},
            // i (20)
            {0b00011000, 0b00000000, 0b00111000, 0b00011000, 0b00011000, 0b00011000, 0b01111110, 0b00000000},
            // n (21)
            {0b00000000, 0b00000000, 0b11111110, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b00000000},
            // s (22)
            {0b00000000, 0b00000000, 0b01111110, 0b11000000, 0b01111110, 0b00000011, 0b11111110, 0b00000000},
            // T (23) 
            {0b11111111, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000},
            // t (24) 
            {0b00000000, 0b00100000, 0b11111110, 0b00100000, 0b00100000, 0b00100000, 0b00110000, 0b00000000},
            // R (25) 
            {0b11111110, 0b11000011, 0b11000011, 0b11111110, 0b11110000, 0b11011000, 0b11001100, 0b11000110},
            // I (26) 
            {0b01111110, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b00011000, 0b01111110},
            // P (27) 
            {0b11111110, 0b11000011, 0b11000011, 0b11111110, 0b11000000, 0b11000000, 0b11000000, 0b11000000},
            // E (28) 
            {0b11111111, 0b11000000, 0b11000000, 0b11111110, 0b11000000, 0b11000000, 0b11000000, 0b11111111},
            // a (29) 
            {0b00000000, 0b00000000, 0b01111110, 0b00000011, 0b01111111, 0b11000011, 0b01111111, 0b00000000},
            // A (30) 
            {0b00111100, 0b01100110, 0b11000011, 0b11000011, 0b11111111, 0b11000011, 0b11000011, 0b11000011},
            // G (31)
            {0b01111110, 0b11000011, 0b11000000, 0b11000000, 0b11001111, 0b11000011, 0b11000011, 0b01111110},
            // m (32)
            {0b00000000, 0b00000000, 0b11101110, 0b11111111, 0b11011011, 0b11011011, 0b11011011, 0b00000000},
            // O (33) - Capital O
            {0b01111110, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b11000011, 0b01111110},
            // d (34) - lowercase d
            {0b00000011, 0b00000011, 0b01111111, 0b11000011, 0b11000011, 0b11000011, 0b01111111, 0b00000000},
            // b (35) - lowercase b  
            {0b11000000, 0b11000000, 0b11111110, 0b11000011, 0b11000011, 0b11000011, 0b11111110, 0b00000000},
            {0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011000, 0b00011000}


        };

        // Copy digit patterns to texture (first row)
        for (int digit = 0; digit < 10; digit++) {
            int baseX = digit * CHAR_PIXEL_SIZE;
            int baseY = 0;

            for (int y = 0; y < 8; y++) {
                unsigned char row = digitPatterns[digit][y];
                for (int x = 0; x < 8; x++) {
                    int pixelX = baseX + x;
                    int pixelY = baseY + y;

                    if (row & (1 << (7 - x))) {
                        fontData[pixelY * ATLAS_SIZE + pixelX] = 255;
                    }
                }
            }
        }

        // Copy letter patterns to texture (starting at position 10)
        for (int letter = 0; letter < 28; letter++) {
            int charIndex = 10 + letter;
            int baseX = (charIndex % 16) * CHAR_PIXEL_SIZE;
            int baseY = (charIndex / 16) * CHAR_PIXEL_SIZE;

            for (int y = 0; y < 8; y++) {
                unsigned char row = letterPatterns[letter][y];
                for (int x = 0; x < 8; x++) {
                    int pixelX = baseX + x;
                    int pixelY = baseY + y;

                    if (row & (1 << (7 - x))) {
                        fontData[pixelY * ATLAS_SIZE + pixelX] = 255;
                    }
                }
            }
        }

        // Create OpenGL texture
        glGenTextures(1, &fontTextureID);
        glBindTexture(GL_TEXTURE_2D, fontTextureID);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, fontData);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void setupTextRendering()
    {
        glGenVertexArrays(1, &textVAO);
        glGenBuffers(1, &textVBO);
        glGenBuffers(1, &textEBO);

        glBindVertexArray(textVAO);

        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

        // Position
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // UV coordinates
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    int getCharIndex(char c) const
    {
        if (c >= '0' && c <= '9') return c - '0';

        switch (c) 
        {
            case 'S': return 10;
            case 'c': return 11;
            case 'o': return 12;
            case 'r': return 13;
            case 'e': return 14;
            case ':': return 15;
            case ' ': return 16;
            case 'L': return 17;
            case 'v': return 18;
            case 'l': return 19;
            case 'i': return 20;
            case 'n': return 21;
            case 's': return 22;
            case 'T': return 23; 
            case 't': return 24;  
            case 'R': return 25;  
            case 'I': return 26;  
            case 'P': return 27; 
            case 'E': return 28;  
            case 'a': return 29; 
            case 'A': return 30; 
            case 'G': return 31;
            case 'm': return 32;
            case 'O': return 33;
            case 'd': return 34;  // New
            case 'b': return 35;  // New
            case '.': return 36;
            default: return 16; // Default to space
        }
    }

    void addCharToText(char c, float x, float y, float scale)
    {
        if (c == ' ') return; // Skip spaces

        int charIndex = getCharIndex(c);

        // Calculate UV coordinates in texture atlas
        float uvX = (charIndex % 16) * CHAR_SIZE;
        float uvY = (charIndex / 16) * CHAR_SIZE;
        float uvW = CHAR_SIZE;
        float uvH = CHAR_SIZE;

        // Character dimensions
        float charW = 0.04f * scale;
        float charH = 0.06f * scale;

        unsigned int baseIndex = textVertices.size() / 4;

        // Add 4 vertices (position + UV)
        textVertices.insert(textVertices.end(), {
            x, y, uvX, uvY + uvH,                    // Bottom left
            x + charW, y, uvX + uvW, uvY + uvH,      // Bottom right
            x + charW, y + charH, uvX + uvW, uvY,    // Top right
            x, y + charH, uvX, uvY                   // Top left
            });

        // Add indices for two triangles
        textIndices.insert(textIndices.end(), {
            baseIndex, baseIndex + 1, baseIndex + 2,
            baseIndex + 2, baseIndex + 3, baseIndex
            });
    }

    void updateTextContent()
    {
        if (!textNeedsUpdate) return;

        textVertices.clear();
        textIndices.clear();

        float charSpacing = 0.045f;
        float currentX, currentY;

        // Add "Score:" label
        currentX = -0.95f;
        currentY = 0.85f;
        std::string scoreLabel = "Score:";
        for (char c : scoreLabel) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Add score digits
        currentX = -0.65f;
        std::string scoreStr = std::to_string(score);
        for (char c : scoreStr) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Add "Level:" label
        currentX = -0.95f;
        currentY = 0.75f;
        std::string levelLabel = "Level:";
        for (char c : levelLabel) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Add level digits
        currentX = -0.65f;
        std::string levelStr = std::to_string(level);
        for (char c : levelStr) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Add "Lines:" label
        currentX = -0.95f;
        currentY = 0.65f;
        std::string linesLabel = "Lines:";
        for (char c : linesLabel) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Add lines digits
        currentX = -0.65f;
        std::string linesStr = std::to_string(linesCleared);
        for (char c : linesStr) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Update GPU buffers
        if (!textVertices.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndices.size() * sizeof(unsigned int), textIndices.data(), GL_DYNAMIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        textNeedsUpdate = false;
        lastScore = score;
        lastLevel = level;
        lastLines = linesCleared;
    }

    void cleanup()
    {
        if (this->isInit)
        {
            glDeleteVertexArrays(1, &backgroundVAO);
            glDeleteBuffers(1, &backgroundVBO);
            glDeleteBuffers(1, &backgroundEBO);

            glDeleteVertexArrays(1, &textVAO);
            glDeleteBuffers(1, &textVBO);
            glDeleteBuffers(1, &textEBO);

            if (fontTextureID) {
                glDeleteTextures(1, &fontTextureID);
            }
        }

        if (this->window)
        {
            glfwDestroyWindow(this->window);
            this->window = nullptr;
        }

        glfwTerminate();
        this->isInit = false;
    }


	GameState currentState = GameState::MAIN_MENU;

public:

    Window(unsigned int w, unsigned int h, const char* windowTitle)
        : window(nullptr), width(w), height(h), title(windowTitle),
        score(0), level(1), linesCleared(0), isInit(false),
        backgroundVAO(0), backgroundVBO(0), backgroundEBO(0),
        textVAO(0), textVBO(0), textEBO(0), fontTextureID(0),
		gameBoard(20, 10, w, h, this->score, this->textNeedsUpdate, this->linesCleared, this->level, this->gameOverFlag) 
    {
        textVertices.reserve(1000);
        textIndices.reserve(1500);
    }

    unsigned int tickCount = 0;

    void update()
    {
		if (!this->isInit)
		{
			std::cerr << "Window Not Initialized." << std::endl;
			return;
		}

        this->tickCount = 0;
        glBindVertexArray(backgroundVAO);
    
		gameBoard.step();
		
    }

    void renderText()
    {
        if (!textIndices.empty())
        {
            textShader.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fontTextureID);
            GLint textureLocation = glGetUniformLocation(textShader.getProgID(), "fontTexture");

            if (textureLocation != -1) {
                glUniform1i(textureLocation, 0);
            }
            else {
                std::cerr << "Failed to set font texture uniform." << std::endl;
            }

            glBindVertexArray(textVAO);
            glDrawElements(GL_TRIANGLES, textIndices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

        }
    }


    bool initialize()
    {
        if (this->isInit)
        {
            std::cerr << "Window Is Already Initialized." << std::endl;
            return false;
        }

        // Initialize GLFW
        if (!glfwInit())
        {
            std::cerr << "Failed to Initialize GLFW." << std::endl;
            return false;
        }

        // Config GLFW Hinting
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

        // Create Our Window
        this->window = glfwCreateWindow(this->width, this->height, this->title.c_str(), nullptr, nullptr);

        if (!this->window)
        {
            std::cerr << "Failed to Create GLFW Window." << std::endl;
            glfwTerminate();
            return false;
        }

        // Set Context To This Window
        glfwMakeContextCurrent(this->window);

        // Set Window User Pointer
        glfwSetWindowUserPointer(this->window, this);

        // Set Callbacks
        glfwSetKeyCallback(this->window, glfwKeyCallback);
        glfwSetMouseButtonCallback(this->window, glfwMouseCallback);
        glfwSetFramebufferSizeCallback(this->window, glfwFramebufferSizeCallback);

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cerr << "Failed to Initialize GLAD." << std::endl;
            glfwDestroyWindow(this->window);
            glfwTerminate();
            return false;
        }

        glViewport(0, 0, this->width, this->height);

        // Enable blending for text transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Load shaders
        if (!backgroundShader.loadShader("./background.vert", "./background.frag")) {
            std::cerr << "Failed to load background shader" << std::endl;
            return false;
        }

        if (!textShader.loadShader("./text.vert", "./text.frag")) {
            std::cerr << "Failed to load text shader" << std::endl;
            return false;
        }

        // Setup rendering
        setupGameBackground();
        createFontTexture();
        setupTextRendering();

		// Initialize game board
        glBindVertexArray(backgroundVAO);
		gameBoard.initialize();
        this->updateBackgroundVertexColorRandom();
        this->setupMainMenuText();

        this->isInit = true;
        std::cout << "Window Initialized Successfully." << std::endl;
        return true;
    }



    void setupMainMenuBackground()
    {

        // Setup Our Main Menu Background Based On Color (Use The Same Background VBO Just Swap Colors)
		glBindVertexArray(backgroundVAO);

		// Set the background color for the main menu
        float vertices[] = {
            -1.0f, -1.0f, 0.0f, 1.0f, 0.5f, 0.0f,   // Bottom Left
            1.0f, -1.0f, 0.0f,  0.0f, 0.5f, 0.0f,   // Bottom Right
            1.0f, 1.0f, 0.0f,   0.0f, 0.85f, 0.6f,  // Top Right
            -1.0f, 1.0f, 0.0f,  0.4f, 0.232f, 0.0f  // Top Left
        };

        glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindVertexArray(0);

    }

    void setupGameOverText()
    {

        // Setup Our Main Menu Text
        textVertices.clear();
        textIndices.clear();

        float charSpacing = 0.045f;
        float currentX = -0.95f;
        float currentY = -0.55f;

		// Add "Game Over" title
        std::string titleText = "Game Over";
        for (char c : titleText) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

		// Add "Score: " label
		currentX = -0.95f;
		currentY -= 0.1f; // Move down for the next line
		std::string scoreLabel = "Score: " + std::to_string(this->score);
        for(char c: scoreLabel) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
		}

        // Add "Press Enter to Restart" instruction
        currentX = -0.95f;
        currentY -= 0.1f; // Move down for the next line
        std::string instructionText = "Press Enter to Restart";
        for (char c : instructionText) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Leaderboard Right Column Of Screen
		currentX = 0.15f;
		currentY = 0.85f;
		std::string leaderboardText = "Leaderboard:";
        for (char c : leaderboardText) {
            addCharToText(c, currentX, currentY, 0.65f);
            currentX += charSpacing;
		}

		// Add scores to leaderboard
		currentY -= 0.08f; // Move down for the next line
        
		std::ifstream leaderboardFile("leaderboard.txt");

		if (leaderboardFile.is_open()) {

            std::string line;
			int rank = 1;

            while (std::getline(leaderboardFile, line) && rank <= 10)
            {

                if (!line.empty())
                {
                    std::string rankText = std::to_string(rank) + ". " + line;

					currentX = 0.195f; // Reset X position for each rank

                    for (char c : rankText) {
                        addCharToText(c, currentX, currentY, 0.5f);
                        currentX += charSpacing;
                    }
                    currentY -= 0.08f; // Move down for the next line
					rank++;
                }

            }

			leaderboardFile.close();

        }
        else {
            std::cerr << "Failed to open leaderboard file." << std::endl;
		}


        // Update GPU buffers
        if (!textVertices.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndices.size() * sizeof(unsigned int), textIndices.data(), GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    void setupMainMenuText()
    {

		// Setup Our Main Menu Text
        textVertices.clear();
        textIndices.clear();

        float charSpacing = 0.045f;
        float currentX = -0.95f;
        float currentY = -0.85f;

        // Add "Tetris" title
        std::string titleText = "Tetris";
        for (char c : titleText) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Add "Press Enter to Start" instruction
        currentX = -0.95f;
        currentY -= 0.1f; // Move down for the next line
        std::string instructionText = "Press Enter to Start";
        for (char c : instructionText) {
            addCharToText(c, currentX, currentY, 1.0f);
            currentX += charSpacing;
        }

        // Update GPU buffers
        if (!textVertices.empty()) {
            glBindBuffer(GL_ARRAY_BUFFER, textVBO);
            glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndices.size() * sizeof(unsigned int), textIndices.data(), GL_DYNAMIC_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

    }

    void renderMainMenuBackground()
    {
        // Use the background shader
        backgroundShader.use();

        // Bind the shared VAO once
        glBindVertexArray(this->backgroundVAO);

        // Render window background
        glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundEBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	}

    bool shouldClose() const
    {
        return window ? glfwWindowShouldClose(this->window) : true;
    }

    void swapBuffers()
    {
        if (this->window)
        {
            glfwSwapBuffers(this->window);
        }
    }

    void pollEvents()
    {
        if (this->window)
        {
            glfwPollEvents();
        }
    }

    void clear()
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    
    // In renderGameBackground():
    void renderGameBackground()
    {

        backgroundShader.use();

        // Slowly move current color toward target color
        currentR += (targetR - currentR) * colorTransitionSpeed;
        currentG += (targetG - currentG) * colorTransitionSpeed;
        currentB += (targetB - currentB) * colorTransitionSpeed;

        // When we get close to the target, or occasionally, pick a new random target
        if ((--colorChangeTimer <= 0) ||
            (std::abs(currentR - targetR) < 0.01f &&
                std::abs(currentG - targetG) < 0.01f &&
                std::abs(currentB - targetB) < 0.01f))
        {
            // Pick a new random target color
            targetR = (static_cast<float>(std::rand()) / 0xAfff);
            targetG = (static_cast<float>(std::rand()) / 0xAfff);
            targetB = (static_cast<float>(std::rand()) / 0xAfff);

            // Set a random timer for when we'll pick a new color
            colorChangeTimer = 5 + (std::rand() % 10);
        }

        // Send current color to shader
        glUniform3f(glGetUniformLocation(backgroundShader.getProgID(), "bgColor"),
            currentR, currentG, currentB);

        // Bind the shared VAO once
        glBindVertexArray(this->backgroundVAO);

        // Render window background
        glBindBuffer(GL_ARRAY_BUFFER, this->backgroundVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->backgroundEBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Send current color to shader
        glUniform3f(glGetUniformLocation(this->backgroundShader.getProgID(), "bgColor"),
            0.0f, 0.0f, 0.0f);

        // Render game board
        glBindVertexArray(backgroundVAO);
        gameBoard.render();
    }

    void renderUI()
    {
        // Check if text needs updating
        if (score != lastScore || level != lastLevel || linesCleared != lastLines) {
            textNeedsUpdate = true;
        }

        updateTextContent();

        if (!textIndices.empty()) {
            textShader.use();

            // Bind font texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fontTextureID);
            GLint texLoc = glGetUniformLocation(textShader.getProgID(), "fontTexture");
            if (texLoc >= 0) {
                glUniform1i(texLoc, 0);
            }

            glBindVertexArray(textVAO);
            glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(textIndices.size()), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    // Game state methods
    void setScore(unsigned int newScore) {
        if (score != newScore) {
            score = newScore;
            textNeedsUpdate = true;
        }
    }

    void addScore(unsigned int points) {
        score += points;
        textNeedsUpdate = true;
    }

    void addToScoreboard(unsigned int points)
    {

		std::vector<unsigned int> scores;
        scores.reserve(11);

		std::ifstream leaderboardFile("leaderboard.txt");
		bool scoreInserted = false;

		if (leaderboardFile.is_open()) {

            std::string line;

			while (std::getline(leaderboardFile, line)) {

                unsigned int score = std::stoul(line);

                if(!scoreInserted && points > score)
                {
                    scores.push_back(points);
                    scoreInserted = true;
				}   

				scores.push_back(score);

            }

			leaderboardFile.close();

        }

		if (!scoreInserted) {
			scores.push_back(points);
        }

        // Write Back To File
		std::ofstream outFile("leaderboard.txt");
        if (outFile.is_open()) {
         
            for(unsigned int score : scores)
            {
                outFile << score << std::endl;
			}

            outFile.close();
        } else {
			std::cerr << "Failed to open leaderboard file for writing." << std::endl;
  
        }
        
    }

    void setLevel(unsigned int newLevel) {
        if (level != newLevel) {
            level = newLevel;
            textNeedsUpdate = true;
        }
    }

    void setLinesCleared(unsigned int lines) {
        if (linesCleared != lines) {
            linesCleared = lines;
            textNeedsUpdate = true;
        }
    }

    unsigned int getScore() const { return score; }
    unsigned int getLevel() const { return level; }
    unsigned int getLinesCleared() const { return linesCleared; }
	GameState getCurrentState() const { return currentState; }
	void setCurrentState(GameState state) { currentState = state; }
	bool isGameOver() const { return gameOverFlag; }

    void resetGame()
    {
        score = 0;
        level = 1;
        linesCleared = 0;
        textNeedsUpdate = true;
        gameOverFlag = false;
		needsRedraw = true;
        tickCount = 0;

        // Set To New Random Background
        this->updateBackgroundVertexColorRandom();
		this->currentR = 0.0f;
		this->currentG = 0.0f;
        this->currentB = 0.0f;

	}

    unsigned int getDropSpeed()
    {
        float normLevel = std::min(level / 10.0f, 1.0f);
		float curve = normLevel * normLevel; // Quadratic curve for speed increase
        return 8 - (curve * 7.0f);

    }

    ~Window()
    {
        cleanup();
        std::cout << "Window Resources Cleaned Up." << std::endl;
    }
};