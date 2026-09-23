#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "aircraft.h"
#include "gfx.h"


static void tubeX(float xa,float xb,float ra,float rb,int slices)
{
    GLUquadric *q=Gfx_Quadric();
    glPushMatrix(); glTranslatef(xa,0,0); glRotatef(90,0,1,0);
    gluCylinder(q,ra,rb,xb-xa,slices,1);
    glPopMatrix();
}

static void diskAt(float x,float rin,float rout)
{
    GLUquadric *q=Gfx_Quadric();
    glPushMatrix(); glTranslatef(x,0,0); glRotatef(90,0,1,0);
    gluDisk(q,rin,rout,24,1);
    glPopMatrix();
}

static void wheel(float x,float y,float z,float rad,float spin)
{
    /* axle runs sideways (z); wheel rolls about it */
    GLUquadric *q=Gfx_Quadric();
    glPushMatrix(); glTranslatef(x,y,z);
    glRotatef(-spin,0,0,1);
    glTranslatef(0,0,-0.16f);
    Gfx_Material(0.05f,0.05f,0.06f,10.0f);
    gluCylinder(q,rad,rad,0.32f,16,1);
    gluDisk(q,0,rad,16,1);
    Gfx_Metal(0.6f,0.62f,0.66f); gluDisk(q,0,rad*0.45f,12,1);
    Gfx_Material(0.05f,0.05f,0.06f,10.0f);
    glTranslatef(0,0,0.32f); gluDisk(q,0,rad,16,1);
    Gfx_Metal(0.6f,0.62f,0.66f); gluDisk(q,0,rad*0.45f,12,1);
    glPopMatrix();
}

static void gearLeg(float x,float ytop,float ybot,float z,float rad,float wr,float spin,int twin)
{
    Gfx_Metal(0.30f,0.32f,0.36f);
    glPushMatrix(); glTranslatef(x,ybot,z); glRotatef(-90,1,0,0);
    gluCylinder(Gfx_Quadric(),rad,rad,ytop-ybot,10,1);
    glPopMatrix();
    if(twin){ wheel(x,ybot,z-0.42f,wr,spin); wheel(x,ybot,z+0.42f,wr,spin); }
    else      wheel(x,ybot,z,wr,spin);
}

static void engine(float z,float spin,int running)
{
    /* nacelle */
    Gfx_Metal(0.82f,0.84f,0.88f);
    glPushMatrix(); glTranslatef(0,-2.35f,z);
    tubeX(-1.2f,4.4f,1.05f,1.0f,22);
    /* intake lip ring */
    Gfx_Metal(0.35f,0.37f,0.42f);
    tubeX(4.3f,4.7f,1.08f,1.0f,22);
    /* dark intake interior */
    Gfx_Material(0.03f,0.03f,0.04f,4.0f);
    diskAt(4.35f,0.0f,1.0f);
    /* spinning fan hub + blades */
    glPushMatrix(); glTranslatef(4.2f,0,0); glRotatef(running?spin:0.0f,1,0,0);
    Gfx_Metal(0.55f,0.57f,0.6f);
    { GLUquadric*q=Gfx_Quadric(); glPushMatrix(); glRotatef(90,0,1,0); gluCylinder(q,0.28f,0.12f,0.5f,10,1); glPopMatrix(); }
    { int i; Gfx_Metal(0.72f,0.74f,0.78f);
      for(i=0;i<14;i++){ glPushMatrix(); glRotatef(i*(360.0f/14.0f),1,0,0);
        Gfx_Box(-0.02f,0.28f,-0.10f,0.02f,0.95f,0.10f); glPopMatrix(); } }
    /* white spinner spiral dot */
    Gfx_Material(0.95f,0.95f,0.97f,30.0f); glPushMatrix(); glTranslatef(0.26f,0,0); glutSolidSphere(0.14f,8,6); glPopMatrix();
    glPopMatrix();
    /* exhaust cone */
    Gfx_Material(0.20f,0.20f,0.22f,20.0f);
    tubeX(-1.6f,-1.1f,0.5f,0.75f,16);
    glPopMatrix();
}

static void wingPanel(float sgn,float r,float g,float b)
{
    /* sgn = +1 right wing (+z), -1 left wing (-z) */
    float top[4][3] = {
        {  3.0f, -0.9f, sgn*1.7f },   /* root leading */
        { -5.5f, -0.4f, sgn*19.5f},   /* tip  leading */
        { -8.8f, -0.4f, sgn*19.5f},   /* tip  trailing */
        { -4.6f, -0.9f, sgn*1.7f }    /* root trailing */
    };
    Gfx_Metal(r,g,b);
    Gfx_Panel(top,0.42f);
    /* winglet */
    { float wl[4][3]={
        {-5.5f,-0.4f,sgn*19.5f},{-6.4f,1.6f,sgn*19.9f},
        {-8.2f,1.6f,sgn*19.9f},{-8.8f,-0.4f,sgn*19.5f} };
      Gfx_Panel(wl,0.25f); }
}

static void hstab(float sgn)
{
    float top[4][3] = {
        {-16.5f,0.9f, sgn*0.8f},
        {-20.2f,1.2f, sgn*6.6f},
        {-21.6f,1.2f, sgn*6.6f},
        {-19.8f,0.9f, sgn*0.8f}
    };
    Gfx_Metal(0.90f,0.91f,0.93f);
    Gfx_Panel(top,0.28f);
}

static void vfin(float r,float g,float b)
{
    /* swept fin in the x-y plane, thin in z */
    float t=0.28f;
    float o[6][2] = { {-13.5f,1.6f},{-16.0f,8.7f},{-18.4f,8.7f},{-21.4f,1.6f} };
    int i;
    Gfx_Metal(r,g,b);
    glBegin(GL_QUADS);
    glNormal3f(0,0,1);  for(i=0;i<4;i++) glVertex3f(o[i][0],o[i][1], t);
    glNormal3f(0,0,-1); for(i=3;i>=0;i--) glVertex3f(o[i][0],o[i][1],-t);
    glEnd();
    glBegin(GL_QUADS);
    for(i=0;i<4;i++){ int j=(i+1)%4;
        glVertex3f(o[i][0],o[i][1], t); glVertex3f(o[i][0],o[i][1],-t);
        glVertex3f(o[j][0],o[j][1],-t); glVertex3f(o[j][0],o[j][1], t); }
    glEnd();
}

static void navLight(float x,float y,float z,float r,float g,float b,int on)
{
    if(!on) return;
    Gfx_Emissive(r,g,b);
    glPushMatrix(); glTranslatef(x,y,z); glutSolidSphere(0.22f,8,6); glPopMatrix();
    Gfx_EmissiveOff();
}

void Aircraft_Draw(float x,float y,float z,float yaw,float pitch,float roll,
                   int livery,int gearDn,float t,int engineOn)
{
    float spin = fmodf(t*1400.0f,360.0f);
    float wspin= fmodf(t*260.0f,360.0f);
    int   blink= (fmodf(t,1.0f)<0.5f);
    float tr = livery? 0.10f:0.05f, tg=livery?0.30f:0.55f, tb=livery?0.70f:0.22f; /* tail colour */

    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(yaw,0,1,0);
    glRotatef(pitch,0,0,1);   /* nose up/down about z (wing axis) */
    glRotatef(roll,1,0,0);

    /* fuselage (white) */
    Gfx_Metal(0.93f,0.94f,0.96f);
    tubeX(-16.0f,12.5f,1.8f,1.8f,26);      /* main tube */
    tubeX(12.5f,17.6f,1.8f,0.25f,24);      /* nose cone */
    /* upswept tail cone */
    glPushMatrix(); glTranslatef(0,0.15f,0);
    tubeX(-22.0f,-16.0f,0.55f,1.8f,24);
    glPopMatrix();
    diskAt(-16.0f,0.0f,1.8f);

    /* belly / lower grey */
    Gfx_Metal(0.62f,0.64f,0.68f);
    { GLUquadric*q=Gfx_Quadric(); glPushMatrix(); glTranslatef(-16.0f,-0.35f,0); glRotatef(90,0,1,0);
      gluCylinder(q,1.5f,1.5f,28.0f,26,1); glPopMatrix(); }

    /* cheat-line stripe along the windows */
    Gfx_Material(tr,tg,tb,40.0f);
    { GLUquadric*q=Gfx_Quadric(); glPushMatrix(); glTranslatef(-15.5f,0.55f,0); glRotatef(90,0,1,0);
      gluCylinder(q,1.83f,1.83f,27.0f,26,1); glPopMatrix(); }
    Gfx_Metal(0.93f,0.94f,0.96f);
    { GLUquadric*q=Gfx_Quadric(); glPushMatrix(); glTranslatef(-15.5f,0.95f,0); glRotatef(90,0,1,0);
      gluCylinder(q,1.84f,1.84f,26.0f,26,1); glPopMatrix(); }

    /* cockpit windows */
    Gfx_Material(0.05f,0.09f,0.14f,90.0f);
    { int i; for(i=-1;i<=1;i++){ glPushMatrix();
        glTranslatef(11.2f-0.0f,0.85f,i*0.7f); glRotatef(90,0,1,0);
        glScalef(1,1,1); Gfx_Box(-0.05f,-0.28f,-0.55f,0.05f,0.30f,0.55f); glPopMatrix(); } }
    Gfx_Material(0.06f,0.10f,0.16f,80.0f);
    { int i; for(i=0;i<22;i++){ float px=8.5f-i*0.95f;
        glPushMatrix(); glTranslatef(px,0.55f, 1.79f); glutSolidSphere(0.12f,6,5); glPopMatrix();
        glPushMatrix(); glTranslatef(px,0.55f,-1.79f); glutSolidSphere(0.12f,6,5); glPopMatrix(); } }
    /* passenger doors */
    Gfx_Material(0.80f,0.81f,0.84f,40.0f);
    { int i; for(i=0;i<2;i++){ float px=9.0f-i*17.0f;
        glPushMatrix(); glTranslatef(px,0.35f,1.80f); Gfx_Box(-0.35f,-0.7f,-0.02f,0.35f,0.7f,0.02f); glPopMatrix(); } }
    Gfx_TextLeft(-6.0f,1.05f, 1.5f,  0,0.72f,0.60f,0.08f,0.12f,"BIMAN BANGLADESH AIRLINES");
    Gfx_TextLeft( 9.0f,1.05f,-1.5f,180,0.72f,0.60f,0.08f,0.12f,"BIMAN BANGLADESH AIRLINES");

    /* wings + engines */
    wingPanel( 1.0f,0.93f,0.94f,0.96f);
    wingPanel(-1.0f,0.93f,0.94f,0.96f);
    /* pylons */
    Gfx_Metal(0.80f,0.82f,0.86f);
    Gfx_Box(-0.8f,-2.0f, 7.1f, 1.6f,-0.7f, 7.9f);
    Gfx_Box(-0.8f,-2.0f,-7.9f, 1.6f,-0.7f,-7.1f);
    engine( 7.5f,spin,engineOn);
    engine(-7.5f,spin,engineOn);

    /* tail surfaces */
    hstab( 1.0f); hstab(-1.0f);
    vfin(tr,tg,tb);
    /* small logo band on the fin */
    Gfx_Emissive(0.95f,0.95f,0.97f);
    glPushMatrix(); glTranslatef(-17.3f,6.4f,0); glutSolidSphere(0.5f,10,8); glPopMatrix();
    Gfx_EmissiveOff();

    /* landing gear */
    if(gearDn){
        gearLeg( 9.0f, -1.6f, -3.05f, 0.0f, 0.13f, 0.42f, wspin, 0);   /* nose */
        gearLeg(-1.5f, -1.6f, -3.05f, 2.6f, 0.17f, 0.55f, wspin, 1);   /* main R */
        gearLeg(-1.5f, -1.6f, -3.05f,-2.6f, 0.17f, 0.55f, wspin, 1);   /* main L */
    }

    /* navigation / strobe lights */
    navLight(-8.6f,-0.3f, 19.6f, 0.1f,1.0f,0.1f, 1);      /* green stbd */
    navLight(-8.6f,-0.3f,-19.6f, 1.0f,0.1f,0.1f, 1);      /* red port  */
    navLight(-21.0f,8.6f,0.0f, 1.0f,1.0f,1.0f, blink);     /* tail strobe */
    navLight(17.4f,0.0f,0.0f, 1.0f,1.0f,0.9f, !blink);     /* nose beacon */

    glPopMatrix();
}
