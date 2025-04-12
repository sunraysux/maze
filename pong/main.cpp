//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"
#include "math.h"
#include "maze.h"

enum class cellType {
    empty, wall,door
};

// ������ ������ ����  
typedef struct {
    float x, y, width, height, speed;
    HBITMAP hBitmap;//����� � ������� ������ 
    cellType cell;
} sprite;

const int maze_width = 25;
const int maze_height = 25;
sprite racket;//������� ������
sprite maze[maze_width][maze_height]; // ��������� ������ ���������

struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
    HWND hWnd;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������

//c����� ����

HBITMAP load(const char* name)
{
    return (HBITMAP)LoadImageA(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}


void genLevel()
{
    for (int i = 0; i < maze_height; i++)
    {
        for (int k = 0; k < maze_width; k++)
        {
            maze[i][k].cell = cellType::empty;
            maze[i][k].width = window.width / maze_width;
            maze[i][k].height = window.height / maze_height;
            maze[i][k].x = i * maze[i][k].width;
            maze[i][k].y = k * maze[i][k].height;

            if (i == 0 || i == maze_height - 1 || k == 0 || k == maze_width - 1)
            {
                maze[i][k].cell = cellType::wall;
            }

            if (i == maze_height / 2 && k > maze_height / 4 && k < maze_height - maze_height / 4)
            {
                maze[i][k].cell = cellType::wall;
            }

        }
    }
}

void genLevel2()
{
    for (int i = 0; i < maze_height; i++) {
        for (int k = 0; k < maze_width; k++) {
            maze[i][k].hBitmap = load("racket_enemy.bmp");
            maze[i][k].cell = cellType::empty;
            maze[i][k].width = window.width / (float)maze_width;
            maze[i][k].height = window.height / (float)maze_height;
            maze[i][k].x = i * maze[i][k].width;
            maze[i][k].y = k * maze[i][k].height;
        }
    }

    auto maze_ = MazeGenerator::generate((maze_width-1)/2, (maze_width-1)/2);
    
    int x = 0;int y = 0;


    for (unsigned i = 0; i < maze_.get()->size(); i++)
    {
        for (unsigned j = 0; j < maze_.get()->at(0).size(); j++)
        {
            auto a = maze_.get()->at(i).at(j);
            if (a == 35) {
                maze[y][x].cell = cellType::wall;
            }
            x++;
        }
        x = 0;;
        y++;

        maze[maze_height -2][maze_width-1].cell = cellType::door;
    }


    for (int i = 0; i < maze_height; i++) 
    {
        for (int k = 0; k < maze_width; k++) 
        {
            if (maze[i][k].cell == cellType::door)
            {
                maze[i][k].hBitmap = load("racket.bmp");
            }
        }
           
    }
}


void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    racket.hBitmap = load("racket.bmp");
    hBack = (HBITMAP)LoadImageA(NULL, "back.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------

    racket.width = 50;
    racket.height = 50;
    racket.speed = 10;//�������� ����������� �������
    racket.x = window.width / 3.;//������� ���������� ����
    racket.y = window.height / 3;//���� ���� ���� ������ - �� ������ �������


    game.score = 0;
    game.balls = 9;
    
    genLevel2();


}

void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
{
    //PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
}

void ShowScore()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    SetBkMode(window.context, TRANSPARENT);
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
    TextOutA(window.context, 10, 10, "Score", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));

    _itoa_s(game.balls, txt, 10);
    TextOutA(window.context, 10, 100, "Balls", 5);
    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT))
    {
        int i = (racket.x - racket.speed) / maze[0][0].width;
        int k = racket.y / maze[0][0].height;
        int k1 = (racket.y + racket.height - 1) / maze[0][0].height;

        if (!(maze[i][k].cell == cellType::wall || maze[i][k1].cell == cellType::wall))
        {
            racket.x -= racket.speed;
        }
        else
        {
            racket.x = (i + 1) * maze[0][0].width;
        }

    }

    if (GetAsyncKeyState(VK_RIGHT))
    {
        int i = (racket.x + racket.speed + racket.width) / maze[0][0].width;
        int k = racket.y / maze[0][0].height;
        int k1 = (racket.y + racket.height - 1) / maze[0][0].height;


        if (!(maze[i][k].cell == cellType::wall || maze[i][k1].cell == cellType::wall))
        {
            racket.x += racket.speed;
        }
        else
        {
            racket.x = i * maze[0][0].width - racket.width;
        }
    }

    if (GetAsyncKeyState(VK_DOWN))
    {
        int i = racket.x / maze[0][0].width;
        int k = (racket.y + racket.speed + racket.height) / maze[0][0].height;
        int i1 = (racket.x + racket.width - 1) / maze[0][0].width;

        if (!(maze[i][k].cell == cellType::wall || maze[i1][k].cell == cellType::wall))
        {
            racket.y += racket.speed;
        }
        else
        {
            racket.y = k * maze[0][0].height - racket.height;
        }
    }
    if (GetAsyncKeyState(VK_UP))
    {
        int i = racket.x / maze[0][0].width;
        int k = (racket.y - racket.speed) / maze[0][0].height;
        int i1 = (racket.x + racket.width - 1) / maze[0][0].width;

        if (!(maze[i][k].cell == cellType::wall || maze[i1][k].cell == cellType::wall))
        {
            racket.y -= racket.speed;
        }
        else
        {
            racket.y = (k + 1) * maze[0][0].height;
        }
    }


    if (!game.action && GetAsyncKeyState(VK_SPACE))
    {
        game.action = true;
        ProcessSound("bounce.wav");
    }
}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (alpha)
        {
            TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
        }
        else
        {
            StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//������ ���

    for (int i = 0; i < maze_height; i++)
    {
        for (int k = 0; k < maze_width; k++)
        {
            if (maze[i][k].cell != cellType::empty)
            {
                ShowBitmap(window.context, maze[i][k].x, maze[i][k].y, maze[i][k].width, maze[i][k].height, maze[i][k].hBitmap);
            }
        }


    }
    ShowBitmap(window.context, racket.x, racket.y, racket.width, racket.height, racket.hBitmap);// ������� ������
}

void LimitRacket()
{
    racket.x = max(racket.x, 0);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
    racket.x = min(racket.x, window.width - racket.width);//���������� ��� ������� ����
    racket.y = max(racket.y, 0);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
    racket.y = min(racket.y, window.height - racket.height);//���������� ��� ������� ����
}







void InitWindow()
{
    SetProcessDPIAware();
    window.hWnd = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);

    RECT r;
    GetClientRect(window.hWnd, &r);
    window.device_context = GetDC(window.hWnd);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.hWnd, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{

    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    //mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    ShowCursor(NULL);

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        ShowScore();//������ ���� � �����
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

        ProcessInput();//����� ����������
        LimitRacket();//���������, ����� ������� �� ������� �� �����
    }

}
