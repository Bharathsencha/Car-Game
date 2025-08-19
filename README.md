# Raylib Setup Guide (Linux Mint )


---

## 1. Install Dependencies

Update packages and install required tools:

```bash
sudo apt update
sudo apt install build-essential git cmake pkg-config
sudo apt install libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev
sudo apt install libgl1-mesa-dev libglu1-mesa-dev
```

---

## 2. Download Raylib

Clone the official repository:

```bash
git clone https://github.com/raysan5/raylib.git
cd raylib
```

---

## 3. Build and Install

```bash
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON
make -j$(nproc)
sudo make install
```

This installs raylib into `/usr/local/lib`.

---

## 4. Fix Library Path (if needed)

If you get an error like:

```
error while loading shared libraries: libraylib.so.XXX: cannot open shared object file
```

Run the following:

```bash
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/raylib.conf
sudo ldconfig
```
and Restart your system
---

## 5. Test with a Simple Program

Create `main.c`:

```c
#include "raylib.h"

int main(void) {
    InitWindow(800, 450, "Hello Raylib!");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawText("Raylib is working!", 190, 200, 20, LIGHTGRAY);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
```


Build and run:

```bash
make
make run
```
