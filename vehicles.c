#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "vehicles.h"
#include "gfx.h"

static void vwheel(float x,float z,float rad,float spin)
{
    /* Tyre axis runs sideways (along z, the vehicle width); the wheel rolls about it,
       so the round face is seen from the side and the tread from front/back. */
    GLUquadric *q=Gfx_Quadric();
    glPushMatrix();
    glTranslatef(x,rad,z);
    glRotatef(-spin,0,0,1);            /* roll about the sideways axle */
    glTranslatef(0,0,-0.17f);           /* half tyre width */
    Gfx_Material(0.05f,0.05f,0.06f,8.0f);
    gluCylinder(q,rad,rad,0.34f,16,1);  /* tyre tread */
    gluDisk(q,0,rad,16,1);              /* sidewall (inner) */
    Gfx_Metal(0.72f,0.74f,0.78f); gluDisk(q,0,rad*0.5f,10,1);        /* hubcap */
    glTranslatef(0,0,0.34f);
    Gfx_Material(0.05f,0.05f,0.06f,8.0f); gluDisk(q,0,rad,16,1);     /* sidewall (outer) */
    Gfx_Metal(0.72f,0.74f,0.78f); gluDisk(q,0,rad*0.5f,10,1);        /* hubcap */
    glPopMatrix();
}

static void headTail(float fx,float bx,float y,float hw,int night)
{
    /* head lights (front) */
    if(night) Gfx_Emissive(1.0f,0.98f,0.8f); else Gfx_Material(0.9f,0.9f,0.85f,80.0f);
    glPushMatrix(); glTranslatef(fx, y, hw); glutSolidSphere(0.14f,8,6); glPopMatrix();
    glPushMatrix(); glTranslatef(fx, y,-hw); glutSolidSphere(0.14f,8,6); glPopMatrix();
    Gfx_EmissiveOff();
    /* tail lights (rear) */
    if(night) Gfx_Emissive(0.9f,0.05f,0.05f); else Gfx_Material(0.6f,0.05f,0.05f,60.0f);
    glPushMatrix(); glTranslatef(bx, y, hw*0.9f); Gfx_Box(-0.05f,-0.1f,-0.12f,0.05f,0.12f,0.12f); glPopMatrix();
    glPushMatrix(); glTranslatef(bx, y,-hw*0.9f); Gfx_Box(-0.05f,-0.1f,-0.12f,0.05f,0.12f,0.12f); glPopMatrix();
    Gfx_EmissiveOff();
}

void Veh_Car(float x,float z,float yaw,float r,float g,float b,float spin,int night)
{
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);

    /* main body */
    Gfx_Metal(r,g,b);
    Gfx_Box(-2.1f,0.42f,-0.82f, 2.1f,1.02f,0.82f);
    /* sloped hood + trunk lids */
    Gfx_Box( 1.3f,0.72f,-0.80f, 2.15f,0.98f,0.80f);   /* hood */
    Gfx_Box(-2.15f,0.72f,-0.80f,-1.3f,1.0f,0.80f);    /* trunk */
    /* cabin / greenhouse */
    Gfx_Metal(r*0.9f,g*0.9f,b*0.9f);
    Gfx_Box(-1.15f,1.02f,-0.74f, 1.15f,1.62f,0.74f);

    /* dark glass: windshield, rear, sides */
    Gfx_Material(0.06f,0.09f,0.12f,110.0f);
    /* windshield (front slope) */
    glBegin(GL_QUADS);
      glNormal3f(0.6f,0.4f,0); glVertex3f(1.15f,1.02f,-0.72f); glVertex3f(1.15f,1.02f,0.72f);
      glVertex3f(1.15f,1.60f,0.70f); glVertex3f(1.15f,1.60f,-0.70f);
    glEnd();
    /* rear window */
    glBegin(GL_QUADS);
      glNormal3f(-0.6f,0.4f,0); glVertex3f(-1.15f,1.60f,-0.70f); glVertex3f(-1.15f,1.60f,0.70f);
      glVertex3f(-1.15f,1.02f,0.72f); glVertex3f(-1.15f,1.02f,-0.72f);
    glEnd();
    /* side windows */
    Gfx_Box(-1.1f,1.10f,0.74f, 1.1f,1.55f,0.76f);
    Gfx_Box(-1.1f,1.10f,-0.76f,1.1f,1.55f,-0.74f);

    /* grille + bumpers */
    Gfx_Material(0.12f,0.12f,0.13f,30.0f);
    Gfx_Box(2.05f,0.5f,-0.6f,2.2f,0.8f,0.6f);
    Gfx_Box(-2.2f,0.45f,-0.7f,-2.05f,0.72f,0.7f);
    Gfx_Box(2.0f,0.35f,-0.78f,2.22f,0.5f,0.78f);

    headTail(2.14f,-2.14f,0.66f,0.6f,night);

    vwheel( 1.35f,-0.86f,0.40f,spin); vwheel( 1.35f,0.86f,0.40f,spin);
    vwheel(-1.35f,-0.86f,0.40f,spin); vwheel(-1.35f,0.86f,0.40f,spin);
    glPopMatrix();
}

void Veh_Taxi(float x,float z,float yaw,float spin,int night)
{
    Veh_Car(x,z,yaw,0.95f,0.78f,0.05f,spin,night);
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    /* roof TAXI sign */
    if(night) Gfx_Emissive(1.0f,0.85f,0.15f); else Gfx_Material(0.95f,0.8f,0.1f,50.0f);
    Gfx_Box(-0.35f,1.62f,-0.28f,0.35f,1.86f,0.28f);
    Gfx_EmissiveOff();
    Gfx_TextFlat(0,1.7f,0.30f,0,0.16f,0.05f,0.05f,0.05f,"TAXI");
    /* checker stripe */
    Gfx_Material(0.1f,0.1f,0.1f,20.0f);
    Gfx_Box(-1.2f,0.66f,0.83f,1.2f,0.82f,0.84f);
    Gfx_Box(-1.2f,0.66f,-0.84f,1.2f,0.82f,-0.83f);
    glPopMatrix();
}

void Veh_Bus(float x,float z,float yaw,float spin,int night)
{
    int i;
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    /* long body */
    Gfx_Metal(0.20f,0.42f,0.78f);
    Gfx_Box(-5.6f,0.5f,-1.25f,5.6f,3.0f,1.25f);
    /* white band */
    Gfx_Metal(0.92f,0.93f,0.95f);
    Gfx_Box(-5.62f,1.7f,-1.27f,5.62f,2.1f,1.27f);
    /* windscreen */
    Gfx_Material(0.06f,0.10f,0.14f,110.0f);
    Gfx_Box(5.5f,1.9f,-1.15f,5.66f,2.85f,1.15f);
    /* passenger windows both sides */
    for(i=0;i<9;i++){ float wx=-4.7f+i*1.05f;
        Gfx_Box(wx,2.15f, 1.26f,wx+0.75f,2.8f, 1.28f);
        Gfx_Box(wx,2.15f,-1.28f,wx+0.75f,2.8f,-1.26f); }
    /* doors */
    Gfx_Material(0.10f,0.12f,0.16f,60.0f);
    Gfx_Box(3.2f,0.55f,1.24f,4.2f,1.9f,1.30f);
    Gfx_Box(-1.2f,0.55f,1.24f,-0.2f,1.9f,1.30f);
    headTail(5.62f,-5.62f,0.95f,1.0f,night);
    /* six wheels */
    vwheel( 3.8f,-1.28f,0.55f,spin); vwheel( 3.8f,1.28f,0.55f,spin);
    vwheel(-3.2f,-1.28f,0.55f,spin); vwheel(-3.2f,1.28f,0.55f,spin);
    vwheel(-4.4f,-1.28f,0.55f,spin); vwheel(-4.4f,1.28f,0.55f,spin);
    glPopMatrix();
}

void Veh_FuelTruck(float x,float z,float yaw,float spin,int night)
{
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    /* cab */
    Gfx_Metal(0.85f,0.8f,0.15f);
    Gfx_Box(2.0f,0.5f,-1.0f,3.6f,2.3f,1.0f);
    Gfx_Material(0.06f,0.10f,0.14f,110.0f);
    Gfx_Box(3.55f,1.4f,-0.9f,3.7f,2.2f,0.9f);
    /* tank */
    Gfx_Metal(0.80f,0.82f,0.86f);
    glPushMatrix(); glTranslatef(-3.5f,1.5f,0); glRotatef(90,0,1,0);
    gluCylinder(Gfx_Quadric(),1.1f,1.1f,5.2f,20,1);
    gluDisk(Gfx_Quadric(),0,1.1f,20,1);
    glTranslatef(0,0,5.2f); gluDisk(Gfx_Quadric(),0,1.1f,20,1);
    glPopMatrix();
    Gfx_TextFlat(-1.0f,1.5f,1.12f,0,0.5f,0.85f,0.1f,0.1f,"JET A-1");
    headTail(3.62f,-4.7f,0.9f,0.9f,night);
    vwheel(2.8f,-1.05f,0.5f,spin); vwheel(2.8f,1.05f,0.5f,spin);
    vwheel(-2.0f,-1.05f,0.5f,spin); vwheel(-2.0f,1.05f,0.5f,spin);
    vwheel(-4.2f,-1.05f,0.5f,spin); vwheel(-4.2f,1.05f,0.5f,spin);
    glPopMatrix();
}

void Veh_BaggageTug(float x,float z,float yaw,float spin,int night)
{
    int c;
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    /* little tractor */
    Gfx_Metal(0.85f,0.55f,0.10f);
    Gfx_Box(0.2f,0.4f,-0.6f,1.6f,1.05f,0.6f);
    Gfx_Metal(0.1f,0.1f,0.12f);
    Gfx_Box(0.6f,1.05f,-0.5f,0.7f,1.7f,-0.4f);
    Gfx_Box(0.6f,1.05f,0.4f,0.7f,1.7f,0.5f);
    Gfx_Box(0.55f,1.65f,-0.55f,1.2f,1.78f,0.55f);  /* roof */
    headTail(1.62f,0.15f,0.7f,0.5f,night);
    vwheel(1.15f,-0.62f,0.34f,spin); vwheel(1.15f,0.62f,0.34f,spin);
    vwheel(0.4f,-0.62f,0.34f,spin);  vwheel(0.4f,0.62f,0.34f,spin);
    /* towed baggage carts with luggage */
    for(c=0;c<3;c++){ float cx=-1.4f-c*2.2f;
        Gfx_Metal(0.35f,0.37f,0.4f);
        Gfx_Box(cx-0.9f,0.35f,-0.8f,cx+0.9f,0.55f,0.8f);
        Gfx_Box(cx-0.9f,0.55f,-0.85f,cx-0.8f,1.4f,0.85f);
        Gfx_Box(cx+0.8f,0.55f,-0.85f,cx+0.9f,1.4f,0.85f);
        { int b; for(b=0;b<3;b++){ float bx=cx-0.6f+b*0.5f;
            Gfx_Material(0.2f+0.25f*b,0.15f,0.12f+0.2f*(b%2),20.0f);
            Gfx_Box(bx,0.55f,-0.6f,bx+0.4f,0.95f,0.6f); } }
        vwheel(cx-0.6f,-0.82f,0.24f,spin); vwheel(cx-0.6f,0.82f,0.24f,spin);
        vwheel(cx+0.6f,-0.82f,0.24f,spin); vwheel(cx+0.6f,0.82f,0.24f,spin);
    }
    glPopMatrix();
}

void Veh_CNG(float x,float z,float yaw,float spin,int night)
{
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    /* lower body (green) */
    Gfx_Material(0.10f,0.45f,0.20f,25.0f);
    Gfx_Box(-1.15f,0.45f,-0.62f,0.75f,1.35f,0.62f);
    /* rounded front nose */
    Gfx_Material(0.10f,0.45f,0.20f,25.0f);
    Gfx_Box(0.75f,0.5f,-0.5f,1.25f,1.15f,0.5f);
    /* yellow roof */
    Gfx_Material(0.95f,0.82f,0.10f,20.0f);
    Gfx_Box(-1.2f,1.72f,-0.66f,0.85f,1.9f,0.66f);
    /* roof posts */
    Gfx_Metal(0.75f,0.76f,0.8f);
    Gfx_Box(-1.15f,1.35f,-0.62f,-1.05f,1.72f,-0.52f);
    Gfx_Box(-1.15f,1.35f,0.52f,-1.05f,1.72f,0.62f);
    Gfx_Box(0.6f,1.35f,-0.62f,0.7f,1.72f,-0.52f);
    Gfx_Box(0.6f,1.35f,0.52f,0.7f,1.72f,0.62f);
    /* cage bars on the sides (Bangladeshi CNG look) */
    Gfx_Metal(0.7f,0.72f,0.76f);
    { int i; for(i=0;i<4;i++){ float bx=-1.0f+i*0.5f;
        Gfx_Box(bx,0.95f,-0.64f,bx+0.05f,1.35f,-0.6f);
        Gfx_Box(bx,0.95f,0.6f,bx+0.05f,1.35f,0.64f); } }
    /* windscreen */
    Gfx_Material(0.10f,0.14f,0.18f,110.0f);
    Gfx_Box(1.2f,0.9f,-0.45f,1.26f,1.5f,0.45f);
    /* driver */
    Gfx_Material(0.2f,0.2f,0.25f,10.0f);
    glPushMatrix(); glTranslatef(0.3f,0,0); Gfx_Box(-0.15f,0.7f,-0.2f,0.15f,1.35f,0.2f);
    Gfx_Material(0.8f,0.62f,0.46f,10.0f); glTranslatef(0,1.5f,0); glutSolidSphere(0.14f,8,6); glPopMatrix();
    headTail(1.24f,-1.14f,0.7f,0.4f,night);
    /* one front wheel, two rear wheels */
    vwheel(1.0f,0.0f,0.34f,spin);
    vwheel(-0.85f,-0.6f,0.36f,spin); vwheel(-0.85f,0.6f,0.36f,spin);
    glPopMatrix();
}

void Veh_Stairs(float x,float z,float yaw,int night)
{
    /* mobile passenger boarding stairs */
    int i; (void)night;
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    Gfx_Metal(0.85f,0.86f,0.9f);
    /* chassis */
    Gfx_Box(-1.2f,0.4f,-1.0f,1.2f,0.8f,1.0f);
    for(i=0;i<12;i++){ float sx=-1.0f+i*0.28f, sy=0.8f+i*0.32f;
        Gfx_Metal(0.8f,0.81f,0.84f);
        Gfx_Box(sx,sy,-0.9f,sx+0.3f,sy+0.12f,0.9f); }
    /* handrails */
    Gfx_Metal(0.6f,0.62f,0.66f);
    Gfx_Box(-1.0f,1.9f,-0.92f,2.4f,2.0f,-0.86f);
    Gfx_Box(-1.0f,1.9f,0.86f,2.4f,2.0f,0.92f);
    /* top platform */
    Gfx_Box(2.3f,4.3f,-0.95f,3.1f,4.42f,0.95f);
    vwheel(0.9f,-1.02f,0.42f,0); vwheel(0.9f,1.02f,0.42f,0);
    vwheel(-0.9f,-1.02f,0.42f,0); vwheel(-0.9f,1.02f,0.42f,0);
    glPopMatrix();
}

void Veh_Pushback(float x,float z,float yaw,float spin,int night)
{
    glPushMatrix(); glTranslatef(x,0,z); glRotatef(yaw,0,1,0);
    Gfx_Metal(0.9f,0.9f,0.92f);
    Gfx_Box(-1.8f,0.35f,-1.1f,1.8f,0.85f,1.1f);   /* low flat body */
    Gfx_Metal(0.2f,0.5f,0.85f);
    Gfx_Box(-1.6f,0.85f,-0.9f,-0.4f,1.9f,0.9f);   /* cab */
    Gfx_Material(0.06f,0.10f,0.14f,110.0f);
    Gfx_Box(-0.45f,1.1f,-0.8f,-0.35f,1.8f,0.8f);
    if(night) Gfx_Emissive(1.0f,0.6f,0.0f); else Gfx_Material(0.9f,0.55f,0.05f,40.0f);
    Gfx_Box(-1.0f,1.9f,-0.3f,-0.6f,2.05f,0.3f);   /* beacon */
    Gfx_EmissiveOff();
    vwheel(1.2f,-1.1f,0.42f,spin); vwheel(1.2f,1.1f,0.42f,spin);
    vwheel(-1.2f,-1.1f,0.42f,spin); vwheel(-1.2f,1.1f,0.42f,spin);
    glPopMatrix();
}
