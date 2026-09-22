#include <math.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "people.h"
#include "gfx.h"

static const float SHIRT[6][3] = {
    {0.80f,0.20f,0.22f},{0.15f,0.35f,0.65f},{0.20f,0.55f,0.35f},
    {0.75f,0.75f,0.78f},{0.95f,0.80f,0.10f},{0.55f,0.25f,0.60f}
};
static const float SKIN[3][3] = {
    {0.80f,0.62f,0.46f},{0.66f,0.48f,0.34f},{0.90f,0.74f,0.60f}
};

static int s_gesture=0;                 /* applied to the next People_Draw, then cleared */
void People_SetGesture(int g){ s_gesture=g; }

/* two-segment limb: rotate thigh/upper by a1 at the pivot, draw down len1,
   then rotate shin/fore by a2 at the knee/elbow and draw down len2.
   returns via out[] the world-ish end position in the caller's local frame. */
static void twoSeg(float px,float py,float pz,float a1,float len1,float a2,float len2,
                   float hw,float r,float g,float b,int foot)
{
    Gfx_Material(r,g,b,8.0f);
    glPushMatrix();
    glTranslatef(px,py,pz);
    glRotatef(a1,0,0,1);
    Gfx_Box(-hw,-len1,-hw,hw,0.02f,hw);           /* upper segment */
    glTranslatef(0,-len1,0);
    glRotatef(a2,0,0,1);
    Gfx_Box(-hw*0.92f,-len2,-hw*0.92f,hw*0.92f,0.02f,hw*0.92f);  /* lower segment */
    if(foot){
        glTranslatef(0,-len2,0);
        glRotatef(-(a1+a2),0,0,1);                /* keep foot roughly flat */
        Gfx_Material(0.06f,0.06f,0.07f,20.0f);
        Gfx_Box(-0.06f,-0.02f,-0.09f,0.20f,0.08f,0.09f);
    }
    glPopMatrix();
}

void People_Draw(float x,float z,float yaw,float phase,float walk,int variant,int bag)
{
    const float *shirt = SHIRT[variant%6];
    const float *skin  = SKIN[(variant+1)%3];
    float p   = phase;
    float A   = 32.0f*walk;                 /* hip swing amplitude */
    float bob = sinf(p*2.0f)*0.02f*walk;    /* vertical body bob   */

    float hipR = A*sinf(p),        hipL = A*sinf(p+3.14159f);
    float kneeR= 46.0f*walk*fmaxf(0.0f, sinf(p+1.1f));
    float kneeL= 46.0f*walk*fmaxf(0.0f, sinf(p+1.1f+3.14159f));
    float shR  = -0.75f*A*sinf(p),  shL = -0.75f*A*sinf(p+3.14159f);
    float elb  = 18.0f + 10.0f*walk;

    glPushMatrix();
    glTranslatef(x,bob,z);
    glRotatef(yaw,0,1,0);

    /* legs (trousers) */
    twoSeg( 0.0f,0.92f, 0.12f, hipR,0.48f, kneeR,0.46f, 0.10f, 0.18f,0.20f,0.28f,1);
    twoSeg( 0.0f,0.92f,-0.12f, hipL,0.48f, kneeL,0.46f, 0.10f, 0.18f,0.20f,0.28f,1);

    /* torso */
    Gfx_Material(shirt[0],shirt[1],shirt[2],12.0f);
    Gfx_Box(-0.17f,0.9f,-0.22f,0.17f,1.5f,0.22f);
    if(variant==4){ Gfx_Emissive(0.95f,0.9f,0.15f);
        Gfx_Box(-0.18f,1.14f,-0.225f,0.18f,1.24f,0.225f); Gfx_EmissiveOff(); }
    Gfx_Material(shirt[0],shirt[1],shirt[2],12.0f);
    Gfx_Box(-0.22f,1.42f,-0.22f,0.22f,1.52f,0.22f);   /* shoulders */

    { float shG=shR, elbG=elb;
      if(s_gesture==1){ shG = 42.0f + 20.0f*sinf(p*3.0f); elbG = 60.0f + 28.0f*sinf(p*4.0f); }
      else if(s_gesture==2){ shG = 150.0f; elbG = 6.0f; }
      twoSeg( 0.0f,1.5f, 0.26f, shG,0.32f, elbG,0.30f, 0.07f, shirt[0],shirt[1],shirt[2],0);
      twoSeg( 0.0f,1.5f,-0.26f, shL,0.32f, elb, 0.30f, 0.07f, shirt[0],shirt[1],shirt[2],0);
    }

    /* neck + head */
    Gfx_Material(skin[0],skin[1],skin[2],14.0f);
    Gfx_Box(-0.06f,1.5f,-0.06f,0.06f,1.6f,0.06f);
    glPushMatrix(); glTranslatef(0,1.73f,0); glutSolidSphere(0.145f,10,8); glPopMatrix();
    Gfx_Material(0.05f,0.04f,0.03f,6.0f);
    glPushMatrix(); glTranslatef(0,1.78f,-0.02f); glutSolidSphere(0.152f,10,8); glPopMatrix();
    Gfx_Material(skin[0],skin[1],skin[2],14.0f);
    glPushMatrix(); glTranslatef(0.10f,1.74f,0); glScalef(0.55f,1,1); glutSolidSphere(0.12f,8,6); glPopMatrix();

    /* luggage */
    if(bag==1){
        Gfx_Material(0.20f,0.55f,0.80f,20.0f); Gfx_Box(0.28f,0.1f,0.30f,0.62f,0.85f,0.62f);
        Gfx_Metal(0.6f,0.6f,0.65f); Gfx_Box(0.44f,0.85f,0.44f,0.48f,1.15f,0.48f);
    } else if(bag==2){
        Gfx_Metal(0.55f,0.57f,0.6f); Gfx_Box(0.35f,0.05f,-0.4f,0.85f,0.12f,0.4f);
        Gfx_Box(0.35f,0.12f,-0.42f,0.42f,1.0f,0.42f);
        Gfx_Material(0.6f,0.15f,0.12f,20.0f); Gfx_Box(0.42f,0.12f,-0.35f,0.8f,0.55f,0.35f);
        Gfx_Material(0.15f,0.4f,0.2f,20.0f);  Gfx_Box(0.42f,0.55f,-0.3f,0.78f,0.9f,0.3f);
    } else if(bag==3){
        Gfx_Material(0.2f,0.3f,0.25f,15.0f);  Gfx_Box(-0.28f,1.0f,-0.2f,-0.17f,1.45f,0.2f);
    }
    glPopMatrix();
    s_gesture=0;                         /* one-shot: clear after drawing */
}
