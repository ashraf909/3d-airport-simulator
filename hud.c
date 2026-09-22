#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "hud.h"
#include "scene.h"
#include "camera.h"
#include "story.h"

static void text(float x,float y,void *font,const char *s)
{
    const char *p; glRasterPos2f(x,y);
    for(p=s;*p;p++) glutBitmapCharacter(font,*p);
}
static float textW(void *font,const char *s)
{
    float w=0; const char *p; for(p=s;*p;p++) w+=glutBitmapWidth(font,*p); return w;
}
static void ctext(float cx,float y,void *font,const char *s)
{ text(cx-textW(font,s)*0.5f,y,font,s); }

static void rect(float x0,float y0,float x1,float y1,float r,float g,float b,float a)
{
    glColor4f(r,g,b,a);
    glBegin(GL_QUADS); glVertex2f(x0,y0);glVertex2f(x1,y0);glVertex2f(x1,y1);glVertex2f(x0,y1); glEnd();
}
static void frame(float x0,float y0,float x1,float y1,float r,float g,float b)
{
    glColor4f(r,g,b,0.7f); glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP); glVertex2f(x0,y0);glVertex2f(x1,y0);glVertex2f(x1,y1);glVertex2f(x0,y1); glEnd();
}
static void disc(float cx,float cy,float rad,float r,float g,float b,float a)
{
    int i; glColor4f(r,g,b,a);
    glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(i=0;i<=28;i++){ float t=i*(6.2831853f/28.0f); glVertex2f(cx+cosf(t)*rad,cy+sinf(t)*rad); }
    glEnd();
}

static void panel(float x0,float y0,float x1,float y1,float a)
{
    rect(x0,y0,x1,y1,0.03f,0.05f,0.09f,a);
    frame(x0,y0,x1,y1,0.3f,0.7f,0.9f);
}

void Hud_Draw(int showHelp,float fps)
{
    int w=glutGet(GLUT_WINDOW_WIDTH), h=glutGet(GLUT_WINDOW_HEIGHT);
    char b[128], clk[8];
    static const char *cam[4]={"F1 First-Person","F2 Runway","F3 Aircraft-Follow","F4 Control Tower"};

    glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity(); glOrtho(0,w,h,0,-1,1);
    glMatrixMode(GL_MODELVIEW);  glPushMatrix(); glLoadIdentity();
    glDisable(GL_DEPTH_TEST); glDisable(GL_LIGHTING);
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    Airport_ClockStr(clk,sizeof clk);

    /* status panel (top-left) */
    panel(12,12,336,150,0.72f);
    glColor3f(1,0.85f,0.3f); text(24,36,GLUT_BITMAP_HELVETICA_18,"HAZRAT SHAHJALAL INTL");
    glColor3f(0.9f,0.95f,1.0f);
    snprintf(b,sizeof b,"Area   : %s",Airport_AreaName());       text(24,60,GLUT_BITMAP_9_BY_15,b);
    snprintf(b,sizeof b,"Camera : %s",cam[g_airport.cameraMode&3]); text(24,78,GLUT_BITMAP_9_BY_15,b);
    snprintf(b,sizeof b,"Mode   : %s   %s",g_airport.dayMode?"DAY":"NIGHT",clk); text(24,96,GLUT_BITMAP_9_BY_15,b);
    snprintf(b,sizeof b,"Depart : %s",Airport_TakeoffStatus()); text(24,114,GLUT_BITMAP_9_BY_15,b);
    snprintf(b,sizeof b,"Arrive : %s",Airport_LandingStatus()); text(24,132,GLUT_BITMAP_9_BY_15,b);

    /* fps (top-right) */
    panel(w-118,12,w-12,40,0.7f);
    glColor3f(0.5f,1.0f,0.6f); snprintf(b,sizeof b,"FPS %.0f",fps); text(w-104,32,GLUT_BITMAP_9_BY_15,b);

    /* objective bar (top-centre) */
    { const char *obj=Story_Objective();
      if(obj){ float tw=textW(GLUT_BITMAP_9_BY_15,obj);
        rect(w/2-tw/2-18,14,w/2+tw/2+18,44,0.05f,0.10f,0.06f,0.8f);
        frame(w/2-tw/2-18,14,w/2+tw/2+18,44,0.3f,0.9f,0.5f);
        glColor3f(0.6f,1.0f,0.7f); text(w/2-tw/2,34,GLUT_BITMAP_9_BY_15,obj); } }

    { int ps=Story_PanelState();
      if(ps){ float bw=470, x0=w/2-bw/2, x1=w/2+bw/2, y0=58, y1=(ps==2)?150:112;
        panel(x0,y0,x1,y1,0.85f);
        glColor3f(1.0f,0.85f,0.3f); ctext(w/2,y0+30,GLUT_BITMAP_HELVETICA_18,Story_PanelTitle());
        if(ps==2){ glColor3f(0.92f,0.95f,1.0f); ctext(w/2,y0+58,GLUT_BITMAP_9_BY_15,Story_PanelLine()); }
        glColor3f(0.5f,1.0f,0.7f); ctext(w/2,(ps==2)?y0+86:y0+58,GLUT_BITMAP_9_BY_15,Story_PanelPrompt());
      } }

    /* controls hint / help */
    if(showHelp){
        panel(w-322,52,w-12,300,0.78f);
        glColor3f(1,0.85f,0.3f); text(w-306,78,GLUT_BITMAP_HELVETICA_18,"CONTROLS");
        glColor3f(0.9f,0.95f,1.0f);
        text(w-306,104,GLUT_BITMAP_9_BY_15,"W A S D   Walk / strafe");
        text(w-306,124,GLUT_BITMAP_9_BY_15,"Mouse     Look (right = right)");
        text(w-306,144,GLUT_BITMAP_9_BY_15,"E / Y / N Talk at the counters");
        text(w-306,164,GLUT_BITMAP_9_BY_15,"F1-F4     Camera views");
        text(w-306,184,GLUT_BITMAP_9_BY_15,"1 / 2     Takeoff / landing");
        text(w-306,204,GLUT_BITMAP_9_BY_15,"L         Day / night");
        text(w-306,224,GLUT_BITMAP_9_BY_15,"R         Restart journey");
        text(w-306,244,GLUT_BITMAP_9_BY_15,"M         Hide this panel");
        text(w-306,264,GLUT_BITMAP_9_BY_15,"Esc       Quit");
    } else {
        glColor3f(0.8f,0.85f,0.9f); text(w-210,30,GLUT_BITMAP_9_BY_15,"Press M for controls");
    }

    if(g_airport.cameraMode==0 && !Story_ShowFlag() && !Story_Complete()){
        glColor4f(1,1,1,0.6f); glLineWidth(1.5f);
        glBegin(GL_LINES);
        glVertex2f(w/2-9,h/2); glVertex2f(w/2-3,h/2);
        glVertex2f(w/2+3,h/2); glVertex2f(w/2+9,h/2);
        glVertex2f(w/2,h/2-9); glVertex2f(w/2,h/2-3);
        glVertex2f(w/2,h/2+3); glVertex2f(w/2,h/2+9);
        glEnd();
    }

    /* story nudge flash */
    { const char *fl=Story_Flash();
      if(fl){ float tw=textW(GLUT_BITMAP_9_BY_15,fl);
        rect(w/2-tw/2-14,h-190,w/2+tw/2+14,h-164,0.25f,0.05f,0.05f,0.85f);
        glColor3f(1.0f,0.6f,0.6f); text(w/2-tw/2,h-172,GLUT_BITMAP_9_BY_15,fl); } }

    /* cabin caption while seated on board */
    if(Story_InCabin()){
        const char *l1="Welcome aboard Biman BG 201 to London.";
        const char *l2="Please fasten your seatbelt for departure.";
        float t1=textW(GLUT_BITMAP_9_BY_15,l1), t2=textW(GLUT_BITMAP_9_BY_15,l2), bw=(t1>t2?t1:t2)+40;
        panel(w/2-bw/2,h-118,w/2+bw/2,h-44,0.85f);
        glColor3f(1.0f,0.85f,0.3f); ctext(w/2,h-90,GLUT_BITMAP_HELVETICA_18,"CABIN CREW");
        glColor3f(0.92f,0.95f,1.0f); ctext(w/2,h-66,GLUT_BITMAP_9_BY_15,l1);
        ctext(w/2,h-50,GLUT_BITMAP_9_BY_15,l2);
    }

    if(Story_InIntro()){
        float x0=w/2-380, x1=w/2+380, y0=h-150, y1=h-40;
        panel(x0,y0,x1,y1,0.86f);
        glColor3f(1.0f,0.85f,0.3f); text(x0+22,y0+28,GLUT_BITMAP_HELVETICA_18,"Hazrat Shahjalal Intl");
        glColor3f(0.92f,0.95f,1.0f); text(x0+22,y0+56,GLUT_BITMAP_9_BY_15,Story_IntroLine());
        glColor3f(0.5f,1.0f,0.7f);  text(x0+22,y0+86,GLUT_BITMAP_9_BY_15,"[E] Begin your journey");
    }

    if(Story_InGround()){
        const char *ln="Oi je dekho, plane!  (Hey, look - a plane!)";
        float tw=textW(GLUT_BITMAP_HELVETICA_18,ln);
        rect(w/2-tw/2-18,h-70,w/2+tw/2+18,h-36,0.05f,0.08f,0.14f,0.85f);
        frame(w/2-tw/2-18,h-70,w/2+tw/2+18,h-36,0.4f,0.85f,1.0f);
        glColor3f(1.0f,0.9f,0.5f); ctext(w/2,h-48,GLUT_BITMAP_HELVETICA_18,ln);
    }

    /* per-step "task complete" tick banner */
    { const char *dm=Story_DoneMsg();
      if(dm){ char b2[64]; float tw; snprintf(b2,sizeof b2,"* %s *",dm);
        tw=textW(GLUT_BITMAP_HELVETICA_18,b2);
        rect(w/2-tw/2-22,54,w/2+tw/2+22,90,0.03f,0.16f,0.06f,0.9f);
        frame(w/2-tw/2-22,54,w/2+tw/2+22,90,0.3f,1.0f,0.4f);
        glColor3f(0.5f,1.0f,0.6f); ctext(w/2,79,GLUT_BITMAP_HELVETICA_18,b2);
      } }

    /* Bangladesh flag salute during departure */
    if(Story_ShowFlag()){
        float cx=w/2.0f, cy=h/2.0f, fw=360, fh=216;
        rect(0,0,w,h,0,0,0,0.35f);
        rect(cx-fw/2,cy-fh/2,cx+fw/2,cy+fh/2,0.02f,0.42f,0.22f,0.95f);   /* green field */
        disc(cx-fw*0.06f,cy,fh*0.30f,0.86f,0.10f,0.12f,0.98f);           /* red circle */
        frame(cx-fw/2,cy-fh/2,cx+fw/2,cy+fh/2,1,1,1);
        glColor3f(1,1,1);
        ctext(cx,cy+fh/2+34,GLUT_BITMAP_HELVETICA_18,"BIMAN BANGLADESH AIRLINES");
        ctext(cx,cy+fh/2+56,GLUT_BITMAP_9_BY_15,"BG 201  DHAKA -> LONDON   -   Bon Voyage!");
    }

    /* completion screen */
    if(Story_Complete()){
        rect(0,0,w,h,0,0,0,0.55f);
        panel(w/2-300,h/2-120,w/2+300,h/2+120,0.92f);
        glColor3f(1,0.85f,0.3f); ctext(w/2,h/2-70,GLUT_BITMAP_HELVETICA_18,"JOURNEY COMPLETE");
        glColor3f(0.92f,0.95f,1.0f);
        ctext(w/2,h/2-30,GLUT_BITMAP_9_BY_15,"You have departed Dhaka aboard BG 201 to London.");
        ctext(w/2,h/2-8, GLUT_BITMAP_9_BY_15,"Your flight has dissolved into the clouds.");
        glColor3f(0.6f,1.0f,0.7f);
        ctext(w/2,h/2+40,GLUT_BITMAP_HELVETICA_18,"[R]  Restart the journey");
        ctext(w/2,h/2+72,GLUT_BITMAP_HELVETICA_18,"[Esc]  Quit");
    }

    glDisable(GL_BLEND); glEnable(GL_DEPTH_TEST); glEnable(GL_LIGHTING);
    glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW); glPopMatrix();
}
