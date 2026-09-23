#include <math.h>
#include <stdio.h>
#include <string.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "scene.h"
#include "layout.h"
#include "gfx.h"
#include "camera.h"
#include "aircraft.h"
#include "vehicles.h"
#include "people.h"
#include "story.h"

AirportState g_airport;

/* takeoff / landing phase ids */
enum { TO_GATE,TO_PUSH,TO_TAXI,TO_HOLD,TO_LINEUP,TO_ROLL,TO_CLIMB,TO_DEPART,TO_DONE };
enum { LN_HOLD,LN_APPR,LN_TOUCH,LN_ROLL,LN_EXIT,LN_TAXI,LN_GATE,LN_DONE };

#define GATE_TO_X   48.0f
#define GATE_LN_X  -48.0f
#define GATE_Z     -50.0f

static float clampf(float v,float a,float b){ return v<a?a:(v>b?b:v); }
static float smooth(float t){ t=clampf(t,0,1); return t*t*(3.0f-2.0f*t); }
static float mixf(float a,float b,float t){ return a+(b-a)*t; }

/* init / control */
void Airport_RestartTakeoff(void)
{
    if(g_airport.rwyOwner==1) g_airport.rwyOwner=0;
    g_airport.toPhase=TO_GATE; g_airport.toT=0;
    g_airport.toX=GATE_TO_X; g_airport.toY=AC_GROUND_CL; g_airport.toZ=GATE_Z;
    g_airport.toYaw=270; g_airport.toPitch=0; g_airport.toGear=1; g_airport.toEng=0;
    g_airport.pbActive=0;
}
void Airport_RestartLanding(void)
{
    if(g_airport.rwyOwner==2) g_airport.rwyOwner=0;
    g_airport.lnPhase=LN_HOLD; g_airport.lnT=0;
    g_airport.lnX=360; g_airport.lnY=72; g_airport.lnZ=RUNWAY_Z;
    g_airport.lnYaw=180; g_airport.lnPitch=5; g_airport.lnGear=1; g_airport.lnEng=1;
}
void Airport_Init(void)
{
    memset(&g_airport,0,sizeof(g_airport));
    g_airport.dayMode=1; g_airport.cameraMode=0;
    g_airport.simClock=8*3600.0f;               /* start at 08:00 */
    g_airport.flyActive=1; g_airport.flyT=0;
    g_airport.rwyOwner=0;
    Airport_RestartTakeoff(); Airport_RestartLanding();
}
void Airport_ToggleDay(void){ g_airport.dayMode=!g_airport.dayMode; }

/* Cutscene departure: the passenger has boarded, so send the jet straight to the
   runway threshold, claim the runway, and let the normal ROLL/CLIMB play out. */
void Airport_BeginDeparture(void)
{
    g_airport.holdTakeoff=0;
    Airport_RestartLanding();      /* send any arriving jet back to a distant hold first */
    g_airport.rwyOwner=1;          /* the departing jet now owns the runway exclusively */
    g_airport.toPhase=TO_LINEUP; g_airport.toT=0;
    g_airport.toX=-235; g_airport.toZ=-110; g_airport.toYaw=180;
    g_airport.toY=AC_GROUND_CL; g_airport.toPitch=0;
    g_airport.toEng=1; g_airport.toGear=1;
}

const char *Airport_TakeoffStatus(void)
{
    switch(g_airport.toPhase){
        case TO_GATE:return "AT GATE"; case TO_PUSH:return "PUSHBACK";
        case TO_TAXI:return "TAXIING"; case TO_LINEUP:return "LINING UP";
        case TO_ROLL:return "TAKEOFF ROLL"; case TO_CLIMB:return "CLIMBING";
        case TO_DEPART:return "DEPARTED"; default:return "READY";
    }
}
const char *Airport_LandingStatus(void)
{
    switch(g_airport.lnPhase){
        case LN_APPR:return "ON APPROACH"; case LN_TOUCH:return "TOUCHDOWN";
        case LN_ROLL:return "ROLL-OUT"; case LN_TAXI:return "TAXI TO GATE";
        case LN_GATE:return "PARKED"; default:return "ARRIVED";
    }
}
const char *Airport_AreaName(void)
{
    float z=g_camera.z,x=g_camera.x;
    if(z>PARK_Z0-4) return "Parking & Drop-off";
    if(z>TERMINAL_Z1) return "Terminal Forecourt";
    if(z>12) return "Check-in Hall";
    if(z>-4) return "Security Screening";
    if(z>TERMINAL_Z0) return (fabsf(x)>40)?"Boarding Gates":"Departure Lounge";
    return "Apron / Runway View";
}
void Airport_ClockStr(char *buf,int n)
{
    int tot=((int)(g_airport.simClock/60.0f))%(24*60);
    snprintf(buf,n,"%02d:%02d",tot/60,tot%60);
}

/* animation */
static void updateTakeoff(float dt)
{
    float *T=&g_airport.toT; *T+=dt;
    switch(g_airport.toPhase){
    case TO_GATE:
        g_airport.toEng=0; g_airport.pbActive=0;
        g_airport.toX=GATE_TO_X; g_airport.toZ=GATE_Z; g_airport.toYaw=270;
        g_airport.toY=AC_GROUND_CL; g_airport.toGear=1; g_airport.toPitch=0;
        if(g_airport.holdTakeoff){ *T=0; break; }   /* wait for the passenger to board */
        if(*T>3){ g_airport.toPhase=TO_PUSH; *T=0; g_airport.pbActive=1; }
        break;
    case TO_PUSH:{
        float t=smooth(*T/6.0f);
        g_airport.toZ=mixf(GATE_Z,-72,t);
        g_airport.toYaw=mixf(270,180,t);
        g_airport.toEng=(*T>3);
        g_airport.pbX=g_airport.toX; g_airport.pbZ=g_airport.toZ+18.5f; g_airport.pbYaw=270;
        if(*T>=6){ g_airport.toPhase=TO_TAXI; *T=0; g_airport.pbActive=0; }
        break; }
    case TO_TAXI:{
        float t=smooth(*T/11.0f);
        g_airport.toX=mixf(48,-235,t);
        g_airport.toZ=mixf(-72,-110,t);
        g_airport.toYaw=180; g_airport.toEng=1;
        if(*T>=11){ g_airport.toPhase=TO_HOLD; *T=0; }
        break; }
    case TO_HOLD:   /* hold short until the runway is clear of the arriving jet */
        g_airport.toX=-235; g_airport.toZ=-110; g_airport.toYaw=180; g_airport.toEng=1;
        if(g_airport.rwyOwner==0){ g_airport.rwyOwner=1; g_airport.toPhase=TO_LINEUP; *T=0; }
        break;
    case TO_LINEUP:{
        float t=smooth(*T/4.0f);
        g_airport.toX=mixf(-235,-252,t);
        g_airport.toZ=mixf(-110,RUNWAY_Z,t);
        g_airport.toYaw=mixf(180,360,t);
        if(*T>=4){ g_airport.toPhase=TO_ROLL; *T=0; g_airport.toYaw=0; }
        break; }
    case TO_ROLL:{
        float t=smooth(*T/8.5f);
        g_airport.toX=mixf(-252,95,t); g_airport.toZ=RUNWAY_Z; g_airport.toY=AC_GROUND_CL;
        g_airport.toPitch = (*T>6.6f)? mixf(0,10,smooth((*T-6.6f)/1.9f)) : 0;
        if(*T>=8.5f){ g_airport.toPhase=TO_CLIMB; *T=0; }
        break; }
    case TO_CLIMB:{
        float t=smooth(*T/6.0f);
        g_airport.toX=mixf(95,250,t);
        g_airport.toY=mixf(AC_GROUND_CL,42,t);
        g_airport.toPitch=13; g_airport.toGear=(*T<1.8f);
        if(g_airport.rwyOwner==1) g_airport.rwyOwner=0;   /* airborne: runway free */
        if(*T>=6){ g_airport.toPhase=TO_DEPART; *T=0; }
        break; }
    case TO_DEPART:{
        float t=smooth(*T/10.0f);
        g_airport.toX=mixf(250,470,t);
        g_airport.toY=mixf(42,135,t);
        g_airport.toZ=mixf(RUNWAY_Z,90,t);
        g_airport.toPitch=9; g_airport.toGear=0;
        if(*T>=10){ g_airport.toPhase=TO_DONE; *T=0; }
        break; }
    default: if(!g_airport.holdTakeoff && *T>4) Airport_RestartTakeoff();
    }
}

static void updateLanding(float dt)
{
    float *T=&g_airport.lnT; *T+=dt;
    switch(g_airport.lnPhase){
    case LN_HOLD:   /* hold in a distant loiter until the runway is free */
        g_airport.lnX=360; g_airport.lnY=72; g_airport.lnZ=RUNWAY_Z; g_airport.lnYaw=180;
        g_airport.lnGear=1; g_airport.lnEng=1;
        if(g_airport.rwyOwner==0 && *T>1.5f){ g_airport.rwyOwner=2; g_airport.lnPhase=LN_APPR; *T=0; }
        break;
    case LN_APPR:{
        float t=smooth(*T/11.0f);
        g_airport.lnX=mixf(360,140,t);
        g_airport.lnY=mixf(72,AC_GROUND_CL,t);
        g_airport.lnZ=RUNWAY_Z; g_airport.lnYaw=180;
        g_airport.lnPitch=mixf(5,0,t); g_airport.lnGear=1; g_airport.lnEng=1;
        if(*T>=11){ g_airport.lnPhase=LN_TOUCH; *T=0; }
        break; }
    case LN_TOUCH:{
        float t=smooth(*T/2.0f);
        g_airport.lnX=mixf(140,116,t); g_airport.lnY=AC_GROUND_CL; g_airport.lnPitch=0;
        if(*T>=2){ g_airport.lnPhase=LN_ROLL; *T=0; }
        break; }
    case LN_ROLL:{
        float t=smooth(*T/8.0f);
        g_airport.lnX=mixf(116,-150,t); g_airport.lnZ=RUNWAY_Z;
        if(*T>=8){ g_airport.lnPhase=LN_EXIT; *T=0; }
        break; }
    case LN_EXIT:{  /* vacate the runway onto the taxiway, then release it */
        float t=smooth(*T/4.0f);
        g_airport.lnX=-150; g_airport.lnZ=mixf(RUNWAY_Z,TAXIWAY_Z,t);
        g_airport.lnYaw=mixf(180,200,t); g_airport.lnEng=1;
        if(*T>=4){ if(g_airport.rwyOwner==2) g_airport.rwyOwner=0; g_airport.lnPhase=LN_TAXI; *T=0; }
        break; }
    case LN_TAXI:{
        float t=smooth(*T/11.0f);
        g_airport.lnX=mixf(-150,GATE_LN_X,t);
        g_airport.lnZ=mixf(TAXIWAY_Z,GATE_Z,t);
        g_airport.lnYaw=mixf(200,270,t); g_airport.lnEng=1;
        if(*T>=11){ g_airport.lnPhase=LN_GATE; *T=0; }
        break; }
    case LN_GATE:
        g_airport.lnEng=0;
        if(*T>6){ g_airport.lnPhase=LN_DONE; *T=0; }
        break;
    default: if(*T>4) Airport_RestartLanding();
    }
}

static void updateFlyover(float dt)
{
    if(!g_airport.flyActive) return;
    g_airport.flyT += dt;
    if(g_airport.flyT < 5.0f){          /* wait a few seconds after start */
        g_airport.flyY = -50;           /* hidden below ground */
        return;
    }
    {
        float t=smooth((g_airport.flyT-5.0f)/11.0f);
        g_airport.flyX = mixf(-6, 6, t);
        g_airport.flyY = mixf(38, 120, t);          /* climbs as it crosses overhead */
        g_airport.flyZ = mixf(200, -300, t);        /* passes over the player toward the runway */
        g_airport.flyYaw = 90;                      /* nose pointing -z */
        g_airport.flyPitch = 8;
        if(g_airport.flyT-5.0f >= 11.0f) g_airport.flyActive=0;   /* dissolved into the distance */
    }
}

void Airport_Update(float dt)
{
    g_airport.time+=dt;
    g_airport.simClock+=dt*40.0f;      /* time runs ~40x for a lively board */
    {
        float target=(g_camera.z<TERMINAL_Z1+6 && g_camera.z>TERMINAL_Z1-4 &&
                      fabsf(g_camera.x)<ENTRANCE_HALF+4)?1.0f:0.0f;
        g_airport.doorOpen += (target-g_airport.doorOpen)*clampf(dt*4.0f,0,1);
    }
    updateTakeoff(dt); updateLanding(dt); updateFlyover(dt);
}

/* sky */
static void drawSky(void)
{
    int i; float t=g_airport.time;
    glDisable(GL_LIGHTING); glDepthMask(GL_FALSE); glDisable(GL_FOG);
    glEnable(GL_BLEND);

    if(g_airport.dayMode){
        float sx=-150,sy=175,sz=-320;
        glBlendFunc(GL_SRC_ALPHA,GL_ONE);           /* additive glow */
        glPushMatrix(); glTranslatef(sx,sy,sz);
        glColor4f(1.0f,0.80f,0.35f,0.14f); glutSolidSphere(70,22,16);
        glColor4f(1.0f,0.85f,0.45f,0.20f); glutSolidSphere(46,22,16);
        glColor4f(1.0f,0.92f,0.60f,0.40f); glutSolidSphere(28,24,18);
        /* sun rays */
        glBegin(GL_TRIANGLES);
        for(i=0;i<12;i++){ float a0=i*(6.2831f/12.0f)+t*0.05f, a1=a0+0.09f;
            glColor4f(1.0f,0.9f,0.55f,0.33f); glVertex3f(0,0,0);
            glColor4f(1.0f,0.9f,0.55f,0.0f);
            glVertex3f(cosf(a0)*120,sinf(a0)*120,0); glVertex3f(cosf(a1)*120,sinf(a1)*120,0); }
        glEnd();
        glColor4f(1.0f,0.98f,0.9f,1.0f);   glutSolidSphere(17,28,20);   /* core */
        glPopMatrix();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    } else {
        /* moon + stars */
        glBlendFunc(GL_SRC_ALPHA,GL_ONE);
        glPushMatrix(); glTranslatef(150,170,-300);
        glColor4f(0.75f,0.80f,0.95f,0.30f); glutSolidSphere(28,18,14);
        glColor4f(0.92f,0.94f,0.99f,1.0f);  glutSolidSphere(12,22,16); glPopMatrix();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glPointSize(2.0f); glColor3f(0.9f,0.9f,1.0f); glBegin(GL_POINTS);
        for(i=0;i<220;i++){ float a=i*12.9898f, b=i*78.233f;
            float rx=(sinf(a)*43758.5f); rx-=floorf(rx);
            float rz=(sinf(b)*13137.1f); rz-=floorf(rz);
            glVertex3f(-400+rx*800, 55+ (i%11)*24, -320-rz*40); }
        glEnd();
    }
    /* clouds drift across the sky */
    for(i=0;i<14;i++){
        float base=-380+i*66;
        float x=base+fmodf(t*(2.5f+(i%3))+i*40,900.0f);
        float y=68+(i%4)*17, z=-240-(i%3)*40;
        float a=g_airport.dayMode?0.88f:0.30f;
        glColor4f(0.98f,0.99f,1.0f,a);
        glPushMatrix(); glTranslatef(x,y,z); glScalef(3.4f,1.0f,1.6f);
        glutSolidSphere(9,12,9); glTranslatef(2.2f,0.2f,0); glutSolidSphere(7.5f,12,9);
        glTranslatef(-4.2f,-0.1f,0.2f); glutSolidSphere(7,12,9);
        glTranslatef(2.6f,0.3f,0.1f); glutSolidSphere(6,12,9); glPopMatrix();
    }
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE); glEnable(GL_LIGHTING); glEnable(GL_FOG);
}

/* waving Bangladesh flag on a pole */
static void drawFlag(float x,float z)
{
    int i,j; float t=g_airport.time;
    Gfx_Cylinder(x,0,z,0.14f,11.0f,0.85f,0.86f,0.9f);   /* pole */
    Gfx_Emissive(0.02f,0.02f,0.02f); Gfx_EmissiveOff();
    for(j=0;j<6;j++){
        for(i=0;i<12;i++){
            float u0=i/12.0f,u1=(i+1)/12.0f, v0=j/6.0f,v1=(j+1)/6.0f;
            float fw=4.2f, fh=2.4f, base=8.4f;
            float wz0=sinf(u0*6.0f + t*4.0f)*0.35f*u0;
            float wz1=sinf(u1*6.0f + t*4.0f)*0.35f*u1;
            /* red disc centred slightly toward the hoist */
            #define INDISC(u,v) (((u)-0.44f)*((u)-0.44f)*1.0f + ((v)-0.5f)*((v)-0.5f) < 0.028f)
            int red = INDISC((u0+u1)*0.5f,(v0+v1)*0.5f);
            if(red) Gfx_Material(0.90f,0.12f,0.12f,10.0f);
            else    Gfx_Material(0.02f,0.45f,0.24f,10.0f);
            glBegin(GL_QUADS);
            glNormal3f(0,0,1);
            glVertex3f(x+u0*fw, base+fh*(1-v0), z+wz0);
            glVertex3f(x+u1*fw, base+fh*(1-v0), z+wz1);
            glVertex3f(x+u1*fw, base+fh*(1-v1), z+wz1);
            glVertex3f(x+u0*fw, base+fh*(1-v1), z+wz0);
            glEnd();
            #undef INDISC
        }
    }
}

/* ground text helper */
static void groundText(float x,float z,float rot,float h,float r,float g,float b,const char*s)
{
    const char*p; float raw=0,k=h/119.05f;
    for(p=s;*p;p++) raw+=glutStrokeWidth(GLUT_STROKE_ROMAN,*p);
    glDisable(GL_LIGHTING); glColor3f(r,g,b);
    glPushMatrix(); glTranslatef(x,0.07f,z); glRotatef(rot,0,1,0); glRotatef(90,1,0,0);
    glScalef(k,k,k); glTranslatef(-raw*0.5f,0,0); glLineWidth(5.0f);
    for(p=s;*p;p++) glutStrokeCharacter(GLUT_STROKE_ROMAN,*p);
    glLineWidth(1.0f); glPopMatrix(); glEnable(GL_LIGHTING);
}

static void lamp(float x,float y,float z,float r,float g,float b,float rad)
{
    if(g_airport.dayMode) Gfx_Material(r,g,b,40.0f); else Gfx_Emissive(r,g,b);
    glPushMatrix(); glTranslatef(x,y,z); glutSolidSphere(rad,8,6); glPopMatrix();
    Gfx_EmissiveOff();
}

/* ground / airfield */
static void drawGround(void)
{
    int i; int night=!g_airport.dayMode;

    /* grass everywhere, then paved surfaces on top */
    Gfx_TexturedFloor(-WORLD_HALF,-WORLD_HALF,WORLD_HALF,WORLD_HALF,-0.06f,TEX_GRASS,90);
    Gfx_TexturedFloor(-170,APRON_Z,170,TERMINAL_Z0,0.0f,TEX_CONCRETE,26);         /* apron */
    Gfx_TexturedFloor(RUNWAY_X0,TAXIWAY_Z-TAXIWAY_HALF,RUNWAY_X1,TAXIWAY_Z+TAXIWAY_HALF,0.012f,TEX_ASPHALT,60); /* taxiway */
    Gfx_TexturedFloor(RUNWAY_X0,RUNWAY_Z-RUNWAY_HALF,RUNWAY_X1,RUNWAY_Z+RUNWAY_HALF,0.02f,TEX_ASPHALT,80);      /* runway */
    Gfx_TexturedFloor(PARK_X0,PARK_Z0,PARK_X1,PARK_Z1,0.0f,TEX_ASPHALT,40);        /* parking */
    Gfx_TexturedFloor(-120,ROAD_Z-4,120,ROAD_Z+6,0.03f,TEX_ASPHALT,24);            /* drop-off road */
    /* link taxiway to runway and apron */
    Gfx_TexturedFloor(-260,RUNWAY_Z,-236,TAXIWAY_Z,0.013f,TEX_ASPHALT,10);
    Gfx_TexturedFloor(30,TERMINAL_Z0,66,TAXIWAY_Z+TAXIWAY_HALF,0.013f,TEX_ASPHALT,10);

    glDisable(GL_LIGHTING);
    /* runway markings */
    glColor3f(0.95f,0.95f,0.92f);
    for(i=-13;i<=13;i++){ float x=i*18.0f;      /* centreline dashes */
        glBegin(GL_QUADS); glVertex3f(x-3,0.03f,RUNWAY_Z-0.5f);glVertex3f(x+3,0.03f,RUNWAY_Z-0.5f);
        glVertex3f(x+3,0.03f,RUNWAY_Z+0.5f);glVertex3f(x-3,0.03f,RUNWAY_Z+0.5f); glEnd(); }
    /* edge lines */
    glBegin(GL_QUADS);
    glVertex3f(RUNWAY_X0,0.03f,RUNWAY_Z-RUNWAY_HALF+0.6f);glVertex3f(RUNWAY_X1,0.03f,RUNWAY_Z-RUNWAY_HALF+0.6f);
    glVertex3f(RUNWAY_X1,0.03f,RUNWAY_Z-RUNWAY_HALF+1.1f);glVertex3f(RUNWAY_X0,0.03f,RUNWAY_Z-RUNWAY_HALF+1.1f);
    glVertex3f(RUNWAY_X0,0.03f,RUNWAY_Z+RUNWAY_HALF-1.1f);glVertex3f(RUNWAY_X1,0.03f,RUNWAY_Z+RUNWAY_HALF-1.1f);
    glVertex3f(RUNWAY_X1,0.03f,RUNWAY_Z+RUNWAY_HALF-0.6f);glVertex3f(RUNWAY_X0,0.03f,RUNWAY_Z+RUNWAY_HALF-0.6f);
    glEnd();
    /* threshold piano keys */
    for(i=0;i<8;i++){ float z=RUNWAY_Z-RUNWAY_HALF+2.5f+i*3.3f;
        glBegin(GL_QUADS);
        glVertex3f(-272,0.03f,z);glVertex3f(-260,0.03f,z);glVertex3f(-260,0.03f,z+2.2f);glVertex3f(-272,0.03f,z+2.2f);
        glVertex3f(260,0.03f,z);glVertex3f(272,0.03f,z);glVertex3f(272,0.03f,z+2.2f);glVertex3f(260,0.03f,z+2.2f);
        glEnd(); }
    /* taxiway centreline (yellow) */
    glColor3f(0.92f,0.8f,0.12f);
    glBegin(GL_QUADS);
    glVertex3f(RUNWAY_X0,0.02f,TAXIWAY_Z-0.4f);glVertex3f(RUNWAY_X1,0.02f,TAXIWAY_Z-0.4f);
    glVertex3f(RUNWAY_X1,0.02f,TAXIWAY_Z+0.4f);glVertex3f(RUNWAY_X0,0.02f,TAXIWAY_Z+0.4f); glEnd();
    /* parking stall lines */
    glColor3f(0.9f,0.9f,0.85f);
    for(i=-8;i<=8;i++){ float x=i*11.0f; glBegin(GL_QUADS);
        glVertex3f(x-0.15f,0.04f,PARK_Z0+2);glVertex3f(x+0.15f,0.04f,PARK_Z0+2);
        glVertex3f(x+0.15f,0.04f,PARK_Z0+34);glVertex3f(x-0.15f,0.04f,PARK_Z0+34); glEnd(); }
    /* pedestrian crossing at entrance */
    glColor3f(0.92f,0.92f,0.9f);
    for(i=0;i<7;i++){ float x=-6.5f+i*1.9f; glBegin(GL_QUADS);
        glVertex3f(x,0.05f,ROAD_Z-3.5f);glVertex3f(x+1.0f,0.05f,ROAD_Z-3.5f);
        glVertex3f(x+1.0f,0.05f,ROAD_Z+3.5f);glVertex3f(x,0.05f,ROAD_Z+3.5f); glEnd(); }
    glEnable(GL_LIGHTING);

    groundText(-232,RUNWAY_Z,0,7.0f,1,1,1,"10");
    groundText( 232,RUNWAY_Z,180,7.0f,1,1,1,"28");

    /* airfield lights */
    for(i=-13;i<=13;i++){ float x=i*20.0f;
        lamp(x,0.25f,RUNWAY_Z-RUNWAY_HALF,0.7f,0.75f,1.0f,0.22f);
        lamp(x,0.25f,RUNWAY_Z+RUNWAY_HALF,0.7f,0.75f,1.0f,0.22f); }
    lamp(-272,0.3f,RUNWAY_Z,0.1f,1.0f,0.2f,0.3f); lamp(272,0.3f,RUNWAY_Z,1.0f,0.15f,0.15f,0.3f);
    for(i=-13;i<=13;i++){ float x=i*20.0f; lamp(x,0.2f,TAXIWAY_Z,0.15f,0.4f,1.0f,0.16f); }

    for(i=-2;i<=2;i++){ float x=i*70.0f+35.0f;
        Gfx_Cylinder(x,0,-74,0.4f,15,0.3f,0.31f,0.34f);
        Gfx_Material(0.2f,0.2f,0.22f,20.0f); Gfx_Box(x-2.5f,14.6f,-75,x+2.5f,15.2f,-73);
        lamp(x-1.6f,14.7f,-73.5f,1.0f,0.98f,0.85f,0.5f); lamp(x,14.7f,-73.5f,1.0f,0.98f,0.85f,0.5f);
        lamp(x+1.6f,14.7f,-73.5f,1.0f,0.98f,0.85f,0.5f); }

    /* parking streetlights + gardens */
    for(i=-4;i<=4;i++){ float x=i*22.0f;
        Gfx_Cylinder(x,0,PARK_Z0+2,0.16f,7,0.22f,0.23f,0.25f);
        Gfx_Material(0.2f,0.2f,0.22f,10.0f); Gfx_Box(x-0.15f,6.8f,PARK_Z0+2,x+1.6f,7.0f,PARK_Z0+2.2f);
        lamp(x+1.4f,6.85f,PARK_Z0+2.1f,1.0f,0.92f,0.6f,0.28f);
        Gfx_Cylinder(x,0,PARK_Z1-2,0.16f,7,0.22f,0.23f,0.25f);
        Gfx_Material(0.2f,0.2f,0.22f,10.0f); Gfx_Box(x-0.15f,6.8f,PARK_Z1-2.2f,x+1.6f,7.0f,PARK_Z1-2.0f);
        lamp(x+1.4f,6.85f,PARK_Z1-2.1f,1.0f,0.92f,0.6f,0.28f); }
    (void)night;

    Gfx_Cylinder(250,0,TAXIWAY_Z+10,0.12f,6,0.8f,0.8f,0.82f);
    { float a=sinf(g_airport.time*0.8f)*20.0f;
      glPushMatrix(); glTranslatef(250,5.7f,TAXIWAY_Z+10);
      glRotatef(90+a,0,1,0);            /* point the cone downwind */
      Gfx_Material(0.95f,0.5f,0.05f,10.0f);
      gluCylinder(Gfx_Quadric(),0.5f,0.28f,2.6f,12,1);
      Gfx_Material(0.95f,0.95f,0.95f,10.0f);
      glTranslatef(0,0,2.6f); gluCylinder(Gfx_Quadric(),0.28f,0.16f,1.4f,12,1);
      glPopMatrix(); }
}

/* trees / gardens */
static void drawTree(float x,float z,float s)
{
    Gfx_Cylinder(x,0,z,0.3f*s,2.6f*s,0.34f,0.22f,0.10f);
    Gfx_Material(0.10f,0.40f,0.14f,6.0f);
    glPushMatrix(); glTranslatef(x,3.2f*s,z);
    glutSolidSphere(1.7f*s,12,9);
    glTranslatef(0.9f*s,0.4f*s,0.2f*s); glutSolidSphere(1.2f*s,10,8);
    glTranslatef(-1.7f*s,-0.1f*s,-0.3f*s); glutSolidSphere(1.25f*s,10,8);
    glPopMatrix();
}

/* control tower */
static void drawTower(void)
{
    Gfx_Material(0.72f,0.73f,0.70f,20.0f);
    Gfx_Cylinder(TOWER_X,0,TOWER_Z,3.2f,26,0.72f,0.73f,0.70f);
    /* cab */
    Gfx_Material(0.3f,0.32f,0.35f,30.0f);
    glPushMatrix(); glTranslatef(TOWER_X,26,TOWER_Z);
    Gfx_Box(-6,0,-6,6,1,6);
    glPopMatrix();
    Gfx_Glass(TOWER_X-5.5f,27,TOWER_Z-5.5f,TOWER_X+5.5f,31,TOWER_Z+5.5f,0.3f,0.55f,0.7f,0.45f);
    Gfx_Material(0.25f,0.26f,0.28f,20.0f);
    glPushMatrix(); glTranslatef(TOWER_X,31,TOWER_Z); Gfx_Box(-6.2f,0,-6.2f,6.2f,1.2f,6.2f); glPopMatrix();
    /* rotating radar */
    Gfx_Cylinder(TOWER_X,32.2f,TOWER_Z,0.2f,3.0f,0.3f,0.3f,0.33f);
    glPushMatrix(); glTranslatef(TOWER_X,35.2f,TOWER_Z); glRotatef(g_airport.time*90.0f,0,1,0);
    Gfx_Metal(0.7f,0.72f,0.76f);
    glPushMatrix(); glRotatef(30,1,0,0); glScalef(1,0.25f,3.2f); glutSolidSphere(1.2f,12,8); glPopMatrix();
    glPopMatrix();
    lamp(TOWER_X,38.5f,TOWER_Z,1.0f,0.1f,0.1f,0.4f);
    Gfx_TextFlat(TOWER_X,20,TOWER_Z+3.3f,0,1.2f,0.9f,0.9f,0.9f,"ATC");
}

/* A tilted radar scope: dark green screen with range rings, a rotating sweep and blips,
   angled back so the controllers (and the F4 camera behind them) can read it. */
static void radarScope(float x,float y,float z,float tilt,float rad)
{
    float t=g_airport.time; int s,ring;
    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(tilt,1,0,0);                          /* tilt the screen back */
    Gfx_Material(0.07f,0.07f,0.09f,40.0f);          /* bezel */
    Gfx_Box(-rad-0.14f,-rad-0.14f,-0.06f, rad+0.14f,rad+0.14f,0.0f);
    Gfx_Material(0.02f,0.07f,0.03f,60.0f);          /* screen glass */
    Gfx_Box(-rad,-rad,0.0f, rad,rad,0.02f);
    glDisable(GL_LIGHTING);
    glColor3f(0.12f,0.8f,0.35f);                    /* range rings */
    for(ring=1;ring<=3;ring++){ float rr=rad*ring/3.0f;
        glBegin(GL_LINE_LOOP); for(s=0;s<30;s++){ float a=s*6.2831853f/30.0f;
            glVertex3f(cosf(a)*rr,sinf(a)*rr,0.03f);} glEnd(); }
    glBegin(GL_LINES);                              /* cross-hairs */
    glVertex3f(-rad,0,0.03f);glVertex3f(rad,0,0.03f); glVertex3f(0,-rad,0.03f);glVertex3f(0,rad,0.03f); glEnd();
    { float sw=t*1.6f; glColor3f(0.4f,1.0f,0.55f);  /* rotating sweep */
      glBegin(GL_LINES); glVertex3f(0,0,0.04f); glVertex3f(cosf(sw)*rad,sinf(sw)*rad,0.04f); glEnd(); }
    glPointSize(4.0f); glColor3f(0.7f,1.0f,0.6f);   /* blips */
    glBegin(GL_POINTS); for(s=0;s<4;s++){ float a=s*1.7f+t*0.12f, rr=rad*(0.28f+0.16f*s);
        glVertex3f(cosf(a)*rr,sinf(a)*rr,0.05f);} glEnd();
    glEnable(GL_LIGHTING);
    glPopMatrix();
}

/* Inside the control cab (F4 view): consoles with radar scopes + data screens + keyboards,
   controllers with chairs, a hanging status board, and a supervisor on binoculars. */
static void drawTowerInterior(void)
{
    float t=g_airport.time; int i;
    glPushMatrix(); glTranslatef(0,27.0f,0);      /* cab floor level */
    Gfx_Material(0.24f,0.25f,0.28f,10.0f);        /* floor */
    Gfx_Box(TOWER_X-5.6f,-0.02f,TOWER_Z-5.6f,TOWER_X+5.6f,0.0f,TOWER_Z+5.6f);

    for(i=-1;i<=1;i++){ float cx2=TOWER_X+i*3.4f;
        /* console desk */
        Gfx_Material(0.16f,0.17f,0.21f,20.0f); Gfx_Box(cx2-1.5f,0,-70.9f, cx2+1.5f,0.95f,-69.5f);
        /* keyboard */
        Gfx_Material(0.10f,0.10f,0.12f,20.0f);  Gfx_Box(cx2-0.55f,0.95f,-69.95f, cx2+0.55f,1.0f,-69.6f);
        radarScope(cx2, 1.55f, -70.6f, 34.0f, 0.6f);
        glPushMatrix(); glTranslatef(cx2+0.95f,1.35f,-70.45f); glRotatef(28,1,0,0);
            Gfx_Material(0.08f,0.08f,0.1f,40.0f); Gfx_Box(-0.42f,-0.42f,-0.04f,0.42f,0.42f,0.0f);
            Gfx_Emissive(0.15f,0.35f,0.62f); Gfx_Box(-0.36f,-0.36f,0.0f,0.36f,0.36f,0.02f); Gfx_EmissiveOff();
        glPopMatrix();
        People_SetGesture(i==0?1:0);
        People_Draw(cx2,-68.7f,90, t*2.0f+i, 0.0f, 3, 0);
        /* office chair behind */
        Gfx_Material(0.12f,0.13f,0.16f,20.0f);
        Gfx_Box(cx2-0.35f,0,-68.1f,cx2+0.35f,0.5f,-67.4f);
        Gfx_Box(cx2-0.35f,0.5f,-67.5f,cx2+0.35f,1.35f,-67.4f);
    }
    Gfx_Material(0.02f,0.03f,0.05f,30.0f); Gfx_Box(TOWER_X-3.6f,3.4f,-66.2f,TOWER_X+3.6f,4.5f,-66.1f);
    Gfx_TextFlat(TOWER_X,3.75f,-66.05f,0,0.4f,0.35f,0.9f,1.0f,"RWY 10   WIND 320/08   QNH 1010");
    /* supervisor with binoculars at the window */
    People_Draw(TOWER_X+4.6f,-71.5f,90, 0,0, 1, 0);
    Gfx_Material(0.04f,0.04f,0.05f,30.0f);
    Gfx_Box(TOWER_X+4.35f,1.62f,-72.05f,TOWER_X+4.85f,1.78f,-71.9f);   /* binoculars */
    /* signaller giving hand signals */
    People_SetGesture(2);
    People_Draw(TOWER_X-4.6f,-68.7f,90, 0,0, 4, 0);
    glPopMatrix();
}

/* terminal */
static void mullions(float z,float x0,float x1,float y0,float y1,int n)
{
    int i; Gfx_Metal(0.78f,0.80f,0.84f);
    for(i=0;i<=n;i++){ float x=mixf(x0,x1,(float)i/n);
        Gfx_Box(x-0.12f,y0,z-0.12f,x+0.12f,y1,z+0.12f); }
    Gfx_Box(x0,y1-0.2f,z-0.12f,x1,y1,z+0.12f);
    Gfx_Box(x0,y0,z-0.12f,x1,y0+0.2f,z+0.12f);
}

static void drawTerminalOpaque(void)
{
    /* end walls */
    Gfx_Material(0.80f,0.79f,0.74f,15.0f);
    Gfx_TexBox(TEX_CONCRETE,TERMINAL_X0,0,TERMINAL_Z0,TERMINAL_X0+1.5f,TERMINAL_H,TERMINAL_Z1,4);
    Gfx_TexBox(TEX_CONCRETE,TERMINAL_X1-1.5f,0,TERMINAL_Z0,TERMINAL_X1,TERMINAL_H,TERMINAL_Z1,4);
    /* roof slab with overhang */
    Gfx_Material(0.72f,0.74f,0.78f,25.0f);
    Gfx_Box(TERMINAL_X0-2,TERMINAL_H,TERMINAL_Z0-2,TERMINAL_X1+2,TERMINAL_H+1.3f,TERMINAL_Z1+2);
    /* white fascia strip below the roof edge */
    Gfx_Material(0.90f,0.91f,0.93f,20.0f);
    Gfx_Box(TERMINAL_X0-2,TERMINAL_H-0.4f,TERMINAL_Z1+1.6f,TERMINAL_X1+2,TERMINAL_H,TERMINAL_Z1+2.0f);
    /* raised clerestory band */
    Gfx_Material(0.82f,0.84f,0.88f,20.0f);
    Gfx_Box(TERMINAL_X0,TERMINAL_H+1.3f,-6,TERMINAL_X1,TERMINAL_H+3.0f,6);

    /* landside mullion frames (glass drawn later) */
    mullions(TERMINAL_Z1,TERMINAL_X0+1.5f,-ENTRANCE_HALF-1,1.0f,TERMINAL_H,10);
    mullions(TERMINAL_Z1,ENTRANCE_HALF+1,TERMINAL_X1-1.5f,1.0f,TERMINAL_H,10);
    /* airside mullion frames */
    mullions(TERMINAL_Z0,TERMINAL_X0+1.5f,TERMINAL_X1-1.5f,1.0f,TERMINAL_H,22);

    /* entrance canopy + columns */
    Gfx_Material(0.5f,0.52f,0.55f,20.0f);
    Gfx_Box(-14,6.4f,TERMINAL_Z1,14,6.9f,TERMINAL_Z1+9);
    Gfx_Cylinder(-12,0,TERMINAL_Z1+8,0.4f,6.4f,0.6f,0.61f,0.63f);
    Gfx_Cylinder( 12,0,TERMINAL_Z1+8,0.4f,6.4f,0.6f,0.61f,0.63f);

    Gfx_Material(0.10f,0.14f,0.20f,30.0f);
    Gfx_Box(-46,TERMINAL_H+0.2f,TERMINAL_Z1+2.0f,46,TERMINAL_H+2.8f,TERMINAL_Z1+2.25f);
    Gfx_TextFlat(0,TERMINAL_H+0.9f,TERMINAL_Z1+2.28f,0,1.5f,0.95f,0.85f,0.20f,
                 "HAZRAT SHAHJALAL INTERNATIONAL AIRPORT");
    Gfx_TextFlat(0,TERMINAL_H+2.9f,TERMINAL_Z1+2.28f,0,0.7f,0.85f,0.9f,1.0f,"DHAKA, BANGLADESH");
    /* a green/red accent bar echoing the flag */
    Gfx_Material(0.0f,0.5f,0.25f,20.0f); Gfx_Box(-46,TERMINAL_H+0.2f,TERMINAL_Z1+2.26f,-40,TERMINAL_H+2.8f,TERMINAL_Z1+2.30f);
    Gfx_Material(0.85f,0.1f,0.1f,20.0f);  Gfx_Box(40,TERMINAL_H+0.2f,TERMINAL_Z1+2.26f,46,TERMINAL_H+2.8f,TERMINAL_Z1+2.30f);

    /* ceiling underside + light panels */
    Gfx_Material(0.85f,0.86f,0.88f,10.0f);
    Gfx_Box(TERMINAL_X0+1.5f,TERMINAL_H-0.25f,TERMINAL_Z0,TERMINAL_X1-1.5f,TERMINAL_H,TERMINAL_Z1);
    { int i,j; for(i=-4;i<=4;i++) for(j=-1;j<=1;j++){
        lamp(i*14.0f,TERMINAL_H-0.35f,j*16.0f,1.0f,0.98f,0.9f,0.6f); } }
}

static void drawTerminalGlass(void)
{
    int i; float d=g_airport.doorOpen*(ENTRANCE_HALF-0.3f);
    /* landside curtain wall (two spans either side of the doors) - bright reflective glazing.
       Fairly opaque so it reads as sunlit glass from the forecourt (the runway view is
       provided by the airside wall below). A pale sky tint is layered over the interior. */
    Gfx_Glass(TERMINAL_X0+1.5f,1.0f,TERMINAL_Z1-0.05f,-ENTRANCE_HALF-1,TERMINAL_H,TERMINAL_Z1+0.05f,0.62f,0.78f,0.92f,0.62f);
    Gfx_Glass(ENTRANCE_HALF+1,1.0f,TERMINAL_Z1-0.05f,TERMINAL_X1-1.5f,TERMINAL_H,TERMINAL_Z1+0.05f,0.62f,0.78f,0.92f,0.62f);
    /* sliding doors (slide apart as doorOpen -> 1) */
    Gfx_Glass(-ENTRANCE_HALF-1-d,0.1f,TERMINAL_Z1-0.02f,-d,5.4f,TERMINAL_Z1+0.06f,0.6f,0.78f,0.88f,0.45f);
    Gfx_Glass(d,0.1f,TERMINAL_Z1-0.02f,ENTRANCE_HALF+1+d,5.4f,TERMINAL_Z1+0.06f,0.6f,0.78f,0.88f,0.45f);
    /* airside curtain wall facing the runway */
    for(i=-5;i<5;i++){ float x0=i*14.0f, x1=x0+13.4f;
        if(x1>TERMINAL_X1-1.5f) x1=TERMINAL_X1-1.5f;
        if(x0<TERMINAL_X0+1.5f) x0=TERMINAL_X0+1.5f;
        Gfx_Glass(x0,1.0f,TERMINAL_Z0-0.05f,x1,TERMINAL_H,TERMINAL_Z0+0.05f,0.52f,0.70f,0.82f,0.28f); }
}

/* interior */
static void plant(float x,float z)
{
    Gfx_Material(0.35f,0.25f,0.18f,10.0f);
    Gfx_Cylinder(x,0,z,0.5f,0.7f,0.4f,0.3f,0.2f);
    Gfx_Material(0.12f,0.45f,0.16f,6.0f);
    glPushMatrix(); glTranslatef(x,1.4f,z); glutSolidSphere(0.9f,10,8);
    glTranslatef(0.4f,0.4f,0); glutSolidSphere(0.6f,9,7); glPopMatrix();
}

static void seatRow(float x,float z,int n)
{
    int i; for(i=0;i<n;i++){ float sx=x+i*1.3f;
        Gfx_Material(0.12f,0.30f,0.5f,40.0f);
        Gfx_Box(sx,0.45f,z-0.35f,sx+1.05f,0.55f,z+0.35f);
        Gfx_Box(sx,0.55f,z+0.24f,sx+1.05f,1.15f,z+0.35f);
        Gfx_Metal(0.6f,0.62f,0.66f);
        Gfx_Box(sx+0.45f,0,z-0.05f,sx+0.6f,0.45f,z+0.05f); }
}

static void securityArch(float x)
{
    Gfx_Material(0.5f,0.52f,0.56f,40.0f);
    Gfx_Box(x-1.3f,0,-0.9f,x-1.05f,2.7f,0.9f);
    Gfx_Box(x+1.05f,0,-0.9f,x+1.3f,2.7f,0.9f);
    Gfx_Box(x-1.3f,2.5f,-0.9f,x+1.3f,2.7f,0.9f);
    Gfx_Emissive(0.1f,0.9f,0.3f);
    glPushMatrix(); glTranslatef(x,2.35f,0); glutSolidSphere(0.1f,6,5); glPopMatrix();
    Gfx_EmissiveOff();
}

static void xrayBelt(float x,float z)
{
    int i; float t=g_airport.time;
    Gfx_Material(0.25f,0.26f,0.28f,20.0f);
    Gfx_Box(x-3.5f,0.55f,z-0.6f,x+3.5f,0.85f,z+0.6f);     /* belt table */
    Gfx_Material(0.15f,0.16f,0.18f,10.0f);
    Gfx_Box(x-1.0f,0.85f,z-0.7f,x+1.0f,2.0f,z+0.7f);      /* scanner box */
    /* rollers */
    Gfx_Metal(0.5f,0.52f,0.55f);
    for(i=0;i<8;i++){ float rx=x-3.2f+i*0.85f;
        glPushMatrix(); glTranslatef(rx,0.86f,z); glRotatef(90,1,0,0); glRotatef(t*180,0,0,1);
        gluCylinder(Gfx_Quadric(),0.08f,0.08f,1.2f,8,1); glPopMatrix(); }
    /* a tray moving through */
    { float bx=x-3.0f+fmodf(t*1.5f,6.0f);
      Gfx_Material(0.4f,0.2f,0.1f,10.0f); Gfx_Box(bx,0.9f,z-0.35f,bx+0.7f,1.15f,z+0.35f); }
}

static void baggageCarousel(float cx,float cz)
{
    int i; float t=g_airport.time;
    Gfx_Material(0.2f,0.21f,0.23f,20.0f);
    glPushMatrix(); glTranslatef(cx,0,cz); glRotatef(-90,1,0,0);
    gluCylinder(Gfx_Quadric(),6.5f,6.5f,0.9f,32,1);
    glPopMatrix();
    Gfx_Metal(0.35f,0.36f,0.4f);
    glPushMatrix(); glTranslatef(cx,0.9f,cz); glRotatef(-90,1,0,0);
    gluDisk(Gfx_Quadric(),4.2f,6.6f,32,1); glPopMatrix();
    /* sloped stainless center */
    Gfx_Metal(0.55f,0.57f,0.6f);
    glPushMatrix(); glTranslatef(cx,0.9f,cz); glRotatef(-90,1,0,0);
    gluCylinder(Gfx_Quadric(),0.5f,4.2f,1.4f,24,1); glPopMatrix();
    /* moving luggage around the belt */
    for(i=0;i<9;i++){ float a=t*0.5f+i*(6.2831f/9.0f);
        float bx=cx+cosf(a)*5.4f, bz=cz+sinf(a)*5.4f;
        Gfx_Material(0.2f+0.2f*(i%3),0.15f+0.1f*(i%2),0.35f-0.05f*(i%3),15.0f);
        glPushMatrix(); glTranslatef(bx,1.05f,bz); glRotatef(-a*57.3f,0,1,0);
        Gfx_Box(-0.45f,0,-0.3f,0.45f,0.45f,0.3f); glPopMatrix(); }
    Gfx_TextFlat(cx,3.2f,cz+7.0f,0,0.5f,0.9f,0.9f,0.95f,"BAGGAGE CLAIM");
}

static void fidsBoard(void)
{
    char clk[8];
    int i, base;
    static const char *fl[9][3] = {
        {"BG 201","LONDON  LHR",  "3"},
        {"EK 585","DUBAI   DXB",  "4"},
        {"BG 385","NEW YORK JFK", "5"},
        {"QR 641","DOHA    DOH",  "6"},
        {"SQ 447","SINGAPORE SIN","2"},
        {"MH 103","KUALA LUMPUR", "7"},
        {"TG 340","BANGKOK BKK",  "1"},
        {"SV 805","JEDDAH  JED",  "8"},
        {"BS 111","KOLKATA CCU",  "2"}
    };
    int off[9]={12,28,55,80,110,140,175,210,250};
    Airport_ClockStr(clk,sizeof clk);
    base=((int)(g_airport.simClock/60.0f))%(24*60);

    /* hung high near the ceiling like a real FIDS. The board panel sits BEHIND the text
       (its front face at z=9.0); all text is drawn at z=9.1, i.e. in front, so it shows. */
    Gfx_Material(0.015f,0.03f,0.05f,40.0f);
    Gfx_Box(-19,6.8f,8.6f,19,14.4f,9.0f);
    Gfx_Material(0.05f,0.12f,0.22f,20.0f); Gfx_Box(-19,13.3f,9.0f,19,14.4f,9.04f);
    Gfx_TextLeft(-17.5f,13.55f,9.1f,0,0.55f,0.25f,0.85f,1.0f,"DEPARTURES");
    Gfx_TextLeft(13.5f,13.55f,9.1f,0,0.55f,1.0f,0.9f,0.3f,clk);
    Gfx_TextLeft(-17.5f,12.6f,9.1f,0,0.32f,0.55f,0.62f,0.72f,"FLIGHT");
    Gfx_TextLeft(-12.0f,12.6f,9.1f,0,0.32f,0.55f,0.62f,0.72f,"DESTINATION");
    Gfx_TextLeft( 3.5f,12.6f,9.1f,0,0.32f,0.55f,0.62f,0.72f,"TIME");
    Gfx_TextLeft( 8.5f,12.6f,9.1f,0,0.32f,0.55f,0.62f,0.72f,"GATE");
    Gfx_TextLeft(11.5f,12.6f,9.1f,0,0.32f,0.55f,0.62f,0.72f,"STATUS");
    for(i=0;i<9;i++){
        int m=(base+off[i])%(24*60);
        int togo=off[i];                       /* minutes until departure (in sim time) */
        float y=11.9f-i*0.55f;
        char tm[8];
        const char *st; float sr,sg,sb;
        if(i==3){ st="DELAYED"; sr=1.0f;sg=0.35f;sb=0.25f; }        /* Doha delayed */
        else if(togo<=6){ st="FINAL CALL"; sr=1.0f;sg=0.55f;sb=0.15f; }
        else if(togo<=20){ st="BOARDING"; sr=0.3f;sg=1.0f;sb=0.45f; }
        else if(togo<=45){ st="GATE OPEN"; sr=0.7f;sg=0.9f;sb=1.0f; }
        else { st="ON TIME"; sr=0.85f;sg=0.9f;sb=0.95f; }
        snprintf(tm,sizeof tm,"%02d:%02d",m/60,m%60);
        Gfx_TextLeft(-17.5f,y,9.1f,0,0.34f,0.95f,0.85f,0.25f,fl[i][0]);   /* flight (amber) */
        Gfx_TextLeft(-12.0f,y,9.1f,0,0.34f,0.92f,0.94f,0.96f,fl[i][1]);   /* destination */
        Gfx_TextLeft(  3.5f,y,9.1f,0,0.34f,0.92f,0.94f,0.96f,tm);         /* time */
        { char g[4]; snprintf(g,sizeof g,"%d",(i%6)+1);
          Gfx_TextLeft(9.0f,y,9.1f,0,0.34f,0.92f,0.94f,0.96f,g); }        /* gate */
        Gfx_TextLeft(11.5f,y,9.1f,0,0.34f,sr,sg,sb,st);                   /* status */
    }
}

static void hangingSign(float x,float z,const char*s,float r,float g,float b)
{
    Gfx_Material(0.08f,0.12f,0.2f,20.0f);
    float w=Gfx_TextWidth(0.5f,s)*0.5f+0.6f;
    Gfx_Box(x-w,TERMINAL_H-2.0f,z-0.08f,x+w,TERMINAL_H-1.1f,z+0.08f);
    Gfx_TextFlat(x,TERMINAL_H-1.8f,z+0.1f,0,0.5f,r,g,b,s);
    Gfx_TextFlat(x,TERMINAL_H-1.8f,z-0.1f,180,0.5f,r,g,b,s);
}

static void jetBridge(float gx)
{
    Gfx_Material(0.6f,0.62f,0.66f,30.0f);
    Gfx_Cylinder(gx,0,TERMINAL_Z0-2,1.6f,5.0f,0.6f,0.62f,0.66f);
    glPushMatrix();
    Gfx_Material(0.72f,0.74f,0.78f,30.0f);
    Gfx_Box(gx-1.4f,3.4f,-46,gx+1.4f,6.0f,TERMINAL_Z0-1);
    Gfx_Glass(gx-1.3f,4.0f,-46,gx+1.3f,5.4f,TERMINAL_Z0-1,0.3f,0.5f,0.65f,0.4f);
    /* support leg + cab */
    Gfx_Cylinder(gx,0,-45,0.3f,3.4f,0.4f,0.41f,0.44f);
    Gfx_Material(0.5f,0.52f,0.56f,20.0f);
    Gfx_Box(gx-1.8f,3.2f,-47.5f,gx+1.8f,6.2f,-45.5f);
    glPopMatrix();
}

/* one check-in counter: desk, monitor to the SIDE (so it never blocks the view),
   a clear TRANSPARENT glass screen, an overhead number, an OPEN light, and an agent
   who gestures while talking. */
static void checkinCounter(float x,int num,int open)
{
    char s[16]; float t=g_airport.time;
    Gfx_Material(0.20f,0.38f,0.58f,30.0f);
    Gfx_Box(x-3.0f,0,12.2f,x+3.0f,1.15f,13.8f);
    Gfx_Material(0.9f,0.9f,0.92f,20.0f);
    Gfx_Box(x-3.0f,1.15f,12.2f,x+3.0f,1.25f,13.2f);
    Gfx_Material(0.4f,0.42f,0.46f,20.0f);
    Gfx_Box(x+1.9f,1.25f,12.95f,x+2.4f,1.62f,13.05f);
    Gfx_Emissive(0.15f,0.7f,0.9f); Gfx_Box(x+1.95f,1.3f,13.05f,x+2.35f,1.58f,13.08f); Gfx_EmissiveOff();
    /* overhead number sign */
    Gfx_Material(0.10f,0.16f,0.28f,20.0f);
    Gfx_Box(x-1.8f,3.6f,12.9f,x+1.8f,4.5f,13.0f);
    sprintf(s,"COUNTER %d",num);
    Gfx_TextFlat(x,3.95f,13.05f,0,0.5f,1.0f,0.85f,0.2f,s);
    if(open){
        Gfx_Emissive(0.1f,0.9f,0.3f); Gfx_Box(x-1.7f,4.55f,12.9f,x+1.7f,4.8f,13.0f); Gfx_EmissiveOff();
        Gfx_TextFlat(x,4.62f,13.05f,0,0.3f,0.05f,0.2f,0.05f,"OPEN");
    }
    /* agent BEHIND the desk, talking (arms move) and facing the passengers (+z).
       No solid pane here - the agent must stay clearly visible to the passenger. */
    People_SetGesture(1);
    People_Draw(x,11.3f,270, t*1.4f + x, 0.0f, 3, 0);
}

/* A living check-in queue at column cx: passengers approach, are served at the desk
   (talking, arms moving) for a couple of seconds, then walk off to the aisle - the line
   keeps flowing instead of teleporting or vanishing. */
static void queueAt(float cx, float seed)
{
    int i; float t=g_airport.time; int N=4;
    float serveZ=14.5f, spacing=1.7f, P=8.0f, lane=cx-1.3f;  /* queue sits left of centre */
    for(i=0;i<N;i++){
        float p=fmodf(t/P + seed*0.13f + (float)i/N, 1.0f);
        float x=lane, z, walk, yaw=90.0f; int served=0;
        int variant=(i*5+(int)seed)%6, bag=(i%2)?1:2;
        if(p<0.55f){                       /* shuffling forward toward the desk */
            z = serveZ + (1.0f-p/0.55f)*(N-1)*spacing; walk=0.45f;
        } else if(p<0.72f){                /* at the desk, being served (talking) */
            z = serveZ; walk=0.0f; served=1;
        } else {                           /* walk away to the aisle and off */
            float wp=(p-0.72f)/0.28f;
            x = lane + wp*9.0f; z = serveZ - wp*1.5f; walk=0.7f; yaw=0.0f;
        }
        if(served) People_SetGesture(1);
        People_Draw(x, z, yaw, t*3.0f+i+seed, walk, variant, bag);
    }
}

/* The PLAYER's own counter: no one is in the line when you arrive (you are served first).
   Only after check-in is done do one or two passengers walk up, one at a time, then it goes
   quiet again - not an endless stream. */
static void queueAtPlayer(float cx)
{
    static float doneT=-1.0f;
    float t=g_airport.time, serveZ=14.5f, lane=cx-1.3f;
    int i;
    if(g_story.step < ST_BAGGAGE){ doneT=-1.0f; return; }   /* empty until you finish check-in */
    if(doneT<0.0f) doneT=t;
    {
        float el=t-doneT;
        for(i=0;i<2;i++){                                    /* just two, in sequence */
            float p=(el-i*6.5f)/8.0f, x=lane, z, walk, yaw=90.0f; int served=0;
            if(p<0.0f || p>1.0f) continue;
            if(p<0.5f){ z=serveZ+(1.0f-p/0.5f)*4.0f; walk=0.5f; }
            else if(p<0.72f){ z=serveZ; walk=0.0f; served=1; }
            else { float wp=(p-0.72f)/0.28f; x=lane+wp*9.0f; z=serveZ-wp*1.5f; walk=0.7f; yaw=0.0f; }
            if(served) People_SetGesture(1);
            People_Draw(x,z,yaw, t*3.0f+i, walk, (i?2:4)%6, i?1:2);
        }
    }
}

/* baggage drop with an X-ray scanner: bags ride the belt in one side, pass through the
   scanner box, and come out the other side (repeating). Belt runs along x. */
static void baggageDrop(float cx,float cz)
{
    int i; float t=g_airport.time;
    static const float BAGCOL[3][3]={{0.95f,0.45f,0.05f},{0.15f,0.55f,0.85f},{0.8f,0.15f,0.5f}};
    /* in-feed and out-feed belt tables */
    Gfx_Metal(0.55f,0.57f,0.60f);
    Gfx_Box(cx-5.4f,0.5f,cz-0.55f,cx-1.5f,0.82f,cz+0.55f);
    Gfx_Box(cx+1.5f,0.5f,cz-0.55f,cx+5.4f,0.82f,cz+0.55f);
    /* belt rollers */
    Gfx_Metal(0.4f,0.42f,0.46f);
    for(i=0;i<12;i++){ float rx=cx-5.2f+i*0.9f; if(fabsf(rx-cx)<1.6f) continue;
        glPushMatrix(); glTranslatef(rx,0.84f,cz-0.55f);
        gluCylinder(Gfx_Quadric(),0.08f,0.08f,1.1f,8,1); glPopMatrix(); }
    Gfx_Material(0.22f,0.24f,0.28f,25.0f); Gfx_Box(cx-1.6f,0.45f,cz-0.95f,cx+1.6f,2.35f,cz+0.95f);
    Gfx_Material(0.02f,0.02f,0.03f,8.0f);  Gfx_Box(cx-1.6f,0.85f,cz-0.45f,cx+1.6f,1.55f,cz+0.45f);
    Gfx_Material(0.10f,0.10f,0.11f,10.0f);
    for(i=0;i<5;i++){ float fx=cx-1.5f+i*0.16f; Gfx_Box(cx+1.55f,0.9f,cz-0.4f+i*0.2f,cx+1.62f,1.5f,cz-0.24f+i*0.2f);(void)fx; }
    Gfx_Emissive(0.1f,0.7f,0.25f); Gfx_Box(cx-1.4f,1.35f,cz+0.96f,cx-0.4f,1.95f,cz+0.99f); Gfx_EmissiveOff();
    Gfx_Material(0.9f,0.8f,0.1f,20.0f); Gfx_Box(cx+0.4f,1.6f,cz+0.96f,cx+1.2f,1.95f,cz+0.98f);
    /* sign */
    Gfx_Material(0.06f,0.1f,0.16f,20.0f); Gfx_Box(cx-2.2f,3.3f,cz-0.05f,cx+2.2f,4.2f,cz+0.05f);
    Gfx_TextFlat(cx,3.55f,cz+0.07f,0,0.4f,0.6f,0.85f,1.0f,"BAGGAGE X-RAY");
    for(i=0;i<3;i++){
        float bp=fmodf(t*0.18f + i/3.0f, 1.0f);
        float bx=cx-4.8f + bp*9.6f;
        Gfx_Material(BAGCOL[i][0],BAGCOL[i][1],BAGCOL[i][2],25.0f);
        Gfx_Box(bx-0.4f,0.82f,cz-0.28f,bx+0.4f,1.2f,cz+0.28f);
    }
    /* handler at the in-feed end, loading bags */
    People_SetGesture(1);
    People_Draw(cx-5.8f,cz-0.2f,90, t*2.0f, 0.0f, 4, 0);
}

/* small in-world speech-bubble marker floating above the agent you're talking to;
   the readable dialogue text itself is drawn by the HUD (Story_Panel* queries). */
static void drawStationPanel(void)
{
    int st=Story_PanelState(); if(!st) return;
    { float px,py,pz,yaw,pulse=0.5f+0.3f*sinf(g_airport.time*4.0f);
      Story_PanelPos(&px,&py,&pz);
      yaw=atan2f(g_camera.x-px, g_camera.z-pz)*57.2957795f;
      glDisable(GL_LIGHTING); glDisable(GL_DEPTH_TEST);
      glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      glPushMatrix(); glTranslatef(px,py+0.4f,pz); glRotatef(yaw,0,1,0);
        glColor4f(0.05f,0.5f,0.75f,0.85f);
        glBegin(GL_QUADS); glVertex3f(-0.7f,-0.5f,0);glVertex3f(0.7f,-0.5f,0);glVertex3f(0.7f,0.5f,0);glVertex3f(-0.7f,0.5f,0); glEnd();
        glColor4f(0.4f,0.85f,1.0f,pulse+0.3f); glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP); glVertex3f(-0.7f,-0.5f,0.01f);glVertex3f(0.7f,-0.5f,0.01f);glVertex3f(0.7f,0.5f,0.01f);glVertex3f(-0.7f,0.5f,0.01f); glEnd();
        glColor4f(0.05f,0.5f,0.75f,0.85f);
        glBegin(GL_TRIANGLES); glVertex3f(-0.25f,-0.5f,0);glVertex3f(0.25f,-0.5f,0);glVertex3f(0,-0.95f,0); glEnd();
        /* three chat dots */
        glColor4f(0.95f,0.98f,1.0f,0.95f); glPointSize(4.0f);
        glBegin(GL_POINTS); glVertex3f(-0.28f,0,0.02f);glVertex3f(0,0,0.02f);glVertex3f(0.28f,0,0.02f); glEnd();
      glPopMatrix();
      glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
    }
}

static void drawInterior(void)
{
    int i;
    static const int CX[6]={-50,-30,-10,10,30,50};
    /* floor */
    Gfx_TexturedFloor(TERMINAL_X0+1.5f,TERMINAL_Z0,TERMINAL_X1-1.5f,TERMINAL_Z1,0.05f,TEX_TILE,20);
    /* lounge carpet near seating */
    Gfx_TexturedFloor(-58,-26,-6,-8,0.06f,TEX_CARPET,8);
    Gfx_TexturedFloor(6,-26,58,-8,0.06f,TEX_CARPET,8);

    for(i=0;i<6;i++){ checkinCounter((float)CX[i], i+1, 1); }
    for(i=0;i<6;i++){ if(CX[i]==-10) queueAtPlayer((float)CX[i]); else queueAt((float)CX[i], (float)i); }
    Gfx_TextFlat(0,5.4f,20.0f,0,1.0f,0.2f,0.5f,0.8f,"CHECK-IN");
    baggageDrop(26,13.0f);

    for(i=-2;i<=2;i++) securityArch(i*6.0f);
    xrayBelt(-16,3.0f); xrayBelt(16,3.0f);
    People_Draw(9,0.6f,270,0,0,4,0);          /* security officers (to the side) */
    People_Draw(-9,0.6f,270,0,0,4,0);
    { float t=g_airport.time; int k;
      for(k=0;k<5;k++){ float lane=(k-2)*6.0f;
        float p=fmodf(t*0.12f + k*0.31f, 1.0f);      /* travels from +z through the gate to -z */
        float z=8.0f - p*14.0f;                       /* 8 (approach) -> -6 (past the gate) */
        People_Draw(lane, z, 90, t*3.5f+k, 1.0f, k%6, (k%2)?3:0); } }
    Gfx_TextFlat(0,4.2f,4.5f,0,0.7f,0.85f,0.3f,0.15f,"SECURITY");

    /* departure lounge seating + plants */
    seatRow(-54,-14,7); seatRow(-54,-20,7);
    seatRow(10,-14,7);  seatRow(10,-20,7);
    plant(-60,-10); plant(-2,-10); plant(60,-10); plant(4,-24); plant(-6,-24);

    /* FIDS + signage */
    fidsBoard();
    hangingSign(-40,6,"DEPARTURES",0.3f,0.8f,1.0f);
    hangingSign(40,6,"ARRIVALS",0.4f,1.0f,0.6f);
    hangingSign(-40,-16,"BOARDING GATES 1-6",1.0f,0.85f,0.3f);
    hangingSign(40,-16,"BAGGAGE CLAIM",0.9f,0.9f,0.95f);

    for(i=0;i<4;i++){ float gx=-45+i*30.0f;
        char s[10]; Gfx_Material(0.18f,0.36f,0.55f,30.0f);
        Gfx_Box(gx-2,0,TERMINAL_Z0+2,gx+2,1.1f,TERMINAL_Z0+3);
        Gfx_Material(0.06f,0.1f,0.16f,20.0f);
        Gfx_Box(gx-1.2f,3.4f,TERMINAL_Z0+2.5f,gx+1.2f,4.2f,TERMINAL_Z0+2.6f);
        sprintf(s,"GATE %d",i+1);
        Gfx_TextFlat(gx,3.65f,TERMINAL_Z0+2.63f,0,0.4f,1.0f,0.85f,0.2f,s);
        People_Draw(gx,TERMINAL_Z0+3.5f,270,0,0,3,0); }        /* gate staff */
    Gfx_Material(0.1f,0.4f,0.2f,20.0f); Gfx_Box(38,0,TERMINAL_Z0+2.4f,42,0.15f,TERMINAL_Z0+5.5f);
    Gfx_TextFlat(40,3.9f,TERMINAL_Z0+2.7f,0,0.4f,0.4f,1.0f,0.6f,"BOARDING");
    People_Draw(40,TERMINAL_Z0+3.5f,270,0,0,1,0);              /* cabin crew */

    /* baggage carousel */
    baggageCarousel(-40,-20);
}

/* dynamic actors */
static void drawParkingCars(void)
{
    int i; int night=!g_airport.dayMode;
    float col[6][3]={{0.75f,0.15f,0.15f},{0.15f,0.2f,0.55f},{0.85f,0.85f,0.88f},
                     {0.1f,0.1f,0.12f},{0.6f,0.6f,0.15f},{0.2f,0.5f,0.3f}};
    for(i=0;i<8;i++){ float x=-77+i*11.0f;
        Veh_Car(x,PARK_Z0+8, 90,col[i%6][0],col[i%6][1],col[i%6][2],0,night); }   /* nose -z */
    for(i=0;i<8;i++){ float x=-77+i*11.0f;
        Veh_Car(x,PARK_Z0+30,270,col[(i+3)%6][0],col[(i+3)%6][1],col[(i+3)%6][2],0,night); } /* nose +z */
    /* a few CNGs parked in the mix */
    for(i=0;i<4;i++) Veh_CNG(-60+i*34.0f,PARK_Z0+19,90,0,night);
}

static void drawTrafficAndService(void)
{
    int night=!g_airport.dayMode; float t=g_airport.time;
    float spin=fmodf(t*300.0f,360.0f);
    Veh_Taxi(-110+fmodf(t*14.0f,230.0f), ROAD_Z+2, 0,  spin,night);   /* east-bound */
    Veh_Bus (-110+fmodf(t*9.0f+120,230.0f), ROAD_Z+3, 0, spin,night);
    Veh_CNG (-110+fmodf(t*12.0f+60,230.0f), ROAD_Z+1, 0, spin,night);
    Veh_Car ( 110-fmodf(t*11.0f,230.0f), ROAD_Z-2, 180, 0.2f,0.45f,0.7f,spin,night); /* west-bound */
    Veh_CNG ( 110-fmodf(t*13.0f+90,230.0f), ROAD_Z-1, 180, spin,night);
    Veh_Car(-20,ROAD_Z-1,180,0.7f,0.1f,0.12f,0,night);
    Veh_CNG(  6,ROAD_Z-1,180,0,night);
    Veh_Car( 34,ROAD_Z-1,180,0.15f,0.18f,0.5f,0,night);
    People_Draw(-17,ROAD_Z+1.2f, 90, t*4,   1.0f, 1, 1);   /* stepping out, walking to terminal */
    People_Draw(  9,ROAD_Z+1.2f, 90, t*4+2, 1.0f, 3, 2);
    People_Draw( 37,ROAD_Z+1.2f, 90, t*4+1, 1.0f, 5, 1);
    /* apron service */
    Veh_BaggageTug(-90+fmodf(t*8.0f,180.0f),-64,0,spin,night);
    Veh_FuelTruck(GATE_LN_X+8,-58,180,0,night);
    if(g_airport.pbActive)
        Veh_Pushback(g_airport.pbX,g_airport.pbZ,270,spin*0.4f,night);
}

static void drawPeople(void)
{
    int i; float t=g_airport.time;
    for(i=0;i<9;i++){ float ph=t*4.5f+i;
        float prog=fmodf(t*0.05f+i*0.11f,1.0f);
        float x=-34+(i%5)*15.0f;
        float z=mixf(PARK_Z0+4,TERMINAL_Z1+2,prog);
        People_Draw(x,z,90,ph,1.0f,i%6,(i%3)); }
    for(i=0;i<7;i++){ float ph=t*4.5f+i;
        float prog=fmodf(t*0.045f+i*0.14f,1.0f);
        float x=24+(i%4)*14.0f;
        float z=mixf(TERMINAL_Z1+2,PARK_Z1-6,prog);
        People_Draw(x,z,270,ph,1.0f,(i+2)%6,(i%2)); }
    for(i=0;i<6;i++){ People_Draw(-70+i*26.0f,PARK_Z1-16,270,0,0,i%6,1); }
    /* seated in lounge (facing the windows/-z) */
    for(i=0;i<7;i++){ People_Draw(-53.5f+i*1.3f,-13.4f,90,0,0,i%6,0); }
    for(i=0;i<7;i++){ People_Draw(10.5f+i*1.3f,-13.4f,90,0,0,(i+1)%6,0); }
    /* walking to the gates (-z) */
    for(i=0;i<6;i++){ float prog=fmodf(t*0.07f+i*0.16f,1.0f);
        People_Draw(-45+i*18.0f, mixf(-6,TERMINAL_Z0+4,prog), 90, t*4.5f+i, 1.0f, i%6, 1); }
    /* apron ground staff (hi-vis) around the jets */
    for(i=0;i<4;i++){ People_Draw(GATE_TO_X-6+i*3.0f,-58, 0, t*4+i, 0.5f, 4, 0); }
    for(i=0;i<3;i++){ People_Draw(GATE_LN_X-4+i*3.0f,-58, 0, t*4+i, 0.5f, 4, 0); }
}

#define CAB_X 380.0f
#define CAB_Z 380.0f
/* a passenger seated in a cabin seat at column sx, row-centre z, facing forward (+x):
   torso against the backrest, thighs on the cushion, shins down, arms on the rests, head up. */
static void seatedPerson(float sx,float z,int v)
{
    static const float SH[6][3]={{0.80f,0.22f,0.24f},{0.16f,0.36f,0.66f},{0.22f,0.55f,0.36f},
                                 {0.78f,0.78f,0.80f},{0.85f,0.62f,0.20f},{0.55f,0.28f,0.62f}};
    static const float SK[3][3]={{0.80f,0.62f,0.46f},{0.66f,0.48f,0.34f},{0.90f,0.74f,0.60f}};
    const float *sh=SH[v%6], *sk=SK[(v+1)%3];
    Gfx_Material(0.18f,0.20f,0.28f,8.0f);
    Gfx_Box(sx+0.16f,1.70f,z-0.17f,sx+0.60f,1.84f,z-0.02f);
    Gfx_Box(sx+0.16f,1.70f,z+0.02f,sx+0.60f,1.84f,z+0.17f);
    Gfx_Box(sx+0.48f,1.50f,z-0.17f,sx+0.62f,1.72f,z-0.02f);
    Gfx_Box(sx+0.48f,1.50f,z+0.02f,sx+0.62f,1.72f,z+0.17f);
    Gfx_Material(sh[0],sh[1],sh[2],12.0f);
    Gfx_Box(sx+0.05f,1.86f,z-0.2f,sx+0.30f,2.52f,z+0.2f);
    /* shoulders + arms down the sides */
    Gfx_Box(sx+0.04f,2.44f,z-0.24f,sx+0.32f,2.56f,z+0.24f);
    Gfx_Box(sx+0.06f,1.95f,z-0.26f,sx+0.22f,2.46f,z-0.19f);
    Gfx_Box(sx+0.06f,1.95f,z+0.19f,sx+0.22f,2.46f,z+0.26f);
    /* head + hair (well above the seat back) */
    Gfx_Material(sk[0],sk[1],sk[2],12.0f);
    glPushMatrix(); glTranslatef(sx+0.2f,2.72f,z); glutSolidSphere(0.15f,9,7); glPopMatrix();
    Gfx_Material(0.05f,0.04f,0.03f,6.0f);
    glPushMatrix(); glTranslatef(sx+0.17f,2.77f,z-0.0f); glutSolidSphere(0.155f,9,7); glPopMatrix();
    Gfx_Material(sk[0],sk[1],sk[2],12.0f);
    glPushMatrix(); glTranslatef(sx+0.3f,2.72f,z); glScalef(0.6f,1,1); glutSolidSphere(0.12f,7,6); glPopMatrix();
}

static void drawCabin(void)
{
    int i,s; float t=g_airport.time;
    float prog=g_story.t/6.0f; if(prog<0)prog=0; if(prog>1)prog=1;
    static const float ZC[4]={-1.6f,-0.92f,0.92f,1.6f};
    glPushMatrix(); glTranslatef(CAB_X,0,CAB_Z);
    Gfx_Material(0.12f,0.16f,0.34f,8.0f);  Gfx_Box(-3,1.4f,-2.3f,20,1.5f,2.3f);
    Gfx_Material(0.86f,0.87f,0.90f,10.0f); Gfx_Box(-3,3.75f,-2.4f,20,3.95f,2.4f);
    Gfx_Box(-3,3.2f,-2.45f,20,3.75f,-2.1f); Gfx_Box(-3,3.2f,2.1f,20,3.75f,2.45f);
    Gfx_Emissive(0.6f,0.6f,0.52f); Gfx_Box(-3,3.73f,-0.45f,20,3.75f,0.45f); Gfx_EmissiveOff();
    /* light side walls */
    Gfx_Material(0.88f,0.89f,0.92f,10.0f);
    Gfx_Box(-3,1.5f,-2.4f,20,3.2f,-2.3f); Gfx_Box(-3,1.5f,2.3f,20,3.2f,2.4f);
    for(i=0;i<9;i++){ float wx=-1.0f+i*2.2f;
        Gfx_Material(0.93f,0.94f,0.96f,20.0f);
        Gfx_Box(wx,2.15f,-2.43f,wx+0.8f,3.0f,-2.36f);
        Gfx_Box(wx,2.15f, 2.36f,wx+0.8f,3.0f, 2.43f);
        Gfx_Emissive(0.55f,0.75f,1.0f);
        Gfx_Box(wx+0.1f,2.25f,-2.45f,wx+0.7f,2.9f,-2.4f);
        Gfx_Box(wx+0.1f,2.25f, 2.4f, wx+0.7f,2.9f, 2.45f); Gfx_EmissiveOff();
    }
    /* overhead bins */
    Gfx_Material(0.82f,0.83f,0.86f,10.0f);
    Gfx_Box(-3,2.95f,-2.3f,20,3.5f,-1.45f); Gfx_Box(-3,2.95f,1.45f,20,3.5f,2.3f);
    for(i=0;i<6;i++){ float sx=1.0f+i*2.7f;
        for(s=0;s<4;s++){ float z=ZC[s];
            Gfx_Material(0.14f,0.24f,0.48f,20.0f);
            Gfx_Box(sx,1.5f,z-0.3f,sx+0.62f,1.86f,z+0.3f);        /* cushion */
            Gfx_Box(sx,1.86f,z-0.3f,sx+0.13f,2.28f,z+0.3f);       /* backrest (lower, so heads show) */
            Gfx_Material(0.10f,0.16f,0.34f,20.0f);
            Gfx_Box(sx-0.02f,2.28f,z-0.28f,sx+0.15f,2.48f,z+0.28f);/* headrest */
            Gfx_Material(0.22f,0.24f,0.28f,20.0f);
            Gfx_Box(sx+0.1f,1.82f,z+0.28f,sx+0.55f,1.96f,z+0.33f);/* armrest */
            if((i*4+s)%7 != 3)                                     /* almost every seat taken */
                seatedPerson(sx, z, i*4+s);
        }
    }
    { float hx=17.0f-prog*13.5f, tx=hx-1.15f;
      Gfx_Metal(0.72f,0.74f,0.78f); Gfx_Box(tx-0.35f,1.5f,-0.45f,tx+0.35f,2.3f,0.45f);  /* cart */
      Gfx_Material(0.45f,0.47f,0.5f,20.0f); Gfx_Box(tx-0.4f,2.3f,-0.5f,tx+0.4f,2.38f,0.5f); /* tray top */
      Gfx_Material(0.90f,0.85f,0.5f,12.0f); Gfx_Box(tx-0.28f,2.38f,-0.32f,tx+0.02f,2.46f,-0.02f); /* meal tray */
      Gfx_Material(0.85f,0.2f,0.15f,12.0f); Gfx_Box(tx-0.22f,2.38f,0.06f,tx+0.0f,2.5f,0.26f);      /* soda can */
      Gfx_Material(0.96f,0.96f,0.98f,12.0f);Gfx_Box(tx+0.12f,2.38f,-0.12f,tx+0.3f,2.52f,0.12f);    /* cup */
      Gfx_Metal(0.75f,0.77f,0.8f); Gfx_Box(tx-0.05f,2.3f,-0.42f,tx+0.05f,2.75f,-0.32f);            /* handle */
      glPushMatrix(); glTranslatef(0,1.5f,0);
        People_SetGesture(1); People_Draw(hx,0.0f,180, t*4.5f, 0.85f, 5, 0);   /* hostess (uniform), walking */
      glPopMatrix();
    }
    glPopMatrix();
}

static void drawVillage(void)
{
    int i; float t=g_airport.time;
    float prog=g_story.t/5.5f; if(prog>1) prog=1;
    float vx=VILLAGE_X, vz=VILLAGE_Z;
    /* green field + dirt road */
    Gfx_TexturedFloor(vx-70,vz-70,vx+70,vz+70,0.0f,TEX_GRASS,34);
    glDisable(GL_LIGHTING); glColor3f(0.46f,0.36f,0.24f);
    glBegin(GL_QUADS); glVertex3f(vx-3.5f,0.02f,vz-70);glVertex3f(vx+3.5f,0.02f,vz-70);
        glVertex3f(vx+3.5f,0.02f,vz+70);glVertex3f(vx-3.5f,0.02f,vz+70); glEnd();
    glEnable(GL_LIGHTING);
    /* tin-roof house */
    Gfx_Material(0.72f,0.60f,0.42f,10.0f); Gfx_Box(vx-16,0,vz-9, vx-9,3.0f,vz-2);
    Gfx_Metal(0.55f,0.57f,0.60f);          Gfx_Box(vx-16.7f,3.0f,vz-9.7f, vx-8.3f,3.4f,vz-1.3f);
    Gfx_Material(0.18f,0.13f,0.10f,8.0f);  Gfx_Box(vx-13.5f,0,vz-2.05f, vx-11.5f,2.0f,vz-1.95f);
    /* trees + a parked CNG */
    drawTree(vx+13,vz-7,1.5f); drawTree(vx-20,vz+7,1.3f); drawTree(vx+18,vz+12,1.4f);
    Veh_CNG(vx+4,vz+6,0,0,!g_airport.dayMode);
    /* villagers looking up, pointing at the plane */
    for(i=0;i<6;i++){ float px=vx-7.5f+i*3.0f, pz=vz+12+(i%2)*1.6f;
        People_SetGesture(2); People_Draw(px,pz,90, t*3.0f+i, 0.0f, i%6, (i%3)); }
    /* the departing jet crossing the sky in a straight line, nose pointing the way it flies
       (moves along +x at a fixed distance, so it never crabs/slides sideways) */
    { float px=vx-120.0f+prog*240.0f, py=52.0f+prog*26.0f, pz=vz-48.0f;
      Aircraft_Draw(px,py,pz, 0, 6, 0, 0, 0, t, 1); }   /* yaw 0 = nose +x = travel direction */
}

/* top-level draw */
void Airport_Draw(void)
{
    if(Story_InCabin()){ drawSky(); drawCabin(); return; }
    if(Story_InGround()){ drawSky(); drawVillage(); return; }
    drawSky();
    drawGround();
    drawTree(-90,PARK_Z1-6,1.3f); drawTree(90,PARK_Z1-6,1.3f);
    drawTree(-90,PARK_Z0+6,1.1f); drawTree(90,PARK_Z0+6,1.1f);
    drawTree(-105,40,1.4f); drawTree(105,40,1.4f);
    drawTower();
    drawTowerInterior();
    drawFlag(-30,PARK_Z1-30); drawFlag(30,PARK_Z1-30);   /* Bangladesh flags at the forecourt */
    drawTerminalOpaque();
    drawInterior();
    jetBridge(GATE_TO_X); jetBridge(GATE_LN_X);
    drawParkingCars();
    drawTrafficAndService();
    drawPeople();

    /* boarding stairs at the apron-parked jets */
    { int night=!g_airport.dayMode;
      Veh_Stairs(-110,-52,90,night); Veh_Stairs(110,-52,90,night); }

    { float wx,wz; int has; Story_Waypoint(&wx,&wz,&has);
      if(has){ int k; float t=g_airport.time;
        glDisable(GL_LIGHTING); glDepthMask(GL_FALSE);
        glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE);
        for(k=0;k<3;k++){ float a=0.5f-k*0.14f;
            glColor4f(0.2f,1.0f,0.5f,a);
            glPushMatrix(); glTranslatef(wx,0,wz);
            gluCylinder(Gfx_Quadric(),0.5f-k*0.12f,0.02f,7.0f,10,1); glPopMatrix(); }
        /* spinning ring */
        glColor4f(0.3f,1.0f,0.6f,0.9f);
        glPushMatrix(); glTranslatef(wx,0.4f+0.3f*sinf(t*3),wz); glRotatef(t*120,0,1,0);
        { int s; glBegin(GL_LINE_LOOP); for(s=0;s<24;s++){ float aa=s*(6.2831f/24);
            glVertex3f(cosf(aa)*1.1f,0,sinf(aa)*1.1f);} glEnd(); }
        glPopMatrix();
        glDisable(GL_BLEND); glDepthMask(GL_TRUE); glEnable(GL_LIGHTING);
      }
    }

    Aircraft_Draw(g_airport.toX,g_airport.toY,g_airport.toZ,g_airport.toYaw,g_airport.toPitch,0,
                  0,g_airport.toGear,g_airport.time,g_airport.toEng);
    /* draw the arriving jet, EXCEPT while a departure is on/over the runway - so there is
       never a plane coming head-on at the departing aircraft. */
    if(!(g_airport.toPhase>=TO_ROLL && g_airport.toPhase<=TO_DEPART))
        Aircraft_Draw(g_airport.lnX,g_airport.lnY,g_airport.lnZ,g_airport.lnYaw,g_airport.lnPitch,0,
                      1,g_airport.lnGear,g_airport.time,g_airport.lnEng);
    Aircraft_Draw(-110,AC_GROUND_CL,-56,270,0,0,1,1,g_airport.time,0);
    Aircraft_Draw(110,AC_GROUND_CL,-56,270,0,0,0,1,g_airport.time,0);
    if(g_airport.flyActive && g_airport.flyY>0)
        Aircraft_Draw(g_airport.flyX,g_airport.flyY,g_airport.flyZ,
                      g_airport.flyYaw,g_airport.flyPitch,0,0,0,g_airport.time,1);

    drawTerminalGlass();      /* transparent last */
    drawStationPanel();       /* in-world dialogue panel on top */
}
