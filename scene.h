#ifndef AIRPORT_SCENE_H
#define AIRPORT_SCENE_H

typedef struct {
    float time;        /* seconds since launch */
    float simClock;    /* simulated clock, seconds (for HH:MM display) */
    int   dayMode;     /* 1 = day, 0 = night */
    int   cameraMode;  /* 0 first-person, 1 runway, 2 aircraft-follow, 3 tower */
    float doorOpen;    /* 0..1 sliding entrance doors */

    /* takeoff aircraft (green livery) */
    int   toPhase; float toT;
    float toX,toY,toZ,toYaw,toPitch; int toGear,toEng;
    /* landing aircraft (blue livery) */
    int   lnPhase; float lnT;
    float lnX,lnY,lnZ,lnYaw,lnPitch; int lnGear,lnEng;

    float pbX,pbZ,pbYaw; int pbActive;

    float flyT; int flyActive;
    float flyX,flyY,flyZ,flyYaw,flyPitch;

    int   rwyOwner;      /* 0 none, 1 takeoff, 2 landing - prevents runway collision */
    int   holdTakeoff;   /* story keeps the departing jet at its gate until boarding */
} AirportState;

extern AirportState g_airport;

void  Airport_Init(void);
void  Airport_Update(float dt);
void  Airport_Draw(void);
void  Airport_ToggleDay(void);
void  Airport_RestartTakeoff(void);
void  Airport_RestartLanding(void);
void  Airport_BeginDeparture(void);   /* jump the departing jet to runway line-up (cutscene) */

const char *Airport_AreaName(void);

#define VILLAGE_X (-400.0f)
#define VILLAGE_Z ( 400.0f)
const char *Airport_TakeoffStatus(void);
const char *Airport_LandingStatus(void);
void        Airport_ClockStr(char *buf,int n);   /* "HH:MM" */

#endif
