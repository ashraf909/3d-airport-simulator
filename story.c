#include <math.h>
#include <stddef.h>
#include "story.h"
#include "scene.h"
#include "camera.h"
#include "layout.h"
#include "aircraft.h"

Story g_story;

typedef struct { float sx,sz, ax,az; const char *name; const char *go; } Station;
static const Station STN[] = {
    /* ST_INTRO (unused) */ { 0,0, 0,0, "", "" },
    /* ST_CHECKIN */        { -10,17.4f, -10,12.5f, "Check-in Counter 3", "Go to Check-in Counter 3" },
    /* ST_BAGGAGE */        {  26,17.4f,  26,13.2f, "Baggage Drop",       "Go to the Baggage Drop belt" },
    /* ST_SECURITY */       {   0, 3.8f,   0, 0.6f, "Security",           "Go through Security screening" },
    /* ST_GATE */           {  15,-24.0f, 15,-28.5f,"Boarding Gate 3",    "Walk to Boarding Gate 3" },
    /* ST_BOARD */          {  40,-24.0f, 40,-28.5f,"Boarding",           "Board your flight to London" },
};

typedef struct { const char *sp; const char *ln; int yn; } Dline; /* yn: 1 = Y/N question */

static const Dline D_CHECKIN[] = {
    {"Check-in Agent","Passport, please?",1},
    {"Check-in Agent","Boarding pass - Gate 3.",0},
};
static const Dline D_BAGGAGE[] = {
    {"Baggage Handler","Bag on the belt?",1},
    {"Baggage Handler","Tagged for LONDON.",0},
};
static const Dline D_SECURITY[] = {
    {"Security Officer","Walk through?",1},
    {"Security Officer","All clear - Gate 3.",0},
};
static const Dline D_GATE[] = {
    {"Gate Staff","Boarding pass?",1},
    {"Gate Staff","Welcome aboard!",0},
};
static const Dline D_BOARD[] = {
    {"Cabin Crew","Ready to depart?",1},
};

static const Dline *dlg(int step,int *len)
{
    switch(step){
        case ST_CHECKIN: *len=(int)(sizeof D_CHECKIN /sizeof D_CHECKIN[0]);  return D_CHECKIN;
        case ST_BAGGAGE: *len=(int)(sizeof D_BAGGAGE /sizeof D_BAGGAGE[0]);  return D_BAGGAGE;
        case ST_SECURITY:*len=(int)(sizeof D_SECURITY/sizeof D_SECURITY[0]); return D_SECURITY;
        case ST_GATE:    *len=(int)(sizeof D_GATE    /sizeof D_GATE[0]);     return D_GATE;
        case ST_BOARD:   *len=(int)(sizeof D_BOARD   /sizeof D_BOARD[0]);    return D_BOARD;
        default: *len=0; return NULL;
    }
}
static int isStation(int step){ return step>=ST_CHECKIN && step<=ST_BOARD; }

void Story_Init(void)
{ g_story.active=1; g_story.step=ST_INTRO; g_story.sub=-1; g_story.t=0; g_story.flash=0;
  g_story.doneFlash=0; g_story.doneMsg=NULL; g_story.doneSound=0; }

void Story_Restart(void)
{
    Story_Init();
    g_airport.holdTakeoff=1;
    g_airport.flyActive=1; g_airport.flyT=0;
    Airport_RestartTakeoff(); Airport_RestartLanding();
    g_airport.cameraMode=0;
    Camera_Reset();
}

void Story_Waypoint(float *x,float *z,int *has)
{
    if(isStation(g_story.step)){ *x=STN[g_story.step].sx; *z=STN[g_story.step].sz; *has=1; }
    else *has=0;
}

static int inRange(void)
{
    float dx,dz;
    if(!isStation(g_story.step)) return 0;
    dx=g_camera.x-STN[g_story.step].sx; dz=g_camera.z-STN[g_story.step].sz;
    return (dx*dx+dz*dz) < 4.5f*4.5f;
}

static void advance(void)
{
    int len; dlg(g_story.step,&len);
    g_story.sub++;
    if(g_story.sub>=len){
        switch(g_story.step){
            case ST_CHECKIN:  g_story.doneMsg="CHECK-IN COMPLETE";      break;
            case ST_BAGGAGE:  g_story.doneMsg="BAGGAGE CHECKED IN";     break;
            case ST_SECURITY: g_story.doneMsg="SECURITY CLEARED";       break;
            case ST_GATE:     g_story.doneMsg="BOARDING PASS ACCEPTED"; break;
            case ST_BOARD:    g_story.doneMsg="BOARDING";               break;
            default:          g_story.doneMsg=NULL;                     break;
        }
        if(g_story.doneMsg){ g_story.doneFlash=2.4f; g_story.doneSound=1; }
        if(g_story.step==ST_BOARD){
            g_story.step=ST_CABIN; g_story.t=0;   /* board -> plane interior scene */
        } else {
            g_story.step++; g_story.sub=-1;
        }
    }
}

void Story_Update(float dt)
{
    if(!g_story.active) return;
    if(g_story.flash>0) g_story.flash-=dt;
    if(g_story.doneFlash>0) g_story.doneFlash-=dt;
    g_airport.holdTakeoff = (g_story.step < ST_DEPART) ? 1 : 0;

    switch(g_story.step){
    case ST_CABIN:                    /* seated in the cabin for a few seconds, then depart */
        g_story.t+=dt;
        if(g_story.t>6.0f){
            g_story.step=ST_DEPART; g_story.t=0;
            Airport_BeginDeparture(); g_airport.cameraMode=2;
        }
        break;
    case ST_DEPART:
        g_story.t+=dt; g_airport.cameraMode=2;
        if(g_airport.toY > AC_GROUND_CL+6.0f || g_story.t>26.0f){ g_story.step=ST_GROUND; g_story.t=0; }
        break;
    case ST_GROUND:                   /* cut to the village: people watch the plane climb over */
        g_story.t+=dt;
        if(g_story.t>5.5f){ g_story.step=ST_FLAG; g_story.t=0; g_airport.cameraMode=2; }
        break;
    case ST_FLAG:
        g_story.t+=dt; g_airport.cameraMode=2;
        if(g_story.t>5.0f){ g_story.step=ST_DONE; g_story.t=0; }
        break;
    default: break;
    }
}

void Story_Interact(void)   /* E or Enter */
{
    if(!g_story.active) return;
    if(g_story.step==ST_INTRO){ g_story.step=ST_CHECKIN; g_story.sub=-1; return; }
    if(!isStation(g_story.step)) return;
    if(!inRange()){ g_story.flash=2.0f; return; }     /* must be at the station */
    if(g_story.sub<0){ g_story.sub=0; return; }        /* start talking */
    {
        int len; const Dline *d=dlg(g_story.step,&len);
        if(d && d[g_story.sub].yn) { advance(); return; }  /* E also accepts a question */
        advance();
    }
}

void Story_Yes(void)
{
    int len; const Dline *d;
    if(!g_story.active) return;
    if(g_story.step==ST_INTRO){ g_story.step=ST_CHECKIN; g_story.sub=-1; return; }
    if(!isStation(g_story.step) || !inRange() || g_story.sub<0) return;
    d=dlg(g_story.step,&len);
    if(d && d[g_story.sub].yn) advance();
}
void Story_No(void)
{
    int len; const Dline *d;
    if(!g_story.active || !isStation(g_story.step) || !inRange() || g_story.sub<0) return;
    d=dlg(g_story.step,&len);
    if(d && d[g_story.sub].yn) g_story.flash=2.5f;
}

/* HUD queries */
int Story_InIntro(void){ return g_story.active && g_story.step==ST_INTRO; }
int Story_InCabin(void){ return g_story.active && g_story.step==ST_CABIN; }
int Story_InGround(void){ return g_story.active && g_story.step==ST_GROUND; }
const char *Story_DoneMsg(void){ return (g_story.doneFlash>0)? g_story.doneMsg : NULL; }
int Story_Complete(void){ return g_story.active && g_story.step==ST_DONE; }
int Story_ShowFlag(void){ return g_story.active && g_story.step==ST_FLAG; }
const char *Story_IntroLine(void)
{ return "Welcome! You are flying BG 201 to London. Walk to each marker and press E."; }

const char *Story_Objective(void)
{
    if(g_story.step==ST_DEPART) return "Departing Dhaka...";
    if(isStation(g_story.step)) return STN[g_story.step].go;
    return NULL;
}
const char *Story_Flash(void)
{ return (g_story.flash>0)? "Walk up to the counter and press E." : NULL; }

/* in-world panel */
int Story_PanelState(void)
{
    if(!g_story.active || !isStation(g_story.step) || !inRange()) return 0;
    return (g_story.sub<0)? 1 : 2;
}
void Story_PanelPos(float *x,float *y,float *z)
{ *x=STN[g_story.step].ax; *y=3.2f; *z=STN[g_story.step].az; }
const char *Story_PanelTitle(void)
{
    int len; const Dline *d;
    if(g_story.sub<0) return STN[g_story.step].name;
    d=dlg(g_story.step,&len); return d? d[g_story.sub].sp : STN[g_story.step].name;
}
const char *Story_PanelLine(void)
{
    int len; const Dline *d;
    if(g_story.sub<0) return "";
    d=dlg(g_story.step,&len); return d? d[g_story.sub].ln : "";
}
const char *Story_PanelPrompt(void)
{
    int len; const Dline *d;
    if(g_story.sub<0) return "[E] Talk to the agent";
    d=dlg(g_story.step,&len);
    if(d && d[g_story.sub].yn) return "[Y] Yes    [N] No";
    return "[E] Continue";
}
