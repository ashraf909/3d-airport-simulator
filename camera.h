#ifndef AIRPORT_CAMERA_H
#define AIRPORT_CAMERA_H

/* First-person camera.
   yaw:   rotation about +y. yaw=0 looks toward -z.
          increasing yaw turns the view to the RIGHT (mouse-right = look-right).
   pitch: + looks up, - looks down. */
typedef struct { float x,y,z,yaw,pitch,speed; } Camera;
extern Camera g_camera;

void Camera_Init(void);
void Camera_Reset(void);
void Camera_Rotate(float dyaw,float dpitch);   /* radians */
void Camera_Move(float forward,float right);    /* metres, with collision */
void Camera_Apply(void);                        /* issues gluLookAt */
int  Camera_Blocked(float x,float z);           /* collision query */

#endif
