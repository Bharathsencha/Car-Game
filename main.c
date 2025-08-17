#include <raylib.h>
#define BACKGROUND_COLOR (Color){186,149,127}
#define CAR_COLOR BLACK

int main(){
    int height = 1300;
    int width = 1000;
    InitWindow(height,width,"Raceee");
    SetTargetFPS(60);

    int car_width = 60;
    int car_height = 100;
    float car_x = width/2 - car_width/2;
    float car_y = height/2 - car_height/2;
    float car_speed = 0;
    float car_max_speed = 100;
    int car_direction = -1;

    while (WindowShouldClose() == false)
    {
        float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground(BACKGROUND_COLOR);
        
        if(IsKeyDown(KEY_UP)){
            car_direction = -1;
            car_speed += 10 * dt;
            if(car_speed > car_max_speed){
                    car_speed = car_max_speed;
            }   
         }
         else if(IsKeyDown(KEY_DOWN)){
            car_direction = 1;
             car_speed -= 10 * dt;
            if(car_speed > car_max_speed){
                    car_speed = car_max_speed;
            }   
        }

        else {
            car_speed += 2 *dt * car_direction;
            if(car_direction == -1 && car_speed < 0){
                car_speed = 0;
            }
            else if(car_direction == 1 && car_speed > 0){
                car_speed = 0;
            }
        }

        
        car_y -= car_speed;

        DrawRectangle(car_x,car_y,car_width,car_height,CAR_COLOR);
        EndDrawing();
    }
    

    return 0;
}