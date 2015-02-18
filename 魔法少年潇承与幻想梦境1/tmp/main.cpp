#include <windows.h>
#include <time.h>
#include <set>
#define KEYDOWN(KEY) (0x8000 & GetKeyState(KEY))
#define WELCOME 0
#define CHOOSE 1
#define GAME 2
#define WIN 3
#define LOSE 4
#define REST 5

using namespace std;

int sqr(int x)
{
	return x*x;
}

struct Shot
{
	unsigned long long id;
	int jx, jy, x, y, w, h, r, hurt, timer, tx, ty, cnt;
	bool uneraseable, unbreakable, clearagg, hitboss;
	int (*dx)(Shot*), (*dy)(Shot*);
	HBITMAP img[4];

	bool operator <(const Shot b)const
	{
		return id < b.id;
	}

	void inc()
	{
		timer++;
	}

	void proc()
	{
		int px = (*dx)(this), py = (*dy)(this);
		x += px, y += py;
	}
}marisaNormal, WXCNormal, starshotShot, dimlightarrowShot, finalsparkShot, doomShot, 
 bluefastchase, redslowstraight, blueslowstraight, yinyangShot, swordShot, explode;
set<Shot> friendly, aggresive;

struct Skill
{
	bool lock;
	int cost, cd, curcd, timer;
	ULONGLONG last;
	HBITMAP name;
	void (*castfunc)(Skill*), (*procfunc)(Skill*);

	void inc()
	{
		if (timer != -1)timer++;
		if (curcd) curcd--;
	}

	void proc()
	{ 
		if(timer != -1)
			(*procfunc)(this);
	}

	void cast()
	{
		if (curcd == 0)
		{
			(*castfunc)(this);
		}
	}
}starshot, dimlightarrow, chakara, blink, finalspark, doom,
 dreamcast0, yinyangyu, doubleenchantment,
 swords, charge;

struct Boss
{
	HBITMAP img;
	int x, y, r, w, h, blood[10], bloodlimit[10], time[10], ms, tot, cur, timer, tx, ty, dx, dy;
	bool exist;
	Skill s[10];

	void inc()
	{
		timer++;
	}

	void proc()
	{
		if (sqr(x + (w >> 1) - dx) + sqr(y + (h >> 1) - dy) > sqr(ms))
		{
			x += ms * (dx - x - (w >> 1)) / (abs(dx - x - (w >> 1)) + abs(dy - y - (h >> 1)));
			y += ms * (dy - y - (h >> 1)) / (abs(dx - x - (w >> 1)) + abs(dy - y - (h >> 1)));
		}

		s[cur].cast();
	}

	void moveto(int kx, int ky)
	{
		dx = kx, dy = ky;
	}
}boss,bosses[7];

struct Hero
{
	HBITMAP img;
	int x, y, r, w, h, md, ms, rest, power, dmagic, magic, magiclimit, timer, protect;
	Skill s1, s2, s3;
	Shot shot;
	bool shooting, guard;

	void inc()
	{
		timer++;
	}
}hero;

const int initdx[] = { 30, 15, -15, -30, -30, -15, 15, 30 }, initdy[] = { 15, 30, 30, 15, -15, -30, -30, -15 };
WNDCLASSEX wcex;
HINSTANCE hInst;
HBITMAP bg, frame, title, bmp, optionimg[2], choosehero[2], gamehero[2],
        wxcguard, wxcskill3, doomtag, doomimg, markup, markright, curname, bossname, resticon, winscreen, losescreen, restscreen;
HDC hdc, mdc, bufdc;
HWND hWnd;
ULONGLONG tPre, tNow, shotid;
int curstat, option, stage, heroSkillNameTimer, bossSkillNameTimer, shakeTimer, doomTimer;

void Transparent(HDC, int, int, int, int, HDC, int, int, int, int, UINT);
ATOM Register(HINSTANCE);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void buildWelcome();
void buildChoose();
void buildGame();
void buildRest();
void buildWin();
void buildLose();
void loadImage();
void loadStage(int);
void welcomeProc();
void chooseProc();
void loadProc();
void gameProc(UINT);
void timerProc();
void initDC();
void initBoss();
void initShot();
void initSkill();
void paint();
void paint(int);

//+shots

int normalDX(Shot *s)
{
	return 0;
}

int normalDY(Shot *s)
{
	return -20;
}

int starshotDX(Shot *s)
{
	if (boss.exist)
	{
		if (s->timer < 1)return 0;
		if (s->timer == 1)s->tx = hero.x - s->x;
		if (s->timer < 60)return hero.x - s->tx - s->x;
		return (s->timer - 60) * (boss.x + (boss.w >> 1) - s->x - s->jx) / (abs(boss.x + (boss.w >> 1) - s->x - s->jx) + abs(boss.y + (boss.h >> 1) - s->y - s->jy));
	}
	else
		return 0;
}

int starshotDY(Shot *s)
{
	if (boss.exist)
	{
		if (s->timer < 1)return 0;
		if (s->timer == 1)s->ty = hero.y - s->y;
		if (s->timer < 60)return hero.y - s->ty - s->y;
		return (s->timer - 60) * (boss.y + (boss.h >> 1) - s->y - s->jy) / (abs(boss.x + (boss.w >> 1) - s->x - s->jx) + abs(boss.y + (boss.h >> 1) - s->y - s->jy));
		return 0;
	}
	else
		return s->timer;
}

int dimlightarrowDX(Shot *s)
{
	return 0;
}

int dimlightarrowDY(Shot *s)
{
	return -(s->timer/3);
}

int finalsparkDX(Shot *s)
{
	if (s->timer < 1)return 0;
	if (s->timer == 1)s->tx = hero.x - s->x;
	return hero.x - s->tx - s->x;
}

int finalsparkDY(Shot *s)
{
	if (s->timer < 1)return 0;
	if (s->timer == 1)s->ty = hero.y - s->y;
	if (s->timer < 250)return hero.y - s->ty - s->y;
	s->uneraseable = false;
	return 1000;
}

int fastchaseDX(Shot *s)
{
	if (s->timer < 80)return 0;
	if(s->timer == 80)s->tx = 6 * (hero.x + (hero.w >> 1) - s->x) / (abs(hero.x + (hero.w >> 1) - s->x) + abs(hero.y + (hero.h >> 1) - s->y));
	return s->tx;
}

int fastchaseDY(Shot *s)
{
	if (s->timer < 80)return 0;
	if(s->timer == 80)s->ty = 6 * (hero.y + (hero.h >> 1) - s->y) / (abs(hero.x + (hero.w >> 1) - s->x) + abs(hero.y + (hero.h >> 1) - s->y));
	return s->ty;
}

int slowstraightDX(Shot *s)
{
	return s->tx;
}

int slowstraightDY(Shot *s)
{
	return s->ty;
}

int yinyangDX(Shot *s)
{
	if (s->x < 5 || s->x > 405)s->tx = -s->tx;
	return s->tx;
}

int yinyangDY(Shot *s)
{
	return s->ty;
}

int swordDX(Shot *s)
{
	return 0;
}

int swordDY(Shot *s)
{
	if (s->timer <= 7)return 7 - s->timer;
	if (s->timer <= 60)return 0;
	return s->timer - 60;
}

int explodeDX(Shot *s)
{
	return 0;
}

int explodeDY(Shot *s)
{
	if (s->timer < 100)return 0;
	return 1000;
}

//-shots

//+skills

void dreamcast0Cast(Skill *s)
{
	s->curcd = s->cd;
	bossSkillNameTimer = 300;
	bossname = s->name;
	s->timer = 0;
}

void dreamcast0Proc(Skill *s)
{
	if (s->timer % 210 == 0)
	{
		boss.moveto(rand() % 355 + 20, rand() % 80 + 20);
	}

	if (s->timer < 70 && s->timer % 7 == 0)
	{
		Shot cur = bluefastchase;
		
		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1) + (rand() % 81 - 40);
		cur.y = boss.y + (boss.h >> 1) + (rand() % 81 - 40);
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1) + (rand() % 81 - 40);
		cur.y = boss.y + (boss.h >> 1) + (rand() % 81 - 40);
		aggresive.insert(cur);
	}

	if (s->timer % 30 == 0)
	{
		Shot cur = redslowstraight;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);

		cur.id = shotid++;
		cur.tx = 2;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = 1;
		cur.ty = 2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -1;
		cur.ty = 2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -2;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -2;
		cur.ty = -1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -1;
		cur.ty = -2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = 1;
		cur.ty = -2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = 2;
		cur.ty = -1;
		aggresive.insert(cur);
	}

	if (s->timer == 210)
		s->timer = -1;
}

void yinyangyuCast(Skill *s)
{
	s->curcd = s->cd;
	bossSkillNameTimer = 300;
	bossname = s->name;
	s->timer = 0;
}

void yinyangyuProc(Skill *s)
{	
	if (s->timer % 70 == 0)
	{
		boss.moveto(hero.x , 100);

		Shot cur = yinyangShot;
		int dir = rand() % 2 + 1;

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);
		cur.tx = dir+1;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);
		cur.tx = -dir-1;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);
		cur.tx = dir;
		cur.ty = 2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);
		cur.tx = -dir;
		cur.ty = 2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);
		cur.tx = 0;
		cur.ty = 3;
		aggresive.insert(cur);
	}

	if (s->timer % 30 == 0)
	{
		Shot cur = blueslowstraight;
		cur.x = boss.x + (boss.w >> 1);
		cur.y = boss.y + (boss.h >> 1);

		cur.id = shotid++;
		cur.tx = 2;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = 1;
		cur.ty = 2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -1;
		cur.ty = 2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -2;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -2;
		cur.ty = -1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = -1;
		cur.ty = -2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = 1;
		cur.ty = -2;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.tx = 2;
		cur.ty = -1;
		aggresive.insert(cur);
	}

	if (s->timer == 210)
		s->timer = -1;
}

void doubleenchantmentCast(Skill *s)
{
	s->curcd = s->cd;
	bossSkillNameTimer = 300;
	bossname = s->name;
	boss.moveto(195, 240);
	s->timer = 0;
}

void doubleenchantmentProc(Skill *s)
{
	if (s->timer % 30 == 0)
	{
		Shot cur = blueslowstraight;
		
		cur.id = shotid++;
		cur.x = 10;
		cur.y = 205;
		cur.tx = 1;
		cur.ty = -1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = 205;
		cur.y = 0;
		cur.tx = 1;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = 410;
		cur.y = 270;
		cur.tx = -1;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = 210;
		cur.y = 470;
		cur.tx = -1;
		cur.ty = -1;
		aggresive.insert(cur);

		cur = redslowstraight;

		cur.id = shotid++;
		cur.x = 0;
		cur.y = 90;
		cur.tx = 1;
		cur.ty = 0;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = 300;
		cur.y = 0;
		cur.tx = 0;
		cur.ty = 1;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = 395;
		cur.y = 380;
		cur.tx = -1;
		cur.ty = 0;
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = 90;
		cur.y = 480;
		cur.tx = 0;
		cur.ty = -1;
		aggresive.insert(cur);
	}

	if (s->timer % 100 == 0)
	{
		Shot cur = bluefastchase;

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1) + (rand() % 81 - 40);
		cur.y = boss.y + (boss.h >> 1) + (rand() % 81 - 40);
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1) + (rand() % 81 - 40);
		cur.y = boss.y + (boss.h >> 1) + (rand() % 81 - 40);
		aggresive.insert(cur);

		cur.id = shotid++;
		cur.x = boss.x + (boss.w >> 1) + (rand() % 81 - 40);
		cur.y = boss.y + (boss.h >> 1) + (rand() % 81 - 40);
		aggresive.insert(cur);
	}

	if (s->timer == 300)
		s->timer = -1;
}

void swordsCast(Skill *s)
{
	s->curcd = s->cd;
	bossSkillNameTimer = 300;
	bossname = s->name;
	boss.moveto(195, 100);
	s->timer = 0;
}

void swordsProc(Skill *s)
{
	if (s->timer % 5 == 0)
	{
		Shot cur = swordShot;

		cur.id = shotid++;
		cur.x = (rand() % 416 + 5);
		cur.y = 0;
		aggresive.insert(cur);
	}

	if (s->timer == 150)
		s->timer = -1;
}

void chargeCast(Skill *s)
{
	s->curcd = s->cd;
	bossSkillNameTimer = 300;
	bossname = s->name;
	s->timer = 0;
}

void chargeProc(Skill *s)
{
	if (s->timer == 100)
	{
		boss.moveto(hero.x + (hero.w - boss.w >> 1), hero.y + (hero.h - boss.h >> 1));
	}

	if (s->timer == 150)
	{
		Shot cur = explode;
		cur.x = boss.x + (boss.w >> 1) - cur.r;
		cur.y = boss.y + (boss.h >> 1) - cur.r;
		aggresive.insert(cur);
		shakeTimer = 100;

		s->timer = -1;
	}
}

void starshotCast(Skill *s)
{
	if (hero.magic >= s->cost)
	{
		hero.magic -= s->cost;
		s->curcd = s->cd;

		Shot cur = starshotShot;

		if (boss.exist)
		{
			for (int i = 0; i < 8; i++)
			{
				cur.id = shotid++;
				cur.x = hero.x + (hero.w >> 1) + initdx[i];
				cur.y = hero.y + (hero.h >> 1) + initdy[i];
				friendly.insert(cur);
			}
		}
		else
		{
			for (int p = 10; p <= 420; p += 50)
			{
				cur.id = shotid++;
				cur.x = p, cur.y = 445;
				friendly.insert(cur);
			}
		}

		heroSkillNameTimer = 300;
		curname = s->name;
	}
}

void starshotProc(Skill *s)
{
	
}

void dimlightarrowCast(Skill *s)
{
	if (hero.magic >= s->cost)
	{
		hero.magic -= s->cost;
		s->curcd = s->cd;
		hero.s1.timer = 0;

		heroSkillNameTimer = 300;
		curname = s->name;
	}
}

void dimlightarrowProc(Skill *s)
{
	if (s->timer >= 200 && s->timer % 10 == 0)
	{
		Shot cur = dimlightarrowShot;
		cur.id = shotid++;
		cur.x = hero.x + (hero.w>>1) - (cur.w>>1);
		cur.y = hero.y;
		friendly.insert(cur);
	}
	
	if (s->timer == 260)
		s->timer = -1;
}

void chakaraCast(Skill *s)
{
	if (hero.magic >= s->cost)
	{
		hero.magic -= s->cost;
		s->curcd = s->cd;
		
		hero.magic += 300;
		hero.s1.curcd = hero.s3.curcd = 0;

		heroSkillNameTimer = 300;
		curname = s->name;
	}
}

void chakaraProc(Skill *s)
{
	
}

void blinkCast(Skill *s)
{
	if (hero.magic >= s->cost)
	{
		hero.magic -= s->cost;
		s->curcd = s->cd;

		if (boss.exist)
		{
			hero.x = boss.x + (boss.w >> 1) - (hero.w >> 1);
			hero.y = 445;
		}
		else
		{
			hero.x = 195;
			hero.y = 410;
		}

		hero.img = wxcguard;
		hero.guard = true;

		s->timer = 0;
		if (hero.s1.timer >= 0 && hero.s1.timer < 198)hero.s1.timer = 198;

		heroSkillNameTimer = 300;
		curname = s->name;
	}
}

void blinkProc(Skill *s)
{
	if (s->timer == 180)
	{
		hero.img = gamehero[1];
		hero.guard = false;
		s->timer = -1;
	}
}

void finalsparkCast(Skill *s)
{
	if (hero.magic >= s->cost)
	{
		hero.magic -= s->cost;
		s->curcd = s->cd;

		hero.guard = true;
		s->timer = 0;

		Shot cur = finalsparkShot;
		cur.x = hero.x + (hero.w>>1) - (cur.w>>1);
		cur.y = hero.y - cur.h;
		cur.uneraseable = true;
		friendly.insert(cur);

		heroSkillNameTimer = 300;
		shakeTimer = 200;
		curname = s->name;
	}
}

void finalsparkProc(Skill *s)
{
	if (s->timer == 200)
	{
		hero.guard = false;
		s->timer = -1;
	}
}

void doomCast(Skill *s)
{
	
	if (hero.magic >= s->cost)
	{
		hero.magic -= s->cost;
		s->curcd = s->cd;
	
		aggresive.clear();
		doomTimer = 300;
		shakeTimer = 300;
		s->timer = 0;

		heroSkillNameTimer = 300;
		curname = s->name;
	}
}

void doomProc(Skill *s)
{
	set<Shot>tmp = aggresive; aggresive.clear();
	
	int x2 = hero.x + (hero.w >> 1), y2 = hero.y + (hero.h >> 1);
	for (set<Shot>::iterator i = tmp.begin(); i != tmp.end(); i++)
	{
		Shot cur = *i;
		int x1 = cur.x + cur.jx, y1 = cur.y + cur.jy;
		
		if (sqr(x1 - x2) + sqr(y1 - y2) > sqr(s->timer>>2))
			aggresive.insert(cur);
	}

	if (boss.exist)
	{
		if (boss.bloodlimit[boss.cur] / boss.blood[boss.cur] >= 4)
		{
			Shot cur = doomShot;
			cur.hurt = boss.blood[boss.cur] + 100;
			cur.x = hero.x + (hero.w >> 1) - 75;
			cur.y = hero.y + (hero.h >> 1) - 75;
			friendly.insert(cur);

			s->timer = -1;
			doomTimer = 0;
			shakeTimer = 150;
		}
	}

	if (s->timer == 300)
	{
		Shot cur = doomShot;
		cur.x = hero.x + (hero.w >> 1) - 75;
		cur.y = hero.y + (hero.h >> 1) - 75;
		friendly.insert(cur);

		s->timer = -1;
		doomTimer = 0;
		shakeTimer = 150;
	}
}

//-skills

INT WINAPI wWinMain(HINSTANCE hInstance,
	HINSTANCE prevInstance,
	LPWSTR cmdLine,
	int cmdShow)
{
	MSG msg;

	Register(hInstance);
	if (!InitInstance(hInstance, cmdShow))
		return FALSE;

	SetTimer(hWnd, 1, 10, NULL);
	srand(time(0));

	loadImage();
	initDC();
	initShot();
	initSkill();
	initBoss();
	paint();

	GetMessage(&msg, NULL, NULL, NULL);
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			TranslateMessage(&msg),
			DispatchMessage(&msg);
		else
		{
			tNow = GetTickCount64();
			if (tNow - tPre >= 10)
				paint();
		}
	}

	return msg.wParam;
}

void Transparent(HDC hdcDest, int nXOriginDest, int nYOriginDest, int nWidthDest, int nHeightDest,
				 HDC hdcSrc, int nXOriginSrc, int nYOriginSrc, int nWidthSrc, int nHeightSrc,
				 UINT crTransparent)
{
	HBITMAP hOldImageBMP, hImageBMP = CreateCompatibleBitmap(hdcDest, nWidthDest, nHeightDest);
	HBITMAP hOldMaskBMP, hMaskBMP = CreateBitmap(nWidthDest, nHeightDest, 1, 1, NULL);
	HDC     hImageDC = CreateCompatibleDC(hdcDest);
	HDC     hMaskDC = CreateCompatibleDC(hdcDest);
	hOldImageBMP = (HBITMAP)SelectObject(hImageDC, hImageBMP);
	hOldMaskBMP = (HBITMAP)SelectObject(hMaskDC, hMaskBMP);

	if (nWidthDest == nWidthSrc && nHeightDest == nHeightSrc)
		BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hdcSrc, nXOriginSrc, nYOriginSrc, SRCCOPY);
	else
		StretchBlt(hImageDC, 0, 0, nWidthDest, nHeightDest,
		hdcSrc, nXOriginSrc, nYOriginSrc, nWidthSrc, nHeightSrc, SRCCOPY);

	SetBkColor(hImageDC, crTransparent);

	BitBlt(hMaskDC, 0, 0, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCCOPY);

	SetBkColor(hImageDC, RGB(0, 0, 0));
	SetTextColor(hImageDC, RGB(255, 255, 255));
	BitBlt(hImageDC, 0, 0, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	SetBkColor(hdcDest, RGB(255, 255, 255));
	SetTextColor(hdcDest, RGB(0, 0, 0));
	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hMaskDC, 0, 0, SRCAND);

	BitBlt(hdcDest, nXOriginDest, nYOriginDest, nWidthDest, nHeightDest, hImageDC, 0, 0, SRCPAINT);

	SelectObject(hImageDC, hOldImageBMP);
	DeleteDC(hImageDC);
	SelectObject(hMaskDC, hOldMaskBMP);
	DeleteDC(hMaskDC);
	DeleteObject(hImageBMP);
	DeleteObject(hMaskBMP);
}

ATOM Register(HINSTANCE hInstance)
{
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = (WNDPROC)WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"canvas";
	wcex.hIconSm = NULL;

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int cmdShow)
{
	hInst = hInstance;
	hWnd = CreateWindow(L"canvas",
		L"魔法少年潇承与幻想梦境(东方魔神传) 1 - 人间篇",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return FALSE;

	MoveWindow(hWnd, 100, 100, 640, 520, SWP_NOZORDER);
	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

void initDC()
{
	hdc = GetDC(hWnd);
	mdc = CreateCompatibleDC(hdc);
	bufdc = CreateCompatibleDC(hdc);
	bmp = CreateCompatibleBitmap(hdc, 640, 480);
	SelectObject(mdc, bmp);
}

void initShot()
{
	marisaNormal.hurt = 19;
	marisaNormal.jx = 4;
	marisaNormal.jy = 8;
	marisaNormal.w = 8;
	marisaNormal.h = 15;
	marisaNormal.r = 4;
	marisaNormal.dx = normalDX;
	marisaNormal.dy = normalDY;
	marisaNormal.unbreakable = false;
	marisaNormal.uneraseable = false;
	marisaNormal.hitboss = false;
	marisaNormal.cnt = 1;
	marisaNormal.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\Marisanormal.bmp", IMAGE_BITMAP, 8, 15, LR_LOADFROMFILE);

	WXCNormal.hurt = 30;
	WXCNormal.jx = 4;
	WXCNormal.jy = 8;
	WXCNormal.w = 8;
	WXCNormal.h = 15;
	WXCNormal.r = 4;
	WXCNormal.dx = normalDX;
	WXCNormal.dy = normalDY;
	WXCNormal.unbreakable = false;
	WXCNormal.uneraseable = false;
	WXCNormal.hitboss = false;
	WXCNormal.cnt = 1;
	WXCNormal.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCnormal.bmp", IMAGE_BITMAP, 8, 15, LR_LOADFROMFILE);

	starshotShot.hurt = 260;
	starshotShot.jx = 6;
	starshotShot.jy = 6;
	starshotShot.w = 11;
	starshotShot.h = 11;
	starshotShot.r = 6;
	starshotShot.dx = starshotDX;
	starshotShot.dy = starshotDY;
	starshotShot.unbreakable = false;
	starshotShot.uneraseable = false;
	starshotShot.hitboss = false;
	starshotShot.cnt = 2;
	starshotShot.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\MarisaSkill1_1.bmp", IMAGE_BITMAP, 11, 11, LR_LOADFROMFILE);
	starshotShot.img[1] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\MarisaSkill1_2.bmp", IMAGE_BITMAP, 11, 11, LR_LOADFROMFILE);

	dimlightarrowShot.hurt = 310;
	dimlightarrowShot.jx = 4;
	dimlightarrowShot.jy = 5;
	dimlightarrowShot.w = 9;
	dimlightarrowShot.h = 40;
	dimlightarrowShot.r = 5;
	dimlightarrowShot.dx = dimlightarrowDX;
	dimlightarrowShot.dy = dimlightarrowDY;
	dimlightarrowShot.unbreakable = true;
	dimlightarrowShot.uneraseable = false;
	dimlightarrowShot.hitboss = false;
	dimlightarrowShot.cnt = 1;
	dimlightarrowShot.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCSkill1.bmp", IMAGE_BITMAP, 9, 40, LR_LOADFROMFILE);

	finalsparkShot.hurt = 37;
	finalsparkShot.jx = 450;
	finalsparkShot.jy = 450;
	finalsparkShot.w = 900;
	finalsparkShot.h = 900;
	finalsparkShot.r = 450;
	finalsparkShot.dx = finalsparkDX;
	finalsparkShot.dy = finalsparkDY;
	finalsparkShot.unbreakable = true;
	finalsparkShot.uneraseable = true;
	finalsparkShot.clearagg = true;
	finalsparkShot.hitboss = false;
	finalsparkShot.cnt = 1;
	finalsparkShot.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\MarisaSkill3.bmp", IMAGE_BITMAP, 900, 900, LR_LOADFROMFILE);

	doomShot.hurt = 1500;
	doomShot.jx = 75;
	doomShot.jy = 75;
	doomShot.w = 150;
	doomShot.h = 150;
	doomShot.r = 75;
	doomShot.dx = starshotDX;
	doomShot.dy = starshotDY;
	doomShot.unbreakable = false;
	doomShot.uneraseable = false;
	doomShot.clearagg = true;
	doomShot.hitboss = false;
	doomShot.cnt = 1;
	doomShot.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\doomShot.bmp", IMAGE_BITMAP, 150, 150, LR_LOADFROMFILE);

	bluefastchase.hurt = 10000;
	bluefastchase.jx = 12;
	bluefastchase.jy = 12;
	bluefastchase.w = 23;
	bluefastchase.h = 23;
	bluefastchase.r = 12;
	bluefastchase.dx = fastchaseDX;
	bluefastchase.dy = fastchaseDY;
	bluefastchase.unbreakable = false;
	bluefastchase.uneraseable = false;
	bluefastchase.hitboss = false;
	bluefastchase.cnt = 1;
	bluefastchase.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\bluebigshot.bmp", IMAGE_BITMAP, 23, 23, LR_LOADFROMFILE);

	redslowstraight.hurt = 10000;
	redslowstraight.jx = 5;
	redslowstraight.jy = 5;
	redslowstraight.w = 11;
	redslowstraight.h = 11;
	redslowstraight.r = 6;
	redslowstraight.dx = slowstraightDX;
	redslowstraight.dy = slowstraightDY;
	redslowstraight.unbreakable = false;
	redslowstraight.uneraseable = false;
	redslowstraight.hitboss = false;
	redslowstraight.cnt = 1;
	redslowstraight.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\redsmallshot.bmp", IMAGE_BITMAP, 11, 11, LR_LOADFROMFILE);

	blueslowstraight.hurt = 10000;
	blueslowstraight.jx = 5;
	blueslowstraight.jy = 5;
	blueslowstraight.w = 11;
	blueslowstraight.h = 11;
	blueslowstraight.r = 6;
	blueslowstraight.dx = slowstraightDX;
	blueslowstraight.dy = slowstraightDY;
	blueslowstraight.unbreakable = false;
	blueslowstraight.uneraseable = false;
	blueslowstraight.hitboss = false;
	blueslowstraight.cnt = 1;
	blueslowstraight.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\bluesmallshot.bmp", IMAGE_BITMAP, 11, 11, LR_LOADFROMFILE);

	yinyangShot.hurt = 10000;
	yinyangShot.jx = 12;
	yinyangShot.jy = 12;
	yinyangShot.w = 23;
	yinyangShot.h = 23;
	yinyangShot.r = 12;
	yinyangShot.dx = yinyangDX;
	yinyangShot.dy = yinyangDY;
	yinyangShot.unbreakable = false;
	yinyangShot.uneraseable = false;
	yinyangShot.hitboss = false;
	yinyangShot.cnt = 4;
	yinyangShot.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\spellshot_1.bmp", IMAGE_BITMAP, 23, 23, LR_LOADFROMFILE);
	yinyangShot.img[1] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\spellshot_2.bmp", IMAGE_BITMAP, 23, 23, LR_LOADFROMFILE);
	yinyangShot.img[2] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\spellshot_3.bmp", IMAGE_BITMAP, 23, 23, LR_LOADFROMFILE);
	yinyangShot.img[3] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\spellshot_4.bmp", IMAGE_BITMAP, 23, 23, LR_LOADFROMFILE);
	
	swordShot.hurt = 10000;
	swordShot.jx = 8;
	swordShot.jy = 40;
	swordShot.w = 18;
	swordShot.h = 80;
	swordShot.r = 12;
	swordShot.dx = swordDX;
	swordShot.dy = swordDY;
	swordShot.unbreakable = false;
	swordShot.uneraseable = false;
	swordShot.hitboss = false;
	swordShot.cnt = 1;
	swordShot.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\swordshot.bmp", IMAGE_BITMAP, 18, 80, LR_LOADFROMFILE);

	explode.hurt = 10000;
	explode.jx = 150;
	explode.jy = 150;
	explode.w = 300;
	explode.h = 300;
	explode.r = 150;
	explode.dx = explodeDX;
	explode.dy = explodeDY;
	explode.unbreakable = true;
	explode.hitboss = false;
	explode.cnt = 1;
	explode.img[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\explode.bmp", IMAGE_BITMAP, 300, 300, LR_LOADFROMFILE);
}

void initSkill()
{
	starshot.lock = 0;
	starshot.cd = 600;
	starshot.curcd = 0;
	starshot.cost = 180;
	starshot.timer = -1;
	starshot.castfunc = starshotCast;
	starshot.procfunc = starshotProc;
	starshot.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\MarisaSkill1name.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);
	
	dimlightarrow.lock = 0;
	dimlightarrow.cd = 250;
	dimlightarrow.curcd = 0;
	dimlightarrow.cost = 50;
	dimlightarrow.timer = -1;
	dimlightarrow.castfunc = dimlightarrowCast;
	dimlightarrow.procfunc = dimlightarrowProc;
	dimlightarrow.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCSkill1name.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	chakara.lock = 0;
	chakara.cd = 2000;
	chakara.curcd = 0;
	chakara.cost = 200;
	chakara.timer = -1;
	chakara.castfunc = chakaraCast;
	chakara.procfunc = chakaraProc;
	chakara.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\marisaSkill2name.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	blink.lock = 0;
	blink.cd = 400;
	blink.curcd = 0;
	blink.cost = 120;
	blink.timer = -1;
	blink.castfunc = blinkCast;
	blink.procfunc = blinkProc;
	blink.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCSkill2name.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	finalspark.lock = 0;
	finalspark.cd = 2000;
	finalspark.curcd = 0;
	finalspark.cost = 300;
	finalspark.timer = -1;
	finalspark.castfunc = finalsparkCast;
	finalspark.procfunc = finalsparkProc;
	finalspark.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\marisaSkill3name.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);
	
	doom.lock = 0;
	doom.cd = 1700;
	doom.curcd = 0;
	doom.cost = 80;
	doom.timer = -1;
	doom.castfunc = doomCast;
	doom.procfunc = doomProc;
	doom.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCSkill3name.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	dreamcast0.lock = 0;
	dreamcast0.cd = 210;
	dreamcast0.curcd = 0;
	dreamcast0.cost = 0;
	dreamcast0.timer = -1;
	dreamcast0.castfunc = dreamcast0Cast;
	dreamcast0.procfunc = dreamcast0Proc;
	dreamcast0.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\dreamcast0.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	yinyangyu.lock = 0;
	yinyangyu.cd = 210;
	yinyangyu.curcd = 0;
	yinyangyu.cost = 0;
	yinyangyu.timer = -1;
	yinyangyu.castfunc = yinyangyuCast;
	yinyangyu.procfunc = yinyangyuProc;
	yinyangyu.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\mengxiangmiaozhu.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	doubleenchantment.lock = 0;
	doubleenchantment.cd = 300;
	doubleenchantment.curcd = 0;
	doubleenchantment.cost = 0;
	doubleenchantment.timer = -1;
	doubleenchantment.castfunc = doubleenchantmentCast;
	doubleenchantment.procfunc = doubleenchantmentProc;
	doubleenchantment.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\doubleenchantment.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	swords.lock = 0;
	swords.cd = 150;
	swords.curcd = 0;
	swords.cost = 0;
	swords.timer = -1;
	swords.castfunc = swordsCast;
	swords.procfunc = swordsProc;
	swords.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\precious.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);

	charge.lock = 0;
	charge.cd = 150;
	charge.curcd = 0;
	charge.cost = 0;
	charge.timer = -1;
	charge.castfunc = chargeCast;
	charge.procfunc = chargeProc;
	charge.name = (HBITMAP)LoadImage(NULL, L"..\\pictures\\charge.bmp", IMAGE_BITMAP, 480, 24, LR_LOADFROMFILE);
}

void initBoss()
{
	bosses[1].x = 195;
	bosses[1].y = 100;
	bosses[1].w = 40;
	bosses[1].h = 76;
	bosses[1].r = 21;
	bosses[1].dx = 195;
	bosses[1].dy = 100;
	bosses[1].ms = 12;
	bosses[1].blood[0] = bosses[1].bloodlimit[0] = 20000;
	bosses[1].blood[1] = bosses[1].bloodlimit[1] = 20000;
	bosses[1].blood[2] = bosses[1].bloodlimit[2] = 20000;
	bosses[1].s[0] = dreamcast0;
	bosses[1].s[1] = yinyangyu;
	bosses[1].s[2] = doubleenchantment;
	bosses[1].tot = 3;
	bosses[1].cur = 0;
	bosses[1].img = (HBITMAP)LoadImage(NULL, L"..\\pictures\\boss1.bmp", IMAGE_BITMAP, 40, 76, LR_LOADFROMFILE);
	bosses[1].exist = 1;

	bosses[2].x = 195;
	bosses[2].y = 100;
	bosses[2].w = 40;
	bosses[2].h = 76;
	bosses[2].r = 21;
	bosses[2].dx = 195;
	bosses[2].dy = 100;
	bosses[2].ms = 12;
	bosses[2].blood[0] = bosses[2].bloodlimit[0] = 20000;
	bosses[2].blood[1] = bosses[2].bloodlimit[1] = 20000;
	//bosses[2].blood[2] = bosses[2].bloodlimit[2] = 20000;
	bosses[2].s[0] = swords;
	bosses[2].s[1] = charge;
	//bosses[2].s[2] = doubleenchantment;
	bosses[2].tot = 2;
	bosses[2].cur = 0;
	bosses[2].img = (HBITMAP)LoadImage(NULL, L"..\\pictures\\boss2.bmp", IMAGE_BITMAP, 40, 76, LR_LOADFROMFILE);
	bosses[2].exist = 1;
}

void loadImage()
{
	optionimg[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\start.bmp", IMAGE_BITMAP, 96, 30, LR_LOADFROMFILE);
	optionimg[1] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\exit.bmp", IMAGE_BITMAP, 96, 30, LR_LOADFROMFILE);
	title = (HBITMAP)LoadImage(NULL, L"..\\pictures\\title.bmp", IMAGE_BITMAP, 426, 70, LR_LOADFROMFILE); 

	choosehero[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\MarisaChoose.bmp", IMAGE_BITMAP, 250, 300, LR_LOADFROMFILE);
	choosehero[1] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCChoose.bmp", IMAGE_BITMAP, 250, 300, LR_LOADFROMFILE);
	gamehero[0] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\Marisa.bmp", IMAGE_BITMAP, 40, 38, LR_LOADFROMFILE);
	gamehero[1] = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXC.bmp", IMAGE_BITMAP, 40, 38, LR_LOADFROMFILE);
	
	wxcguard = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCSkill2.bmp", IMAGE_BITMAP, 40, 38, LR_LOADFROMFILE);
	doomtag = (HBITMAP)LoadImage(NULL, L"..\\pictures\\WXCSkill3.bmp", IMAGE_BITMAP, 150, 150, LR_LOADFROMFILE);
	doomimg = (HBITMAP)LoadImage(NULL, L"..\\pictures\\doomShot.bmp", IMAGE_BITMAP, 150, 150, LR_LOADFROMFILE);

	markup = (HBITMAP)LoadImage(NULL, L"..\\pictures\\markup.bmp", IMAGE_BITMAP, 30, 30, LR_LOADFROMFILE);
	markright = (HBITMAP)LoadImage(NULL, L"..\\pictures\\markright.bmp", IMAGE_BITMAP, 30, 30, LR_LOADFROMFILE);
	resticon = (HBITMAP)LoadImage(NULL, L"..\\pictures\\rest.bmp", IMAGE_BITMAP, 21, 21, LR_LOADFROMFILE);

	winscreen = (HBITMAP)LoadImage(NULL, L"..\\pictures\\youwin.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);
	losescreen = (HBITMAP)LoadImage(NULL, L"..\\pictures\\youlose.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);
	restscreen = (HBITMAP)LoadImage(NULL, L"..\\pictures\\restscreen.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);
	bg = (HBITMAP)LoadImage(NULL, L"..\\pictures\\bg.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);
	frame = (HBITMAP)LoadImage(NULL, L"..\\pictures\\frame.bmp", IMAGE_BITMAP, 640, 480, LR_LOADFROMFILE);
}

void loadStage(int s)
{
	friendly.clear();
	aggresive.clear();
	shakeTimer = 0;
	heroSkillNameTimer = 0;
	bossSkillNameTimer = 0;

	boss = bosses[s];
}

void Paint()
{
	BitBlt(hdc, 0, 0, 640, 480, mdc, 0, 0, SRCCOPY);
}

void paint()
{
	if (curstat == WELCOME)
		buildWelcome();
	else if (curstat == CHOOSE)
		buildChoose();
	else if (curstat == GAME)
		buildGame();
	else if (curstat == WIN)
		buildWin();
	else if (curstat == LOSE)
		buildLose();
	else if (curstat == REST)
		buildRest();
	Paint();
}

void buildWelcome()
{
	SelectObject(bufdc, bg);
	BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);

	SelectObject(bufdc, title);
	Transparent(mdc, 100, 110, 426, 70, bufdc, 0, 0, 426, 70, RGB(0, 0, 0));

	SelectObject(bufdc, optionimg[0]);
	Transparent(mdc, 250, 300, 96, 30, bufdc, 0, 0, 96, 30, RGB(0, 0, 0));
	SelectObject(bufdc, optionimg[1]);
	Transparent(mdc, 250, 340, 96, 30, bufdc, 0, 0, 96, 30, RGB(0, 0, 0));

	SelectObject(bufdc, markright);
	Transparent(mdc, 225, 305 + option * 40, 20, 20, bufdc, 0, 0, 30, 30, RGB(0, 0, 0));
}

void buildChoose()
{
	SelectObject(bufdc, bg);
	BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);

	SelectObject(bufdc, title);
	Transparent(mdc, 20, 30, 426, 70, bufdc, 0, 0, 426, 70, RGB(0, 0, 0));

	SelectObject(bufdc, choosehero[0]);
	Transparent(mdc, 50, 110, 250, 300, bufdc, 0, 0, 250, 300, RGB(0, 0, 0));
	SelectObject(bufdc, choosehero[1]);
	Transparent(mdc, 325, 110, 250, 300, bufdc, 0, 0, 250, 300, RGB(0, 0, 0));

	SelectObject(bufdc, markup);
	Transparent(mdc, 160 + option * 275, 420, 30, 30, bufdc, 0, 0, 30, 30, RGB(0, 0, 0));
}

void buildGame()
{
	int dx = 0, dy = 0;
	if (shakeTimer)
	{
		if (shakeTimer % 5 == 0)
			dx = rand() % 21 - 10, dy = rand() % 21 - 10;
	}

	SelectObject(bufdc, bg);
	BitBlt(mdc, 0, 0, 640, 480, bufdc, 0, 0, SRCCOPY);

	if (doomTimer)
	{
		int a = (300 - doomTimer) >> 1;
		SelectObject(bufdc, doomimg);
		Transparent(mdc, hero.x + (hero.w >> 1) - (a >> 1) + dx, hero.y + (hero.h >> 1) - (a >> 1) + dy, a, a, bufdc, 0, 0, 150, 150, RGB(0, 0, 0));
	}

	for (set<Shot>::iterator i = aggresive.begin(); i != aggresive.end(); i++)
	{
		SelectObject(bufdc, i->img[i->timer/5 % i->cnt]);
		Transparent(mdc, i->x + dx, i->y + dy, i->w, i->h, bufdc, 0, 0, i->w, i->h, RGB(0, 0, 0));
	}

	for (set<Shot>::iterator i = friendly.begin(); i != friendly.end(); i++)
	{
		SelectObject(bufdc, i->img[i->timer/5 % i->cnt]);
		Transparent(mdc, i->x + dx, i->y + dy, i->w, i->h, bufdc, 0, 0, i->w, i->h, RGB(0, 0, 0));
	}

	if (boss.exist)
	{
		if (doomTimer)
		{
			SelectObject(bufdc, doomtag);
			Transparent(mdc, boss.x + (boss.w >> 1) - 75 + dx, boss.y + (boss.w >> 1) -75 + dy, 150, 150, bufdc, 0, 0, 150, 150, RGB(0, 0, 0));
		}

		SelectObject(bufdc, boss.img);
		Transparent(mdc, boss.x + dx, boss.y + dy, boss.w, boss.h, bufdc, 0, 0, boss.w, boss.h, RGB(0, 0, 0));

		Rectangle(mdc, 30, 40, 30 + 375 * boss.blood[boss.cur] / boss.bloodlimit[boss.cur], 50);
	}
	
	SelectObject(bufdc, hero.img);
	Transparent(mdc, hero.x + dx, hero.y + dy, hero.w, hero.h, bufdc, 0, 0, hero.w, hero.h, RGB(0, 0, 0));

	if (heroSkillNameTimer)
	{
		SelectObject(bufdc, curname);
		Transparent(mdc, 5, 445, 480, 24, bufdc, 0, 0, 480, 24, RGB(0, 0, 0));
		heroSkillNameTimer--;
	}
	if (bossSkillNameTimer)
	{
		SelectObject(bufdc, bossname);
		Transparent(mdc, 5, 12, 480, 24, bufdc, 0, 0, 480, 24, RGB(0, 0, 0));
		bossSkillNameTimer--;
	}

	SelectObject(bufdc, frame);
	Transparent(mdc, 0, 0, 640, 480, bufdc, 0, 0, 640, 480, RGB(0, 0, 0));

	TextOut(mdc, 435, 50, L"剩余", 2);
	SelectObject(bufdc, resticon);
	for (int i = hero.rest-1; i>=0; i--)
		Transparent(mdc, 480 + i * 30, 50, 21, 21, bufdc, 0, 0, 21, 21, RGB(0, 0, 0));

	TextOut(mdc, 435, 80, L"魔力", 2);
	Rectangle(mdc, 480, 80, 480 + 130 * hero.magic / hero.magiclimit, 90);

	TextOut(mdc, 435, 110, L"冷却1", 3);
	Rectangle(mdc, 480, 110, 480 + 130 * hero.s1.curcd / hero.s1.cd, 120);

	TextOut(mdc, 435, 140, L"冷却2", 3);
	Rectangle(mdc, 480, 140, 480 + 130 * hero.s2.curcd / hero.s2.cd, 150);

	TextOut(mdc, 435, 170, L"冷却3", 3);
	Rectangle(mdc, 480, 170, 480 + 130 * hero.s3.curcd / hero.s3.cd, 180);
}

void buildWin()
{
	SelectObject(bufdc, winscreen);
	Transparent(mdc, 0, 0, 640, 480, bufdc, 0, 0, 640, 480, RGB(0, 0, 0));
}

void buildLose()
{
	SelectObject(bufdc, losescreen);
	Transparent(mdc, 0, 0, 640, 480, bufdc, 0, 0, 640, 480, RGB(0, 0, 0));
}

void buildRest()
{
	SelectObject(bufdc, restscreen);
	Transparent(mdc, 0, 0, 640, 480, bufdc, 0, 0, 640, 480, RGB(0, 0, 0));
}

void welcomeProc()
{
	if (option == 0)
		curstat = CHOOSE, option = 0, stage = 0;
	else
		SendMessage(hWnd, WM_DESTROY, 0, 0);
}

void chooseProc()
{
	if (option == 0)
	{
		hero.img = gamehero[0];
		hero.x = 195;
		hero.y = 410;
		hero.r = 2;
		hero.rest = 2;
		hero.timer = -1;
		hero.ms = 8;
		hero.magic = 500;
		hero.magiclimit = 2400;
		hero.dmagic = 21;
		hero.shot = marisaNormal;
		hero.s1 = starshot;
		hero.s2 = chakara;
		hero.s3 = finalspark;
		hero.w = 40;
		hero.h = 38;
		hero.guard = 0;
	}
	else
	{
		hero.img = gamehero[1];
		hero.x = 195;
		hero.y = 410;
		hero.r = 4;
		hero.rest = 2;
		hero.timer = -1;
		hero.ms = 8;
		hero.magic = 500;
		hero.magiclimit = 1800;
		hero.dmagic = 13;
		hero.shot = WXCNormal;
		hero.s1 = dimlightarrow;
		hero.s2 = blink;
		hero.s3 = doom;
		hero.w = 40;
		hero.h = 38;
		hero.guard = 0;
	}
}

void loadProc()
{
	curstat = GAME;
	loadStage(++stage);
}

void gameProc(UINT msg)
{
	if(msg == WM_TIMER)
		timerProc();
	else if (msg == WM_KEYDOWN)
	{
		if (KEYDOWN(VK_ESCAPE))
		{
			PostQuitMessage(0);
		}if (KEYDOWN(VK_UP))
		{
			hero.md |= 1;
		}if (KEYDOWN(VK_DOWN))
		{
			hero.md |= 2;
		}if (KEYDOWN(VK_LEFT))
		{
			hero.md |= 4;
		}if (KEYDOWN(VK_RIGHT))
		{
			hero.md |= 8;
		}if (KEYDOWN(VK_SHIFT))
		{
			hero.ms = 4;
		}if (KEYDOWN('Z'))
		{
			hero.shooting = true;
		}if (KEYDOWN('X'))
		{
			if (!hero.s1.lock)
			{
				hero.s1.lock = true;
				hero.s1.cast();
			}
		}if (KEYDOWN('C'))
		{
			if (!hero.s2.lock)
			{
				hero.s2.lock = true;
				hero.s2.cast();
			}
		}if (KEYDOWN(' '))
		{
			if (!hero.s3.lock)
			{
				hero.s3.lock = true;
				hero.s3.cast();
			}
		}
	}
	else if (msg == WM_KEYUP)
	{
		if (!KEYDOWN(VK_UP))
		{
			hero.md &= 14;
		}if (!KEYDOWN(VK_DOWN))
		{
			hero.md &= 13;
		}if (!KEYDOWN(VK_LEFT))
		{
			hero.md &= 11;
		}if (!KEYDOWN(VK_RIGHT))
		{
			hero.md &= 7;
		}if (!KEYDOWN(VK_SHIFT))
		{
			hero.ms = 8;
		}if (!KEYDOWN('Z'))
		{
			hero.shooting = false;
		}if (!KEYDOWN('X'))
		{
			hero.s1.lock = false;
		}if (!KEYDOWN('C'))
		{
			hero.s2.lock = false;
		}if (!KEYDOWN(' '))
		{
			hero.s3.lock = false;
		}
	}
}

void timerProc()
{
	//hero operation
	hero.inc();
	
	//hero shoot
	if (hero.timer % 2 == 0)
	{
		if (hero.shooting)
		{
			Shot cur = hero.shot;

			cur.id = shotid++;
			cur.x = hero.x;
			cur.y = hero.y + 20;
			friendly.insert(cur);

			cur.id = shotid++;
			cur.x = hero.x + 30;
			cur.y = hero.y + 20;
			friendly.insert(cur);
		}
	}

	//hero move
	if (hero.timer % 2 == 0)
	{
		bool flag = false;
		if (hero.md == 5 && (hero.y - (3 * hero.ms >> 2) >= 0) && (hero.x - (3 * hero.ms >> 2) >= -5))  //upleft
		{
			hero.y -= (3 * hero.ms >> 2);
			hero.x -= (3 * hero.ms >> 2);
			flag = true;
		}
		else if (hero.md == 6 && (hero.y + (3 * hero.ms >> 2) <= 445) && (hero.x - (3 * hero.ms >> 2) >= 0)) //downleft
		{
			hero.y += (3 * hero.ms >> 2);
			hero.x -= (3 * hero.ms >> 2);
			flag = true;
		}
		else if (hero.md == 9 && (hero.y - (3 * hero.ms >> 2) >= 0) && (hero.x + (3 * hero.ms >> 2) <= 395))  //upright
		{
			hero.y -= (3 * hero.ms >> 2);
			hero.x += (3 * hero.ms >> 2);
			flag = true;
		}
		else if (hero.md == 10 && (hero.y + (3 * hero.ms >> 2) <= 445) && (hero.x + (3 * hero.ms >> 2) <= 395))  //downright
		{
			hero.y += (3 * hero.ms >> 2);
			hero.x += (3 * hero.ms >> 2);
			flag = true;
		}

		if (!flag)
		{
			if (hero.md & 1)  //up
			{
				if (hero.y - hero.ms >= 0)hero.y -= hero.ms;
			}
			if (hero.md & 2) //down
			{
				if (hero.y + hero.ms <= 445)hero.y += hero.ms;
			}
			if (hero.md & 4)  //left
			{
				if (hero.x - hero.ms >= 0)hero.x -= hero.ms;
			}
			if (hero.md & 8)  //right
			{
				if (hero.x + hero.ms <= 395)hero.x += hero.ms;
			}
		}
	}

	if (hero.timer % 100 == 0)
		hero.magic = min(hero.magic + hero.dmagic, hero.magiclimit);

	//hero skill
	hero.s1.inc();
	hero.s1.proc();
	hero.s2.inc();
	hero.s2.proc();
	hero.s3.inc();
	hero.s3.proc();

	if (shakeTimer)shakeTimer--;
	if (doomTimer)doomTimer--;

	//shot move
	set<Shot> tmp = friendly;friendly.clear();
	bool shutdown = false;
	for (set<Shot>::iterator i = tmp.begin(); i != tmp.end(); i++)
	{
		Shot cur = *i;
		bool flag = true;
		int x1 = cur.x + cur.jx, y1 = cur.y + cur.jy, x2, y2;

		if (cur.clearagg)
		{
			set<Shot> tmp2 = aggresive; aggresive.clear();
			for (set<Shot>::iterator j = tmp2.begin(); j != tmp2.end(); j++)
			{
				Shot cur2 = *j;
				x2 = cur2.x + cur2.jx, y2 = cur2.y + cur2.jy;
				if (sqr(x1 - x2) + sqr(y1 - y2) > sqr(cur.r + cur2.r))
					aggresive.insert(cur2);
			}
		}

		if (boss.exist && (cur.uneraseable || !cur.hitboss))
		{
			x2 = boss.x + (boss.w >> 1), y2 = boss.y + (boss.h >> 1);
			if (sqr(x1 - x2) + sqr(y1 - y2) < sqr(cur.r + boss.r))
			{
				boss.blood[boss.cur] -= cur.hurt;
				if (boss.blood[boss.cur] <= 0)
					shutdown = true;
				cur.hitboss = true;
				flag = false;
			}
		}

		if (cur.uneraseable || ((cur.unbreakable || flag) && cur.x >= -200 && cur.x <= 595 && cur.y >= -200 && cur.y <= 645))
		{
			cur.inc();
			cur.proc();
			friendly.insert(cur);
		}
	}

	if (shutdown)
	{
		aggresive.clear();
		boss.timer = 0;
		boss.cur++;
		if (boss.cur == boss.tot)
		{
			if (stage == 2)
				curstat = WIN;
			else
				curstat = REST;
			return;
		}
	}

	//boss skill
	if (boss.exist)
	{
		boss.inc();
		boss.proc();
		boss.s[boss.cur].inc();
		boss.s[boss.cur].proc();
	}

	tmp = aggresive; aggresive.clear(); bool flag = false;
	for (set<Shot>::iterator i = tmp.begin(); i != tmp.end(); i++)
	{
		Shot cur = *i;
		int x1 = cur.x + cur.jx, y1 = cur.y + cur.jy, x2, y2;

		if (!hero.guard)
		{
			x2 = hero.x + (hero.w >> 1), y2 = hero.y + (hero.h >> 1);
			if (sqr(x1 - x2) + sqr(y1 - y2) < sqr(cur.r + hero.r))
			{
				flag = true;
				hero.s1.timer = -1;
				hero.s2.timer = -1;
				hero.s1.curcd = 0;
				hero.s2.curcd = 0;
				hero.s3.curcd = 0;
				hero.magic = max(hero.magic, 500);
				hero.x = 195;
				hero.y = 410;
				hero.rest--;
				
				if (hero.rest == -1)
				{
					curstat = LOSE;
					return;
				}
				break;
			}
		}

		if (cur.x >= -200 && cur.x <= 595 && cur.y >= -200 && cur.y <= 645)
		{
			cur.inc();
			cur.proc();
			aggresive.insert(cur);
		}
	}
	if (flag)
		aggresive.clear();
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (curstat == GAME)
		gameProc(message);
	else
	{
		switch (message)
		{
		case WM_KEYDOWN:
			if (curstat == WELCOME)
			{
				if (KEYDOWN(VK_DOWN))
				{
					option++;
					if (option == 2)option = 0;
				}
				else if (KEYDOWN(VK_UP))
				{
					option--;
					if (option == -1) option = 1;
				}
				else if (KEYDOWN('Z'))
				{
					welcomeProc();
				}
			}
			else if (curstat == CHOOSE)
			{
				if (KEYDOWN(VK_RIGHT))
				{
					option++;
					if (option == 2)option = 0;
				}
				else if (KEYDOWN(VK_LEFT))
				{
					option--;
					if (option == -1) option = 1;
				}
				else if (KEYDOWN('Z'))
				{
					chooseProc();
					curstat = REST;
				}
			}
			else if (curstat == WIN)
			{
				if (KEYDOWN(' '))
				{
					option = 0;
					curstat = WELCOME;
				}
			}
			else if (curstat == LOSE)
			{
				if (KEYDOWN(' '))
				{
					option = 0;
					curstat = WELCOME;
				}
			}
			else if (curstat == REST)
			{
				if (KEYDOWN('Z'))
				{
					loadProc();
				}
			}
			break;

		case WM_DESTROY:
			DeleteDC(mdc);
			DeleteDC(bufdc);
			ReleaseDC(hWnd, hdc);
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}

	return 0;
}
