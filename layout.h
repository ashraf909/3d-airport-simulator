#ifndef AIRPORT_LAYOUT_H
#define AIRPORT_LAYOUT_H

/* ============================================================
   layout.h - all world positions and constants in one place.
   Coordinate convention:
     +x = east  (runs along the terminal face and the runway)
     +y = up
     +z = landside / parking side  (player starts far in +z)
     -z = airside / apron / runway
   ============================================================ */

#define AP_PI      3.14159265358979323846f
#define DEG2RAD(a) ((a)*AP_PI/180.0f)

#define TERMINAL_X0   -72.0f     /* west wall  */
#define TERMINAL_X1    72.0f     /* east wall  */
#define TERMINAL_Z0   -32.0f     /* airside wall (faces runway) */
#define TERMINAL_Z1    26.0f     /* landside wall (faces parking, main entrance) */
#define TERMINAL_H     17.0f     /* wall height */
#define ENTRANCE_HALF   6.5f     /* half width of the sliding-door entrance gap */

/* Landside: parking, drop-off road, gardens */
#define ROAD_Z          30.0f    /* drop-off road just outside the entrance */
#define PARK_Z0         38.0f
#define PARK_Z1        128.0f
#define PARK_X0        -96.0f
#define PARK_X1         96.0f

/* Airside: apron, taxiway, runway */
#define APRON_Z         -78.0f   /* apron stretches from terminal airside to here */
#define TAXIWAY_Z       -96.0f
#define TAXIWAY_HALF      7.0f
#define RUNWAY_Z       -124.0f
#define RUNWAY_HALF      16.0f   /* half width of runway */
#define RUNWAY_X0      -280.0f
#define RUNWAY_X1       280.0f

/* Control tower */
#define TOWER_X         150.0f
#define TOWER_Z         -68.0f

/* Player / camera */
#define PLAYER_RADIUS    0.45f
#define EYE_HEIGHT       1.72f
#define START_X          0.0f
#define START_Z         98.0f    /* start out in the parking area */
#define START_YAW        0.0f    /* yaw 0 looks toward -z, i.e. at the terminal */
#define WORLD_HALF      420.0f   /* half size of the grass ground plane */

#endif
