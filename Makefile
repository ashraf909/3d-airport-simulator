# 3D Airport Simulator - Hazrat Shahjalal International Airport, Dhaka
# Legacy OpenGL + GLU + FreeGLUT, C99. Target: Windows / MinGW.

CC      = gcc
CFLAGS  = -Wall -Wextra -std=c99 -O2
SRC     = main.c camera.c gfx.c scene.c aircraft.c vehicles.c people.c story.c hud.c
OUT     = airport_simulator.exe
LIBS    = -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32

all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(OUT) $(LIBS)

# Convenience target that builds and runs.
run: $(OUT)
	./$(OUT)

clean:
	del /Q $(OUT) 2>nul || rm -f $(OUT)

.PHONY: all run clean
