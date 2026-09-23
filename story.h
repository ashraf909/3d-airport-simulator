#ifndef AIRPORT_STORY_H
#define AIRPORT_STORY_H

/* Physical, station-based passenger journey. You must walk to each station and press E;
   the conversation is shown on an in-world panel at the counter, not as a subtitle. */
enum {
    ST_INTRO=0, ST_CHECKIN, ST_BAGGAGE, ST_SECURITY, ST_GATE, ST_BOARD,
    ST_CABIN, ST_DEPART, ST_GROUND, ST_FLAG, ST_DONE
};

typedef struct {
    int   active;
    int   step;      /* ST_* */
    int   sub;       /* -1 = not talking yet (need to press E); >=0 = line index */
    float t;         /* cut-scene timer */
    float flash;     /* nudge message seconds */
    const char *flashMsg; /* what the nudge says */
    float doneFlash;      /* "task complete" banner timer */
    const char *doneMsg;  /* text for the banner */
    int   doneSound;      /* set to 1 to request a chime (main plays + clears) */
} Story;
extern Story g_story;

void Story_Init(void);
void Story_Restart(void);
void Story_Update(float dt);

/* input */
void Story_Interact(void);   /* E / Enter */
void Story_Yes(void);        /* Y */
void Story_No(void);         /* N */

/* HUD (full-screen bits) */
int         Story_InIntro(void);
int         Story_InCabin(void);     /* seated in the plane interior scene */
int         Story_InGround(void);    /* ground village scene during takeoff */
int         Story_Complete(void);
int         Story_ShowFlag(void);
const char *Story_DoneMsg(void);     /* "task complete" banner text, or NULL */
const char *Story_Objective(void);   /* top bar; NULL when none */
const char *Story_Flash(void);       /* NULL when none */
const char *Story_IntroLine(void);

/* guidance beacon (player target) */
void Story_Waypoint(float *x,float *z,int *has);

int         Story_PanelState(void);              /* 0 none, 1 "press E" hint, 2 dialogue */
void        Story_PanelPos(float *x,float *y,float *z);
const char *Story_PanelTitle(void);              /* station / speaker name */
const char *Story_PanelLine(void);               /* what the agent says */
const char *Story_PanelPrompt(void);             /* "[E] ...", "[Y] Yes  [N] No" */

#endif
