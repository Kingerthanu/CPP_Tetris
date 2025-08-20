# CPP_Tetris

OpenGL C++ Program Which Emulates A Traditional Tetris Program In A Low-level, Optimized Way. Program Is Mainly Prioritized In The Development Of A More Full E2E Program; Providing Proper Main Menus Before Starting The Game, As Well As Game Over Screens Which Load In The Current Leaderboards Locally. Program Mainly Was To Have A Fun Program To Develop While Keeping Up On Quality Design Decisions While Upholding Proper Encapsulation And Procedural Development When Necessary For A Optimized Runtime.

----


🔍 The Breakdown:

  The Program Will Be Initialized By Firstly Initializing Our Instance Of Our Game Window With Our `Window` Class. This Class Will Work With _GLFW_ To Create A Window, Window Event Handlers, As Well As The Shaders We Will Use On Our GPU Utilizing The `Shader` Class Which Will Call On Our .frag & .vert Files For Specific Shading Protocols. Other Local Initialization Of Member Variables Will Be Done As Well; Mainly For Our `Window::GameState` enum Type For Window State As Well Our Window's `Board` Instance For Our Game Of Tetris--Including Game State Variables--As Well As Rendering For Our Main Menu, Game Menu, As Well As Game Over Menu Utilizing Members Like `backgroundVAO` As Well As `textVAO`. For Text, We Render Based On A 8x8 Procedural Texture Atlas Which Will Quicky Render On The CPU The Text Positioning For Batch Rendering With 1 Draw Call For The Text.
  
  The `Board` Class Will Be Passed In References To Altered Game State In It's Member Variables It Also Will Handle The Main Rendering Of Our Live Game Of Tetris Going On. This Means The Board Will Have Its Own `boardVBO` & `boardEBO` But Will Share A Given `backgroundVAO` With It's Parent `Window`. The Board Will Create Its `gameGrid` For The Active Game Of Tetris As Well As A `previewGrid` Which Will Be Utilized To Display To The User The Next Shape That Will Spawn In After The Current Shape Being Dropped. Other Than This, Other Helper Members Are Initialized Like `vertices` * `indicies` As Well As Offsets In This Shared Vector To Locate Where Specific Sections Of Data Reside--Like Where Lines Are Residing For Line Rendering Vs. Triangle Rendering.

  For Our `Board` Class, We Will Have Fundamental `Block` Instances Which Will Give Organization To The Grid For Logic As A `Block` Instance Will Have Its `BlockState` enum Determining If It's Empty, Occupied, Or A Ghost; As Well As Its Offset (`Block::vboOffset`) In It's Parent Board's `boardVBO`, This Quickly Allows Us To Access And Update Data In The Buffer Without Excessive Parsing.

  After Our Runtime Initialization Of Our `Window` Instance In `driver.cpp` We Get Into Our Rendering Loop Where We Update State-Based Actions Of Our Window (Mainly Game State Updates For When Our Window Has It's `GameState::PLAYING` \[Will Be Utilized For Per-Iteration State-Specific Logic, Like Animations\]). We Then Only Render Changes In The Window If A Redrawing Is Necessary, Utilizing `Widow::needsRedraw`--This Is To Avoid Lazy, Expensive Rendering Calls. When Rendering, We Call `Window::GameState`-Specific Rendering Pipelines. This Simple Update-&-Redraw Staging In Our Mainloop Allows A Timely And Optimized Way For Rendering Our Game.

  When The User Presses **Enter** On Their Keyboard, The `Window` Instance Will State Change Of `Window::GameState` To Start The Rendering, State-Specific Event Handling, And Updating Of A New Instance Of A `Board` That's Orchestrating Our Game Of Tetris. After **5 Rows Cleared** The Drop Speed Of The Blocks Will Speed Up By One Increment (Up To Level 10). After A Game Is Failed, The `Board` Instance Will Set A Shared Flag With `Window` To Determine If We Cleanup, And Display Our Game Over Menu Which Displays Our Score And The User's Ranking In The Locally Stored `scoreboard.txt`. This Simple State Changing Between The 3 `Window::GameState` enums We Allow The Game To Cleanly Update, Render, And Handle Events For Specific States Of Our Running Tetris Process.


  ---
