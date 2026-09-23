# 3D Airport Simulator, Simple Code Guide (for the group)

This is a 3D airport game written in **C** using **OpenGL + FreeGLUT**.
You walk through Hazrat Shahjalal Airport (Dhaka), check in, clear security, board a
Biman flight to London, and watch it take off. Day/night, 4 cameras.

The code is split into small files. Each file is a **module**, it does one job.
Think of it like this: some files **draw things**, some files **run the logic**, and one
file **starts the program**.


## What every file does (one line each)

| File | What it does (simple) |
|------|------------------------|
| `main.c` | Starts the program: opens the window, reads keyboard/mouse, switches cameras, runs the loop. |
| `layout.h` | Just numbers, the positions and sizes of the airport (where the terminal, runway, etc. are). |
| `gfx.c` / `gfx.h` | The **drawing toolbox** everyone uses: draw a box, a cylinder, set colours/materials, make textures, draw text. |
| `camera.c` / `camera.h` | The first-person camera, walking with W/A/S/D, mouse look, and not walking through walls (collision). |
| `aircraft.c` / `aircraft.h` | The **airplane** model (body, wings, tail, engines, wheels) and its takeoff/landing movement. |
| `scene.c` / `scene.h` | The **big world file**: builds the terminal, runway, control tower, village, plane cabin, and puts everyone together each frame. |
| `story.c` / `story.h` | The **journey logic**: the step-by-step story (check-in → baggage → security → gate → board → takeoff). |
| `vehicles.c` / `vehicles.h` | Draws all the **vehicles**: cars, CNG auto-rickshaw, bus, fuel/baggage trucks, boarding stairs. |
| `people.c` / `people.h` | Draws the **human characters** (passengers, staff): standing, walking, sitting, pointing, talking. |
| `hud.c` / `hud.h` | The **2D screen text**: the info box top-left, controls panel, the "✓ complete" banners, captions. |


## How we split it for 2 people

We divide the code so each person **owns** a few files and can explain them.

### 🟢 Member B, the EASY part (self-contained "draw" files)

These 3 modules are the simplest. They only **draw shapes**, they don't control the game,
they just make the models look right. You can understand them without reading the rest.

- **`vehicles.c` / `vehicles.h`**, draws every vehicle out of simple boxes and cylinders
  (car, CNG, bus, trucks, stairs, and the spinning wheels). One function per vehicle.
- **`people.c` / `people.h`**, draws a human figure from boxes/spheres (body, head, arms,
  legs) and animates walking, sitting, and hand gestures.
- **`hud.c` / `hud.h`**, draws the flat 2D text and panels on top of the screen
  (info box, controls list, the green "complete" banners, the flag/complete screens).

**Member B should be able to explain:** how a car / person is built from basic shapes,
how the wheels/legs move, and how the on-screen text panels are drawn.

### 🔵 Member A, the core part (the rest)

- **`main.c`**, program start, window, input, camera modes, the main loop.
- **`scene.c` / `scene.h`**, the whole airport world + the cut-scenes (cabin, village).
- **`story.c` / `story.h`**, the step-by-step passenger journey.
- **`aircraft.c` / `aircraft.h`**, the plane model + takeoff/landing motion.
- **`camera.c` / `camera.h`**, walking camera + wall collision.
- **`gfx.c` / `gfx.h`**, the shared drawing toolbox.
- **`layout.h`**, the world position/size constants.

**Member A should be able to explain:** how the program starts and loops, how the world is
built, how the story/steps work, and how the plane flies.

> Note: **all files use `gfx.c`** (the toolbox) and **`scene.c` calls `vehicles`, `people`,
> `aircraft` and `hud`**. So Member B's files are *used by* Member A's `scene.c`, but you can
> still learn and explain them separately.


## How to build and run

Windows (MinGW + FreeGLUT):
```
gcc -Wall -Wextra -std=c99 -O2 *.c -o airport_simulator.exe -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
```
To just run it: keep `airport_simulator.exe` and `freeglut.dll` in the same folder and
double-click the exe (64-bit Windows).

**Controls:** W/A/S/D walk · mouse look · E/Y/N talk at counters · F1–F4 cameras ·
1/2 takeoff/landing · L day/night · R restart · M hide panel · Esc quit.
