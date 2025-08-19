#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define BACKGROUND_COLOR (Color){186,149,127}  // Background color for the game window
#define CAR_COLOR BLACK                        // Car color (not directly used since car is textured)
#define ROTATION_SPEED 52                      // Rotation speed of the car in degrees per second

// Enum to define game states (screens)
typedef enum {
    MENU,       // Main menu (shows Play + Settings options)
    GAME,       // Gameplay screen (car driving in the world)
    SETTINGS    // Settings screen (adjust FPS, go back)
} GameState;

int main(){
    // ----------------------------
    // WINDOW AND INITIALIZATION
    // ----------------------------
    int width = 1300;
    int height = 1000;
    InitWindow(width, height, "Raceee");   
    // InitWindow() sets up a window with OpenGL context where everything is drawn.

    InitAudioDevice(); 
    // Initializes the audio system (needed before loading or playing any sounds/music).

    GameState current_state = MENU; // The game starts on the main menu screen
    int target_fps = 60;            // Default FPS set to 60
    SetTargetFPS(target_fps);       
    // SetTargetFPS() makes the game loop run consistently across computers.

    // ----------------------------
    // LOAD RESOURCES (textures, sounds, music)
    // ----------------------------
    Texture2D menu_background = LoadTexture("home_page_background.jpg"); // Menu background image
    Sound button_sound = LoadSound("coin-collect-retro-8-bit-sound-effect-145251.mp3"); // Sound effect when buttons clicked
    Music menu_music = LoadMusicStream("8-bit-heaven-26287.mp3"); // Background music for the menu

    Texture2D arrow_left = LoadTexture("left-arrow.png");  // Arrow for settings menu
    Texture2D arrow_right = LoadTexture("right-arrow.png");

    PlayMusicStream(menu_music); // Start playing background menu music in a loop

    // ----------------------------
    // UI BUTTONS (Rectangles used for detecting clicks)
    // ----------------------------
    Rectangle play_button = {width/2 - 100, height/2 - 50, 200, 60};     // "Play" button in menu
    Rectangle settings_button = {width/2 - 100, height/2 + 50, 200, 60}; // "Settings" button in menu

    Rectangle back_button = {50, 50, 100, 40}; // Back button in settings screen
    Rectangle fps_left_arrow = {width/2 - 80, height/2 + 5, 30, 30};
    Rectangle fps_right_arrow = {width/2 + 50, height/2 + 5, 30, 30};

    // ----------------------------
    // GAME WORLD (map and textures)
    // ----------------------------
    int world_width = 15000;  // World dimensions (large world for car to move in)
    int world_height = 15000;

    // Load ground texture (tiles repeated for world)
    Image soil_image = LoadImage("Soil_Tile.png");
    ImageRotateCW(&soil_image); // Rotate image to proper orientation
    Texture2D soil_texture = LoadTextureFromImage(soil_image);

    // Load car texture
    Image car_image = LoadImage("Car_1_01.png");
    ImageRotateCW(&car_image);
    Texture2D car_texture = LoadTextureFromImage(car_image);

    // Define the region of the car texture (full image)
    Rectangle car_texture_rec = {
        .x = 0,
        .y = 0,
        .width = car_texture.width,
        .height = car_texture.height,
    };

    // Resize car proportionally to desired width
    int desired_car_width = 120;
    float scale_factor = (float)desired_car_width / car_texture.width;
    int car_width = desired_car_width;
    int car_height = (int)(car_texture.height * scale_factor);

    // ----------------------------
    // CAR VARIABLES
    // ----------------------------
    float car_x = world_width/2 - car_width/2;  // Start car in middle of world
    float car_y = world_height/2 - car_height/2;
    float car_speed = 0;                        // Car's current speed
    float car_max_speed = 100;                  // Maximum car speed
    int car_direction = -1;                     // Forward (-1) or backward (+1)
    float car_rotation = -90;                   // Initial facing direction (up)
    float car_speedup = 10;                     // Acceleration per second
    float car_slowdown = 10;                    // Deceleration when not moving

    // ----------------------------
    // CAMERA (follows car smoothly)
    // ----------------------------
    Camera2D camera = {
        .offset = (Vector2){width/2, height/2},                        // Center camera on screen
        .target = (Vector2){car_x + car_width/2, car_y + car_height/2}, // Target is car position
        .rotation = 0,
        .zoom = 1.0,
    };
    
    float camera_threshold = 100.0f;  // How far car can move before camera follows

    // ----------------------------
    // MAIN GAME LOOP
    // ----------------------------
    while (!WindowShouldClose()) { // Runs until user presses ESC or closes window

        float dt = GetFrameTime(); // Time in seconds between each frame
        // dt is crucial because it makes movement consistent across different FPS.

        Vector2 mouse_pos = GetMousePosition(); // Current mouse position for button clicks

        UpdateMusicStream(menu_music); // Keeps menu music streaming smoothly

        // ----------------------------
        // HANDLE GAME STATES
        // ----------------------------
        switch(current_state) {
            case MENU:
                // Play button clicked -> go to GAME
                if (CheckCollisionPointRec(mouse_pos, play_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    StopMusicStream(menu_music);
                    current_state = GAME;
                }
                // Settings button clicked -> go to SETTINGS
                if (CheckCollisionPointRec(mouse_pos, settings_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    current_state = SETTINGS;
                }
                break;

            case SETTINGS:
                // Back button clicked -> return to MENU
                if (CheckCollisionPointRec(mouse_pos, back_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    current_state = MENU;
                }
                // Left arrow clicked -> set FPS = 30
                if (CheckCollisionPointRec(mouse_pos, fps_left_arrow) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    if (target_fps == 60) {
                        target_fps = 30;
                        SetTargetFPS(target_fps);
                    }
                }
                // Right arrow clicked -> set FPS = 60
                if (CheckCollisionPointRec(mouse_pos, fps_right_arrow) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    if (target_fps == 30) {
                        target_fps = 60;
                        SetTargetFPS(target_fps);
                    }
                }
                break;

            case GAME:                // ----------------------------
                // CAR MOVEMENT
                // ----------------------------
                /**
                 * INPUT → SPEED MODEL (FRAME-INDEPENDENT)
                 * ---------------------------------------
                 * We read the UP/DOWN keys each frame and convert them into acceleration on the car.
                 *
                 *  - dt = GetFrameTime() is the time (in seconds) between frames. Multiplying any
                 *    per-second quantity by dt makes the motion frame-rate independent: at 30 FPS
                 *    or 144 FPS the car covers the same distance per real second.
                 *
                 *  - car_speedup is the acceleration magnitude (units: pixels/second² in this setup).
                 *    When UP is held, speed increases by (car_speedup * dt). When DOWN is held,
                 *    we decrease speed (reverse acceleration) and flip car_direction to +1.
                 *
                 *  - car_direction encodes whether the car is considered moving forward (-1) or
                 *    backward (+1). Your sprite faces upward at -90 degrees, so using -1 for
                 *    forward keeps the sign conventions aligned with your existing rotation math.
                 *
                 *  - The "else" branch models passive slowdown (friction/drag). We push speed back
                 *    toward 0 by adding a value opposite to the current direction. Then we clamp
                 *    across zero so the car doesn’t start accelerating in the opposite direction
                 *    just because slowdown overshot.
                 *
                 *  - car_max_speed caps the absolute value of speed so the car can’t accelerate
                 *    forever.
                 */
                if(IsKeyDown(KEY_UP)){
                    car_direction = -1; // Move forward
                    car_speed += car_speedup * dt; // Accelerate
                    if(car_speed > car_max_speed) car_speed = car_max_speed;
                }
                else if(IsKeyDown(KEY_DOWN)){
                    car_direction = 1; // Move backward
                    car_speed -= car_speedup * dt;
                    if(car_speed > car_max_speed) car_speed = car_max_speed;
                }
                else {
                    // Apply slowdown when no key pressed
                    car_speed += car_slowdown * dt * car_direction;
                    if(car_direction == -1 && car_speed < 0) car_speed = 0;
                    else if(car_direction == 1 && car_speed > 0) car_speed = 0;
                }                // Car rotation (turn left/right)
                /**
                 * ROTATION CONTROL
                 * ----------------
                 * Holding LEFT/RIGHT changes car_rotation by ROTATION_SPEED degrees per second.
                 * We scale by dt so it turns at the same angular speed regardless of FPS.
                 *
                 * car_rotation is in DEGREES (0–360). Positive values rotate clockwise in Raylib's
                 * screen coordinate system (Y grows downward). We normalize later to keep it within
                 * [0, 360) which avoids numeric drift.
                 */
                if(IsKeyDown(KEY_LEFT))  car_rotation -= ROTATION_SPEED * dt;
                else if(IsKeyDown(KEY_RIGHT)) car_rotation += ROTATION_SPEED * dt;                // Normalize rotation angle to [0, 360)
                /**
                 * ANGLE NORMALIZATION
                 * -------------------
                 * Keeping angles within a fixed range prevents overflow and simplifies logic.
                 * Any value < 0 gets 360 added, any value ≥ 360 gets 360 subtracted.
                 * This preserves direction while avoiding huge or negative angles.
                 */
                if (car_rotation >= 360) car_rotation -= 360;
                if (car_rotation < 0) car_rotation += 360;                // Convert car rotation into movement vector
                /**
                 * FROM ANGLE + SPEED → X/Y VELOCITY
                 * ---------------------------------
                 * We must convert car_rotation (degrees) to radians for C’s trig functions.
                 *
                 *   radian = car_rotation * PI / 180
                 *   x_move = car_speed * cos(radian)
                 *   y_move = car_speed * sin(radian)
                 *
                 * Note on axes: In screen coords, +X is right, +Y is DOWN. Because your sprite
                 * was rotated to face "up" and you use direction -1 for forward, the movement
                 * appears natural (up means smaller Y). If you ever flip sprite orientation,
                 * be ready to adjust the sign conventions here.
                 */
                float radian = car_rotation * PI / 180.0f;
                float x_move = car_speed * cosf(radian);
                float y_move = car_speed * sinf(radian);                // Move car but keep inside world bounds
                /**
                 * WORLD BOUNDARIES
                 * ----------------
                 * We compute tentative new_car_x/y, then only commit if the car remains inside
                 * the world rectangle [0..world_width - car_width] × [0..world_height - car_height].
                 * This prevents the camera/car from leaving the tile field.
                 */
                float new_car_x = car_x + x_move;
                float new_car_y = car_y + y_move;
                if (new_car_x >= 0 && new_car_x <= world_width - car_width) car_x = new_car_x;
                if (new_car_y >= 0 && new_car_y <= world_height - car_height) car_y = new_car_y;                // ----------------------------
                // CAMERA FOLLOWING
                // ----------------------------
                /**
                 * CAMERA LERP WITH DEAD-ZONE
                 * --------------------------
                 * We point camera.target at the car but only move it when the car drifts farther
                 * than `camera_threshold` from the current target (a dead-zone). This avoids a
                 * jittery camera that constantly chases minor movements.
                 *
                 * Steps:
                 *   1) Compute vector from current camera.target to car_center.
                 *   2) If the distance exceeds camera_threshold, nudge camera.target toward the car
                 *      by a small fraction each frame: target += delta * (lerp_speed * dt).
                 *      This is exponential smoothing (a simple first-order low-pass filter).
                 *   3) Clamp camera.target so that, with the current viewport size, the camera
                 *      never shows space outside the world. We use half the screen size as margin.
                 *
                 * Tuning tips:
                 *   - Increase threshold for a looser camera; decrease for a tighter follow.
                 *   - Increase lerp_speed for snappier camera motion.
                 */
                Vector2 car_center = {car_x + car_width/2, car_y + car_height/2};
                Vector2 screen_center = {camera.target.x, camera.target.y};
                float distance_x = car_center.x - screen_center.x;
                float distance_y = car_center.y - screen_center.y;
                float total_distance = sqrt(distance_x * distance_x + distance_y * distance_y);

                if (total_distance > camera_threshold) {
                    float lerp_speed = 2.0f * dt; // smooth camera movement
                    camera.target.x += distance_x * lerp_speed;
                    camera.target.y += distance_y * lerp_speed;                // Clamp camera to world boundaries
                /**
                 * CAMERA CLAMPING
                 * ---------------
                 * camera.target represents the world-space point that will appear at the center
                 * of the screen (because camera.offset = screen_center). To ensure the camera
                 * never shows beyond the world edges, we clamp target so that the visible extents
                 * (half-width/half-height around target) stay inside the world rect.
                 */
                    float camera_margin_x = width / 2;
                    float camera_margin_y = height / 2;
                    if (camera.target.x < camera_margin_x) camera.target.x = camera_margin_x;
                    if (camera.target.x > world_width - camera_margin_x) camera.target.x = world_width - camera_margin_x;
                    if (camera.target.y < camera_margin_y) camera.target.y = camera_margin_y;
                    if (camera.target.y > world_height - camera_margin_y) camera.target.y = world_height - camera_margin_y;
                }                // ESC -> return to MENU
                /**
                 * STATE SHORTCUT
                 * --------------
                 * Pressing ESC during gameplay switches back to MENU and resumes menu music.
                 * Using IsKeyPressed ensures this triggers once per key press, not every frame
                 * the key is held.
                 */
                if (IsKeyPressed(KEY_ESCAPE)) {
                    PlayMusicStream(menu_music);
                    current_state = MENU;
                }
                break;
        }

        // ----------------------------
        // DRAWING
        // ----------------------------
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);

        switch(current_state) {
            case MENU:
                // Draw menu screen with buttons
                DrawTexturePro(menu_background, (Rectangle){0, 0, menu_background.width, menu_background.height}, (Rectangle){0, 0, width, height}, (Vector2){0, 0}, 0, WHITE);
                DrawText("Raceee", width/2 - MeasureText("Raceee", 80)/2, height/4, 80, BLACK);
                Color play_color = CheckCollisionPointRec(mouse_pos, play_button) ? DARKGRAY : GRAY;
                Color settings_color = CheckCollisionPointRec(mouse_pos, settings_button) ? DARKGRAY : GRAY;
                DrawRectangleRec(play_button, play_color);
                DrawRectangleLines(play_button.x, play_button.y, play_button.width, play_button.height, BLACK);
                DrawText("PLAY", width/2 - MeasureText("PLAY", 30)/2, height/2 - 35, 30, WHITE);
                DrawRectangleRec(settings_button, settings_color);
                DrawRectangleLines(settings_button.x, settings_button.y, settings_button.width, settings_button.height, BLACK);
                DrawText("SETTINGS", width/2 - MeasureText("SETTINGS", 30)/2, height/2 + 65, 30, WHITE);
                break;

            case SETTINGS:
                // Draw settings screen with FPS adjustment
                DrawTexturePro(menu_background, (Rectangle){0, 0, menu_background.width, menu_background.height}, (Rectangle){0, 0, width, height}, (Vector2){0, 0}, 0, WHITE);
                DrawText("Settings", width/2 - MeasureText("Settings", 60)/2, height/4, 60, BLACK);
                Color back_color = CheckCollisionPointRec(mouse_pos, back_button) ? DARKGRAY : GRAY;
                DrawRectangleRec(back_button, back_color);
                DrawRectangleLines(back_button.x, back_button.y, back_button.width, back_button.height, BLACK);
                DrawText("BACK", 75, 65, 20, WHITE);
                DrawText("FPS:", width/2 - 150, height/2 + 10, 30, BLACK);
                DrawTexturePro(arrow_left,(Rectangle){0, 0, arrow_left.width, arrow_left.height}, fps_left_arrow, (Vector2){0, 0}, 0, WHITE);
                DrawTexturePro(arrow_right,(Rectangle){0, 0, arrow_right.width, arrow_right.height}, fps_right_arrow, (Vector2){0, 0}, 0, WHITE);
                char fps_text[10];
                sprintf(fps_text, "%d", target_fps);
                DrawText(fps_text, width/2 - MeasureText(fps_text, 30)/2, height/2 + 5, 30, BLACK);
                break;

            case GAME:
                // Draw the world (tiles + car)
                BeginMode2D(camera); // Enable camera mode
                int tile_size = 512;
                int start_tile_x = (int)((camera.target.x - width/2) / tile_size) - 1;
                int start_tile_y = (int)((camera.target.y - height/2) / tile_size) - 1;
                int end_tile_x = (int)((camera.target.x + width/2) / tile_size) + 1;
                int end_tile_y = (int)((camera.target.y + height/2) / tile_size) + 1;
                if (start_tile_x < 0) start_tile_x = 0;
                if (start_tile_y < 0) start_tile_y = 0;
                if (end_tile_x > world_width / tile_size) end_tile_x = world_width / tile_size;
                if (end_tile_y > world_height / tile_size) end_tile_y = world_height / tile_size;
                for(int x = start_tile_x; x <= end_tile_x; x++){
                    for(int y = start_tile_y; y <= end_tile_y; y++){
                        DrawTexture(soil_texture, x * tile_size, y * tile_size, WHITE);
                    }
                }
                Rectangle car_rec = {.x = car_x,.y = car_y,.width = car_width,.height = car_height};
                Vector2 car_origin = {.x = car_width / 2,.y = car_height / 2};
                DrawTexturePro(car_texture, car_texture_rec, car_rec, car_origin, car_rotation, WHITE);
                EndMode2D();
                DrawText("Press ESC to return to menu", 10, 10, 20, WHITE);
                break;
        }

        EndDrawing(); // End frame rendering
    }

    // ----------------------------
    // CLEANUP (unload resources)
    // ----------------------------
    UnloadTexture(menu_background);
    UnloadTexture(arrow_left);
    UnloadTexture(arrow_right);
    UnloadSound(button_sound);
    UnloadMusicStream(menu_music);
    UnloadImage(car_image);
    UnloadTexture(car_texture);
    UnloadTexture(soil_texture);
    UnloadImage(soil_image);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

// Credits: Back/Next icons from Flaticon
