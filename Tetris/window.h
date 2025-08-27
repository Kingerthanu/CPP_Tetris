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
#include "config.h"



class Window
{

    public:
        // Current Game State For State Handling
        enum class GameState
        {

            MAIN_MENU,
            PLAYING,
            GAME_OVER

        };

        // Rendering Flag To Determine If We Need To Redraw (Ensure We Don't Redudnantly Redraw)
        bool needsRedraw = true;


    private:

        // Tetris Game Board
        Board gameBoard;

        // Cached values for change detection
        unsigned int lastScore = UINT32_MAX;
        unsigned int lastLevel = UINT32_MAX;
        unsigned int lastLines = UINT32_MAX;
        bool textNeedsUpdate = true;

        // UI/Game State
        unsigned int score;
        unsigned int level;
        unsigned int linesCleared;
        bool isInit, gameOverFlag = false;

        // Shader For Background And Window Text
        Shader backgroundShader;
        Shader textShader;

        // Background Rendering
        GLuint backgroundVAO, backgroundVBO, backgroundEBO;

        // Text Rendering with Texture Atlas
        GLuint textVAO, textVBO, textEBO;
        GLuint fontTextureID;
        std::vector<float> textVertices;
        std::vector<unsigned int> textIndices;

        // Character Mapping Constants
        static constexpr float CHAR_SIZE = 1.0f / 16.0f;  // 16x16 Grid
        static constexpr int ATLAS_SIZE = 128;  // 128x128 Texture
        static constexpr int CHAR_PIXEL_SIZE = 8;  // 8x8 Pixels Per Character

        // Our GLFW Instance Of Window (Size & Name)
        GLFWwindow* window;
        unsigned int width, height;
        std::string title;

        // Values For Background Color Animations
        float currentR = 0.0f, currentG = 0.0f, currentB = 0.0f;
        float targetR = 0.0f, targetG = 0.0f, targetB = 0.0f;
        float colorTransitionSpeed = 0.01f;
        int colorChangeTimer = 0;



        /*

            Desc: Event Handler For The Window Instance, Routing To A Instance Specific Handler.

            Preconditions:
                1.) Is Implicitly Called By The GLFW Runtime.
                2.) key, scancode, action, & mods Are Formatted For The GLFW Codex
				3.) window Is A Valid Instance Of A Window's GLFWwindow Pointer.

            Postconditions:
				1.) Routes Event Handling To Window-Specific Key Event Handler (Window::handleKeyInput(...)).
				2.) Handles Key Input For The Current Game State.
                3.) If Invalid Window Class Instance, Won't Do Any Handling
    
        */
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
        {

            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (win) win->handleKeyInput(key, scancode, action, mods);

        }


        /*

			Desc: Updates The Background Vertex Color For The Background Quad. It Will Alter Them 
	        To The Specified RGB Valued Parameters Which Are Assumed In The Range Of [0.0f, 1.0f].

            Preconditions:
                1.) r, g, b Are In The Range Of [0.0f, 1.0f].
				2.) backgroundVBO Is A Valid OpenGL Buffer Object For The Background Quad.

            Postconditions:
				1.) Updates The Background Quad's Vertex Color To The Specified RGB Values.
                2.) The Background Quad Will Be Rendered With The New Color On The Next Draw Call.
				3.) If Invalid backgroundVBO, Won't Do Any Updates

        */
        void updateBackgroundVertexColor(const float& r, const float& g, const float& b)
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


        /*

			Desc: Updates The Background Vertex Color To A Random Color. It Will Alter Them
            To Random RGB Values In The Range Of [0.0f, 1.0f].

            Preconditions:
                1.) backgroundVBO Is A Valid OpenGL Buffer Object For The Background Quad.

			Postconditions:
				1.) Updates The Background Quad's Vertex Color To A Random RGB Value.
				2.) The Background Quad Will Be Rendered With The New Color On The Next Draw Call.
                3.) If Invalid backgroundVBO, Won't Do Any Updates

        */
        void updateBackgroundVertexColorRandom()
        {

            float vertices[] = {
                -1.0f, -1.0f, 0.0f, (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX),
                1.0f, -1.0f, 0.0f, (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX),
                1.0f, 1.0f, 0.0f, (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX),
                -1.0f, 1.0f, 0.0f, (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX), (rand() / (float)RAND_MAX)
            };

            // Set Our Current
            glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        }


        /*
* 
            Desc: GLFW Mouse Button Callback For Handling Mouse Clicks.
            Preconditions:
                1.) window Is A Valid GLFWwindow Pointer.
                2.) button, action, & mods Are Formatted For The GLFW Codex.

            Postconditions:
                1.) If Left Mouse Button Clicked, Will Handle The Click Event.
                2.) If Invalid Window Class Instance, Won't Do Any Handling

		*/
        static void glfwMouseCallback(GLFWwindow* window, int button, int action, int mods)
        {

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                // Handle left mouse button click
            }

        }


        /*
* 
            Desc: GLFW Framebuffer Size Callback For Handling Window Resizes.

            Preconditions:
                1.) window Is A Valid GLFWwindow Pointer.
                2.) width & height Are The New Dimensions Of The Window.

            Postconditions:
                1.) Updates The OpenGL Viewport To The New Width & Height.
                2.) If Invalid Window Class Instance, Won't Do Any Handling

		*/
        static void glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
        {

            Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
            if (win)
            {
                win->handleResize(width, height);
            }

        }


        /*
 
            Desc: Resizes The OpenGL Viewport To The New Width & Height.

            Preconditions:
                1.) newWidth & newHeight Are The New Dimensions Of The Window.
                2.) window Is A Valid GLFWwindow Pointer.

            Postconditions:
                1.) Updates The OpenGL Viewport To The New Width & Height.
                2.) Sets needsRedraw To True To Indicate A Redraw Is Needed.

		*/
        void handleResize(const int& newWidth, const int& newHeight)
        {

		    glViewport(0, 0, newWidth, newHeight);
            this->needsRedraw = true;

        }


        /*

			Desc: Handles Key Input For The Current Game State. The Window Will Handle Escape 
            Key Differently Based On The Current Game State. It Will Also Handle Key Inputs 
            For The Main Menu, Playing, And Game Over States.

            Preconditions:
                1.) key, scancode, action, & mods Are Formatted For The GLFW Codex.
                2.) window Is A Valid GLFWwindow Pointer.
				3.) currentState Is A Valid GameState Enum Value.

            Postconditions:
                1.) If Escape Key Pressed, Will Handle It Based On The Current Game State.
                2.) If Other Keys Pressed, Will Handle Them Based On The Current Game State.
				3.) If Invalid Window Class Instance, Won't Do Any Handling

        */
        void handleKeyInput(const int& key, const int& scancode, const int& action, const int& mods)
        {

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {

                // Handle Escape Differently For Current State
                switch(currentState)
                {
                    case GameState::PLAYING:
                    case GameState::GAME_OVER:
                        // Return To Main Menu
                        currentState = GameState::MAIN_MENU;
                        break;
                    case GameState::MAIN_MENU:
                        // Exit The Game
                        glfwSetWindowShouldClose(window, true);
					    break;
			    }

                glfwSetWindowShouldClose(window, true);

            }

            // Handle Other Key Presses
            if (action == GLFW_PRESS) 
            {

                switch (currentState) 
                {
                    case GameState::MAIN_MENU:
                        switch (key) 
                        {
                            case GLFW_KEY_ENTER:
                                this->needsRedraw = true;
                                this->update();
                                currentState = GameState::PLAYING;
                                break;
                        }
                        break;

                    case GameState::PLAYING:
                        // Game Controls
                        switch (key) 
                        {
                            case GLFW_KEY_SPACE:
                                addScore(CONFIG::SPACE_BAR_POINTS);
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
                        switch (key) 
                        {
                            case GLFW_KEY_ENTER:
                                currentState = GameState::MAIN_MENU;
						        // Reset The Game State For Next Game
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


        /*

			Desc: Sets Up The Game Background Quad For Rendering. It Creates A Fullscreen Quad
			And The Winding Order Of Rendering. It Also Creates Our Vertex Array Object (VAO),
			Vertex Buffer Object (VBO), And Element Buffer Object (EBO) For The Background Quad.
            
            Preconditions:
                1.) OpenGL Context Is Initialized And Ready For Rendering Binding.

           Postconditions:
                1.) Creates A Fullscreen Quad For The Background.
                2.) Sets Up Vertex Attributes For Position And Color.
				3.) If Invalid OpenGL Context, Won't Do Any Setup
                4.) The Vertex Buffers Will Be Linked And Properly Contextualized To A VAO

        */
        void setupGameBackground()
        {

            float vertices[] = {
                -1.0f, -1.0f, 0.0f, 1.0f, 0.5f, 0.0f,   // Bottom Left
                1.0f, -1.0f, 0.0f,  0.0f, 0.5f, 0.0f,   // Bottom Right
                1.0f, 1.0f, 0.0f,   0.0f, 0.85f, 0.6f,  // Top Right
                -1.0f, 1.0f, 0.0f,  0.4f, 0.232f, 0.0f  // Top Left
            };

            unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

            // Generate Our Buffers
            glGenVertexArrays(1, &backgroundVAO);
            glGenBuffers(1, &backgroundVBO);
            glGenBuffers(1, &backgroundEBO);

            glBindVertexArray(backgroundVAO);

            // Link Data
            glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundEBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

			// Contextualize Vertex Attributes
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);

        }


        /*

            Desc: Creates A Simple Procedural Font Texture Atlas For Rendering Text.
            It Generates A Texture Atlas With Characters 0-9, S, c, o, r, e, :, space, L, v
            And Stores It In The fontTextureID OpenGL Texture Object.

            Preconditions:
                1.) OpenGL Context Is Initialized And Ready For Texture Creation.
                2.) ATLAS_SIZE Is Defined As The Size Of The Texture Atlas (128x128).
				3.) CHAR_PIXEL_SIZE Is Defined As The Size Of Each Character In The Atlas (8x8).
				4.) CHAR_SIZE Is Defined As The Size Of Each Character In UV Coordinates (1.0f / 16.0f).
				5.) fontTextureID Will Contain The OpenGL Texture ID For The Font Atlas.

            Postconditions:
                1.) Creates A Font Texture Atlas For Rendering Text.
				2.) The Texture Atlas Will Be Used For Rendering Text In The Game.
				3.) The Text Atlas Will Be Held In The fontTextureID OpenGL Texture Object.

        */
        void createFontTexture()
        {

            // Create A Simple Procedural Font Texture (8x8 Pixels Per Character)
            unsigned char fontData[CONFIG::FONT_ATLAS_SIZE * CONFIG::FONT_ATLAS_SIZE];
            memset(fontData, 0, sizeof(fontData));

            // Define Simple Bitmap Patterns For Digits 0-9
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

            // Letter Patterns For Our UI Text
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

            // Copy Digit Patterns To Texture (First Row)
            for (int digit = 0; digit < 10; digit++) {
                int baseX = digit * CHAR_PIXEL_SIZE;
                int baseY = 0;

                for (int y = 0; y < 8; y++) {
                    unsigned char row = digitPatterns[digit][y];
                    for (int x = 0; x < 8; x++) {
                        int pixelX = baseX + x;
                        int pixelY = baseY + y;

                        if (row & (1 << (7 - x))) 
                        {
                            fontData[pixelY * ATLAS_SIZE + pixelX] = 255;
                        }
                    }
                }
            }

            // Copy Letter Patterns To Texture (Starting At Position 10)
            for (int letter = 0; letter < 28; letter++) {
                int charIndex = 10 + letter;
                int baseX = (charIndex % 16) * CHAR_PIXEL_SIZE;
                int baseY = (charIndex / 16) * CHAR_PIXEL_SIZE;

                for (int y = 0; y < 8; y++) {
                    unsigned char row = letterPatterns[letter][y];
                    for (int x = 0; x < 8; x++) {
                        int pixelX = baseX + x;
                        int pixelY = baseY + y;

                        if (row & (1 << (7 - x))) 
                        {
                            fontData[pixelY * ATLAS_SIZE + pixelX] = 255;
                        }
                    }
                }
            }

            // Create OpenGL Texture
            glGenTextures(1, &fontTextureID);
            glBindTexture(GL_TEXTURE_2D, fontTextureID);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, ATLAS_SIZE, ATLAS_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, fontData);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            glBindTexture(GL_TEXTURE_2D, 0);

        }


        /*
 
			Desc: Sets Up The Main Window's Text Rendering VAO, VBO, and EBO For Text Rendering.
            The Text For Each Window Will Be Held In This Given Set Of Text Buffers And Will Be Cycled
            Out To Avoid Excessive Specialized Members For Each Window State.

            Preconditions:
                1.) textVAO, textVBO, textEBO Will Be A OpenGL Buffers For Text Rendering.
                2.) fontTextureID Will Be A OpenGL Texture ID For The Font Atlas.

            Postconditions:
				1.) Sets Up The Text VAO, VBO, and EBO For Text Rendering.
				2.) Contextualize The Vertex Attributes For Position And UV Coordinates.

		*/ 
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


        /*

			Desc: Maps The Current Character Index For The Given Character Provided Located 
			In Our Texture Atlas.

            Preconditions:
				1.) c Is A Valid Character To Map To An Index.

		   Postconditions:
				1.) Returns The Index Of The Character In The Texture Atlas.
				2.) If Character Is Not Found, Returns 16 (Space Index).

        */
        int getCharIndex(const char& c) const
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
                case 'd': return 34;  
                case 'b': return 35; 
                case '.': return 36;
                default: return 16; // Default to space
            }

        }


        /*

            Desc: Adds A Specific Character To The Text Buffer For Rendering 
.           Based Upon Its Provided Location (x,y) In Normalized Space [0.0f, 1.0f]
            And The Scale Factor For The Character Size.

            Preconditions:
                1.) c Is A Valid Character To Add To The Text Buffer.
                2.) x, y Are Normalized Coordinates In The Range [0.0f, 1.0f].
                3.) scale Is A Valid Scale Factor For The Character Size.
				4.) textVertices, textIndices Are Valid Buffers For Text Rendering.

            Postconditions:
                1.) Adds The Character's Vertices And UV Coordinates To The Text Buffers.
				2.) If Character Is A Space, It Will Be Skipped.
				3.) If Invalid Character, Will Add Space Instead.

        */
        void addCharToText(const char& c, const float& x, const float& y, const float& scale)
        {

            if (c == ' ') return; // Skip Spaces

            int charIndex = getCharIndex(c);

            // Calculate UV Coordinates In Texture Atlas
            float uvX = (charIndex % 16) * CHAR_SIZE;
            float uvY = (charIndex / 16) * CHAR_SIZE;
            float uvW = CHAR_SIZE;
            float uvH = CHAR_SIZE;

            // Character Dimensions
            float charW = 0.04f * scale;
            float charH = 0.06f * scale;

            unsigned int baseIndex = textVertices.size() / 4;

            // Add 4 vertices (position + UV)
            textVertices.insert(textVertices.end(), {
                x, y, uvX, uvY + uvH,                    // Bottom Left
                x + charW, y, uvX + uvW, uvY + uvH,      // Bottom Right
                x + charW, y + charH, uvX + uvW, uvY,    // Top Right
                x, y + charH, uvX, uvY                   // Top Left
                });

            // Add Indices For Two Triangles
            textIndices.insert(textIndices.end(), {
                baseIndex, baseIndex + 1, baseIndex + 2,
                baseIndex + 2, baseIndex + 3, baseIndex
                });

        }


        /*

            Desc: Updates The Current Text Content For The Main Game Loop, Updating
			The Score , Level, and Lines Cleared Text Buffers. It Will Rebuild The Text
            Buffers If The Score, Level, or Lines Cleared Have Changed Since The Last Update.

            Preconditions:
                1.) textNeedsUpdate Is True If The Text Buffers Need To Be Updated.
				2.) score, level, linesCleared Are Valid Game State Values.

            Postconditions:
				1.) Updates The Text Buffers With The Current Score, Level, and Lines Cleared.
				2.) If No Changes, Will Not Update The Buffers.
    
        */
        void updateTextContent()
        {

            if (!textNeedsUpdate) return;

            textVertices.clear();
            textIndices.clear();

            float charSpacing = CONFIG::CHAR_SPACING;
            float currentX, currentY;

            // Add "Score:" Label
            currentX = -0.95f;
            currentY = 0.85f;
            std::string scoreLabel = "Score:";
            for (char c : scoreLabel) {
                addCharToText(c, currentX, currentY, 1.0f);
                currentX += charSpacing;
            }

            // Add Score Digits
            currentX = -0.65f;
            std::string scoreStr = std::to_string(score);
            for (char c : scoreStr) {
                addCharToText(c, currentX, currentY, 1.0f);
                currentX += charSpacing;
            }

            // Add "Level:" Label
            currentX = -0.95f;
            currentY = 0.75f;
            std::string levelLabel = "Level:";
            for (char c : levelLabel) {
                addCharToText(c, currentX, currentY, 1.0f);
                currentX += charSpacing;
            }

            // Add Level Digits
            currentX = -0.65f;
            std::string levelStr = std::to_string(level);
            for (char c : levelStr) {
                addCharToText(c, currentX, currentY, 1.0f);
                currentX += charSpacing;
            }

            // Add "Lines:" Label
            currentX = -0.95f;
            currentY = 0.65f;
            std::string linesLabel = "Lines:";
            for (char c : linesLabel) {
                addCharToText(c, currentX, currentY, 1.0f);
                currentX += charSpacing;
            }

            // Add Lines Digits
            currentX = -0.65f;
            std::string linesStr = std::to_string(linesCleared);
            for (char c : linesStr) {
                addCharToText(c, currentX, currentY, 1.0f);
                currentX += charSpacing;
            }

            // Update GPU Buffers
            if (!textVertices.empty()) 
            {
                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndices.size() * sizeof(unsigned int), textIndices.data(), GL_DYNAMIC_DRAW);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

			// Reset Update Flag
            textNeedsUpdate = false;

			// Update Last Score, Level, and Lines Cleared
            lastScore = score;
            lastLevel = level;
            lastLines = linesCleared;

        }


        /*

            Desc: Cleans Up All OpenGL Resources, GLFW Window, and Shaders.
            It Will Delete All VAOs, VBOs, EBOs, Textures, and Shaders Used By The Window.

            Preconditions:
                1.) OpenGL GLFW Context Is Initialized And Ready For Cleanup.
                2.) window Is A Valid GLFWwindow Pointer.

            Postconditions:
                1.) Cleans Up All OpenGL Resources Used By The Window.
                2.) Sets isInit To False To Indicate The Window Is No Longer Initialized.

		*/
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

                if (fontTextureID) 
                {
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

        /*

            Desc: Constructor For The Window Class. It Initializes The Window With Given Width, Height,
            and Title. It Also Initializes The Game Board With Default Values.

            Preconditions:
                1.) w, h Are Valid Unsigned Integers For Width and Height.
                2.) windowTitle Is A Valid C-String For The Window Title.

            Postconditions:
                1.) Initializes The Window Pointer To nullptr.
                2.) Sets Up Default Values For Width, Height, Title, Score, Level, Lines Cleared.
				3.) Initializes OpenGL Buffers For Background and Text Rendering.


        */
        Window(unsigned int w = CONFIG::WINDOW_WIDTH, unsigned int h = CONFIG::WINDOW_HEIGHT, const char* windowTitle = CONFIG::WINDOW_TITLE)
            : window(nullptr), width(w), height(h), title(windowTitle),
            score(0), level(1), linesCleared(0), isInit(false),
            backgroundVAO(0), backgroundVBO(0), backgroundEBO(0),
            textVAO(0), textVBO(0), textEBO(0), fontTextureID(0),
		    gameBoard(CONFIG::BOARD_ROWS, CONFIG::BOARD_COLS, w, h, this->score, this->textNeedsUpdate, this->linesCleared, this->level, this->gameOverFlag) 
        {

            textVertices.reserve(1000);
            textIndices.reserve(1500);

        }

        unsigned int tickCount = 0;


        /*

			Desc: Updates The Game State And Renders The Game Board. Will Step The Game Board One Tick,
            Keeping Tabs On The tickCount For Controlled Updates.

            Preconditions:
                1.) OpenGL Context Of Our gameBoard Is Initialized And Ready For Rendering.
				2.) gameBoard Is A Valid Game Board Object Initialized With The Current Game State.

           Postconditions:
				1.) Updates The Game Board State By Stepping It One Tick.
				2.) If Window Is Not Initialized, Will Print An Error Message.
				3.) gameBoard Will Be Ready For Rendering The Next Frame.

        */
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


        /*
        
            Desc: Will Utilize The textShader To Render The Text Located In The textVAO, textVBO, textEBO,
            Utilizing The Provided Texture Atlas In fontTextureID.

            Preconditions:
                1.) fontTextureID Is Bound To A Font Texture Atlas
                2.) textVAO, textVBO & textEBO Are Bound To Data
            
            Postconditions:
                1.) Will Render The Text In The Vertex Arrays To The Screen
                2.) Returns Error If There Isn't A Font Texture Atlas Coupled To fontTextureID
    
        */
        void renderText()
        {

            if (!textIndices.empty())
            {
                textShader.use();

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fontTextureID);
                GLint textureLocation = glGetUniformLocation(textShader.getProgID(), "fontTexture");

                if (textureLocation != -1)
                {
                    glUniform1i(textureLocation, 0);
                }
                else 
                {
                    std::cerr << "Failed to set font texture uniform." << std::endl;
                }

                glBindVertexArray(textVAO);
                glDrawElements(GL_TRIANGLES, textIndices.size(), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);

            }

        }


        /*

            Desc: Will Initialize The GLFW & GLAD Context As Well As Window And It's Associated Event Handlers
            As Well As Vertex & Shader Data.

            Preconditions:
                1.) User Has GLFW & GLAD Locally Installed & In Scope
            
            Postconditions:
                1.) Will Initialize Our Window & It's Dependencies (GLFW & GLAD)
                2.) Will Initialize Our Window's Board Instance
                3.) If Our Window Is Already Initialized, Return False & Status
                4.) If Cannot Initialize GLFW, Return False & Status
                5.) If Cannot Intialize GLAD, Return False & Status            

        */
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

            // Enable Blending Tor Text Transparency
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Load Shaders
            if (!backgroundShader.loadShader(CONFIG::SHADERS::BACKGROUND_VERTEX, CONFIG::SHADERS::BACKGROUND_FRAGMENT)) 
            {
                std::cerr << "Failed to load background shader" << std::endl;
                return false;
            }

            if (!textShader.loadShader(CONFIG::SHADERS::TEXT_VERTEX, CONFIG::SHADERS::TEXT_FRAGMENT)) 
            {
                std::cerr << "Failed to load text shader" << std::endl;
                return false;
            }

            // Setup Rendering
            setupGameBackground();
            createFontTexture();
            setupTextRendering();

		    // Initialize Game Board
            glBindVertexArray(backgroundVAO);
		    gameBoard.initialize();
            this->updateBackgroundVertexColorRandom();
            this->setupMainMenuText();

            this->isInit = true;
            std::cout << "Window Initialized Successfully." << std::endl;
            return true;

        }


        /*
        
            Desc: Initialize Our Main Menu's Background Vertex Data.

            Preconditions:
                1.) backgroundVAO & backgroundVBO Are Initialized
            
            Postconditions:
                1.) Will Emplace A Known Background Into The Vertex Buffer For Rendering
    
        */
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


        /*
        
            Desc: Will Set-Up The Game Over Text For The Game Over Menu, This Will
            Include Game Over Text As Well As The User's Score, And The Scoreboard.
            This Data Will Be Stored In The textVBO.
            
            Preconditions:
                1.) textVAO, textVBO & textEBO Are Allocated Buffers
                2.) leaderboard.txt Contains The Top Scoring In Sorted Order
            
            Postconditions:
                1.) The Game Over Text Will Be Emplaced In The textVBO & textEBO For Rendering
                2.) The Previous Text Held Will Be Cleared From textVertices & textIndices
                3.) If leaderboards.txt Is Is Non-Existent, Will Return & Notify

        */
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

		    if (leaderboardFile.is_open()) 
            {

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
            if (!textVertices.empty()) 
            {
                glBindBuffer(GL_ARRAY_BUFFER, textVBO);
                glBufferData(GL_ARRAY_BUFFER, textVertices.size() * sizeof(float), textVertices.data(), GL_DYNAMIC_DRAW);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, textEBO);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, textIndices.size() * sizeof(unsigned int), textIndices.data(), GL_DYNAMIC_DRAW);
			    glBindBuffer(GL_ARRAY_BUFFER, 0);
            }

        }


        /*
        
            Desc: Will Set-Up The Text For The Main Menu On The Bottom Left Corner Of The User's
            Window.

            Preconditions:
                1.) textVAO, textVBO & textEBO Are Allocated Buffers

            Postconditions:
                1.) The Main Menu Text Will Be Emplaced In The textVBO & textEBO For Rendering
                2.) The Previous Text Held Will Be Cleared From textVertices & textIndices

        */
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


        /*

            Desc: Will Render The Main Menu Of Our Game, Different From The Game Background
            Rendering As Our Main Menu Rendering Is Static.

            Preconditions:
                1.) backgroundShader Is Bound To A Valid .vert & .frag Shader
                2.) The backgroundVAO, backgroundVBO & backgroundEBO Are Holding Valid Vert. Data

            Postconditions:
                1.) Will Render To The Window A Static Background Color Located Within Buffers
                2.) Will Enable The backgroundShader To This Thread

        */
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


        /*

            Desc: Will Be A Flag Check To See If We Should Close Our GLFW Window And Start Cleanup 
            Externally To The Window's Scope.
            
            Preconditions:
                1.) window Is Coupled To A GLFW Instance

            Postconditions:
                1.) Will Return True If User Wants To Close The Given Window
                2.) Will Return False If User Didn't Specify To Close The Window
    
        */
        bool shouldClose() const
        {

            return window ? glfwWindowShouldClose(this->window) : true;
        
        }


        /*
        
            Desc: Swaps The Back And Front Buffer Of Our Rendering When Called;
            This IS Only Done When We Have A Valid GLFW Window Instance.

            Preconditions:
                1.) window Should Be A Bound GLFW Instance

            Postconditions:
                1.) If There Is A GLFW Instance, It Will Swap The Buffers
                2.) If There Isn't A GLFW Instance, No Swapping Will Happen

        */
        void swapBuffers()
        {

            if (this->window)
            {
                glfwSwapBuffers(this->window);
            }

        }


        /*

            Desc: Will Poll Events Happening To The Given Window Instance For Handling.

            Preconditions:
                1.) window Should Be A Bound GLFW Instance

            Postconditions:
                1.) If There Is A GLFW Window, Will Poll & Handle Events
                2.) If No GLFW Window, Will Do Nothing

        */
        void pollEvents()
        {

            if (this->window)
            {
                glfwPollEvents();
            }

        }


        /*
        
            Desc: Will Wipe The Back Buffer Of The Contextualized GLFW Window
            With A Flat Grey Color.

            Preconditions:
                1.) GLFW Is Instantiated

            Postconditions:
                1.) Will Clear The Back Buffer, Filling It With A Flat Grey Color

        */
        void clear()
        {

            glClearColor(CONFIG::COLORS::CLEAR_R, CONFIG::COLORS::CLEAR_G, CONFIG::COLORS::CLEAR_B, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        }
    
        
        /*

            Desc: Will Render The Game Background With It's Color Shifting Animation
            As Well As Rendering The gameBoard Of Tetris Being Played Currently.

            Preconditions:
                1.) gameBoard Is Initialized
                2.) backgroundShader Is Initialized With bgColor Uniform For Coloring

            Postconditions:
                1.) Will Render The Game Menu Background With Its Shifting Color
                2.) Will Render The Game Board
                3.) Increment currentR, currentG, currentB Members

        */
        void renderGameBackground()
        {

            backgroundShader.use();

            // Slowly move current color toward target color
            currentR += (targetR - currentR) * CONFIG::COLORS::BACKGROUND_TRANSITION_SPEED;
            currentG += (targetG - currentG) * CONFIG::COLORS::BACKGROUND_TRANSITION_SPEED;
            currentB += (targetB - currentB) * CONFIG::COLORS::BACKGROUND_TRANSITION_SPEED;

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
                colorChangeTimer = CONFIG::COLORS::COLOR_CHANGE_TIMER_MIN +
					(std::rand() % (CONFIG::COLORS::COLOR_CHANGE_TIMER_MAX - CONFIG::COLORS::COLOR_CHANGE_TIMER_MIN));
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


        /*

            Desc: Will Render The Game Menu Text, Ensuring Repeat Render Calls For Text Don't  
            Recompute Expensive Procedural Text Rendering.

            Preconditions:
                1.) textShader Is Initialized To UV-Rendered Text Shaders
                2.) Text Data Is Pointed To By Our Vertex Arrays
                3.) fontTexture Is Utilized In The Text Shader For Onboarding Atlas
                4.) fontTextureID Holds Given Font Texture Atlas

            Postconditions:
                1.) Will Render To Screen The Given UI Text For Game Menu State
                2.) Will Only Update Text If We Have Changes In Scoring

        */
        void renderUI()
        {

            // Check if text needs updating
            if (score != lastScore || level != lastLevel || linesCleared != lastLines)
            {
                textNeedsUpdate = true;
            }

            updateTextContent();

            if (!textIndices.empty()) 
            {
                textShader.use();

                // Bind font texture
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, fontTextureID);
                GLint texLoc = glGetUniformLocation(textShader.getProgID(), "fontTexture");
                if (texLoc >= 0)
                {
                    glUniform1i(texLoc, 0);
                }

                glBindVertexArray(textVAO);
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(textIndices.size()), GL_UNSIGNED_INT, 0);
                glBindVertexArray(0);
            }

        }


        /*

            Desc: Will Update Our Scoring In The Game Menu.

            Preconditions:
                1.) None
            
            Postconditions:
                1.) score Will Be Updated Only When It's A New Score
                2.) textNeedsUpdate Will Only Be Flagged When Changes Occur

        */
        void setScore(const unsigned int& newScore) 
        {

            if (score != newScore)
            {
                score = newScore;
                textNeedsUpdate = true;
            }

        }


        /*

            Desc: Increments Our User's score When In The Game Menu.

            Preconditions:
                1.) [score + points] Shouldn't Be In Unsigned Overflow Territory
        
            Postconditions:
                1.) score Will Be Incremented By points
                2.) textNeedsUpdate Will Be Flagged

        */
        void addScore(const unsigned int& points) 
        {

            score += points;
            textNeedsUpdate = true;

        }


        /*

            Desc: Will Write A Given Score Into leaderboard.txt In Place To Where
            It Resides In The Scoring Keeping Sorted, Descending Order Of 
            leaderboard.txt.

            Preconditions:
                1.) leaderboard.txt Is A File Formatted With Each Line Having Unsigned Ints
            
            Postconditions:
                1.) Will Update leaderboard.txt In Order With The New Score, points 

        */
        void addToScoreboard(const unsigned int& points)
        {

		    std::vector<unsigned int> scores;
            scores.reserve(CONFIG::MAX_LEADERBOARD_ENTRIES + 1);

		    std::ifstream leaderboardFile(CONFIG::LEADERBOARD_FILE);
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

		    if (!scoreInserted)
		    {
			    scores.push_back(points);
            }

            // Write Back To File
		    std::ofstream outFile(CONFIG::LEADERBOARD_FILE);
            if (outFile.is_open()) {
         
				for (size_t i = 0; i < scores.size() && i < CONFIG::MAX_LEADERBOARD_ENTRIES; ++i) 
                {
                    outFile << score << std::endl;
			    }

                outFile.close();
            } else {
			    std::cerr << "Failed to open leaderboard file for writing." << std::endl;
  
            }
        
        }

        
        /*
        
            Desc: Sets The Given Level Of Our Game Of Tetris.
        
            Preconditions:
                1.) None

            Postconditions:
                1.) Will Update level IFF Its a New Scoring
                2.) textNeedsUpdate Will Be Flagged If Changes Occur
        
        */
        void setLevel(const unsigned int& newLevel) 
        {

            if (level != newLevel)
            {
                level = newLevel;
                textNeedsUpdate = true;
            }

        }


        /*
        
            Desc: Will Update The Amount Of Lines Cleared In A Game Of Tetris

            Preconditions:
                1.) None
    
            Postconditions:
                1.) Will Update level IFF Its a New Scoring
                2.) textNeedsUpdate Will Be Flagged If Changes Occur
        
        */
        void setLinesCleared(const unsigned int& lines) 
        {

            if (linesCleared != lines)
            {
                linesCleared = lines;
                textNeedsUpdate = true;
            }

        }


        // Getters & Setters
        unsigned int getScore() const { return score; }
        unsigned int getLevel() const { return level; }
        unsigned int getLinesCleared() const { return linesCleared; }
	    GameState getCurrentState() const { return currentState; }
	    void setCurrentState(GameState state) { currentState = state; }
	    bool isGameOver() const { return gameOverFlag; }


        /*

            Desc: Will Reset/Default The Tetris Game State Variables For 
            After A Given Game Has Concluded.

            Preconditions:
                1.) Expects Vertex Arrays To Be Initialized

            Postconditions:
                1.) Will Update All Essential Data For Tetris Game Menu Logic To Their Defaults

        */
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


        /*

            Desc: Will Receive The Given Drop Speed Of Shapes In The Game Of 
            Tetris Based On A Quadratic Increase Curve; Maxing Out At Level 10
            For Speed.

            Preconditions:
                1.) None

            Postconditions:
                1.) Will Return The Amount Of Ticks That Should Elapse Before Dropping One Row
        
        */
        unsigned int getDropSpeed()
        {

            float normLevel = std::min(level / (float)CONFIG::MAX_LEVEL, 1.0f);
		    float curve = normLevel * normLevel; // Quadratic curve for speed increase
            return CONFIG::BASE_DROP_SPEED - (curve * CONFIG::SPEED_CURVE_FACTOR);


        }


        /*

            Desc: The Cleanup Routine For Our GLFW Instance, Including Orchestrating 
            The Closure Of Our gameBoard.
        
        */
        ~Window()
        {

            cleanup();
            std::cout << "Window Resources Cleaned Up." << std::endl;

        }

};