#ifndef AIRPORT_VEHICLES_H
#define AIRPORT_VEHICLES_H

/* All vehicles are modelled with +x = forward, sitting on the ground (y=0).
   'spin' is the wheel rotation angle in degrees. 'night' toggles lit lamps. */
void Veh_Car   (float x,float z,float yaw,float r,float g,float b,float spin,int night);
void Veh_Taxi  (float x,float z,float yaw,float spin,int night);
void Veh_Bus    (float x,float z,float yaw,float spin,int night);
void Veh_FuelTruck (float x,float z,float yaw,float spin,int night);
void Veh_BaggageTug(float x,float z,float yaw,float spin,int night);
void Veh_Pushback  (float x,float z,float yaw,float spin,int night);
void Veh_CNG       (float x,float z,float yaw,float spin,int night);  /* Bangladeshi auto-rickshaw */
void Veh_Stairs    (float x,float z,float yaw,int night);             /* mobile boarding stairs */

#endif
