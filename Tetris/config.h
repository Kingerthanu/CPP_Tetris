#pragma once


namespace CONFIG
{


    // ==============================================
    // GAME TIMING & PERFORMANCE SETTINGS
    // ==============================================

    // Base Game Speed (Lower = Faster)
    constexpr unsigned int BASE_DROP_SPEED = 13;

    // Frame Rate Settings
    constexpr double TARGET_FPS = 45.0;
    constexpr double FRAME_TIME_MS = 1000.0 / TARGET_FPS;



    // ==============================================
    // WINDOW & DISPLAY SETTINGS
    // ==============================================

    // Window Dimensions
    constexpr unsigned int WINDOW_WIDTH = 1200;
    constexpr unsigned int WINDOW_HEIGHT = 1200;
    constexpr const char* WINDOW_TITLE = "Tetris";

    // Game Board Dimensions
    constexpr unsigned int BOARD_ROWS = 20;
    constexpr unsigned int BOARD_COLS = 10;



    // ==============================================
    // TETRIS GAME MECHANICS
    // ==============================================

    // Difficulty Progression
    constexpr unsigned int MAX_LEVEL = 10;
    constexpr unsigned int LINES_PER_LEVEL = 5;
    constexpr float SPEED_CURVE_FACTOR = 11.0f;

    // Scoring System
    constexpr unsigned int POINTS_PER_LINE = 100;
    constexpr unsigned int SPACE_BAR_POINTS = 100;

    // Ghost Piece Settings
    constexpr unsigned int MIN_GHOST_DISTANCE = 4;



    // ==============================================
    // UI & RENDERING SETTINGS
    // ==============================================

    // Text Rendering
    constexpr float CHAR_SPACING = 0.045f;
    constexpr unsigned int FONT_ATLAS_SIZE = 128;
    constexpr float CHAR_PIXEL_SIZE = 8.0f;
    constexpr float CHAR_SIZE = 1.0f / 16.0f;

    // Background Colors (RGB values 0.0-1.0)
    namespace COLORS
    {
        constexpr float BACKGROUND_TRANSITION_SPEED = 0.01f;
        constexpr int COLOR_CHANGE_TIMER_MIN = 5;
        constexpr int COLOR_CHANGE_TIMER_MAX = 10;

        // Grid Colors
        constexpr float GRID_R = 0.3f;
        constexpr float GRID_G = 0.3f;
        constexpr float GRID_B = 0.3f;

        // Ghost Piece Color
        constexpr float GHOST_R = 0.5f;
        constexpr float GHOST_G = 0.5f;
        constexpr float GHOST_B = 0.5f;

        // Clear/Background Color
        constexpr float CLEAR_R = 0.1f;
        constexpr float CLEAR_G = 0.1f;
        constexpr float CLEAR_B = 0.1f;
    }


    // Leaderboard Settings
    constexpr unsigned int MAX_LEADERBOARD_ENTRIES = 10;
    constexpr const char* LEADERBOARD_FILE = "leaderboard.txt";



    // ==============================================
    // SHADER FILE PATHS
    // ==============================================

    namespace SHADERS
    {
        constexpr const char* BACKGROUND_VERTEX = "./background.vert";
        constexpr const char* BACKGROUND_FRAGMENT = "./background.frag";
        constexpr const char* TEXT_VERTEX = "./text.vert";
        constexpr const char* TEXT_FRAGMENT = "./text.frag";
    }


}