/*  
    Unusual Suspects 
    3D routines headers. 
*/

#include "includes.prl"

#include "common.h"
// #include "protos.h"

#define VERT_COUNT(n) (sizeof(n)/sizeof(n[0])/3)
#define FACE_COUNT(n) (sizeof(n)/sizeof(n[0])/4)
#define MAX_VERTICE_COUNT 1024

#define PREPARE_3D_MESH(OBJECT_HANDLER, OBJECT_VERT_LIST, OBJECT_FACE_LIST, ZOOM_LEVEL, Z_DISTANCE, FLAG_CULL) \
                  OBJECT_HANDLER.verts = (int const *)&OBJECT_VERT_LIST; \
                  OBJECT_HANDLER.nverts = VERT_COUNT(OBJECT_VERT_LIST); \
                  OBJECT_HANDLER.faces = (int const *)&OBJECT_FACE_LIST; \
                  OBJECT_HANDLER.nfaces = FACE_COUNT(OBJECT_FACE_LIST); \
                  OBJECT_HANDLER.zoom = ZOOM_LEVEL; \
                  OBJECT_HANDLER.distance = Z_DISTANCE; \
                  OBJECT_HANDLER.flag_cull_backfaces = FLAG_CULL;

#define vX(I) (3 * I + 0)
#define vY(I) (3 * I + 1)
#define vZ(I) (3 * I + 2)

#define Fc0(I) (4 * I + 0)
#define Fc1(I) (4 * I + 1)
#define Fc2(I) (4 * I + 2)
#define Fc3(I) (4 * I + 3)

#define fixed_pt_pre  512
#define fixed_pt_shift 9

struct obj_3d
{
    int const* verts;
    int nverts;
    int const* faces;
    int nfaces;
    int zoom;
    int distance;
    int flag_cull_backfaces;
};

void DrawAALine(int x1, int y1, int x2, int y2);

void Prepare3DVertexList(void);

void Delete3DVertexList(void);

int Draw3DMesh(int rx, int ry, int y_offset, int m_scale_x);