#ifndef AIRPORT_GFX_H
#define AIRPORT_GFX_H
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

enum {
    TEX_ASPHALT = 0,  /* runway / taxiway / road */
    TEX_CONCRETE,     /* apron */
    TEX_TILE,         /* terminal floor */
    TEX_GRASS,        /* landscaping */
    TEX_METAL,        /* aircraft / structures */
    TEX_CARPET,       /* lounge */
    TEX_COUNT
};

void  Gfx_Init(void);
GLUquadric *Gfx_Quadric(void);

void  Gfx_Material(float r, float g, float b, float shininess);      /* plastic-ish */
void  Gfx_Metal(float r, float g, float b);                          /* shiny metal */
void  Gfx_Emissive(float r, float g, float b);                       /* self-lit (lamps) */
void  Gfx_EmissiveOff(void);

/* Primitives. */
void  Gfx_Box(float x0,float y0,float z0,float x1,float y1,float z1);
void  Gfx_Cylinder(float x,float y,float z,float radius,float height,float r,float g,float b);
void  Gfx_TexturedFloor(float x0,float z0,float x1,float z1,float y,int tex,float repeat);
void  Gfx_Glass(float x0,float y0,float z0,float x1,float y1,float z1,
                float r,float g,float b,float a);
void  Gfx_TexBox(int tex,float x0,float y0,float z0,float x1,float y1,float z1,float rep);

void  Gfx_Panel(const float top[4][3], float thickness);

/* Text.
   Gfx_TextFlat: stroke text on a flat plane. 'faceDeg' = the direction the text
   FRONT faces (0=+z, 90=+x, 180=-z, 270=-x). Text is auto-centred and never mirrored
   as long as it is read from its front. 'height' is world height of the caps.
   Gfx_TextBill: billboard text that always faces the camera (pass camera x,z). */
void  Gfx_TextFlat(float x,float y,float z,float faceDeg,float height,
                   float r,float g,float b,const char *s);
void  Gfx_TextLeft(float x,float y,float z,float faceDeg,float height,
                   float r,float g,float b,const char *s);
void  Gfx_TextBill(float x,float y,float z,float camx,float camz,float height,
                   float r,float g,float b,const char *s);
float Gfx_TextWidth(float height,const char *s);

#endif
