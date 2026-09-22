#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef _WIN32
#include <windows.h>       /* PlaySound for the step-complete chime */
#endif
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "scene.h"
#include "camera.h"
#include "gfx.h"
#include "layout.h"
#include "hud.h"
#include "story.h"

static int   keyW,keyS,keyA,keyD;
static int   showHelp=1;
static int   lastMs;
static int   warpReady=0;
static float g_fps=60.0f;
static const float MOUSE_SENS=0.0026f;

/* lighting */
static void setupLighting(void)
{
    int day=g_airport.dayMode;
    GLfloat sunDir[4] = { -0.45f, 0.85f, -0.30f, 0.0f };   /* directional */
    GLfloat moonDir[4]= {  0.40f, 0.80f, -0.35f, 0.0f };
    GLfloat sunDif[4] = { 1.15f, 1.08f, 0.94f, 1 };
    GLfloat sunAmb[4] = { 0.45f, 0.47f, 0.52f, 1 };
    GLfloat moonDif[4]= { 0.40f, 0.45f, 0.62f, 1 };
    GLfloat moonAmb[4]= { 0.18f, 0.20f, 0.30f, 1 };

    GLfloat inPos[4]  = { 0, 14, -4, 1 };
    GLfloat inCol[4]  = { 1.0f, 0.94f, 0.80f, 1 };
    GLfloat apPos[4]  = { 0, 17, -60, 1 };
    GLfloat apCol[4]  = { 0.82f, 0.88f, 1.0f, 1 };

    glLightfv(GL_LIGHT0,GL_POSITION, day?sunDir:moonDir);
    glLightfv(GL_LIGHT0,GL_DIFFUSE , day?sunDif:moonDif);
    glLightfv(GL_LIGHT0,GL_AMBIENT , day?sunAmb:moonAmb);

    glLightfv(GL_LIGHT1,GL_POSITION,inPos);
    glLightfv(GL_LIGHT1,GL_DIFFUSE ,inCol);
    glLightf (GL_LIGHT1,GL_CONSTANT_ATTENUATION, day?1.4f:0.7f);
    glLightf (GL_LIGHT1,GL_LINEAR_ATTENUATION , 0.012f);

    glLightfv(GL_LIGHT2,GL_POSITION,apPos);
    glLightfv(GL_LIGHT2,GL_DIFFUSE ,apCol);
    glLightf (GL_LIGHT2,GL_CONSTANT_ATTENUATION, day?1.6f:0.6f);
    glLightf (GL_LIGHT2,GL_LINEAR_ATTENUATION , 0.010f);

    glEnable(GL_LIGHT0); glEnable(GL_LIGHT1); glEnable(GL_LIGHT2);
}

static void initGL(void)
{
    GLfloat globalAmb[4]={0.34f,0.35f,0.40f,1};
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE,1);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,1);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT,globalAmb);
    glEnable(GL_FOG);
    glHint(GL_FOG_HINT,GL_NICEST);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT,GL_NICEST);

    Gfx_Init(); Camera_Init(); Airport_Init(); Story_Init();
    g_airport.holdTakeoff=1;
    lastMs=glutGet(GLUT_ELAPSED_TIME);
    glutSetCursor(GLUT_CURSOR_NONE);
}

/* cameras */
static void applyView(void)
{
    if(Story_InCabin()){   /* seated inside the plane, looking down the aisle over the seats */
        gluLookAt(377.5f,3.3f,380.0f, 398.0f,2.1f,380.0f, 0,1,0);
        return;
    }
    if(Story_InGround()){  /* standing in the village, looking up at the plane */
        gluLookAt(VILLAGE_X, 2.0f, VILLAGE_Z+20.0f, VILLAGE_X, 36.0f, VILLAGE_Z-40.0f, 0,1,0);
        return;
    }
    switch(g_airport.cameraMode){
    case 1: /* runway: diagonal view down the strip, clear of the flood poles */
        gluLookAt(150,11,RUNWAY_Z+46, -60,4,RUNWAY_Z, 0,1,0);
        break;
    case 2: { /* aircraft follow (the departing jet), pulled back for a full view */
        float yaw=DEG2RAD(g_airport.toYaw);
        float dx=cosf(yaw), dz=-sinf(yaw);
        float ex=g_airport.toX-dx*46+4, ey=g_airport.toY+17, ez=g_airport.toZ-dz*46+16;
        gluLookAt(ex,ey,ez, g_airport.toX,g_airport.toY+3,g_airport.toZ, 0,1,0);
        break; }
    case 3: /* inside the control cab, over the controllers' shoulders at their screens + the window */
        gluLookAt(TOWER_X+3.0f, 29.6f, TOWER_Z+4.5f, TOWER_X-1.0f, 27.2f, TOWER_Z-9.0f, 0,1,0);
        break;
    default:
        Camera_Apply();
    }
}

static void display(void)
{
    int w=glutGet(GLUT_WINDOW_WIDTH), h=glutGet(GLUT_WINDOW_HEIGHT);
    float aspect = h? (float)w/h : 1.0f;
    GLfloat fogDay[4]={0.62f,0.74f,0.86f,1}, fogNight[4]={0.03f,0.05f,0.11f,1};

    if(g_airport.dayMode) glClearColor(0.55f,0.72f,0.90f,1);
    else                  glClearColor(0.02f,0.035f,0.09f,1);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glFogi(GL_FOG_MODE,GL_LINEAR);
    glFogfv(GL_FOG_COLOR,g_airport.dayMode?fogDay:fogNight);
    glFogf(GL_FOG_START, g_airport.dayMode?160.0f:90.0f);
    glFogf(GL_FOG_END,   g_airport.dayMode?520.0f:300.0f);

    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(62.0f,aspect,0.15f,900.0f);
    glMatrixMode(GL_MODELVIEW);  glLoadIdentity();

    applyView();
    setupLighting();
    Airport_Draw();
    Hud_Draw(showHelp,g_fps);
    glutSwapBuffers();
}

/* timer */
static void tick(int v)
{
    int now=glutGet(GLUT_ELAPSED_TIME);
    float dt=(now-lastMs)/1000.0f; lastMs=now;
    if(dt>0.05f) dt=0.05f;
    if(dt>0.0001f) g_fps = g_fps*0.9f + (1.0f/dt)*0.1f;

    if(g_airport.cameraMode==0){
        float f=0,r=0;
        if(keyW) f+=g_camera.speed*dt;
        if(keyS) f-=g_camera.speed*dt;
        if(keyD) r+=g_camera.speed*dt;
        if(keyA) r-=g_camera.speed*dt;
        if(f||r) Camera_Move(f,r);
    }
    Airport_Update(dt);
    Story_Update(dt);
#ifdef _WIN32
    if(g_story.doneSound){ PlaySound("SystemAsterisk",NULL,SND_ALIAS|SND_ASYNC); g_story.doneSound=0; }
#else
    g_story.doneSound=0;
#endif
    glutPostRedisplay();
    glutTimerFunc(16,tick,0);
    (void)v;
}

/* input */
static void keyDown(unsigned char k,int x,int y)
{
    (void)x;(void)y;
    switch(k){
    case 'w':case 'W': keyW=1; break;
    case 's':case 'S': keyS=1; break;
    case 'a':case 'A': keyA=1; break;
    case 'd':case 'D': keyD=1; break;
    case '1': g_airport.holdTakeoff=0; Airport_RestartTakeoff(); g_airport.cameraMode=2; break;
    case '2': Airport_RestartLanding(); g_airport.cameraMode=1; break;
    case 'l':case 'L': Airport_ToggleDay(); break;
    case 'm':case 'M': showHelp=!showHelp; break;
    case 'r':case 'R': Story_Restart(); warpReady=0; break;
    case 'e':case 'E': Story_Interact(); break;  /* interact at a station */
    case 13:           Story_Interact(); break;  /* Enter also interacts / begins */
    case 'y':case 'Y': Story_Yes(); break;
    case 'n':case 'N': Story_No(); break;
    case 27: exit(0);
    }
}
static void keyUp(unsigned char k,int x,int y)
{
    (void)x;(void)y;
    switch(k){
    case 'w':case 'W': keyW=0; break;
    case 's':case 'S': keyS=0; break;
    case 'a':case 'A': keyA=0; break;
    case 'd':case 'D': keyD=0; break;
    }
}
static void special(int k,int x,int y)
{
    (void)x;(void)y;
    switch(k){
    case GLUT_KEY_F1: g_airport.cameraMode=0; warpReady=0; break;
    case GLUT_KEY_F2: g_airport.cameraMode=1; break;
    case GLUT_KEY_F3: g_airport.cameraMode=2; break;
    case GLUT_KEY_F4: g_airport.cameraMode=3; break;
    case GLUT_KEY_LEFT:  Camera_Rotate(-0.06f,0); break;   /* left  = look left  */
    case GLUT_KEY_RIGHT: Camera_Rotate( 0.06f,0); break;   /* right = look right */
    case GLUT_KEY_UP:    Camera_Rotate(0, 0.05f); break;   /* up    = look up    */
    case GLUT_KEY_DOWN:  Camera_Rotate(0,-0.05f); break;
    }
}
static void mouseLook(int x,int y)
{
    int cx=glutGet(GLUT_WINDOW_WIDTH)/2, cy=glutGet(GLUT_WINDOW_HEIGHT)/2;
    int dx,dy;
    if(!warpReady){ glutWarpPointer(cx,cy); warpReady=1; return; }
    dx=x-cx; dy=y-cy;
    if(dx==0 && dy==0) return;
    if(g_airport.cameraMode==0)
        Camera_Rotate(dx*MOUSE_SENS, -dy*MOUSE_SENS);
    glutWarpPointer(cx,cy);
}
static void reshape(int w,int h){ if(h<1)h=1; glViewport(0,0,w,h); }

int main(int argc,char **argv)
{
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
    glutInitWindowSize(1280,800);
    glutCreateWindow("3D Airport Simulator - Hazrat Shahjalal International Airport, Dhaka");
    initGL();
    glutIgnoreKeyRepeat(1);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyDown);
    glutKeyboardUpFunc(keyUp);
    glutSpecialFunc(special);
    glutPassiveMotionFunc(mouseLook);
    glutMotionFunc(mouseLook);
    glutTimerFunc(16,tick,0);
    glutMainLoop();
    return 0;
}
