/*  
    Unusual Suspects 
    3D routines. 
*/

#include "3d_routines.h"
#include "Assets/cosine_table.h"
#include "common.h"

extern struct BitMap theBitMap;
extern struct BitMap theBitMap_3bpl;
extern struct BitMap theBitMap_2bpl;
extern struct BitMap theBitMap_1bpl;
extern struct RastPort theRP;
extern struct RastPort theRP_3bpl;
extern struct RastPort theRP_2bpl;
extern struct RastPort theRP_1bpl;

extern struct GfxBase *GfxBase;

struct  obj_3d o;
short     *verts_tr = NULL;

#define BB_3D_DRAW_OUTLINE    8

/********* 3D Code *********/

void DrawAALine(short x1, short y1, short x2, short y2)
{
  short xo = 0, yo = 0;
  if (2 * abs(x1 - x2) > abs(y1 - y2))
  {
    xo = 1;
    yo = 0;
  }

  if (abs(x1 - x2) < 2 * abs(y1 - y2))
  {
    xo = 0;
    yo = 1;
  }

  if (xo || yo)
  {
    SetAPen(&theRP_2bpl, 1);
    Move(&theRP_2bpl, x1 + xo, y1 + yo);
    Draw(&theRP_2bpl, x2 + xo, y2 + yo);
    Move(&theRP_2bpl, x1 - xo, y1 - yo);
    Draw(&theRP_2bpl, x2 - xo, y2 - yo);
  }
}

void Prepare3DVertexList(void)
{  verts_tr = (short *)AllocMem(sizeof(short) * MAX_VERTICE_COUNT * 3, 0L); } // (short *)malloc(sizeof(short) * MAX_VERTICE_COUNT * 3); }

void Delete3DVertexList(void)
{  
  if (verts_tr != NULL)
  {
    FreeMem(verts_tr, sizeof(short) * MAX_VERTICE_COUNT * 3); // free(verts_tr);
    verts_tr = NULL;
  }
}

short Draw3DMesh(short rx, short ry, short y_offset, short m_scale_x)
{

  short i,tx,ty,
  x1,x2,x3,x4, 
  y1,y2,y3,y4,
  hidden = 0;

  short XC,YC;

  short cs, ss, cc, sc;

  XC = 160;
  YC = 128 + y_offset;

  /*  Transform & project the vertices */
  //  pre-rotations
  cs = (tcos[rx] * tsin[ry]) >> fixed_pt_shift;
  ss = (tsin[ry] * tsin[rx]) >> fixed_pt_shift;
  cc = (tcos[rx] * tcos[ry]) >> fixed_pt_shift;
  sc = (tsin[rx] * tcos[ry]) >> fixed_pt_shift;

  for (i = 0; i < o.nverts; ++i)
  {
    /* 
        Rotation on 3 axis of each vertex
    */
    verts_tr[vX(i)] = (o.verts[vX(i)] * tsin[rx] + o.verts[vY(i)] * tcos[rx]) >> fixed_pt_shift; // / fixed_pt_pre;
    verts_tr[vY(i)] = (o.verts[vX(i)] * cs - o.verts[vY(i)] * ss + o.verts[vZ(i)] * tcos[ry]) >> fixed_pt_shift; // / fixed_pt_pre;
    verts_tr[vZ(i)] = (o.verts[vX(i)] * cc - o.verts[vY(i)] * sc - o.verts[vZ(i)] * tsin[ry]) >> fixed_pt_shift; // / fixed_pt_pre;

    /*
      Classic 3D -> 2D projection
    */
    tx = (verts_tr[vX(i)] * o.zoom) / (verts_tr[vZ(i)] + o.distance);
    ty = (verts_tr[vY(i)] * o.zoom) / (verts_tr[vZ(i)] + o.distance);
    verts_tr[vX(i)] = tx;
    verts_tr[vY(i)] = ty;
  }

  for (i = 0; i < o.nfaces; ++i)
  {
    x1 = verts_tr[vX(o.faces[Fc0(i)])];
    y1 = YC + verts_tr[vY(o.faces[Fc0(i)])];

    x2 = verts_tr[vX(o.faces[Fc1(i)])];
    y2 = YC + verts_tr[vY(o.faces[Fc1(i)])];

    x3 = verts_tr[vX(o.faces[Fc2(i)])];
    y3 = YC + verts_tr[vY(o.faces[Fc2(i)])];

    x4 = verts_tr[vX(o.faces[Fc3(i)])];
    y4 = YC + verts_tr[vY(o.faces[Fc3(i)])];

    if (m_scale_x > 0)
    {
      x1 = x1 >> m_scale_x;
      x2 = x1 >> m_scale_x;
      x3 = x1 >> m_scale_x;
      x4 = x1 >> m_scale_x;
    }

    x1 += XC;
    x2 += XC;
    x3 += XC;
    x4 += XC;

    //  should we draw the face ?
    hidden = (x3 - x1) * (y2 - y1) - (x2 - x1) * (y3 - y1);

    if (hidden > 0)
    {           
      SetAPen(&theRP_2bpl, 2);

      Move(&theRP_2bpl, x1, y1);
      Draw(&theRP_2bpl, x2, y2);
      Draw(&theRP_2bpl, x3, y3);
      Draw(&theRP_2bpl, x4, y4);
      Draw(&theRP_2bpl, x1, y1);
    }
    else
    {
      if (!o.flag_cull_backfaces)
      {
        SetAPen(&theRP_2bpl, 1);

        Move(&theRP_1bpl, x1, y1);
        Draw(&theRP_1bpl, x2, y2);
        Draw(&theRP_1bpl, x3, y3);
        Draw(&theRP_1bpl, x4, y4);
        Draw(&theRP_1bpl, x1, y1);        
      }
    }
  }

  return 0;
}