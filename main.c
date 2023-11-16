#include <windows.h>
#include <gl/gl.h>
#include <math.h>

#include "../CameraLib/camera.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../StbLib/stb_image.h"

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

float theta = 0.0f;

#include "main.h"

void Set_PositionObject(TObject* obj)
{
    obj->x = rand() % mapW;
    obj->y = rand() % mapH;
    obj->z = Map_GetHeight(obj->x, obj->y);
}

void Anim_Set(TAnim* anm, TObject* obj)
{
    if(anm->obj != NULL) return;
    anm->obj = obj;
    anm->cnt = 10;

    anm->dx = (camera.x - obj->x) / (float)anm->cnt;
    anm->dy = (camera.y - obj->y) / (float)anm->cnt;
    anm->dz = ((camera.z - obj->scale - 0.2) - obj->z) / (float)anm->cnt;
}

void Anim_Move(TAnim* anm)
{
    if (anm->obj != NULL)
    {
        anm->obj->x += anm->dx;
        anm->obj->y += anm->dy;
        anm->obj->z += anm->dz;
        anm->cnt--;
        if (anm->cnt < 1)
        {
            Set_PositionObject(anm->obj);
            anm->obj = NULL;
        }
    }

}

void LoadTexture(char *file_name, int *target)
{
    int width, height, cnt;
    unsigned char *data = stbi_load(file_name, &width, &height, &cnt, 0);

    glGenTextures(1, target);
    glBindTexture(GL_TEXTURE_2D, *target);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,
                                    0, cnt == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

#define sqr(a) (a)*(a)
void CalcNormal(TCell a, TCell b, TCell c, TCell *n)
{
    float wrki;
    TCell v1, v2;

    v1.x = a.x - b.x;
    v1.y = a.y - b.y;
    v1.z = a.z - b.z;
    v2.x = b.x - c.x;
    v2.y = b.y - c.y;
    v2.z = b.z - c.z;

    n->x = (v1.y * v2.z - v1.z * v2.y);
    n->y = (v1.z * v2.x - v1.x * v2.z);
    n->z = (v1.x * v2.y - v1.y * v2.x);
    wrki = sqrt(sqr(n->x) + sqr(n->y) + sqr(n->z));
    n->x /= wrki;
    n->y /= wrki;
    n->z /= wrki;
}

BOOL IsCoordInMap(float x, float y)
{
    return (x >= 0) && (x < mapW) && (y >= 0) && (y < mapH);
}

void DrawCoords(float x, float y, float z)
{
    glBegin(GL_LINES);
        glColor3f(1,1,1);
        glVertex3f(0,0,0);
        glVertex3f(0,0,z);
        glColor3f(0,1,0);
        glVertex3f(0,0,0);
        glVertex3f(0,y,0);
        glColor3f(1,0,0);
        glVertex3f(0,0,0);
        glVertex3f(x,0,0);
    glEnd();
}

void Create_Tree(TObjGroup* obj, int type, float x, float y)
{
    obj->type = type;
    float z = Map_GetHeight(x+0.5, y+0.5) - 0.5;
    int logs = 6;
    int leafs = 5*5*2 - 2 + 3*3*2;

    obj->x = x;
    obj->y = y;
    obj->z = z;

    obj->itemsCnt = logs + leafs;
    obj->items = malloc(sizeof(TObject) * obj->itemsCnt);

    for (int i = 0; i < logs; i++)
    {
        obj->items[i].type = 1;
        obj->items[i].x = x;
        obj->items[i].y = y;
        obj->items[i].z = z + i;
    }
    int pos = logs;
    for (int k = 0; k < 2; k++)
        for (int i = x-2; i <= x+2; i++)
            for (int j = y-2; j <= y+2; j++)
                if ((i != x) || (j != y))
                {
                   obj->items[pos].type = 2;
                   obj->items[pos].x = i;
                   obj->items[pos].y = j;
                   obj->items[pos].z = z + logs - 2 + k;
                   pos++;
                }
    for (int k = 0; k < 2; k++)
        for (int i = x-1; i <= x+1; i++)
            for (int j = y-1; j <= y+1; j++)
                {
                   obj->items[pos].type = 2;
                   obj->items[pos].x = i;
                   obj->items[pos].y = j;
                   obj->items[pos].z = z + logs + k;
                   pos++;
                }
}

void Map_CreateHill(int posX, int posY, int rad, int height)
{
    for (int i = posX-rad; i <= posX+rad; i++)
        for (int j = posY-rad; j <= posY+rad; j++)
            if (IsCoordInMap(i, j))
            {
                float len = sqrt( pow(posX-i, 2) + pow(posY-j,2));
                if (len < rad)
                {
                    len = len / rad * M_PI_2;
                    map[i][j].z += cos(len) * height;
                }
            }
}

float Map_GetHeight(float x, float y)
{
    if (IsCoordInMap(x, y) == NULL) return 0;
    int cX = (int)x;
    int cY = (int)y;

    float h1 = ((1-(x-cX))*map[cX][cY].z + (x-cX)*map[cX+1][cY].z);
    float h2 = ((1-(x-cX))*map[cX][cY+1].z + (x-cX)*map[cX+1][cY+1].z);

    return (1-(y-cY))*h1 + (y-cY)*h2;
}

void Map_Init()
{
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);

    glEnable(GL_DEPTH_TEST);

    LoadTexture("textures/pole.png",    &tex_pole);
    LoadTexture("textures/trava.png",   &tex_trava);
    LoadTexture("textures/flower.png",  &tex_flower);
    LoadTexture("textures/flower2.png", &tex_flower2);
    LoadTexture("textures/grib.png",    &tex_grib);
    LoadTexture("textures/tree.png",    &tex_tree);
    LoadTexture("textures/tree2.png",   &tex_tree2);
    LoadTexture("textures/wood.png",   &tex_wood);

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.99);

    for(int i = 0; i < mapW; i++)
        for (int j = 0; j < mapH; j++)
            {
                map[i][j].x = i;
                map[i][j].y = j;
                map[i][j].z = (rand() % 10) * 0.05;

                mapUV[i][j].u = i;
                mapUV[i][j].v = j;
            }

    for (int i = 0; i < mapW-1; i++)
    {
        int pos = i * mapH;
        for (int j = 0; j < mapH-1; j++)
        {
            mapInd[i][j][0] = pos;
            mapInd[i][j][1] = pos + 1;
            mapInd[i][j][2] = pos + 1 + mapH;

            mapInd[i][j][3] = pos + 1 + mapH;
            mapInd[i][j][4] = pos + mapH;
            mapInd[i][j][5] = pos;


            pos++;
        }
    }

    for (int i = 0; i < 10; i++)
       Map_CreateHill(rand() % mapW, rand() % mapH, rand() % 50, rand() % 10);

    for (int i = 0; i < mapW-1; i++)
        for (int j = 0; j < mapH-1; j++)
            CalcNormal(map[i][j], map[i+1][j], map[i][j+1], &mapNormal[i][j]);

    int travaN = 2000;
    int gribN = 40;
    int treeN = 30;
    plantCnt = travaN + gribN + treeN;
    plantArr = realloc(plantArr, sizeof(*plantArr) * plantCnt);
    for (int i = 0; i < plantCnt; i++)
    {
        if (i < travaN)
        {
            plantArr[i].type = rand() % 100 != 0 ? tex_trava :
                               (rand() % 2 == 0 ? tex_flower : tex_flower2);
            plantArr[i].scale = 0.7 + (rand() % 5) * 0.1;
        }
        else if (i < (travaN + gribN))
        {
            plantArr[i].type = tex_grib;
            plantArr[i].scale = 0.2 + (rand() % 10) * 0.01;
        }
        else
        {
            plantArr[i].type = (rand() % 2 == 0 ? tex_tree : tex_tree2);
            plantArr[i].scale = 4 * (rand() % 14);
        }

        Set_PositionObject(&plantArr[i]);
        /*plantArr[i].x = rand() % mapW;
        plantArr[i].y = rand() % mapH;
        plantArr[i].z = Map_GetHeight(plantArr[i].x, plantArr[i].y);*/
    }

    treeCnt = 50;
    trees = realloc(trees, sizeof(*trees) * treeCnt);
    for(int i = 0; i < treeCnt; i++)
        Create_Tree(trees+i, tex_wood, rand() % mapW, rand() % mapH);
}

void Tree_Show(TObjGroup obj)
{
    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, cube);
        glColor3f(0.7,0.7,0.7);
        glNormal3f(0,0,1);
        glBindTexture(GL_TEXTURE_2D, obj.type);
        for (int i = 0; i < obj.itemsCnt; i++)
        {
            if (obj.items[i].type == 1) glTexCoordPointer(2, GL_FLOAT, 0, cubeUVlog);
            else glTexCoordPointer(2, GL_FLOAT, 0, cubeUVleaf);

            glPushMatrix();
                glTranslatef(obj.items[i].x, obj.items[i].y, obj.items[i].z);
                glDrawElements(GL_TRIANGLES, cubeIndCnt, GL_UNSIGNED_INT, cubeInd);
            glPopMatrix();
        }
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

void Sun_Show(float k, float alpha)
{
    glPushMatrix();
            glRotatef(-camera.xRot, 1,0,0);
            glRotatef(-camera.zRot, 0,0,1);
            glRotatef(alpha, 0,1,0);
            glTranslatef(0,0,20);
            glDisable(GL_DEPTH_TEST);

            glDisable(GL_TEXTURE_2D);
                glColor3f(1,1-k*0.8,1-k);
                glEnableClientState(GL_VERTEX_ARRAY);
                    glVertexPointer(3, GL_FLOAT, 0, sun);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
                glDisableClientState(GL_VERTEX_ARRAY);
            glEnable(GL_TEXTURE_2D);
            glEnable(GL_DEPTH_TEST);

        glPopMatrix();
}

void Game_Show()
{
    static float alpha = 0;
    alpha += 0.03;
    if (alpha > 180) alpha -= 360;

    #define abs(a) ((a) > 0 ? (a) : -(a))
    float kss = 1 - (abs(alpha) / 180);

    #define sakat 40.0f
    float k = 90 - abs(alpha);
    k = (sakat - abs(k));
    k = k < 0 ? 0 : k / sakat;


    if (selectMode) glClearColor(0,0,0,0);
    else glClearColor(0.6f * kss, 0.7f * kss, 0.8f * kss, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (selectMode)
    {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_LIGHTING);
    }
    else
    {
        glEnable(GL_LIGHTING);
        glEnable(GL_TEXTURE_2D);
    }

    Anim_Move(&animation);

    glPushMatrix();

        if (!selectMode)
            Sun_Show(k, alpha);

        Camera_Apply();

        glPushMatrix();
            glRotatef(alpha, 0,1,0);
            GLfloat pos[] = {0,0,1,0};
            glLightfv(GL_LIGHT0, GL_POSITION, pos);
            float arr[] = {1+k*0.8,1,1,0};
            glLightfv(GL_LIGHT0, GL_DIFFUSE, arr);

            float clr = kss*0.3 + 0.13;
            float arr1[] = {clr, clr, clr, 0};
            glLightModelfv(GL_LIGHT_MODEL_AMBIENT, arr1);
        glPopMatrix();

        if(!selectMode)
        {
            glEnableClientState(GL_VERTEX_ARRAY);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glEnableClientState(GL_NORMAL_ARRAY);

                glVertexPointer(3, GL_FLOAT, 0, map);
                glTexCoordPointer(2, GL_FLOAT, 0, mapUV);
                glColor3f(0.7, 0.7, 0.7);
                glNormalPointer(GL_FLOAT, 0, mapNormal);
                glBindTexture(GL_TEXTURE_2D, tex_pole);
                glDrawElements(GL_TRIANGLES, mapIndCnt, GL_UNSIGNED_INT, mapInd);

            glDisableClientState(GL_NORMAL_ARRAY);
            glDisableClientState(GL_VERTEX_ARRAY);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);

            for (int i = 0; i < treeCnt; i++)
                    Tree_Show(trees[i]);
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

            glVertexPointer(3, GL_FLOAT, 0, plant);
            glTexCoordPointer(2, GL_FLOAT, 0, plantUV);
            glColor3f(0.7, 0.7, 0.7);
            glNormal3f(0,0,1);
            selectArrCnt = 0;
            int selectColor = 1;
            for (int i = 0; i < plantCnt; i++)
            {
                if (selectMode)
                {
                    if ((plantArr[i].type == tex_tree) || (plantArr[i].type == tex_tree2))
                        continue;
                    static const radius = 3;
                    if (  (plantArr[i].x > camera.x - radius)
                        &&(plantArr[i].x < camera.x + radius)
                        &&(plantArr[i].y > camera.y - radius)
                        &&(plantArr[i].y < camera.y + radius))
                    {
                        glColor3ub(selectColor,0,0);
                        selectArr[selectArrCnt].colorInd = selectColor;
                        selectArr[selectArrCnt].plantArr_Ind = i;
                        selectArrCnt++;
                        selectColor++;
                        if(selectColor >= 255)
                            break;
                    }
                    else
                        continue;
                }
                glBindTexture(GL_TEXTURE_2D, plantArr[i].type);
                glPushMatrix();
                    glTranslatef(plantArr[i].x, plantArr[i].y, plantArr[i].z);
                    glScalef(plantArr[i].scale, plantArr[i].scale, plantArr[i].scale);
                    glDrawElements(GL_TRIANGLES, plantIndCnt, GL_UNSIGNED_INT, plantInd);
                glPopMatrix();
            }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);


        DrawCoords(1,1,1);
    glPopMatrix();

}

void Player_Take(HWND hwnd)
{
    selectMode = TRUE;
    Game_Show();
    selectMode = FALSE;

    RECT rect;
    GLubyte clr[3];
    GetClientRect(hwnd,&rect);
    glReadPixels(rect.right / 2, rect.bottom / 2, 1,1, GL_RGB, GL_UNSIGNED_BYTE, clr);
    if (clr[0] > 0)
        for(int i = 0; i < selectArrCnt; i++)
            if(selectArr[i].colorInd == clr[0])
            {
                Anim_Set(&animation, plantArr + selectArr[i].plantArr_Ind);
            }
}

void Player_Move()
{
    Camera_MoveDirection(GetKeyState('W') < 0 ? 1 : (GetKeyState('S') < 0 ? -1 : 0),
                         GetKeyState('D') < 0 ? 1 : (GetKeyState('A') < 0 ? -1 : 0),
                         0.6);
    Camera_MoveByMouse(1440/2, 450, 0.2);
    camera.z = Map_GetHeight(camera.x, camera.y) + 1.7;
}

void WndResize(int x, int y)
{
    glViewport(0,0, x,y);
    float k = (float)x / (float)y;
    float sz = 0.1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-k*sz,k*sz, -k*sz,k*sz, sz, 200);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          1100,
                          700,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    srand(time(NULL));

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    RECT rect;
    GetClientRect(hwnd, &rect);
    WndResize(rect.right, rect.bottom);


    Map_Init();


    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */



            if (GetForegroundWindow() == hwnd)
                    Player_Move();

            Game_Show();

            SwapBuffers(hDC);

            theta += 1.0f;
            Sleep (1);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{

    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_SIZE:
            WndResize(LOWORD(lParam), HIWORD(lParam));
        break;

        case WM_LBUTTONDOWN:
            Player_Take(hwnd);
        break;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

