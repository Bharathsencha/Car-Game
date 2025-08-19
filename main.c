#include <raylib.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define BACKGROUND_COLOR (Color){186,149,127}
#define CAR_COLOR BLACK
#define ROTATION_SPEED 52

typedef enum {
    MENU,
    GAME,
    SETTINGS
} GameState;

int main(){
    int width = 1300;
    int height = 1000;
    InitWindow(width, height, "Raceee");
    
    InitAudioDevice();
    
    GameState current_state = MENU;
    int target_fps = 60;
    SetTargetFPS(target_fps);
    
    Texture2D menu_background = LoadTexture("home_page_background.jpg");
    Sound button_sound = LoadSound("coin-collect-retro-8-bit-sound-effect-145251.mp3");
    Music menu_music = LoadMusicStream("8-bit-heaven-26287.mp3");
    
    Texture2D arrow_left = LoadTexture("left-arrow.png");
    Texture2D arrow_right = LoadTexture("right-arrow.png");
    
    PlayMusicStream(menu_music);
    
    Rectangle play_button = {width/2 - 100, height/2 - 50, 200, 60};
    Rectangle settings_button = {width/2 - 100, height/2 + 50, 200, 60};
    
    Rectangle back_button = {50, 50, 100, 40};
    Rectangle fps_left_arrow = {width/2 - 80, height/2 + 5, 30, 30};
    Rectangle fps_right_arrow = {width/2 + 50, height/2 + 5, 30, 30};
    
    int world_width = 15000;  
    int world_height = 15000;
    
    Image soil_image = LoadImage("Soil_Tile.png");
    ImageRotateCW(&soil_image);
    Texture2D soil_texture = LoadTextureFromImage(soil_image);
    
    Image car_image = LoadImage("Car_1_01.png");
    ImageRotateCW(&car_image);
    Texture2D car_texture = LoadTextureFromImage(car_image);
    
    Rectangle car_texture_rec = {
        .x = 0,
        .y = 0,
        .width = car_texture.width,
        .height = car_texture.height,
    };
    
    int desired_car_width = 120;
    float scale_factor = (float)desired_car_width / car_texture.width;
    int car_width = desired_car_width;
    int car_height = (int)(car_texture.height * scale_factor);
  
    float car_x = world_width/2 - car_width/2;
    float car_y = world_height/2 - car_height/2;
    float car_speed = 0;
    float car_max_speed = 100;        
    int car_direction = -1;           
    float car_rotation = -90;
    float car_speedup = 10;           
    float car_slowdown = 10;          

    Camera2D camera = {
        .offset = (Vector2){width/2, height/2},
        .target = (Vector2){car_x + car_width/2, car_y + car_height/2},
        .rotation = 0,
        .zoom = 1.0,
    };
    
    float camera_threshold = 100.0f;  
    
    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 mouse_pos = GetMousePosition();
        
        UpdateMusicStream(menu_music);
        
        switch(current_state) {
            case MENU:
                if (CheckCollisionPointRec(mouse_pos, play_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    StopMusicStream(menu_music);
                    current_state = GAME;
                }
                if (CheckCollisionPointRec(mouse_pos, settings_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    current_state = SETTINGS;
                }
                break;
                
            case SETTINGS:
                if (CheckCollisionPointRec(mouse_pos, back_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    current_state = MENU;
                }
                if (CheckCollisionPointRec(mouse_pos, fps_left_arrow) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    if (target_fps == 60) {
                        target_fps = 30;
                        SetTargetFPS(target_fps);
                    }
                }
                if (CheckCollisionPointRec(mouse_pos, fps_right_arrow) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    PlaySound(button_sound);
                    if (target_fps == 30) {
                        target_fps = 60;
                        SetTargetFPS(target_fps);
                    }
                }
                break;
                
            case GAME:
                if(IsKeyDown(KEY_UP)){
                    car_direction = -1;
                    car_speed += car_speedup * dt;
                    if(car_speed > car_max_speed){
                        car_speed = car_max_speed;
                    }
                }
                else if(IsKeyDown(KEY_DOWN)){
                    car_direction = 1;
                    car_speed -= car_speedup * dt;
                    if(car_speed > car_max_speed){
                        car_speed = car_max_speed;
                    }
                }
                else {
                    car_speed += car_slowdown * dt * car_direction;
                    if(car_direction == -1 && car_speed < 0){
                        car_speed = 0;
                    }
                    else if(car_direction == 1 && car_speed > 0){
                        car_speed = 0;
                    }
                }

                if(IsKeyDown(KEY_LEFT)){
                    car_rotation -= ROTATION_SPEED * dt;
                }
                else if(IsKeyDown(KEY_RIGHT)){
                    car_rotation += ROTATION_SPEED * dt;
                }
                
                if (car_rotation >= 360) car_rotation -= 360;
                if (car_rotation < 0) car_rotation += 360;
                
                float radian = car_rotation * PI / 180.0f;
                float x_move = car_speed * cosf(radian);
                float y_move = car_speed * sinf(radian);
                
                float new_car_x = car_x + x_move;
                float new_car_y = car_y + y_move;
                
                if (new_car_x >= 0 && new_car_x <= world_width - car_width) {
                    car_x = new_car_x;
                }
                if (new_car_y >= 0 && new_car_y <= world_height - car_height) {
                    car_y = new_car_y;
                }

                Vector2 car_center = {car_x + car_width/2, car_y + car_height/2};
                Vector2 screen_center = {camera.target.x, camera.target.y};
                
                float distance_x = car_center.x - screen_center.x;
                float distance_y = car_center.y - screen_center.y;
                float total_distance = sqrt(distance_x * distance_x + distance_y * distance_y);
                
                if (total_distance > camera_threshold) {
                    float lerp_speed = 2.0f * dt;
                    camera.target.x += distance_x * lerp_speed;
                    camera.target.y += distance_y * lerp_speed;
                    
                    float camera_margin_x = width / 2;
                    float camera_margin_y = height / 2;
                    
                    if (camera.target.x < camera_margin_x) {
                        camera.target.x = camera_margin_x;
                    }
                    if (camera.target.x > world_width - camera_margin_x) {
                        camera.target.x = world_width - camera_margin_x;
                    }
                    if (camera.target.y < camera_margin_y) {
                        camera.target.y = camera_margin_y;
                    }
                    if (camera.target.y > world_height - camera_margin_y) {
                        camera.target.y = world_height - camera_margin_y;
                    }
                }
                
                if (IsKeyPressed(KEY_ESCAPE)) {
                    PlayMusicStream(menu_music);
                    current_state = MENU;
                }
                break;
        }
        
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        
        switch(current_state) {
            case MENU:
                DrawTexturePro(menu_background, 
                    (Rectangle){0, 0, menu_background.width, menu_background.height},
                    (Rectangle){0, 0, width, height},
                    (Vector2){0, 0}, 0, WHITE);
                
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
                DrawTexturePro(menu_background, 
                    (Rectangle){0, 0, menu_background.width, menu_background.height},
                    (Rectangle){0, 0, width, height},
                    (Vector2){0, 0}, 0, WHITE);
                
                
                DrawText("Settings", width/2 - MeasureText("Settings", 60)/2, height/4, 60, BLACK);
                
                Color back_color = CheckCollisionPointRec(mouse_pos, back_button) ? DARKGRAY : GRAY;
                DrawRectangleRec(back_button, back_color);
                DrawRectangleLines(back_button.x, back_button.y, back_button.width, back_button.height, BLACK);
                DrawText("BACK", 75, 65, 20, WHITE);
                
                DrawText("FPS:", width/2 - 150, height/2 + 10, 30, BLACK);
                
                DrawTexturePro(arrow_left,
                    (Rectangle){0, 0, arrow_left.width, arrow_left.height},
                    fps_left_arrow, (Vector2){0, 0}, 0, WHITE);
                DrawTexturePro(arrow_right,
                    (Rectangle){0, 0, arrow_right.width, arrow_right.height},
                    fps_right_arrow, (Vector2){0, 0}, 0, WHITE);
                
                char fps_text[10];
                sprintf(fps_text, "%d", target_fps);
                DrawText(fps_text, width/2 - MeasureText(fps_text, 30)/2, height/2 + 5, 30, BLACK);
                break;
                
            case GAME:
                BeginMode2D(camera);
                
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
                
                Rectangle car_rec = {
                    .x = car_x,
                    .y = car_y,
                    .width = car_width,
                    .height = car_height,
                };
                
                Vector2 car_origin = {
                    .x = car_width / 2,
                    .y = car_height / 2,
                };
                
                DrawTexturePro(car_texture, car_texture_rec, car_rec, car_origin, car_rotation, WHITE);
                
                EndMode2D();
                
                DrawText("Press ESC to return to menu", 10, 10, 20, WHITE);
                break;
        }
        
        EndDrawing();
    }
    
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

//<a href="https://www.flaticon.com/free-icons/back" title="back icons">Back icons created by Roundicons - Flaticon</a> 
//<a href="https://www.flaticon.com/free-icons/next" title="next icons">Next icons created by Roundicons - Flaticon</a>