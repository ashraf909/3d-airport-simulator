#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "camera.h"
#include "layout.h"

Camera g_camera;

void Camera_Reset(void)
{
    g_camera.x=START_X; g_camera.y=EYE_HEIGHT; g_camera.z=START_Z;
    g_camera.yaw=START_YAW; g_camera.pitch=0; g_camera.speed=13.0f;
}
void Camera_Init(void){ Camera_Reset(); }

void Camera_Rotate(float dyaw,float dpitch)
{
    g_camera.yaw += dyaw;
    g_camera.pitch += dpitch;
    if(g_camera.pitch> 1.45f) g_camera.pitch= 1.45f;
    if(g_camera.pitch<-1.45f) g_camera.pitch=-1.45f;
}

int Camera_Blocked(float x,float z)
{
    float R=PLAYER_RADIUS;

    /* Terminal outer shell: block the walls but leave the entrance gap and
       let the player walk fully inside. */
    if(z>TERMINAL_Z0-R && z<TERMINAL_Z1+R){
        /* west / east walls */
        if(x<TERMINAL_X0+R || x>TERMINAL_X1-R){
            if(z>TERMINAL_Z0 && z<TERMINAL_Z1) return 1;
        }
        /* landside wall except the central doorway */
        if(z>TERMINAL_Z1-R && z<TERMINAL_Z1+R && fabsf(x)>ENTRANCE_HALF) return 1;
        if(z>TERMINAL_Z0-R && z<TERMINAL_Z0+R) return 1;
    }

    /* Check-in counters: the player waits BEHIND the line (blocked up to z=16.4) while the
       NPC at the desk is served. Gaps between counters stay open to walk through. */
    if(z>11.8f && z<16.4f){
        int c; float cxs[6]={-50,-30,-10,10,30,50};
        for(c=0;c<6;c++) if(fabsf(x-cxs[c])<3.4f) return 1;
    }
    /* baggage X-ray belt: wait in front of it */
    if(z>11.8f && z<16.4f && x>19.5f && x<32.5f) return 1;
    /* baggage carousel */
    {
        float dx=x-(-40.0f), dz=z-(-20.0f);
        if(dx*dx+dz*dz < (6.5f+R)*(6.5f+R) && dx*dx+dz*dz > (2.0f)*(2.0f)) return 1;
    }
    /* control tower base */
    {
        float dx=x-TOWER_X, dz=z-TOWER_Z;
        if(dx*dx+dz*dz < (6.0f+R)*(6.0f+R)) return 1;
    }
    return 0;
}

void Camera_Move(float forward,float right)
{
    float fx= sinf(g_camera.yaw), fz=-cosf(g_camera.yaw);
    float rx= cosf(g_camera.yaw), rz= sinf(g_camera.yaw);
    float nx=g_camera.x + fx*forward + rx*right;
    float nz=g_camera.z + fz*forward + rz*right;
    if(!Camera_Blocked(nx,g_camera.z)) g_camera.x=nx;
    if(!Camera_Blocked(g_camera.x,nz)) g_camera.z=nz;
    /* keep inside the world */
    if(g_camera.x<-WORLD_HALF+2) g_camera.x=-WORLD_HALF+2;
    if(g_camera.x> WORLD_HALF-2) g_camera.x= WORLD_HALF-2;
    if(g_camera.z<-WORLD_HALF+2) g_camera.z=-WORLD_HALF+2;
    if(g_camera.z> WORLD_HALF-2) g_camera.z= WORLD_HALF-2;
}

void Camera_Apply(void)
{
    float cp=cosf(g_camera.pitch);
    float fx=sinf(g_camera.yaw)*cp, fy=sinf(g_camera.pitch), fz=-cosf(g_camera.yaw)*cp;
    gluLookAt(g_camera.x,g_camera.y,g_camera.z,
              g_camera.x+fx,g_camera.y+fy,g_camera.z+fz,
              0,1,0);
}
