# 3D Airport Simulator, Hazrat Shahjalal International Airport, Dhaka

An interactive, living 3D airport built in **C** with **legacy OpenGL + GLU + FreeGLUT**.
It plays as a guided **passenger-journey scenario**: you arrive at the Dhaka terminal,
check in (the agent asks for your passport), clear security, walk to your boarding gate,
board Biman flight **BG 201 to London**, and watch your jet take off past a Bangladesh-flag
salute and dissolve into the clouds, by day or by night. You can also free-roam, watch
arrivals and departures, and switch between four cameras.

No C++, no engine, no external image files. All textures are generated procedurally in code.

## The passenger scenario (physical, step-by-step)

When the program starts, a jet passes overhead and a welcome prompt appears. Press **E** to
begin, then **physically walk** to each station (a glowing beacon and a top-of-screen objective
guide you). Nothing happens until you are actually standing at the station, a speech bubble
appears over the staff member and the dialogue shows only then:

1. **Check-in Counter 3** (its number is lit on the sign), **your line is empty**, so you're
   served first; the agent (arms moving as they talk) asks for your passport (**Y**), then hands
   you the boarding pass (**E**). Only afterwards do one or two more passengers walk up.
2. **Baggage Drop**, drop your bag; it rides the belt through an **X-ray scanner** (**Y** / **E**).
3. **Security**, walk to the lane and go through the scanner (**Y** / **E**).
4. **Boarding Gate 3**, show your boarding pass (**Y** / **E**).
5. **Board**, reach the boarding point (**E**); the view cuts to the **plane cabin interior**.
6. **Departure**, the jet lines up and takes off; the view cuts to a **Bangladeshi village**
   where people look up and shout *"Oi je dekho, plane!"* as it climbs over, then a Bangladesh
   flag salute plays and the jet dissolves into the clouds. **Journey Complete** → **[R]** / **[Esc]**.

Every completed step pops a green **"✓ …COMPLETE"** banner with a chime. The other five counters
keep light background queues; the flight board overhead shows live times/status to London, Dubai,
New York, Doha, Singapore, Kuala Lumpur, Bangkok, Jeddah and Kolkata. **F4** puts you inside the
**control tower** (controllers at consoles, a supervisor on binoculars).

## Build (Windows / MinGW + FreeGLUT)

```bash
gcc -Wall -Wextra -std=c99 -O2 *.c -o airport_simulator.exe -lfreeglut -lopengl32 -lglu32 -lwinmm -lgdi32
```

or simply:

```bash
mingw32-make
```

or open **airport_simulator.cbp** in Code::Blocks and press Build & Run.

If FreeGLUT is used as a DLL, keep **libfreeglut.dll** next to the executable (it already is).

Then run:

```bash
airport_simulator.exe
```

## Controls

| Key / Input | Action |
|-------------|--------|
| **W A S D** | Walk forward / left / back / right (strafe) |
| **Mouse**   | Look around, moving the mouse **right turns right**, up looks up |
| **Enter**   | Begin the journey / advance a conversation |
| **Y / N**   | Answer the airport staff (yes / no) |
| **Arrow keys** | Alternate look controls |
| **F1**      | First-person camera (walk the airport) |
| **F2**      | Runway camera |
| **F3**      | Aircraft-follow camera |
| **F4**      | Control-tower camera |
| **1**       | Trigger / restart the **takeoff** sequence |
| **2**       | Trigger / restart the **landing** sequence |
| **L**       | Toggle **day / night** |
| **R**       | Restart the passenger journey (and reset position) |
| **M**       | Show / hide the controls panel |
| **Esc**     | Quit |

The HUD (top-left) shows the current area, camera mode, day/night, a live clock,
the takeoff & landing status, and an FPS counter.

## What's in the scene

**Outside / landside**
- Broad terminal with a glass curtain-wall facade and the illuminated sign
  *"HAZRAT SHAHJALAL INTERNATIONAL AIRPORT, DHAKA, BANGLADESH"*
- Parking with marked stalls and parked cars, a drop-off road with moving taxis, buses and cars
- Automatic sliding glass entrance doors, entrance canopy, streetlights, trees, pedestrian crossing
- Animated passengers walking to/from the terminal, waiting with luggage, and hi-vis staff

**Terminal interior**
- Tiled floor, check-in desks with queue posts, security metal-detector arches, X-ray belts
- Departure lounge seating, indoor plants, boarding-gate desks, a rotating baggage carousel
- A **flight-information display** with live times and status (ON TIME / BOARDING / DELAYED)
- Large glass walls facing the apron, you can see the runway and aircraft through them

**Airside**
- Runway with centreline dashes, edge lines, threshold "piano keys" and runway numbers (10 / 28)
- Taxiway with yellow centreline, apron with gate stands and jet bridges
- Runway edge lights, taxiway lights, apron floodlights, a windsock
- Control tower with a continuously **rotating radar** and lit cab windows

**Aircraft (two detailed jet airliners)**
- Fuselage, swept wings with winglets, tail fin + stabilisers, under-wing engines with
  spinning turbine fans, landing gear with rotating wheels, cockpit & passenger windows,
  Biman-style green livery, and red/green/white navigation & strobe lights
- **Aircraft 1** runs the full **takeoff** sequence: gate → pushback → taxi → line-up →
  take-off roll → rotate → climb → depart into the distance
- **Aircraft 2** runs the full **landing** sequence: approach → touchdown → roll-out →
  taxi → park at the gate

**Lighting & environment**
- Day: bright sun with glow, blue sky, drifting clouds, soft fill light
- Night: dark sky with moon and stars, lit runway/taxiway/apron/terminal/vehicle lights
- Directional sun/moon + interior + apron lights, realistic ambient/diffuse/specular materials
  (`glMaterialfv`), procedural textures, alpha-blended glass, and distance fog

## Code layout

| File | Responsibility |
|------|----------------|
| `main.c` | Window, render loop, input, timer, lighting, fog, camera modes |
| `layout.h` | All world positions and constants |
| `camera.c/.h` | First-person camera, mouse look, movement, collision |
| `gfx.c/.h` | Materials, procedural textures, primitives, glass, non-mirrored text |
| `aircraft.c/.h` | Detailed airliner model |
| `vehicles.c/.h` | Cars, taxi, bus, CNG auto-rickshaw, fuel truck, baggage tug, pushback tug, boarding stairs |
| `people.c/.h` | Animated low-poly passengers and staff (two-segment limbs) |
| `scene.c/.h` | Terminal, interior, runway, tower, sky/sun, flag, signage, flight state machines, overhead fly-over |
| `story.c/.h` | The guided passenger-journey scenario (objectives, dialogue, cut-scene) |
| `hud.c/.h` | On-screen HUD, flight status, objective + dialogue + flag + completion overlays |

Build system: `Makefile` and `airport_simulator.cbp` (Code::Blocks).
