#include <Windows.h>
#include <mmsystem.h> //mci 播放声音
#include <time.h>
#include <tchar.h>

#pragma comment (lib,"Msimg32.lib")
#pragma comment (lib,"winmm.lib") //mci播放声音

#define WINDOW_TITLE L"飞机大战"
#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 852

enum GameState
{
	GS_Menu,
	GS_Playing,
	GS_Result,
};

HDC g_hdc;
HDC g_mdc;
HDC g_bufdc;

int iBakcgroundOffset = 0;
int iHeroIndex = 0;
int iHeroTimer = 0;
int iPlayerPositionX = 190;
int iPlayerPositionY = 600;
int iBulletTimer = 0;
bool bIsMouseDown = false;
GameState gameState;
int iEnemySpawnTimer = 0;
int iScore = 0;

void ChangeToState(GameState gs,HWND hwnd);

struct GameMenu
{
	HBITMAP hBackground;
	HBITMAP hTitle;
	HBITMAP hGameLoading;


	void Init(HWND hwnd)  
	{
		hBackground = (HBITMAP)LoadImage(NULL,L"image/background.bmp",IMAGE_BITMAP,WINDOW_WIDTH,WINDOW_HEIGHT,LR_LOADFROMFILE);
		hTitle = (HBITMAP)LoadImage(NULL,L"image/title.bmp",IMAGE_BITMAP,429,84,LR_LOADFROMFILE);
		hGameLoading = (HBITMAP)LoadImage(NULL,L"image/game_loading.bmp",IMAGE_BITMAP,176,36,LR_LOADFROMFILE);
	}

	void Start(HWND hwnd)
	{
		SelectObject(g_bufdc,hBackground);
		BitBlt(g_mdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_bufdc,0,0,SRCCOPY);


		SelectObject(g_bufdc,hTitle);
		TransparentBlt(g_mdc,20,50,429,84,g_bufdc,0,0,429,84,RGB(0,0,0));

		SelectObject(g_bufdc,hGameLoading);
		TransparentBlt(g_mdc,150,600,176,36,g_bufdc,0,0,176,36,RGB(255,255,255));

		BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_mdc,0,0,SRCCOPY);

	}

	void Update(HWND hwnd)
	{
	
	}

	//处理鼠标和键盘事件
	void OnWindowMessage(HWND hwnd,UINT message,WPARAM wparam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_LBUTTONUP:
			ChangeToState(GS_Playing,hwnd);
			break;
		}
	}


	void Destroy(HWND hwnd)
	{

	}
};


struct Bullet
{
	int x ;
	int y;
	bool isExist ;
};


struct Enemy
{
	int x;
	int y;
	bool isExist;
	bool isDie ;
	int iDieAnimationTimer;
	int iDieAnimationIndex;
};


struct GamePlaying
{
	HBITMAP hBackground;
	HBITMAP hHeroArray[2];

	Bullet bulletArray[30];
	HBITMAP hBullet;
	
	POINT pPreMousePoint;


	Enemy enemyArray[30];
	HBITMAP hEnemyArray[5];


	void Init(HWND hwnd)  
	{
		hBackground = (HBITMAP)LoadImage(NULL,L"image/background.bmp",IMAGE_BITMAP,WINDOW_WIDTH,WINDOW_HEIGHT,LR_LOADFROMFILE);
		hHeroArray[0] = (HBITMAP)LoadImage(NULL,L"image/hero1.bmp",IMAGE_BITMAP,100,124,LR_LOADFROMFILE);
		hHeroArray[1] = (HBITMAP)LoadImage(NULL,L"image/hero2.bmp",IMAGE_BITMAP,100,124,LR_LOADFROMFILE);
		hBullet = (HBITMAP)LoadImage(NULL,L"image/bullet1.bmp",IMAGE_BITMAP,9,21,LR_LOADFROMFILE);
		hEnemyArray[0] = (HBITMAP)LoadImage(NULL,L"image/enemy0.bmp",IMAGE_BITMAP,51,39,LR_LOADFROMFILE);
		hEnemyArray[1] = (HBITMAP)LoadImage(NULL,L"image/enemy1.bmp",IMAGE_BITMAP,51,39,LR_LOADFROMFILE);
		hEnemyArray[2] = (HBITMAP)LoadImage(NULL,L"image/enemy2.bmp",IMAGE_BITMAP,51,39,LR_LOADFROMFILE);
		hEnemyArray[3] = (HBITMAP)LoadImage(NULL,L"image/enemy3.bmp",IMAGE_BITMAP,51,39,LR_LOADFROMFILE);
		hEnemyArray[4] = (HBITMAP)LoadImage(NULL,L"image/enemy4.bmp",IMAGE_BITMAP,51,39,LR_LOADFROMFILE);

		//随机数种子初始化
		srand((unsigned)time(NULL));
	}

	void Start(HWND hwnd)
	{
		mciSendString(L"open sound/game_music.wav",NULL,0,0);
		mciSendString(L"play sound/game_music.wav",NULL,0,0);
	}

	void Update(HWND hwnd)
	{
		//渲染背景
		SelectObject(g_bufdc,hBackground);
		iBakcgroundOffset += 2;

		//重置背景
		if(iBakcgroundOffset > WINDOW_HEIGHT)
		{
			iBakcgroundOffset -= WINDOW_HEIGHT;
		}

		//背景运动
		BitBlt(g_mdc,0,iBakcgroundOffset,WINDOW_WIDTH,WINDOW_HEIGHT,g_bufdc,0,0,SRCCOPY);
		BitBlt(g_mdc,0,-(WINDOW_HEIGHT - iBakcgroundOffset),WINDOW_WIDTH,WINDOW_HEIGHT,g_bufdc,0,0,SRCCOPY);
		
		//渲染主角
		iHeroTimer++;
		if(iHeroTimer >= 10)
		{
			iHeroIndex++;
			iHeroIndex %= 2;
		}
		SelectObject(g_bufdc,hHeroArray[iHeroIndex]);
		TransparentBlt(g_mdc,iPlayerPositionX,iPlayerPositionY,100,124,g_bufdc,0,0,100,124,RGB(255,255,255));

		//控制子弹的生成
		iBulletTimer++;
		if(iBulletTimer%10 == 0)
		{
			for(int i = 0; i < 30 ; i++)
			{
				if(bulletArray[i].isExist == false)
				{
					//生成子弹
					int x = iPlayerPositionX + 50 - 4;
					bulletArray[i].x = x;
					bulletArray[i].y = iPlayerPositionY;
					bulletArray[i].isExist = true;
					break;
				}
			}
		}

		//绘制子弹 控制子弹的运动，控制子弹的死亡
		SelectObject (g_bufdc ,hBullet);
		for(int i = 0; i<30;i++)
		{
			if(bulletArray[i].isExist)
			{
				bulletArray[i].y -= 20;
				if(bulletArray[i].y < -21)
				{
					bulletArray[i].isExist = false;
					continue;
				}
				TransparentBlt(g_mdc,bulletArray[i].x,bulletArray[i].y,9,21,g_bufdc,0,0,9,21,RGB(255,255,255));
			}
		}
		if(bIsMouseDown)
		{
			POINT pNowMousePoint;
			GetCursorPos(&pNowMousePoint);
			int xOffset = pNowMousePoint.x - pPreMousePoint.x;
			int yOffset = pNowMousePoint.y -pPreMousePoint.y;

			iPlayerPositionX += xOffset;
			iPlayerPositionY += yOffset; 
			pPreMousePoint = pNowMousePoint;

			if(iPlayerPositionX < 0)
				iPlayerPositionX=0;

			if(iPlayerPositionX > WINDOW_WIDTH - 100)
				iPlayerPositionX = WINDOW_WIDTH - 100;
			
			if(iPlayerPositionY < 0)
				iPlayerPositionY = 0;

			if(iPlayerPositionY > WINDOW_HEIGHT - 124)
				iPlayerPositionY = WINDOW_HEIGHT - 124;
		}

		//控制敌人的产生
		iEnemySpawnTimer++;
		if(iEnemySpawnTimer % 10 ==0)
		{
			for(int i = 0; i < 30;i++)
			{
				if(enemyArray[i].isExist == false)
				{
					int y = -39;
					int x = rand() %(WINDOW_WIDTH - 51);
					enemyArray[i].x = x;
					enemyArray[i].y = y;
					enemyArray[i].isExist = true;
					enemyArray[i].isDie = false;
					enemyArray[i].iDieAnimationTimer = 0;
					enemyArray[i].iDieAnimationIndex = 0;
					break;
				}
			}
		}

		//控制敌人运动
		for(int i = 0; i < 30; i++)
		{
			if(enemyArray[i].isExist && enemyArray[i].isDie == false)
			{
				enemyArray[i].y += 6;
				if(enemyArray[i].y > WINDOW_HEIGHT)
				{
					enemyArray[i].isExist = false;
				}
			}
		}



		//渲染敌人
		for(int i = 0; i < 30;i++)
		{
			if(enemyArray[i].isExist)
			{
				if(enemyArray[i].isDie)
				{
					//渲染死亡动画(爆炸动画)
					enemyArray[i].iDieAnimationTimer++;
					enemyArray[i].iDieAnimationIndex = enemyArray[i].iDieAnimationTimer / 5;
					if(enemyArray[i].iDieAnimationIndex > 4)
					{
						enemyArray[i].isExist = false;
						enemyArray[i].isDie = false;
						continue;
					}
					SelectObject(g_bufdc,hEnemyArray[enemyArray[i].iDieAnimationIndex]);
					TransparentBlt(g_mdc,enemyArray[i].x,enemyArray[i].y,51,39,g_bufdc,0,0,51,39,RGB(255,255,255));
				}
				else
				{
					SelectObject(g_bufdc,hEnemyArray[0]);
					TransparentBlt(g_mdc,enemyArray[i].x,enemyArray[i].y,51,39,g_bufdc,0,0,51,39,RGB(255,255,255));
				}
			}
		}

		//子弹和敌人的碰撞检测
		for(int i = 0; i < 30; i++)
		{
			if(enemyArray[i].isExist && enemyArray[i].isDie == false)
			{
				for(int j = 0; j < 30; j++ )
				{
					if(bulletArray[j].isExist)
					{
						if(isInclude(enemyArray[i],bulletArray[j].x + 4 ,bulletArray[j].y + 10))
						{
							//发生碰撞
							bulletArray[j].isExist = false;
							enemyArray[i].isDie = true;
							iScore++;
							PlaySound(L"sound/enemy0_down.wav",NULL,SND_FILENAME | SND_ASYNC);
							break;
						}
					}
				}
			}
		}
	
		//主角和敌人的碰撞检测 
		for(int i = 0; i<30;i++)
		{
			if(enemyArray[i].isExist && enemyArray[i].isDie == false)
			{
				bool isInclude1 = isInclude(enemyArray[i],iPlayerPositionX + 50,iPlayerPositionY); 
				bool isInclude2 = isInclude(enemyArray[i],iPlayerPositionX,iPlayerPositionY+80);
				bool isInclude3 = isInclude(enemyArray[i],iPlayerPositionX + 100,iPlayerPositionY + 84);

				if(isInclude1 || isInclude2 || isInclude3)
				{
					//发生碰撞
					mciSendString(L"stop sound/game_music.wav",NULL,0,0);
					PlaySound(L"sound/game_over.wav",NULL,SND_FILENAME | SND_ASYNC);
					ChangeToState(GS_Result,hwnd);
					break;
				}
			}
		}


		BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_mdc,0,0,SRCCOPY);
	}


	bool isInclude(Enemy enemy ,int x ,int y)
	{
		if(x > enemy.x && y > enemy.y && x < enemy.x + 51 && y < enemy.y + 39)
		{
			return true;
		}
		return false;
	}

	//处理鼠标和键盘事件
	void OnWindowMessage(HWND hwnd,UINT message,WPARAM wparam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_LBUTTONDOWN:
			bIsMouseDown = true;
			GetCursorPos(&pPreMousePoint);
			break;
		case WM_LBUTTONUP:
			bIsMouseDown = false;
			break;
		}
	}
	void Destroy(HWND hwnd)
	{

	}
};

struct GameResult
{
	HBITMAP hGameOver;
	wchar_t scoreText[10];

	void Init(HWND hwnd)  
	{
		hGameOver = (HBITMAP) LoadImage(NULL,L"image/gameover.bmp",IMAGE_BITMAP,WINDOW_WIDTH,WINDOW_HEIGHT,LR_LOADFROMFILE);

		HFONT font = CreateFont(40,0,0,0,0,0,0,0,GB2312_CHARSET,0,0,0,0,TEXT("微软雅黑"));
		SelectObject(g_mdc,font);
		SetBkMode(g_mdc,TRANSPARENT); ///设置文字的背景为透明

	}

	void Start(HWND hwnd)
	{
		swprintf_s(scoreText,L"%d",iScore);
		
	}

	void Update(HWND hwnd)
	{
		SelectObject(g_bufdc,hGameOver);

		BitBlt(g_mdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_bufdc,0,0,SRCCOPY);

		//绘制文本
		TextOut(g_mdc,200,600,scoreText,wcslen(scoreText));

		BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_mdc,0,0,SRCCOPY);

	}

	//处理鼠标和键盘事件
	void OnWindowMessage(HWND hwnd,UINT message,WPARAM wparam, LPARAM lParam)
	{

	}

	void Destroy(HWND hwnd)
	{

	}
};

GameMenu gameMenu; //开始界面
GamePlaying gamePlaying; //游戏对战界面 
GameResult gameResult;  //游戏结束界面 


LRESULT CALLBACK WndProc(HWND hwnd,UINT message, WPARAM wparam,LPARAM lParam);
void GameStart(HWND hwnd);
void GameUpdate(HWND hwnd);
void GameEnd(HWND hwnd);

int WINAPI WinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPSTR lpCmdLine,_In_ int nShowCmd)
{

	WNDCLASSEX wndClass = {0};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.lpszClassName = L"myclassname";
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL,IDC_ARROW);


	if(RegisterClassEx(&wndClass) == false)
	{
		return -1;
	}

	HWND hwnd = CreateWindow(L"myclassname",WINDOW_TITLE,WS_OVERLAPPEDWINDOW,CW_USEDEFAULT,CW_USEDEFAULT,WINDOW_WIDTH,WINDOW_HEIGHT,NULL,NULL,hInstance,NULL);

	ShowWindow(hwnd,nShowCmd);
	UpdateWindow(hwnd);
	//进行游戏初始化的代码 
	GameStart(hwnd);

	DWORD tNow = GetTickCount();
	DWORD tPre = GetTickCount();

	//消息处理
	MSG msg = {0};
	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg , 0,NULL ,NULL , PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			tNow = GetTickCount();
			if(tNow - tPre > 10)
			{
				GameUpdate(hwnd);
				tPre = tNow;
			}
		}
	}


	GameEnd(hwnd);
	UnregisterClass(L"myclassname",hInstance);

	return 0;
}


void ChangeToState(GameState gs,HWND hwnd)
{
	gameState = gs;
	switch (gs)
	{
	case GS_Menu:
		gameMenu.Start(hwnd);
		break;
	case GS_Playing:
		gamePlaying.Start(hwnd);
		break;
	case GS_Result:
		gameResult.Start(hwnd);
		break;
	}
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT message, WPARAM wparam,LPARAM lParam)
{
	switch (message)
	{
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_KEYDOWN:
		switch (gameState)
		{
		case GS_Menu:
			gameMenu.OnWindowMessage(hwnd,message,wparam,lParam);
			break;
		case GS_Playing:
			gamePlaying.OnWindowMessage(hwnd,message,wparam,lParam);
			break;
		case GS_Result:
			gameResult.OnWindowMessage(hwnd,message,wparam,lParam);
			break;
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd,message,wparam,lParam);
		break;
	}
	return 0;
}

void GameStart(HWND hwnd)
{
	g_hdc = GetDC(hwnd);
	g_mdc = CreateCompatibleDC(g_hdc);
	g_bufdc = CreateCompatibleDC(g_hdc);

	HBITMAP whiteBmp = CreateCompatibleBitmap(g_hdc,WINDOW_WIDTH,WINDOW_HEIGHT);
	SelectObject(g_mdc,whiteBmp);



	gameMenu.Init(hwnd);
	gamePlaying.Init(hwnd);
	gameResult.Init(hwnd);

	//设置默认状态
	gameState = GS_Menu;
	gameMenu.Start(hwnd);
}

void GameUpdate(HWND hwnd)
{
	switch (gameState)
	{
	case GS_Menu:
		gameMenu.Update(hwnd);
		break;
	case GS_Playing:
		gamePlaying.Update(hwnd);
		break;
	case GS_Result:
		gameResult.Update(hwnd);
		break;
	default:
		break;
	}
}

void GameEnd(HWND hwnd)
{
	DeleteDC(g_bufdc);
	DeleteDC(g_mdc);
	ReleaseDC(hwnd,g_hdc);

	gameMenu.Destroy(hwnd);
	gamePlaying.Destroy(hwnd);
	gameResult.Destroy(hwnd);
}