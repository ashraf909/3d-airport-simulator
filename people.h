#ifndef AIRPORT_PEOPLE_H
#define AIRPORT_PEOPLE_H

/* A simple low-poly human, facing +x, standing on the ground (y=0).
   phase   : animation phase (radians) for the walk cycle
   walk    : 0 = standing still, 1 = full stride
   variant : selects clothing colour / role (staff use hi-vis)
   bag     : 0 none, 1 rolling suitcase, 2 trolley, 3 shoulder bag */
void People_Draw(float x,float z,float yaw,float phase,float walk,int variant,int bag);

/* Set a one-shot arm gesture applied to the NEXT People_Draw call, then auto-cleared:
   0 = none, 1 = talking (one hand gestures), 2 = pointing up at the sky. */
void People_SetGesture(int g);

#endif
