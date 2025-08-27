#include "window.h"
#include <chrono>
#include <thread>



constexpr unsigned int WINDOW_H = 1200, WINDOW_W = 1200;


int main()
{

	// Create Our Window
	Window gameWindow(WINDOW_H, WINDOW_W, "Tetris");
	if (!gameWindow.initialize())
	{
		return -1; // Initialization failed
	}


	// Throttle to 60 FPS (16.67ms per frame)
	const std::chrono::duration<double, std::milli> frameDuration(1000.0 / 45.0);
	std::chrono::steady_clock::time_point frameStart, frameEnd;
	std::chrono::nanoseconds frameTime;

	unsigned int frameCount = 0;

	while (!gameWindow.shouldClose())
	{

		// Start measuring frame time
		frameStart = std::chrono::high_resolution_clock::now();

		gameWindow.clear();
		gameWindow.pollEvents();

		// Specify Update Based On State
		switch (gameWindow.getCurrentState())
		{

			case Window::GameState::MAIN_MENU:
				break;
			case Window::GameState::PLAYING:
				if (++gameWindow.tickCount == (gameWindow.getDropSpeed()))
				{
					gameWindow.update(); // Update game logic
					gameWindow.needsRedraw = true; // Mark for redraw

					if (gameWindow.isGameOver())
					{
						gameWindow.setCurrentState(Window::GameState::GAME_OVER);
						gameWindow.addToScoreboard(gameWindow.getScore());
						gameWindow.setupGameOverText();
					
					}

				}
				break;
			case Window::GameState::GAME_OVER:
				break;

		}

		// If we need to redraw, do so
		if (gameWindow.needsRedraw)
		{

			// Pick Rendering Technique Based On State
			switch (gameWindow.getCurrentState()) 
			{
				case Window::GameState::MAIN_MENU:
					gameWindow.renderMainMenuBackground();
					gameWindow.renderText();
					break;

				case Window::GameState::PLAYING:
					gameWindow.renderGameBackground(); // Uses bgColor for animations
					gameWindow.renderUI();
					break;

				case Window::GameState::GAME_OVER:
					//gameWindow.renderGameOverBackground();
					gameWindow.renderText();
					break;
			}

			gameWindow.swapBuffers();
			gameWindow.needsRedraw = false; // Reset redraw flag

		}

		// Calculate how much time we should wait
		frameEnd = std::chrono::high_resolution_clock::now();
		frameTime = frameEnd - frameStart;

		// If we completed the frame faster than our target duration, sleep for the remainder
		if (frameTime < frameDuration)
		{
			std::this_thread::sleep_for(frameDuration - frameTime);
		}

	}


}