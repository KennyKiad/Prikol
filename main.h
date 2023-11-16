#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

float Map_GetHeight(float x, float y);

typedef struct
{
    float x,y,z;
} TCell;

typedef struct
{
    float r,g,b;
} TColor;

typedef struct
{
    float u,v;
} TUV;

typedef struct
{
    float x,y,z;
    int type;
    float scale;
} TObject;

typedef struct
{
    TObject* items;
    int type;
    int itemsCnt;
    float x,y,z;
} TObjGroup;

#define mapW 100
#define mapH 100

TCell map[mapW][mapH];
TCell mapNormal[mapW][mapH];
TUV mapUV[mapW][mapH];

GLuint mapInd[mapW-1][mapH-1][6];
int mapIndCnt = sizeof(mapInd) / sizeof(GLuint);

float plant[] = {-0.5,0,0, 0.5,0,0, 0.5,0,1, -0.5,0,1,
                 0,-0.5,0, 0,0.5,0, 0,0.5,1, 0,-0.5,1};
float plantUV[] = {0,1, 1,1, 1,0, 0,0, 0,1, 1,1, 1,0, 0,0};
GLuint plantInd[] = {0,1,2, 2,3,0, 4,5,6, 6,7,4};
int plantIndCnt = sizeof(plantInd) / sizeof(GLuint);

TObject* plantArr = NULL;
int plantCnt = 0;

int tex_flower,tex_flower2,tex_pole,tex_trava,tex_grib,tex_tree,tex_tree2, tex_wood;

float cube[] = {0,0,0, 1,0,0, 1,1,0, 0,1,0,
                0,0,1, 1,0,1, 1,1,1, 0,1,1,
                0,0,0, 1,0,0, 1,1,0, 0,1,0,
                0,0,1, 1,0,1, 1,1,1, 0,1,1};

float cubeUVlog[] = {0.5,0.5, 1,0.5, 1,0, 0.5,0,
                     0.5,0.5, 1,0.5, 1,0, 0.5,0,
                     0,0.5, 0.5,0.5, 0,0.5, 0.5,0.5,
                     0,0, 0.5,0, 0,0, 0.5,0};

float cubeUVleaf[] = {0,1, 0.5,1, 0.5,0.5, 0,0.5,
                      0,1, 0.5,1, 0.5,0.5, 0,0.5,
                      0,0.5, 0.5,0.5, 0,0.5, 0.5,0.5,
                      0,1, 0.5,1, 0,1, 0.5,1};

GLuint cubeInd[] = {0,1,2, 2,3,0, 4,5,6, 6,7,4, 8,9,13, 13,12,8,
                    9,10,14, 14,13,9, 10,11,15, 15,14,10, 11,8,12, 12,15,11};
int cubeIndCnt = sizeof(cubeInd) / sizeof(GLuint);

TObjGroup* trees = NULL;
int treeCnt = 0;

float sun[] = {-1,-1,0, 1,-1,0, 1,1,0, -1,1,0};

BOOL selectMode = FALSE;

#define ObjListCnt 255
typedef struct
{
    int plantArr_Ind;
    int colorInd;
} TSelectObj;
TSelectObj selectArr[ObjListCnt];
int selectArrCnt = 0;


#endif // MAIN_H_INCLUDED
