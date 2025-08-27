#pragma once


#include "glm/glm.hpp"
#include <queue>
#include <glad/glad.h>
#include <vector>
#include <iostream>
#include "block.h"
#include "config.h"

/*

    Desc: Contains All Given Shapes We Can Expect To Drop And Utilize
    In A Given Game Of Tetris.

*/
static const std::vector<std::vector<bool>> shapes[] =
{

    { {1, 1, 1, 1} },           // I Shape
    { {1, 1}, {1, 1} },         // O Shape
    { {0, 1, 0}, {1, 1, 1} },   // T Shape
    { {1, 1, 0}, {0, 1, 1} },   // S Shape
    { {0, 1, 1}, {1, 1, 0} },   // Z Shape
    { {1, 0, 0}, {1, 1, 1} },   // L Shape
    { {0, 0, 1}, {1, 1, 1} }    // J Shape

};



/*

    Desc: Class Is Utilized To Contain All Data Pertaining To
    The Visual, As Well As Logistical Pieces Of Our Game Of Tetris.

*/
class Board
{

    private:

        // Current Game Of Tetris
        Block** gameGrid;
        std::vector<std::vector<bool>> upcomingShape;
        unsigned int activePieceRow, activePieceCol; // Position Of The Active Piece On The Board
        glm::vec3 activePieceColor;                  // Color Of The Active Piece
        unsigned int& score;
        unsigned int& totalRowsCleared;
        unsigned int& level;

        // Window Displaying Next Up Shape
        Block** previewGrid;
        static const unsigned int PREVIEW_ROWS = 4;
        static const unsigned int PREVIEW_COLS = 4;
        unsigned int previewGridIndices;            // Indices For Preview Grid Lines
        unsigned int previewCellIndices;            // Indices For Preview Cells
        std::vector<std::vector<bool>> nextShape;
        glm::vec3 nextShapeColor;

        // Tetris Game Window Sizing
        unsigned int rows, cols;
        unsigned int width, height;                 // Width And Height Of The Board In Pixels

        // Rendering 
        GLuint boardVBO, boardEBO;
        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        unsigned int boardIndices;                  // Indices For Board Background
        unsigned int gridLineIndices;               // Indices For Grid Lines
        unsigned int cellIndices;                   // Indices For All Blocks

        // Update Flags
        bool& updateText;
        bool& gameOver;


        /*

            Desc: Randomly Selects One Of Our Available Shapes To Be The
            Next Shape To Come Up After The Current Shape Is Dropped
            (Next In The previewGrid) It Also Will Randomly Select A Color
            For It

            Preconditions:
                1.) None

            Postconditions:
                1.) This->nextShape Will Be Now A Random Shape From shapes
                2.) This->nextShape Will Be Colored A Random Color


        */
        void generateNextShape()
        {
        
            auto tmp = shapes[std::rand() % 7];
            nextShape = std::vector<std::vector<bool>>(tmp.begin(), tmp.end());
            nextShapeColor = glm::vec3((std::rand() % 256) / 255.0f, (std::rand() % 256) / 255.0f, (std::rand() % 256) / 255.0f);
        
        }


        /*

            Desc: Function Will Swap Out The Shape Shown In This->previewGrid To Now
            Be The Active Shape Being Dropped On The Tetris Game Checking If We
            Have A Game Over Based On If It Is Able To Place The Given Shape
            On The Grid. Likewise, Will Render The Ghost Piece For The Given Shape.

            Preconditions:
                1.) This->nextShape Contains A Given Shape
                2.) This->gameGrid Is Initialized

            Postconditions:
                1.) Will Swap Out This->nextShape To Active, This->upcomingShape
                2.) This->nextShape Will Be Set To A New Shape, Updating This->previewGrid
                3.) If Game Isn't Over, Will Color Origin With This->upcomingShape
                4.) If Game Isn't Over, Will Render Ghost Piece
                5.) If Game Is Over, Will Return False And Update Parent's Flag
                6.) If Game Isn't Over, Will Return True

        */
        bool startNewPiece()
        {

            // Move Next Shape To Current
            upcomingShape = nextShape;
            activePieceColor = nextShapeColor;

            // Generate New Next Shape
            generateNextShape();
            updatePreviewDisplay();

            // Set Starting Position (Center Top)
            activePieceRow = 0;
            activePieceCol = cols / 2 - upcomingShape[0].size() / 2;

            // Check If We Can Place The New Shape (Game Over Check)
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j])
                    {
                        unsigned int targetRow = activePieceRow + i;
                        unsigned int targetCol = activePieceCol + j;

                        if (targetRow >= rows || targetCol >= cols ||
                            gameGrid[targetRow][targetCol].occupied == OCCUPIED)
                        {
                            // Game Over
                            gameOver = true;
                            return false;
                        }
                    }
                }
            }

            // Place The New Shape On Board
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j])
                    {
                        setCellColor(activePieceRow + i, activePieceCol + j, activePieceColor);
                    }
                }
            }

            // Generate Ghost Piece
            ghostPiece();
            return true;

        }


    public:

        /*

            Desc: Takes In The Amount Of Rows And Columns Our Game Of Tetris Will Use
            As Well As The Sizing; Also The Parent References To Our Top-Level Window's
            Members For Updating Game State.

            Preconditions:
                1.) Rows And Cols Are Valid Positive Integers For Grid Dimensions.
                2.) Width And Height Are Valid Screen Dimensions.
                3.) Reference Parameters Are Valid And Accessible.

            Postconditions:
                1.) Initializes Game Grid And Preview Grid With Proper Dimensions.
                2.) Sets Up References To Parent Window's Game State Variables.
                3.) Seeds Random Number Generator For Shape Generation.
                4.) Generates Initial Next Shape For Preview Display.

        */
        Board(unsigned int rows, unsigned int cols, unsigned int width, unsigned int height
            , unsigned int& score, bool& textUpdate, unsigned int& linesCleared, unsigned int& curLevel, bool& gameOverFlag)
            : score(score), updateText(textUpdate),
            rows(rows), cols(cols), width(width), height(height), boardVBO(0), boardEBO(0),
            boardIndices(0), gridLineIndices(0), cellIndices(0), totalRowsCleared(linesCleared),
            level(curLevel), gameOver(gameOverFlag), previewGridIndices(0), previewCellIndices(0)
        {

            // Seed Our Random Number Generator
            std::srand(static_cast<unsigned int>(std::time(nullptr)));

            // Initialize Our Main Game Grid
            gameGrid = new Block * [rows];
            for (unsigned int i = 0; i < rows; ++i) {
                gameGrid[i] = new Block[cols];
                for (unsigned int j = 0; j < cols; ++j) {
                    gameGrid[i][j] = { 0, 0, EMPTY };
                }
            }

            // Initialize Preview Grid
            previewGrid = new Block * [PREVIEW_ROWS];
            for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
                previewGrid[i] = new Block[PREVIEW_COLS];
                for (unsigned int j = 0; j < PREVIEW_COLS; ++j) {
                    previewGrid[i][j] = { 0, 0, EMPTY };
                }
            }

            // Generate The First Shape
            generateNextShape();

        }


        /*

            Desc: Used For Resetting The Entire Game State (Mainly To Be Utilized As
            A Cleanup Routine For After Game Over Happens) It Will Clear The Game Grid,
            As Well As The Next Upcoming Shape

            Preconditions:
                1.) Should Be Called After Game Over Occurs

            Postconditions:
                1.) Will Reset This->gameGrid To Default
                2.) Clears Out The Current Upcoming Shape To Be Dropped (This->upcomingShape)
                3.) Will Grab A Upcoming Shape For Next Game For Preview Window (This->nextShape)

        */
        void reset()
        {
            // Reset The Game Grid
            for (unsigned int i = 0; i < rows; ++i) {
                for (unsigned int j = 0; j < cols; ++j) {
                    gameGrid[i][j] = { 0, 0, EMPTY };
                }
            }
            // Clear The Upcoming Shape
            upcomingShape.clear();

            // Generate New Next Shape
            generateNextShape();

        }


        /*

            Desc: Sets Up Our Board For A Given Game Of Tetris By Binding Our
            Vertex-Arrays, Including The Cells For The Game Of Tetris, As Well
            As The Delimiter Lines That Must Be Rendered To Make The Game
            Look Grid-Like.

            Preconditions:
                1.) This->width, This->height, This->cols & This->rows Are Initialized

            Postconditions:
                1.) Will Bind Up This->boardVAO, This->boardVBO & This->boardEBO
                2.) All Vertex Arrays Will Be Populated For Rendering Tetris Grid
                3.) Will Initialize Our Preview Window For Rendering

        */
        void initialize()
        {
            // Create Board Rectangle - Positioned To Center-Right Of Screen
            float boardWidth = 0.85f;
            float boardHeight = 1.5f;
            float boardX = 0.0f;
            float boardY = 0.0f;

            // Calculate Grid Cell Dimensions
            float cellWidth = boardWidth / cols;
            float cellHeight = boardHeight / rows;

            // Calculate The Starting Position (Top-Left Of The Board)
            float startX = boardX - boardWidth / 2.0f;
            float startY = boardY + boardHeight / 2.0f;

            // Clear The Vectors
            vertices.clear();
            indices.clear();

            unsigned int baseIndex = 0;

            // 1. Add Board Outline Vertices
            // Bottom-Left
            vertices.push_back(startX);
            vertices.push_back(startY - boardHeight);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);

            // Bottom-Right
            vertices.push_back(startX + boardWidth);
            vertices.push_back(startY - boardHeight);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);

            // Top-Right
            vertices.push_back(startX + boardWidth);
            vertices.push_back(startY);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);

            // Top-Left
            vertices.push_back(startX);
            vertices.push_back(startY);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);

            // Add Indices For The Board Rectangle
            indices.push_back(0);
            indices.push_back(1);
            indices.push_back(2);
            indices.push_back(2);
            indices.push_back(3);
            indices.push_back(0);

            boardIndices = 6;  // We Used 6 Indices For The Board
            baseIndex = 4;     // We Used 4 Vertices For The Board

            // 2. Generate Grid Lines
            float gridR = 0.3f, gridG = 0.3f, gridB = 0.3f;

            // Vertical Grid Lines
            for (unsigned int i = 0; i <= cols; i++) {
                float x = startX + i * cellWidth;

                // Top Vertex Of The Line
                vertices.push_back(x);
                vertices.push_back(startY);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Bottom Vertex Of The Line
                vertices.push_back(x);
                vertices.push_back(startY - boardHeight);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Add Indices For This Line
                indices.push_back(baseIndex);
                indices.push_back(baseIndex + 1);

                baseIndex += 2;
            }

            // Horizontal Grid Lines
            for (unsigned int i = 0; i <= rows; i++) {
                float y = startY - i * cellHeight;

                // Left Vertex Of The Line
                vertices.push_back(startX);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Right Vertex Of The Line
                vertices.push_back(startX + boardWidth);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Add Indices For This Line
                indices.push_back(baseIndex);
                indices.push_back(baseIndex + 1);

                baseIndex += 2;
            }

            gridLineIndices = (cols + 1) * 2 + (rows + 1) * 2;  // Two Indices Per Line

            // 3. Generate Cells For Each Grid Position
            float cellPadding = 0.005f;  // Small Padding To See The Grid Lines

            for (unsigned int row = 0; row < rows; row++) {
                for (unsigned int col = 0; col < cols; col++) {
                    // Calculate Cell Position With Padding
                    float cellX = startX + col * cellWidth + cellPadding;
                    float cellY = startY - row * cellHeight - cellPadding;
                    float cellW = cellWidth - 2 * cellPadding;
                    float cellH = cellHeight - 2 * cellPadding;

                    // Default Color (Transparent/Black)
                    float r = 0.0f, g = 0.0f, b = 0.0f;

                    // Store Where This Cell's Vertices Begin In The VBO
                    gameGrid[row][col].vboOffset = vertices.size() / 6;  // 6 Floats Per Vertex
                    gameGrid[row][col].indexOffset = indices.size();

                    // Bottom-Left
                    vertices.push_back(cellX);
                    vertices.push_back(cellY - cellH);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    // Bottom-Right
                    vertices.push_back(cellX + cellW);
                    vertices.push_back(cellY - cellH);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    // Top-Right
                    vertices.push_back(cellX + cellW);
                    vertices.push_back(cellY);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    // Top-Left
                    vertices.push_back(cellX);
                    vertices.push_back(cellY);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    // Add Indices For This Cell
                    indices.push_back(baseIndex);
                    indices.push_back(baseIndex + 1);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 3);
                    indices.push_back(baseIndex);

                    baseIndex += 4;  // Each Cell Uses 4 Vertices
                }
            }

            cellIndices = rows * cols * 6;  // 6 Indices Per Cell

            // Create VBO And EBO For Board
            glGenBuffers(1, &boardVBO);
            glGenBuffers(1, &boardEBO);
            glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boardEBO);

            // Build Preview Window Geometry Into The Same Vertex/Index Arrays
            this->initializeNextShapeWindow();

            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            // Now It's Safe To Update Preview (Uses glBufferSubData On An Allocated Buffer)
            this->updatePreviewDisplay();

        }


        /*

            Desc: Initializes The Next Shape Preview Window By Creating A Small Black Box
            In The Top-Right Corner Of The Screen For Displaying The Upcoming Tetris Shape.

            Preconditions:
                1.) Vertex And Index Arrays Are Initialized And Ready For Data Addition.
                2.) Box Dimensions And Position Are Properly Calculated.

            Postconditions:
                1.) Creates A Visual Border Box For The Next Shape Preview Area.
                2.) Adds Box Vertices And Indices To The Main Vertex Arrays.
                3.) Calls initializePreviewGrid To Set Up The Internal Grid Structure.

        */
        void initializeNextShapeWindow()
        {

            // Create Small Black Box In Top-Right Corner
            float boxWidth = 0.25f;
            float boxHeight = 0.25f;

            float boxX = 0.6f; // Right Side Of The Screen
            float boxY = 0.9f; // Top Of The Screen

            unsigned int baseIndex = vertices.size() / 6;
            // Bottom-Left
            vertices.push_back(boxX);
            vertices.push_back(boxY - boxHeight);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);
            // Bottom-Right
            vertices.push_back(boxX + boxWidth);
            vertices.push_back(boxY - boxHeight);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);
            // Top-Right
            vertices.push_back(boxX + boxWidth);
            vertices.push_back(boxY);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);
            // Top-Left
            vertices.push_back(boxX);
            vertices.push_back(boxY);
            vertices.push_back(0.0f);
            vertices.push_back(0.1f);
            vertices.push_back(0.1f);
            vertices.push_back(0.2f);
            // Add Indices For The Box Rectangle
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);
            indices.push_back(baseIndex);

            baseIndex += 4;

            // Create Mini-Grid Inside The Box
            initializePreviewGrid(boxX, boxY, boxWidth, boxHeight, baseIndex);

        }


        /*

            Desc: Creates The Internal Grid Structure Within The Preview Box For Displaying
            The Next Tetris Shape, Including Grid Lines And Individual Cell Areas.

            Preconditions:
                1.) Box Position And Dimensions Are Provided Through Parameters.
                2.) Vertex And Index Arrays Are Available For Data Addition.
                3.) Preview Grid Cell Array Is Initialized.

            Postconditions:
                1.) Creates Vertical And Horizontal Grid Lines Within The Preview Box.
                2.) Initializes Individual Preview Cells With Proper VBO And Index Offsets.
                3.) Updates baseIndex Reference To Reflect Added Vertices.

        */
        void initializePreviewGrid(float boxX, float boxY, float boxWidth, float boxHeight, unsigned int& baseIndex)
        {

            float gridPadding = 0.02f;
            float gridStartX = boxX + gridPadding;
            float gridStartY = boxY - gridPadding;
            float gridWidth = boxWidth - 2 * gridPadding;
            float gridHeight = boxHeight - 2 * gridPadding;

            float cellWidth = gridWidth / PREVIEW_COLS;
            float cellHeight = gridHeight / PREVIEW_ROWS;

            // Grid Line Color
            float gridR = 0.4f, gridG = 0.4f, gridB = 0.4f;

            // Vertical Grid Lines
            for (unsigned int i = 0; i <= PREVIEW_COLS; i++)
            {
                float x = gridStartX + i * cellWidth;
                // Top Vertex Of The Line
                vertices.push_back(x);
                vertices.push_back(gridStartY);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Bottom Vertex Of The Line
                vertices.push_back(x);
                vertices.push_back(gridStartY - gridHeight);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Add Indices For This Line
                indices.push_back(baseIndex);
                indices.push_back(baseIndex + 1);
                baseIndex += 2;
            }

            // Horizontal Grid Lines
            for (unsigned int i = 0; i <= PREVIEW_ROWS; i++) {
                float y = gridStartY - i * cellHeight;

                // Left Vertex
                vertices.push_back(gridStartX);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Right Vertex
                vertices.push_back(gridStartX + gridWidth);
                vertices.push_back(y);
                vertices.push_back(0.0f);
                vertices.push_back(gridR);
                vertices.push_back(gridG);
                vertices.push_back(gridB);

                // Add Line Indices
                indices.push_back(baseIndex);
                indices.push_back(baseIndex + 1);
                baseIndex += 2;
            }

            previewGridIndices = (PREVIEW_COLS + 1) * 2 + (PREVIEW_ROWS + 1) * 2;

            // Create Preview Cells
            float cellPadding = 0.002f;

            for (unsigned int row = 0; row < PREVIEW_ROWS; row++) {
                for (unsigned int col = 0; col < PREVIEW_COLS; col++) {
                    float cellX = gridStartX + col * cellWidth + cellPadding;
                    float cellY = gridStartY - row * cellHeight - cellPadding;
                    float cellW = cellWidth - 2 * cellPadding;
                    float cellH = cellHeight - 2 * cellPadding;

                    // Store Preview Cell Offsets
                    previewGrid[row][col].vboOffset = vertices.size() / 6;
                    previewGrid[row][col].indexOffset = indices.size();

                    // Default Transparent Color
                    float r = 0.0f, g = 0.0f, b = 0.0f;

                    // Add 4 Vertices For This Preview Cell
                    vertices.push_back(cellX);
                    vertices.push_back(cellY - cellH);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    vertices.push_back(cellX + cellW);
                    vertices.push_back(cellY - cellH);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    vertices.push_back(cellX + cellW);
                    vertices.push_back(cellY);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    vertices.push_back(cellX);
                    vertices.push_back(cellY);
                    vertices.push_back(0.0f);
                    vertices.push_back(r);
                    vertices.push_back(g);
                    vertices.push_back(b);

                    // Add Triangle Indices
                    indices.push_back(baseIndex);
                    indices.push_back(baseIndex + 1);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 2);
                    indices.push_back(baseIndex + 3);
                    indices.push_back(baseIndex);

                    baseIndex += 4;
                }
            }

            previewCellIndices = PREVIEW_ROWS * PREVIEW_COLS * 6;

        }


        /*

            Desc: Sets The Color Of A Specific Cell In The Preview Grid And Updates
            The GPU Buffer With The New Color Information.

            Preconditions:
                1.) Row And Column Are Within Valid Preview Grid Bounds.
                2.) OpenGL Context Is Active And Buffer Objects Are Created.
                3.) Color Values Are In Valid Range [0.0f, 1.0f].

            Postconditions:
                1.) Updates The Color Of All Four Vertices For The Specified Preview Cell.
                2.) Uploads The Color Changes To The GPU Buffer Immediately.
                3.) If Invalid Coordinates, Function Returns Without Changes.

        */
        void setPreviewCellColor(unsigned int row, unsigned int col, const glm::vec3& color)
        {

            if (row >= PREVIEW_ROWS || col >= PREVIEW_COLS) return;

            // Each Preview Cell Has 4 Vertices, Each Vertex Has 6 Floats (x,y,z,r,g,b)
            unsigned int vboStart = previewGrid[row][col].vboOffset;

            // Update The Color In The VBO For All 4 Vertices Of This Cell
            for (int i = 0; i < 4; i++) {
                unsigned int colorOffset = (vboStart + i) * 6 + 3; // +3 To Skip x,y,z
                vertices[colorOffset] = color.r;
                vertices[colorOffset + 1] = color.g;
                vertices[colorOffset + 2] = color.b;
            }

            // Upload The Updated Colors To The GPU
            glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
            glBufferSubData(GL_ARRAY_BUFFER, vboStart * 6 * sizeof(float),
                4 * 6 * sizeof(float), &vertices[vboStart * 6]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

        }


        /*

            Desc: Clears A Specific Cell In The Preview Grid By Setting Its Color To Black
            And Marking It As Empty.

            Preconditions:
                1.) Row And Column Are Within Valid Preview Grid Bounds.
                2.) Preview Grid Is Properly Initialized.

            Postconditions:
                1.) Sets The Specified Preview Cell Color To Black (0.0f, 0.0f, 0.0f).
                2.) Marks The Cell's Occupation Status As EMPTY.

        */
        void clearPreviewCell(unsigned int row, unsigned int col) 
        {

            setPreviewCellColor(row, col, glm::vec3(0.0f, 0.0f, 0.0f));
            previewGrid[row][col].occupied = EMPTY;

        }


        /*

            Desc: Clears All Cells In The Preview Grid By Setting Each Cell To Black
            And Marking Them As Empty.

            Preconditions:
                1.) Preview Grid Is Properly Initialized.
                2.) All Preview Grid Cells Are Accessible.

            Postconditions:
                1.) All Preview Grid Cells Are Set To Black Color.
                2.) All Preview Grid Cells Are Marked As EMPTY.

        */
        void clearPreviewGrid() 
        {

            for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
                for (unsigned int j = 0; j < PREVIEW_COLS; ++j) {
                    clearPreviewCell(i, j);
                }
            }

        }


        /*

            Desc: Renders The Entire Board Including Background, Grid Lines, Cells,
            Preview Window, And All Visual Elements Using OpenGL Draw Calls.

            Preconditions:
                1.) OpenGL Context Is Active And Rendering State Is Properly Set.
                2.) All Buffer Objects Are Created And Contain Valid Data.
                3.) Vertex Attributes Are Properly Configured.

            Postconditions:
                1.) Draws Board Background, Grid Lines, And All Game Cells.
                2.) Renders The Preview Window With Its Grid And Cells.
                3.) All Visual Elements Are Displayed On Screen.

        */
        void render() 
        {

            glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boardEBO);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

            // Ensure Attributes Are Enabled (In Case No VAO Or It Wasn't Set Elsewhere)
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);

            unsigned int offset = 0;
            glDrawElements(GL_TRIANGLES, boardIndices, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
            offset += boardIndices;

            glDrawElements(GL_LINES, gridLineIndices, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
            offset += gridLineIndices;

            glDrawElements(GL_TRIANGLES, cellIndices, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
            offset += cellIndices;

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
            offset += 6;

            glDrawElements(GL_LINES, previewGridIndices, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
            offset += previewGridIndices;

            glDrawElements(GL_TRIANGLES, previewCellIndices, GL_UNSIGNED_INT, (void*)(offset * sizeof(unsigned int)));
        
        }


        /*

            Desc: Updates The Preview Display Window To Show The Next Tetris Shape
            By Clearing The Previous Display And Drawing The New Shape.

            Preconditions:
                1.) Preview Grid Is Initialized And Functional.
                2.) nextShape Contains A Valid Tetris Shape.
                3.) nextShapeColor Contains A Valid Color.

            Postconditions:
                1.) Clears All Cells In The Preview Grid.
                2.) Centers And Displays The Next Shape In The Preview Window.
                3.) Sets Appropriate Colors For The Shape Cells.

        */
        void updatePreviewDisplay() 
        {
    
            // Clear All Preview Cells First
            for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
                for (unsigned int j = 0; j < PREVIEW_COLS; ++j) {
                    // Just Like clearCell Does For The Main Grid
                    setPreviewCellColor(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
                    previewGrid[i][j].occupied = EMPTY;
                }
            }

            nextShape = shapes[std::rand() % 7]; // Randomly Select A New Shape

            // Skip If No Next Shape
            if (!nextShape.empty()) {
                // Center The Shape In The Preview Grid
                unsigned int startRow = (PREVIEW_ROWS - nextShape.size()) / 2;
                unsigned int startCol = (PREVIEW_COLS - nextShape[0].size()) / 2;

                // Set Colors For The Next Shape Cells
                for (unsigned int i = 0; i < nextShape.size(); ++i) {
                    for (unsigned int j = 0; j < nextShape[i].size(); ++j) {
                        if (nextShape[i][j])
                        {
                            // Use nextShapeColor Instead Of Random Colors
                            setPreviewCellColor(startRow + i, startCol + j, nextShapeColor);
                            previewGrid[startRow + i][startCol + j].occupied = OCCUPIED;
                        }
                    }
                }
            }

        }


        /*

            Desc: Sets The Color Of A Specific Cell In The Main Game Grid And Updates
            The GPU Buffer With The New Color Information.

            Preconditions:
                1.) Row And Column Are Within Valid Game Grid Bounds.
                2.) OpenGL Context Is Active And Buffer Objects Are Created.
                3.) Color Values Are In Valid Range [0.0f, 1.0f].

            Postconditions:
                1.) Updates The Color Of All Four Vertices For The Specified Game Cell.
                2.) Uploads The Color Changes To The GPU Buffer Immediately.
                3.) If Invalid Coordinates, Function Returns Without Changes.

        */
        void setCellColor(unsigned int row, unsigned int col, const glm::vec3& color) 
        {

            if (row >= rows || col >= cols) return;

            // Each Cell Has 4 Vertices, Each Vertex Has 6 Floats (x,y,z,r,g,b)
            // Color Starts At Index 3 For Each Vertex
            unsigned int vboStart = gameGrid[row][col].vboOffset;

            // Update The Color In The VBO For All 4 Vertices Of This Cell
            for (int i = 0; i < 4; i++) {
                unsigned int colorOffset = (vboStart + i) * 6 + 3; // +3 To Skip x,y,z
                vertices[colorOffset] = color.r;
                vertices[colorOffset + 1] = color.g;
                vertices[colorOffset + 2] = color.b;
            }

            // Upload The Updated Colors To The GPU
            glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
            glBufferSubData(GL_ARRAY_BUFFER, vboStart * 6 * sizeof(float),
                4 * 6 * sizeof(float), &vertices[vboStart * 6]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

        }


        /*

            Desc: Clears A Specific Cell In The Main Game Grid By Setting Its Color
            To Black And Marking It As Empty.

            Preconditions:
                1.) Row And Column Are Within Valid Game Grid Bounds.
                2.) Game Grid Is Properly Initialized.

            Postconditions:
                1.) Sets The Specified Game Cell Color To Black (0.0f, 0.0f, 0.0f).
                2.) Marks The Cell's Occupation Status As EMPTY.

        */
        void clearCell(unsigned int row, unsigned int col) 
        {
    
            setCellColor(row, col, glm::vec3(0.0f, 0.0f, 0.0f));
            gameGrid[row][col].occupied = EMPTY;
    
        }


        /*

            Desc: Colors Random Cells In The Game Grid With Different Colors For
            Testing And Demonstration Purposes.

            Preconditions:
                1.) Game Grid Is Properly Initialized.
                2.) Cell Coloring Functions Are Working Correctly.

            Postconditions:
                1.) Sets Specific Cells To Red, Green, Blue, And Yellow Colors.
                2.) Provides Visual Verification That Cell Coloring Is Functioning.

        */
        void colorRandomCells() 
        {

            // Color Some Cells For Testing
            setCellColor(0, 0, glm::vec3(1.0f, 0.0f, 0.0f)); // Red
            setCellColor(1, 2, glm::vec3(0.0f, 1.0f, 0.0f)); // Green
            setCellColor(3, 5, glm::vec3(0.0f, 0.0f, 1.0f)); // Blue
            setCellColor(5, 7, glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow

        }


        /*

            Desc: Retrieves The Current Color Of A Specific Cell In The Game Grid
            From The Vertex Buffer Data.

            Preconditions:
                1.) Row And Column Are Within Valid Game Grid Bounds.
                2.) Game Grid And Vertex Buffer Are Properly Initialized.

            Postconditions:
                1.) Returns The RGB Color Values Of The Specified Cell.
                2.) If Invalid Coordinates, Returns Black Color (0.0f, 0.0f, 0.0f).

        */
        glm::vec3 getBlockColor(unsigned int row, unsigned int col) const 
        {

            if (row >= rows || col >= cols) return glm::vec3(0.0f, 0.0f, 0.0f);

            // Each Cell Has 4 Vertices, Each Vertex Has 6 Floats (x,y,z,r,g,b)
            unsigned int vboStart = gameGrid[row][col].vboOffset;
            // Get The Color From The First Vertex Of The Cell
            return glm::vec3(vertices[vboStart * 6 + 3], vertices[vboStart * 6 + 4], vertices[vboStart * 6 + 5]);

        }


        /*

            Desc: Advances The Game State By One Step, Managing Active Piece Movement,
            Collision Detection, Line Clearing, And New Piece Generation.

            Preconditions:
                1.) Game Grid Is Properly Initialized.
                2.) Game State Variables Are Valid.

            Postconditions:
                1.) Moves Active Piece Down One Row If Possible.
                2.) Finalizes Piece Position And Checks For Completed Lines.
                3.) Generates New Piece If Current Piece Has Landed.
                4.) Handles Game Over Conditions If No New Piece Can Be Placed.

        */
        void step()
        {

            // If No Active Shape, Start A New One
            if (upcomingShape.empty()) 
            {

                if (!startNewPiece()) 
                {
                    // Game Over - Reset The Game
                    std::cout << "Game Over!" << std::endl;
                    for (unsigned int i = 0; i < rows; ++i) {
                        for (unsigned int j = 0; j < cols; ++j) {
                            clearCell(i, j);
                        }
                    }
                    clearPreviewGrid();
                    return;
                }
                return; // New Piece Placed, Wait For Next Step
            }

            // Try To Move Current Shape Down
            bool canShiftDown = true;

            // Check If Shape Can Move Down
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j])
                    {
                        unsigned int targetRow = activePieceRow + i + 1;
                        unsigned int targetCol = activePieceCol + j;

                        if (targetRow >= rows || targetCol >= cols ||
                            gameGrid[targetRow][targetCol].occupied == OCCUPIED) 
                        {
                            canShiftDown = false;
                            break;
                        }
                    }
                }
                if (!canShiftDown) break;
            }

            if (canShiftDown)
            {
                // Move Shape Down

                // Clear Current Position
                for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                        if (upcomingShape[i][j])
                        {
                            clearCell(activePieceRow + i, activePieceCol + j);
                        }
                    }
                }

                // Move Down
                activePieceRow++;

                // Draw In New Position
                for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                        if (upcomingShape[i][j])
                        {
                            setCellColor(activePieceRow + i, activePieceCol + j, activePieceColor);
                        }
                    }
                }

                // Update Ghost Piece
                ghostPiece();
            }
            else 
            {
                // Shape Has Landed - Finalize It

                // Mark Cells As Occupied
                for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                        if (upcomingShape[i][j])
                        {
                            gameGrid[activePieceRow + i][activePieceCol + j].occupied = OCCUPIED;
                        }
                    }
                }

                // Clear The Active Shape So A New One Will Be Generated Next Step
                upcomingShape.clear();

                // Check For Completed Lines
                checkForPoints();

                // Clear Ghost Pieces
                for (unsigned int i = 0; i < rows; ++i) {
                    for (unsigned int j = 0; j < cols; ++j) {
                        if (gameGrid[i][j].occupied == GHOST)
                        {
                            clearCell(i, j);
                            gameGrid[i][j].occupied = EMPTY;
                        }
                    }
                }

            }

        }


        /*

            Desc: Moves The Currently Active Tetris Piece In The Specified Direction
            With Collision Detection And Boundary Checking.

            Preconditions:
                1.) An Active Piece (upcomingShape) Exists On The Board.
                2.) Direction Parameter Is Valid ('L', 'R', Or 'D').
                3.) Game Grid Is Properly Initialized.

            Postconditions:
                1.) Moves Active Piece Left, Right, Or Fast Down If Movement Is Valid.
                2.) Updates Visual Display To Reflect New Piece Position.
                3.) If Movement Is Invalid Due To Collision, No Movement Occurs.

        */
        void shiftActivePiece(char direction)
        {

            // User Shifts Left, Right, Or Fast Down
            if (upcomingShape.empty()) 
            {
                return;
            }

            bool canShift = true;
            switch (direction)
            {

                case 'L': // Shift Left
                    for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                        for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                            if (upcomingShape[i][j])
                            {
                                unsigned int targetRow = activePieceRow + i;
                                unsigned int targetCol = activePieceCol + j - 1;
                                // Check If The Target Cell Is Within Bounds And Not Occupied
                                if (targetCol < 0 || targetCol >= cols || gameGrid[targetRow][targetCol].occupied == OCCUPIED)
                                {
                                    canShift = false;
                                    break;
                                }
                            }
                        }
                        if (!canShift) break;
                    }
                    if (canShift)
                    {
                        // Clear The Current Shape From The Board
                        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                                if (upcomingShape[i][j])
                                {
                                    clearCell(activePieceRow + i, activePieceCol + j);
                                }
                            }
                        }
                        // Move The Shape Left
                        activePieceCol--;
                        // Draw The Shape In Its New Position
                        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                                if (upcomingShape[i][j])
                                {

                                    setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
                                }
                            }
                        }
                    }
                    break;
                case 'R': // Shift Right
                    for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                        for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                            if (upcomingShape[i][j])
                            {
                                unsigned int targetRow = activePieceRow + i;
                                unsigned int targetCol = activePieceCol + j + 1;
                                // Check If The Target Cell Is Within Bounds And Not Occupied
                                if (targetCol >= cols || gameGrid[targetRow][targetCol].occupied == OCCUPIED)
                                {
                                    canShift = false;
                                    break;
                                }
                            }
                        }
                        if (!canShift) break;
                    }
                    if (canShift)
                    {
                        // Clear The Current Shape From The Board
                        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                                if (upcomingShape[i][j])
                                {
                                    clearCell(activePieceRow + i, activePieceCol + j);
                                }
                            }
                        }
                        // Move The Shape Right
                        activePieceCol++;
                        // Draw The Shape In Its New Position
                        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                                if (upcomingShape[i][j])
                                {
                                    setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
                                }
                            }
                        }
                    }
                    break;
                case 'D': // Fast Down
                    while (true) {
                        bool canShiftDown = true;
                        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                                if (upcomingShape[i][j])
                                {
                                    unsigned int targetRow = activePieceRow + i + 1;
                                    unsigned int targetCol = activePieceCol + j;
                                    // Check If The Target Cell Is Within Bounds And Not Occupied
                                    if ((targetRow >= rows) || (targetCol >= cols) ||
                                        (gameGrid[targetRow][targetCol].occupied == OCCUPIED))
                                    {
                                        canShiftDown = false; // Cannot Shift Down, Hit The Bottom Or Another Piece
                                        break;
                                    }
                                }
                            }
                            if (!canShiftDown) break;
                        }
                        if (!canShiftDown) break;
                        // Clear The Current Shape From The Board
                        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                                if (upcomingShape[i][j])
                                {
                                    clearCell(activePieceRow + i, activePieceCol + j);
                                }
                            }
                        }
                        // Move The Shape Down
                        activePieceRow++;

                    }

                    // Draw The Shape In Its New Position
                    for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                        for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                            if (upcomingShape[i][j])
                            {
                                // Set The Cell Color
                                setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
                            }
                        }
                    }
                    break;
            }

        }


        /*

            Desc: Rotates The Currently Active Tetris Piece 90 Degrees Clockwise
            With Collision Detection And Boundary Checking.

            Preconditions:
                1.) An Active Piece (upcomingShape) Exists On The Board.
                2.) Game Grid Is Properly Initialized.

            Postconditions:
                1.) Rotates Active Piece If Rotation Is Valid And No Collisions Occur.
                2.) Updates Visual Display To Show Rotated Piece.
                3.) If Rotation Is Invalid, Piece Remains In Original Orientation.

        */
        void rotateActivePiece()
        {

            if (upcomingShape.empty()) 
            {
                return;
            }

            // Create A New Rotated Shape
            std::vector<std::vector<bool>> rotatedShape(upcomingShape[0].size(), std::vector<bool>(upcomingShape.size(), false));

            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    rotatedShape[j][upcomingShape.size() - 1 - i] = upcomingShape[i][j];
                }
            }

            // Check If The Rotated Shape Can Fit In The Current Position
            bool canRotate = true;
            for (unsigned int i = 0; i < rotatedShape.size(); ++i) {
                for (unsigned int j = 0; j < rotatedShape[i].size(); ++j) {
                    if (rotatedShape[i][j])
                    {
                        unsigned int targetRow = activePieceRow + i;
                        unsigned int targetCol = activePieceCol + j;
                        // Check If The Target Cell Is Within Bounds And Not Occupied
                        if (targetRow >= rows || targetCol >= cols || gameGrid[targetRow][targetCol].occupied == OCCUPIED)
                        {
                            canRotate = false;
                            break;
                        }
                    }
                }
                if (!canRotate) break;
            }

            if (canRotate)
            {
                // Clear The Current Shape From The Board
                for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                        if (upcomingShape[i][j])
                        {
                            clearCell(activePieceRow + i, activePieceCol + j);
                        }
                    }
                }
                // Update The Upcoming Shape To The Rotated Version
                upcomingShape = rotatedShape;
                // Draw The Rotated Shape In Its New Position
                for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                        if (upcomingShape[i][j])
                        {
                            setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
                        }
                    }
                }
            }

        }


        /*

            Desc: Creates And Displays A Ghost Piece That Shows Where The Active
            Piece Will Land When Dropped Straight Down.

            Preconditions:
                1.) An Active Piece (upcomingShape) Exists On The Board.
                2.) Game Grid Is Properly Initialized.

            Postconditions:
                1.) Clears Any Previous Ghost Piece From The Board.
                2.) Calculates The Landing Position For The Active Piece.
                3.) Displays Ghost Piece In Gray Color At The Landing Position.
                4.) If Ghost Would Be Too Close To Active Piece, No Ghost Is Shown.

        */
        void ghostPiece()
        {

            if (upcomingShape.empty())
            {
                return; // No Upcoming Shape, No Ghost Piece
            }

            // Create A Ghost Piece That Shows Where The Active Piece Would Land
            unsigned int ghostRow = activePieceRow;
            unsigned int ghostCol = activePieceCol;
            // Move The Ghost Piece Down Until It Can't Go Further
            while (true) {
                // Check If The Ghost Piece Can Shift Down
                bool canShiftDown = true;
                for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                        if (upcomingShape[i][j])
                        {
                            unsigned int targetRow = ghostRow + i + 1;
                            unsigned int targetCol = ghostCol + j;
                            // Check If The Target Cell Is Within Bounds And Not Occupied
                            if ((targetRow >= rows) || (targetCol >= cols) ||
                                (gameGrid[targetRow][targetCol].occupied == OCCUPIED))
                            {
                                canShiftDown = false; // Cannot Shift Down, Hit The Bottom Or Another Piece
                                break;
                            }
                        }
                    }
                    if (!canShiftDown) break;
                }
                if (!canShiftDown) break;
                ghostRow++;
            }
            // Clear Any Previous Ghost Piece
            for (unsigned int i = 0; i < rows; ++i) {
                for (unsigned int j = 0; j < cols; ++j) {
                    if (gameGrid[i][j].occupied == GHOST)
                    {
                        clearCell(i, j);
                    }
                }
            }

            if (ghostRow - activePieceRow < 4)
            {
                // If The Ghost Piece Is Too Close To The Active Piece, Don't Draw It
                return;
            }

            // Draw The Ghost Piece In Its Position
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j])
                    {
                        setCellColor(ghostRow + i, ghostCol + j, glm::vec3(0.5f, 0.5f, 0.5f)); // Gray Color For Ghost Piece
                        gameGrid[ghostRow + i][ghostCol + j].occupied = GHOST; // Mark As Ghost
                    }
                }
            }

        }


        /*

            Desc: Checks For Completed Horizontal Lines, Clears Them, Shifts Remaining
            Blocks Down, And Updates Score And Level Accordingly.

            Preconditions:
                1.) Game Grid Is Properly Initialized With Valid Block States.
                2.) Score, Level, And Line Count References Are Valid.

            Postconditions:
                1.) Identifies And Clears All Completed Horizontal Lines.
                2.) Shifts All Blocks Above Cleared Lines Down Appropriately.
                3.) Updates Score Based On Number Of Lines Cleared.
                4.) Increments Level Every 5 Lines Cleared.
                5.) Sets Update Text Flag To Refresh UI Display.

        */
        void checkForPoints()
        {

            glm::vec3 oldColor;
            bool rowCleared = false;
            int rowsCleared = 0;

            // Check For Completed Rows
            for (unsigned int row = 0; row < rows; ++row) {
                bool complete = true;
                for (unsigned int col = 0; col < cols; ++col) {
                    if (gameGrid[row][col].occupied == EMPTY)
                    {
                        complete = false;
                        break;
                    }
                }
                if (complete)
                {
                    rowCleared = true;
                    rowsCleared++;

                    // Clear The Completed Row
                    for (unsigned int col = 0; col < cols; ++col) {
                        clearCell(row, col);
                        gameGrid[row][col].occupied = EMPTY;
                    }

                    // Shift All Rows Above Down
                    for (int r = row - 1; r >= 0; --r) {
                        for (unsigned int c = 0; c < cols; ++c) {
                            if (gameGrid[r][c].occupied == OCCUPIED)
                            {
                                // Grab The Color Of The Cell Above
                                oldColor = getBlockColor(r, c);
                                clearCell(r, c);
                                setCellColor(r + 1, c, oldColor);
                                gameGrid[r + 1][c].occupied = OCCUPIED;
                                gameGrid[r][c].occupied = EMPTY;
                            }
                        }
                    }

                    // Recheck The Current Row After Shifting
                    row--;
                }
            }

            if (rowCleared)
            {
                // Add Score Based On Rows Cleared
                this->score += rowsCleared * CONFIG::POINTS_PER_LINE;
                this->totalRowsCleared += rowsCleared;

                if (!(this->totalRowsCleared % 5))
                {
                    this->level++;
                }

                this->updateText = true;


            }

        }


        /*

            Desc: Destructor For The Board Class That Cleans Up Allocated Memory
            And OpenGL Resources.

            Preconditions:
                1.) Board Object Is Being Destroyed Or Going Out Of Scope.

            Postconditions:
                1.) Deletes All Dynamically Allocated Grid Arrays.
                2.) Cleans Up OpenGL Buffer Objects.
                3.) Prevents Memory Leaks And Resource Cleanup.

        */
        ~Board() 
        {

            // Cleanup Board Buffers
            if (boardVBO)
            {
                glDeleteBuffers(1, &boardVBO);
                glDeleteBuffers(1, &boardEBO);
            }

            // Cleanup Game Grid
            if (gameGrid)
            {
                for (unsigned int i = 0; i < rows; ++i) {
                    delete[] gameGrid[i];
                }
                delete[] gameGrid;
            }

            // Cleanup Preview Grid
            if (previewGrid)
            {
                for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
                    delete[] previewGrid[i];
                }
                delete[] previewGrid;
            }

        }

};