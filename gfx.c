#include <math.h>
#include <string.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include "gfx.h"

#define STROKE_CAP 119.05f   /* cap height of GLUT_STROKE_ROMAN */

static GLUquadric *g_q;
static GLuint      g_tex[TEX_COUNT];

/* procedural texture generator (128x128 RGB) */
static unsigned char g_pix[128*128*3];

static int irand(int *seed){ *seed = *seed*1103515245 + 12345; return (*seed>>16)&0x7fff; }

static void makeTexture(int idx,int r,int g,int b,int noise,int grid,int seed)
{
    int x,y;
    for(y=0;y<128;y++) for(x=0;x<128;x++){
        int n = noise ? (irand(&seed)%(2*noise+1))-noise : 0;
        int rr=r+n, gg=g+n, bb=b+n;
        if(grid>0 && (x%grid<2 || y%grid<2)){ rr-=45; gg-=45; bb-=45; }
        rr = rr<0?0:(rr>255?255:rr);
        gg = gg<0?0:(gg>255?255:gg);
        bb = bb<0?0:(bb>255?255:bb);
        g_pix[(y*128+x)*3+0]=(unsigned char)rr;
        g_pix[(y*128+x)*3+1]=(unsigned char)gg;
        g_pix[(y*128+x)*3+2]=(unsigned char)bb;
    }
    glBindTexture(GL_TEXTURE_2D,g_tex[idx]);
    gluBuild2DMipmaps(GL_TEXTURE_2D,GL_RGB,128,128,GL_RGB,GL_UNSIGNED_BYTE,g_pix);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
}

void Gfx_Init(void)
{
    g_q = gluNewQuadric();
    gluQuadricNormals(g_q,GLU_SMOOTH);
    gluQuadricTexture(g_q,GL_TRUE);
    glGenTextures(TEX_COUNT,g_tex);
    makeTexture(TEX_ASPHALT , 58, 60, 64, 10, 0, 7);
    makeTexture(TEX_CONCRETE,150,152,155, 12, 32, 13);
    makeTexture(TEX_TILE    ,205,208,212,  6, 16, 21);
    makeTexture(TEX_GRASS   , 60,120, 52, 22, 0, 99);
    makeTexture(TEX_METAL   ,175,182,190,  8, 0, 5);
    makeTexture(TEX_CARPET  , 42, 66,110, 14, 0, 33);
}

GLUquadric *Gfx_Quadric(void){ return g_q; }

/* materials */
void Gfx_Material(float r,float g,float b,float shininess)
{
    GLfloat amb[]={r*0.28f,g*0.28f,b*0.28f,1};
    GLfloat dif[]={r,g,b,1};
    GLfloat spc[]={0.30f,0.30f,0.30f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT ,amb);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE ,dif);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spc);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,shininess);
    glColor3f(r,g,b);
}

void Gfx_Metal(float r,float g,float b)
{
    GLfloat amb[]={r*0.35f,g*0.35f,b*0.35f,1};
    GLfloat dif[]={r,g,b,1};
    GLfloat spc[]={0.85f,0.88f,0.92f,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT ,amb);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE ,dif);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spc);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,90.0f);
    glColor3f(r,g,b);
}

void Gfx_Emissive(float r,float g,float b)
{
    GLfloat e[]={r,g,b,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,e);
    glColor3f(r,g,b);
}
void Gfx_EmissiveOff(void)
{
    GLfloat e[]={0,0,0,1};
    glMaterialfv(GL_FRONT_AND_BACK,GL_EMISSION,e);
}

/* primitives */
void Gfx_Box(float x0,float y0,float z0,float x1,float y1,float z1)
{
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);  glVertex3f(x0,y1,z0);glVertex3f(x0,y1,z1);glVertex3f(x1,y1,z1);glVertex3f(x1,y1,z0);
    glNormal3f(0,-1,0); glVertex3f(x0,y0,z0);glVertex3f(x1,y0,z0);glVertex3f(x1,y0,z1);glVertex3f(x0,y0,z1);
    glNormal3f(0,0,1);  glVertex3f(x0,y0,z1);glVertex3f(x1,y0,z1);glVertex3f(x1,y1,z1);glVertex3f(x0,y1,z1);
    glNormal3f(0,0,-1); glVertex3f(x1,y0,z0);glVertex3f(x0,y0,z0);glVertex3f(x0,y1,z0);glVertex3f(x1,y1,z0);
    glNormal3f(1,0,0);  glVertex3f(x1,y0,z1);glVertex3f(x1,y0,z0);glVertex3f(x1,y1,z0);glVertex3f(x1,y1,z1);
    glNormal3f(-1,0,0); glVertex3f(x0,y0,z0);glVertex3f(x0,y0,z1);glVertex3f(x0,y1,z1);glVertex3f(x0,y1,z0);
    glEnd();
}

void Gfx_TexBox(int tex,float x0,float y0,float z0,float x1,float y1,float z1,float rep)
{
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D,g_tex[tex]); glColor3f(1,1,1);
    glBegin(GL_QUADS);
    glNormal3f(0,1,0);  glTexCoord2f(0,0);glVertex3f(x0,y1,z0);glTexCoord2f(0,rep);glVertex3f(x0,y1,z1);glTexCoord2f(rep,rep);glVertex3f(x1,y1,z1);glTexCoord2f(rep,0);glVertex3f(x1,y1,z0);
    glNormal3f(0,0,1);  glTexCoord2f(0,0);glVertex3f(x0,y0,z1);glTexCoord2f(rep,0);glVertex3f(x1,y0,z1);glTexCoord2f(rep,rep);glVertex3f(x1,y1,z1);glTexCoord2f(0,rep);glVertex3f(x0,y1,z1);
    glNormal3f(0,0,-1); glTexCoord2f(0,0);glVertex3f(x1,y0,z0);glTexCoord2f(rep,0);glVertex3f(x0,y0,z0);glTexCoord2f(rep,rep);glVertex3f(x0,y1,z0);glTexCoord2f(0,rep);glVertex3f(x1,y1,z0);
    glNormal3f(1,0,0);  glTexCoord2f(0,0);glVertex3f(x1,y0,z1);glTexCoord2f(rep,0);glVertex3f(x1,y0,z0);glTexCoord2f(rep,rep);glVertex3f(x1,y1,z0);glTexCoord2f(0,rep);glVertex3f(x1,y1,z1);
    glNormal3f(-1,0,0); glTexCoord2f(0,0);glVertex3f(x0,y0,z0);glTexCoord2f(rep,0);glVertex3f(x0,y0,z1);glTexCoord2f(rep,rep);glVertex3f(x0,y1,z1);glTexCoord2f(0,rep);glVertex3f(x0,y1,z0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void Gfx_Cylinder(float x,float y,float z,float radius,float height,float r,float g,float b)
{
    Gfx_Material(r,g,b,25.0f);
    glPushMatrix(); glTranslatef(x,y,z); glRotatef(-90,1,0,0);
    gluCylinder(g_q,radius,radius,height,18,1);
    gluDisk(g_q,0,radius,18,1);
    glTranslatef(0,0,height); gluDisk(g_q,0,radius,18,1);
    glPopMatrix();
}

void Gfx_TexturedFloor(float x0,float z0,float x1,float z1,float y,int tex,float repeat)
{
    glEnable(GL_TEXTURE_2D); glBindTexture(GL_TEXTURE_2D,g_tex[tex]); glColor3f(1,1,1);
    Gfx_Material(1,1,1,4.0f);
    glBegin(GL_QUADS); glNormal3f(0,1,0);
    glTexCoord2f(0,0);          glVertex3f(x0,y,z0);
    glTexCoord2f(0,repeat);     glVertex3f(x0,y,z1);
    glTexCoord2f(repeat,repeat);glVertex3f(x1,y,z1);
    glTexCoord2f(repeat,0);     glVertex3f(x1,y,z0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

void Gfx_Glass(float x0,float y0,float z0,float x1,float y1,float z1,
               float r,float g,float b,float a)
{
    /* Colour+alpha must come from the material (GL_COLOR_MATERIAL is off), otherwise
       the pane renders opaque with a stale colour. Diffuse alpha drives the blend. */
    GLfloat dif[]={r,g,b,a};
    GLfloat amb[]={r*0.45f,g*0.45f,b*0.45f,a};
    GLfloat spc[]={0.95f,0.97f,1.0f,a};
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT ,amb);
    glMaterialfv(GL_FRONT_AND_BACK,GL_DIFFUSE ,dif);
    glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,spc);
    glMaterialf (GL_FRONT_AND_BACK,GL_SHININESS,120.0f);
    glColor4f(r,g,b,a);
    Gfx_Box(x0,y0,z0,x1,y1,z1);
    glDepthMask(GL_TRUE); glDisable(GL_BLEND);
}

void Gfx_Panel(const float t[4][3], float th)
{
    int i;
    float ux=t[1][0]-t[0][0], uy=t[1][1]-t[0][1], uz=t[1][2]-t[0][2];
    float vx=t[3][0]-t[0][0], vy=t[3][1]-t[0][1], vz=t[3][2]-t[0][2];
    float nx=uy*vz-uz*vy, ny=uz*vx-ux*vz, nz=ux*vy-uy*vx;
    float len=sqrtf(nx*nx+ny*ny+nz*nz); if(len<1e-6f)len=1; nx/=len;ny/=len;nz/=len;
    glBegin(GL_QUADS);
    glNormal3f(nx,ny,nz);
    for(i=0;i<4;i++) glVertex3f(t[i][0],t[i][1],t[i][2]);
    glNormal3f(-nx,-ny,-nz);
    for(i=3;i>=0;i--) glVertex3f(t[i][0],t[i][1]-th,t[i][2]);
    glEnd();
    /* edges */
    glBegin(GL_QUADS);
    for(i=0;i<4;i++){
        int j=(i+1)%4;
        glVertex3f(t[i][0],t[i][1],t[i][2]);
        glVertex3f(t[i][0],t[i][1]-th,t[i][2]);
        glVertex3f(t[j][0],t[j][1]-th,t[j][2]);
        glVertex3f(t[j][0],t[j][1],t[j][2]);
    }
    glEnd();
}

/* text */
float Gfx_TextWidth(float height,const char *s)
{
    float w=0; const char *p;
    for(p=s;*p;p++) w+=glutStrokeWidth(GLUT_STROKE_ROMAN,*p);
    return w*(height/STROKE_CAP);
}

void Gfx_TextFlat(float x,float y,float z,float faceDeg,float height,
                  float r,float g,float b,const char *s)
{
    GLboolean lit=glIsEnabled(GL_LIGHTING);
    const char *p;
    float k=height/STROKE_CAP;
    float raw=0; for(p=s;*p;p++) raw+=glutStrokeWidth(GLUT_STROKE_ROMAN,*p);
    glDisable(GL_LIGHTING); glColor3f(r,g,b);
    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(faceDeg,0,1,0);
    glScalef(k,k,k);
    glTranslatef(-raw*0.5f,0,0);
    glLineWidth(2.0f);
    for(p=s;*p;p++) glutStrokeCharacter(GLUT_STROKE_ROMAN,*p);
    glPopMatrix();
    if(lit) glEnable(GL_LIGHTING);
}

void Gfx_TextLeft(float x,float y,float z,float faceDeg,float height,
                  float r,float g,float b,const char *s)
{
    GLboolean lit=glIsEnabled(GL_LIGHTING);
    const char *p;
    float k=height/STROKE_CAP;
    glDisable(GL_LIGHTING); glColor3f(r,g,b);
    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(faceDeg,0,1,0);
    glScalef(k,k,k);
    glLineWidth(2.0f);
    for(p=s;*p;p++) glutStrokeCharacter(GLUT_STROKE_ROMAN,*p);
    glPopMatrix();
    if(lit) glEnable(GL_LIGHTING);
}

void Gfx_TextBill(float x,float y,float z,float camx,float camz,float height,
                  float r,float g,float b,const char *s)
{
    float ang = atan2f(camx-x, camz-z)*57.2957795f;
    Gfx_TextFlat(x,y,z,ang,height,r,g,b,s);
}
