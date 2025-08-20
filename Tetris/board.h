#pragma once

#include "glm/glm.hpp"
#include <queue>
#include <glad/glad.h>
#include <vector>
#include <iostream>


enum BlockState
{
	EMPTY = 0, // Empty cell
	OCCUPIED = 1, // Cell occupied by a piece
	GHOST = 2 // Ghost piece (for preview)
};;


struct Block 
{
    unsigned int vboOffset;   // Starting index in vertices array
    unsigned int indexOffset; // Starting index in indices array
    BlockState occupied;
};


static const std::vector<std::vector<bool>> shapes[] = {

	// Define shapes here, e.g.:
	{ {1, 1, 1, 1} }, // I shape
	{ {1, 1}, {1, 1} }, // O shape
	{ {0, 1, 0}, {1, 1, 1} }, // T shape
	{ {1, 1, 0}, {0, 1, 1} }, // S shape
	{ {0, 1, 1}, {1, 1, 0} }, // Z shape
	{ {1, 0, 0}, {1, 1, 1} }, // L shape
	{ {0, 0, 1}, {1, 1, 1} } // J shape

};

struct Shape {
    std::vector<std::vector<unsigned int>> grid; // 2D grid representation of the shape
    glm::vec3 color; // Color of the shape
};

class Board {
private:
    Block** gameGrid;
    Block** previewGrid;
    std::vector<std::vector<bool>> upcomingShape;

    static const unsigned int PREVIEW_ROWS = 4;
    static const unsigned int PREVIEW_COLS = 4;
    unsigned int previewGridIndices;    // Indices for preview grid lines
    unsigned int previewCellIndices;    // Indices for preview cells

    std::vector<std::vector<bool>> nextShape;
    glm::vec3 nextShapeColor;

	unsigned int activePieceRow, activePieceCol; // Position of the active piece on the board
	glm::vec3 activePieceColor; // Color of the active piece

    unsigned int rows, cols;
    unsigned int width, height; // Width and Height of the Board in Pixels
    GLuint boardVBO, boardEBO;

    // Tracking indices for different parts of the geometry
    unsigned int boardIndices;      // Indices for board background
    unsigned int gridLineIndices;   // Indices for grid lines
    unsigned int cellIndices;       // Indices for all 
    unsigned int& score;
    unsigned int& totalRowsCleared;
    unsigned int& level;
    bool& updateText;
    bool& gameOver;

    std::vector<float> vertices;
    std::vector<unsigned int> indices;


    void generateNextShape() {
        auto tmp = shapes[std::rand() % 7];
        nextShape = std::vector<std::vector<bool>>(tmp.begin(), tmp.end());
        nextShapeColor = glm::vec3((std::rand() % 256) / 255.0f, (std::rand() % 256) / 255.0f, (std::rand() % 256) / 255.0f);
    }

    bool startNewPiece() {
        // Move next shape to current
        upcomingShape = nextShape;
        activePieceColor = nextShapeColor;

        // Generate new next shape
        generateNextShape();
        updatePreviewDisplay();

        // Set starting position
        activePieceRow = 0;
        activePieceCol = cols / 2 - upcomingShape[0].size() / 2;

        // Check if we can place the new shape (Game Over check)
        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                if (upcomingShape[i][j]) {
                    unsigned int targetRow = activePieceRow + i;
                    unsigned int targetCol = activePieceCol + j;

                    if (targetRow >= rows || targetCol >= cols ||
                        gameGrid[targetRow][targetCol].occupied == OCCUPIED) {
                        // Game Over
                        gameOver = true;
                        return false;
                    }
                }
            }
        }

        // Place the new shape on board
        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                if (upcomingShape[i][j]) {
                    setCellColor(activePieceRow + i, activePieceCol + j, activePieceColor);
                }
            }
        }

        // Generate ghost piece
        ghostPiece();
        return true;
    }

public:
    Board(unsigned int rows, unsigned int cols, unsigned int width, unsigned int height
		, unsigned int& score, bool& textUpdate, unsigned int& linesCleared, unsigned int& curLevel, bool& gameOverFlag)
          : score(score), updateText(textUpdate),
            rows(rows), cols(cols), width(width), height(height), boardVBO(0), boardEBO(0),
            boardIndices(0), gridLineIndices(0), cellIndices(0), totalRowsCleared(linesCleared),
		level(curLevel), gameOver(gameOverFlag), previewGridIndices(0), previewCellIndices(0)
    {

		// Seed Our Random Number Generator
		std::srand(static_cast<unsigned int>(std::time(nullptr)));

        gameGrid = new Block * [rows];
        for (unsigned int i = 0; i < rows; ++i) {
            gameGrid[i] = new Block[cols];
            for (unsigned int j = 0; j < cols; ++j) {
                gameGrid[i][j] = { 0, 0, EMPTY };
            }
        }

        // Initialize preview grid
        previewGrid = new Block * [PREVIEW_ROWS];
        for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
            previewGrid[i] = new Block[PREVIEW_COLS];
            for (unsigned int j = 0; j < PREVIEW_COLS; ++j) {
                previewGrid[i][j] = { 0, 0, EMPTY };
            }
        }

		// Generate the first shape
        generateNextShape();

    }

    void reset() {
        // Reset the game grid
        for (unsigned int i = 0; i < rows; ++i) {
            for (unsigned int j = 0; j < cols; ++j) {
                gameGrid[i][j] = { 0, 0, EMPTY };
            }
        }
        // Clear the upcoming shape
        upcomingShape.clear();

        // Generate new next shape
        generateNextShape();
        //updatePreviewDisplay();

	}

    void initialize() {
        // Create board rectangle - positioned to center-right of screen
        float boardWidth = 0.85f;
        float boardHeight = 1.5f;
        float boardX = 0.0f;
        float boardY = 0.0f;

        // Calculate grid cell dimensions
        float cellWidth = boardWidth / cols;
        float cellHeight = boardHeight / rows;

        // Calculate the starting position (top-left of the board)
        float startX = boardX - boardWidth / 2.0f;
        float startY = boardY + boardHeight / 2.0f;

        // Clear the vectors
        vertices.clear();
        indices.clear();

        unsigned int baseIndex = 0;

        // 1. Add board outline vertices
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

        // Add indices for the board rectangle
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(0);

        boardIndices = 6;  // We used 6 indices for the board
        baseIndex = 4;     // We used 4 vertices for the board

        // 2. Generate grid lines
        float gridR = 0.3f, gridG = 0.3f, gridB = 0.3f;

        // Vertical grid lines
        for (unsigned int i = 0; i <= cols; i++) {
            float x = startX + i * cellWidth;

            // Top vertex of the line
            vertices.push_back(x);
            vertices.push_back(startY);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Bottom vertex of the line
            vertices.push_back(x);
            vertices.push_back(startY - boardHeight);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Add indices for this line
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);

            baseIndex += 2;
        }

        // Horizontal grid lines
        for (unsigned int i = 0; i <= rows; i++) {
            float y = startY - i * cellHeight;

            // Left vertex of the line
            vertices.push_back(startX);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Right vertex of the line
            vertices.push_back(startX + boardWidth);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Add indices for this line
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);

            baseIndex += 2;
        }

        gridLineIndices = (cols + 1) * 2 + (rows + 1) * 2;  // Two indices per line

        // 3. Generate cells for each grid position
        float cellPadding = 0.005f;  // Small padding to see the grid lines

        for (unsigned int row = 0; row < rows; row++) {
            for (unsigned int col = 0; col < cols; col++) {
                // Calculate cell position with padding
                float cellX = startX + col * cellWidth + cellPadding;
                float cellY = startY - row * cellHeight - cellPadding;
                float cellW = cellWidth - 2 * cellPadding;
                float cellH = cellHeight - 2 * cellPadding;

                // Default color (transparent/black)
                float r = 0.0f, g = 0.0f, b = 0.0f;

                // Store where this cell's vertices begin in the VBO
                gameGrid[row][col].vboOffset = vertices.size() / 6;  // 6 floats per vertex
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

                // Add indices for this cell
                indices.push_back(baseIndex);
                indices.push_back(baseIndex + 1);
                indices.push_back(baseIndex + 2);
                indices.push_back(baseIndex + 2);
                indices.push_back(baseIndex + 3);
                indices.push_back(baseIndex);

                baseIndex += 4;  // Each cell uses 4 vertices
            }
        }

        cellIndices = rows * cols * 6;  // 6 indices per cell

        // Create VBO and EBO for board
        glGenBuffers(1, &boardVBO);
        glGenBuffers(1, &boardEBO);
        glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boardEBO);

        // Build preview window geometry into the same vertex/index arrays
        this->initializeNextShapeWindow();
        
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

        // Now it’s safe to update preview (uses glBufferSubData on an allocated buffer)
        this->updatePreviewDisplay();
		

    }

    void initializeNextShapeWindow()
    {

        // Create Small Black Box In Top-Right Corner
		float boxWidth = 0.25f;
		float boxHeight = 0.25f;

        float boxX = 0.6f; // Right side of the screen
        float boxY = 0.9f; // Top of the screen

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
        // Add indices for the box rectangle
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + 1);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 2);
        indices.push_back(baseIndex + 3);
		indices.push_back(baseIndex);

        baseIndex += 4;

        // Create mini-grid inside the box
        initializePreviewGrid(boxX, boxY, boxWidth, boxHeight, baseIndex);

    }

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

		// Vertical grid lines
        for (unsigned int i = 0; i <= PREVIEW_COLS; i++)
        {
            float x = gridStartX + i * cellWidth;
            // Top vertex of the line
            vertices.push_back(x);
            vertices.push_back(gridStartY);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Bottom vertex of the line
            vertices.push_back(x);
            vertices.push_back(gridStartY - gridHeight);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Add indices for this line
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);
			baseIndex += 2;
        }

        // Horizontal grid lines
        for (unsigned int i = 0; i <= PREVIEW_ROWS; i++) {
            float y = gridStartY - i * cellHeight;

            // Left vertex
            vertices.push_back(gridStartX);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Right vertex
            vertices.push_back(gridStartX + gridWidth);
            vertices.push_back(y);
            vertices.push_back(0.0f);
            vertices.push_back(gridR);
            vertices.push_back(gridG);
            vertices.push_back(gridB);

            // Add line indices
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);
            baseIndex += 2;
        }

        previewGridIndices = (PREVIEW_COLS + 1) * 2 + (PREVIEW_ROWS + 1) * 2;

        // Create preview cells
        float cellPadding = 0.002f;

        for (unsigned int row = 0; row < PREVIEW_ROWS; row++) {
            for (unsigned int col = 0; col < PREVIEW_COLS; col++) {
                float cellX = gridStartX + col * cellWidth + cellPadding;
                float cellY = gridStartY - row * cellHeight - cellPadding;
                float cellW = cellWidth - 2 * cellPadding;
                float cellH = cellHeight - 2 * cellPadding;

                // Store preview cell offsets
                previewGrid[row][col].vboOffset = vertices.size() / 6;
                previewGrid[row][col].indexOffset = indices.size();

                // Default transparent color
                float r = 0.0f, g = 0.0f, b = 0.0f;

                // Add 4 vertices for this preview cell
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

                // Add triangle indices
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

    void setPreviewCellColor(unsigned int row, unsigned int col, const glm::vec3& color) 
    {
        if (row >= PREVIEW_ROWS || col >= PREVIEW_COLS) return;

        // Each preview cell has 4 vertices, each vertex has 6 floats (x,y,z,r,g,b)
		unsigned int vboStart = previewGrid[row][col].vboOffset;

        // Update the color in the VBO for all 4 vertices of this cell
        for (int i = 0; i < 4; i++) {
            unsigned int colorOffset = (vboStart + i) * 6 + 3; // +3 to skip x,y,z
            vertices[colorOffset] = color.r;
            vertices[colorOffset + 1] = color.g;
            vertices[colorOffset + 2] = color.b;
        }

        // Upload the updated colors to the GPU
        glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
        glBufferSubData(GL_ARRAY_BUFFER, vboStart * 6 * sizeof(float),
            4 * 6 * sizeof(float), &vertices[vboStart * 6]);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

	}

    void clearPreviewCell(unsigned int row, unsigned int col) {
        setPreviewCellColor(row, col, glm::vec3(0.0f, 0.0f, 0.0f));
        previewGrid[row][col].occupied = EMPTY;
	}

    void clearPreviewGrid() {
        for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
            for (unsigned int j = 0; j < PREVIEW_COLS; ++j) {
                clearPreviewCell(i, j);
            }
        }
	}

    void render() {
        glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boardEBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

        // Ensure attributes are enabled (in case no VAO or it wasn’t set elsewhere)
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


    void updatePreviewDisplay() {
        // Clear all preview cells first
        for (unsigned int i = 0; i < PREVIEW_ROWS; ++i) {
            for (unsigned int j = 0; j < PREVIEW_COLS; ++j) {
                // Just like clearCell does for the main grid
                setPreviewCellColor(i, j, glm::vec3(0.0f, 0.0f, 0.0f));
                previewGrid[i][j].occupied = EMPTY;
            }
        }

		nextShape = shapes[std::rand() % 7]; // Randomly select a new shape

        // Skip if no next shape
        if (!nextShape.empty()) {
            // Center the shape in the preview grid
            unsigned int startRow = (PREVIEW_ROWS - nextShape.size()) / 2;
            unsigned int startCol = (PREVIEW_COLS - nextShape[0].size()) / 2;

            // Set colors for the next shape cells
            for (unsigned int i = 0; i < nextShape.size(); ++i) {
                for (unsigned int j = 0; j < nextShape[i].size(); ++j) {
                    if (nextShape[i][j]) {
                        // Use nextShapeColor instead of random colors
                        setPreviewCellColor(startRow + i, startCol + j, nextShapeColor);
                        previewGrid[startRow + i][startCol + j].occupied = OCCUPIED;
                    }
                }
            }
        }
    }

    // Set color of a specific cell
    void setCellColor(unsigned int row, unsigned int col, const glm::vec3& color) {
        if (row >= rows || col >= cols) return;

        // Each cell has 4 vertices, each vertex has 6 floats (x,y,z,r,g,b)
        // Color starts at index 3 for each vertex
        unsigned int vboStart = gameGrid[row][col].vboOffset;

        // Update the color in the VBO for all 4 vertices of this cell
        for (int i = 0; i < 4; i++) {
            unsigned int colorOffset = (vboStart + i) * 6 + 3; // +3 to skip x,y,z
            vertices[colorOffset] = color.r;
            vertices[colorOffset + 1] = color.g;
            vertices[colorOffset + 2] = color.b;
        }

        // Upload the updated colors to the GPU
        glBindBuffer(GL_ARRAY_BUFFER, boardVBO);
        glBufferSubData(GL_ARRAY_BUFFER, vboStart * 6 * sizeof(float),
            4 * 6 * sizeof(float), &vertices[vboStart * 6]);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    // Clear a cell (set it to black/transparent)
    void clearCell(unsigned int row, unsigned int col) {
        setCellColor(row, col, glm::vec3(0.0f, 0.0f, 0.0f));
        gameGrid[row][col].occupied = EMPTY;
    }

    // Test coloring functions
    void colorRandomCells() {
        // Color some cells for testing
        setCellColor(0, 0, glm::vec3(1.0f, 0.0f, 0.0f)); // Red
        setCellColor(1, 2, glm::vec3(0.0f, 1.0f, 0.0f)); // Green
        setCellColor(3, 5, glm::vec3(0.0f, 0.0f, 1.0f)); // Blue
        setCellColor(5, 7, glm::vec3(1.0f, 1.0f, 0.0f)); // Yellow
    }

	glm::vec3 getBlockColor(unsigned int row, unsigned int col) const {
		if (row >= rows || col >= cols) return glm::vec3(0.0f, 0.0f, 0.0f);
		// Each cell has 4 vertices, each vertex has 6 floats (x,y,z,r,g,b)
		unsigned int vboStart = gameGrid[row][col].vboOffset;
		// Get the color from the first vertex of the cell
		return glm::vec3(vertices[vboStart * 6 + 3], vertices[vboStart * 6 + 4], vertices[vboStart * 6 + 5]);
	}

    // Step Through Dropping Of Shape
    void step()
    {

        // If no active shape, start a new one
        if (upcomingShape.empty()) {

            if (!startNewPiece()) {
                // Game Over - reset the game
                std::cout << "Game Over!" << std::endl;
                for (unsigned int i = 0; i < rows; ++i) {
                    for (unsigned int j = 0; j < cols; ++j) {
                        clearCell(i, j);
                    }
                }
                clearPreviewGrid();
                return;
            }
            return; // New piece placed, wait for next step
        }

        // Try to move current shape down
        bool canShiftDown = true;

        // Check if shape can move down
        for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
            for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                if (upcomingShape[i][j]) {
                    unsigned int targetRow = activePieceRow + i + 1;
                    unsigned int targetCol = activePieceCol + j;

                    if (targetRow >= rows || targetCol >= cols ||
                        gameGrid[targetRow][targetCol].occupied == OCCUPIED) {
                        canShiftDown = false;
                        break;
                    }
                }
            }
            if (!canShiftDown) break;
        }

        if (canShiftDown) {
            // Move shape down

            // Clear current position
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j]) {
                        clearCell(activePieceRow + i, activePieceCol + j);
                    }
                }
            }

            // Move down
            activePieceRow++;

            // Draw in new position
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j]) {
                        setCellColor(activePieceRow + i, activePieceCol + j, activePieceColor);
                    }
                }
            }

            // Update ghost piece
            ghostPiece();
        }
        else {
            // Shape has landed - finalize it

            // Mark cells as occupied
            for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
                for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
                    if (upcomingShape[i][j]) {
                        gameGrid[activePieceRow + i][activePieceCol + j].occupied = OCCUPIED;
                    }
                }
            }

            // Clear the active shape so a new one will be generated next step
            upcomingShape.clear();

            // Check for completed lines
            checkForPoints();

            // Clear ghost pieces
            for (unsigned int i = 0; i < rows; ++i) {
                for (unsigned int j = 0; j < cols; ++j) {
                    if (gameGrid[i][j].occupied == GHOST) {
                        clearCell(i, j);
                        gameGrid[i][j].occupied = EMPTY;
                    }
                }
            }
        }
    }

	void shiftActivePiece(char direction) 
    {

        // User Shifts Left, Right, Or Fast Down
		if (upcomingShape.empty()) {
			return;
		}

		bool canShift = true;
        switch (direction)
        {

		    case 'L': // Shift left
			    for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
				    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
					    if (upcomingShape[i][j]) {
						    unsigned int targetRow = activePieceRow + i;
						    unsigned int targetCol = activePieceCol + j - 1;
						    // Check if the target cell is within bounds and not occupied
						    if (targetCol < 0 || targetCol >= cols || gameGrid[targetRow][targetCol].occupied == OCCUPIED) {
							    canShift = false;
							    break;
						    }
					    }
				    }
				    if (!canShift) break;
			    }
			    if (canShift) {
				    // Clear the current shape from the board
				    for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
					    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
						    if (upcomingShape[i][j]) {
							    clearCell(activePieceRow + i, activePieceCol + j);
						    }
					    }
				    }
				    // Move the shape left
				    activePieceCol--;
				    // Draw the shape in its new position
				    for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
					    for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
						    if (upcomingShape[i][j]) {
							    
							    setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
						    }
					    }
				    }
			    }
			    break;
			case 'R': // Shift right
				for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
					for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
						if (upcomingShape[i][j]) {
							unsigned int targetRow = activePieceRow + i;
							unsigned int targetCol = activePieceCol + j + 1;
							// Check if the target cell is within bounds and not occupied
							if (targetCol >= cols || gameGrid[targetRow][targetCol].occupied == OCCUPIED) {
								canShift = false;
								break;
							}
						}
					}
					if (!canShift) break;
				}
				if (canShift) {
					// Clear the current shape from the board
					for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
						for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
							if (upcomingShape[i][j]) {
								clearCell(activePieceRow + i, activePieceCol + j);
							}
						}
					}
					// Move the shape right
					activePieceCol++;
					// Draw the shape in its new position
					for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
						for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
							if (upcomingShape[i][j]) {
								setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
							}
						}
					}
				}
				break;
			case 'D': // Fast down
				while (true) {
					bool canShiftDown = true;
					for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
						for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
							if (upcomingShape[i][j]) {
								unsigned int targetRow = activePieceRow + i + 1;
								unsigned int targetCol = activePieceCol + j;
								// Check if the target cell is within bounds and not occupied
								if ((targetRow >= rows) || (targetCol >= cols) ||
									(gameGrid[targetRow][targetCol].occupied == OCCUPIED))
								{
									canShiftDown = false; // Cannot shift down, hit the bottom or another piece
									break;
								}
							}
						}
						if (!canShiftDown) break;
					}
					if (!canShiftDown) break;
					// Clear the current shape from the board
					for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
						for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
							if (upcomingShape[i][j]) {
								clearCell(activePieceRow + i, activePieceCol + j);
							}
						}
					}
					// Move the shape down
					activePieceRow++;

				}

				// Draw the shape in its new position
				for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
					for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
						if (upcomingShape[i][j]) {
							// Set the cell color
							setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
						}
					}
				}
				break;  
        }
				
            

    }

    void rotateActivePiece()
	{
		
		if (upcomingShape.empty()) {
			return;
        }

		// Create a new rotated shape
		std::vector<std::vector<bool>> rotatedShape(upcomingShape[0].size(), std::vector<bool>(upcomingShape.size(), false));

		for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
			for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
				rotatedShape[j][upcomingShape.size() - 1 - i] = upcomingShape[i][j];
			}
		}

		// Check if the rotated shape can fit in the current position
		bool canRotate = true;
		for (unsigned int i = 0; i < rotatedShape.size(); ++i) {
			for (unsigned int j = 0; j < rotatedShape[i].size(); ++j) {
				if (rotatedShape[i][j]) {
					unsigned int targetRow = activePieceRow + i;
					unsigned int targetCol = activePieceCol + j;
					// Check if the target cell is within bounds and not occupied
					if (targetRow >= rows || targetCol >= cols || gameGrid[targetRow][targetCol].occupied == OCCUPIED) {
						canRotate = false;
						break;
					}
				}
			}
			if (!canRotate) break;
		}

		if (canRotate) {
			// Clear the current shape from the board
			for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
				for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
					if (upcomingShape[i][j]) {
						clearCell(activePieceRow + i, activePieceCol + j);
					}
				}
			}
			// Update the upcoming shape to the rotated version
			upcomingShape = rotatedShape;
			// Draw the rotated shape in its new position
			for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
				for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
					if (upcomingShape[i][j]) {
						setCellColor(activePieceRow + i, activePieceCol + j, this->activePieceColor);
					}
				}
			}
		}

	}

	void ghostPiece()
	{

		if (upcomingShape.empty())
		{
			return; // No upcoming shape, no ghost piece
		}

		// Create a ghost piece that shows where the active piece would land
		unsigned int ghostRow = activePieceRow;
		unsigned int ghostCol = activePieceCol;
		// Move the ghost piece down until it can't go further
		while (true) {
            // Check if the ghost piece can shift down
			bool canShiftDown = true;
			for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
				for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
					if (upcomingShape[i][j]) {
						unsigned int targetRow = ghostRow + i + 1;
						unsigned int targetCol = ghostCol + j;
						// Check if the target cell is within bounds and not occupied
						if ((targetRow >= rows) || (targetCol >= cols) ||
							(gameGrid[targetRow][targetCol].occupied == OCCUPIED))
						{
							canShiftDown = false; // Cannot shift down, hit the bottom or another piece
							break;
						}
					}
				}
				if (!canShiftDown) break;
			}
			if (!canShiftDown) break;
			ghostRow++;
		}
		// Clear any previous ghost piece
		for (unsigned int i = 0; i < rows; ++i) {
			for (unsigned int j = 0; j < cols; ++j) {
				if (gameGrid[i][j].occupied == GHOST) {
					clearCell(i, j);
				}
			}
		}

		if (ghostRow - activePieceRow < 4)
		{
			// If the ghost piece is too close to the active piece, don't draw it
			return;
		}

		// Draw the ghost piece in its position
		for (unsigned int i = 0; i < upcomingShape.size(); ++i) {
			for (unsigned int j = 0; j < upcomingShape[i].size(); ++j) {
				if (upcomingShape[i][j]) {
					setCellColor(ghostRow + i, ghostCol + j, glm::vec3(0.5f, 0.5f, 0.5f)); // Gray color for ghost piece
					gameGrid[ghostRow + i][ghostCol + j].occupied = GHOST; // Mark as ghost
				}
			}
		}
	}

    void checkForPoints()
    {
        glm::vec3 oldColor;
        bool rowCleared = false;
        int rowsCleared = 0;

        // Check for completed rows
        for (unsigned int row = 0; row < rows; ++row) {
            bool complete = true;
            for (unsigned int col = 0; col < cols; ++col) {
                if (gameGrid[row][col].occupied == EMPTY) {
                    complete = false;
                    break;
                }
            }
            if (complete) {
                rowCleared = true;
                rowsCleared++;

                // Clear the completed row
                for (unsigned int col = 0; col < cols; ++col) {
                    clearCell(row, col);
                    gameGrid[row][col].occupied = EMPTY;
                }

                // Shift all rows above down
                for (int r = row - 1; r >= 0; --r) {
                    for (unsigned int c = 0; c < cols; ++c) {
                        if (gameGrid[r][c].occupied == OCCUPIED) {
                            // Grab the color of the cell above
                            oldColor = getBlockColor(r, c);
                            clearCell(r, c);
                            setCellColor(r + 1, c, oldColor);
                            gameGrid[r + 1][c].occupied = OCCUPIED;
                            gameGrid[r][c].occupied = EMPTY;
                        }
                    }
                }

                // Recheck the current row after shifting
                row--;
            }
        }

        if (rowCleared) {
            // Add score based on rows cleared
            this->score += rowsCleared * 100;
            this->totalRowsCleared += rowsCleared;
    
            if (!(this->totalRowsCleared % 5))
            {
                this->level++;
            }

            this->updateText = true;


        }
    }

    ~Board() {
        // Cleanup board buffers
        if (boardVBO) {
            glDeleteBuffers(1, &boardVBO);
            glDeleteBuffers(1, &boardEBO);
        }

        // Cleanup game grid
        if (gameGrid) {
            for (unsigned int i = 0; i < rows; ++i) {
                delete[] gameGrid[i];
            }
            delete[] gameGrid;
        }
    }
};