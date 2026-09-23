#ifndef AIRPORT_AIRCRAFT_H
#define AIRPORT_AIRCRAFT_H

#define AC_GROUND_CL 3.4f

/* Draw a detailed twin-engine jet airliner in local space, then transformed.
   livery : 0 = Biman-style (green tail), 1 = blue tail
   gearDn : 1 = landing gear extended
   t      : global time (drives turbine spin + strobe blink)
   engine : 1 = engines running (fans spin) */
void Aircraft_Draw(float x,float y,float z,float yaw,float pitch,float roll,
                   int livery,int gearDn,float t,int engine);

#endif
