#include <stdio.h>
#include <graphics.h> //easyx图形库
#include <time.h>
#include <math.h>
#include "tools.h"
#include "vector2.h"

#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define _CRT_SECURE_NO_DEPRECATE  //关闭安全检查

//窗体大小
#define WIN_WIDTH 900
#define WIN_HEIGHT 600
//僵尸最大数量
#define ZM_MAX 9

//枚举类型，正好是0,1,2,n
enum  //植物种类
{
	WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT
};

enum //游戏状态
{
	Going,Win, FAIL
};
int killCount;//死亡的僵尸个数
int zmCount;//已经出现的僵尸个数
int gameStatus;

IMAGE imgBg;//  表示背景图片
IMAGE imgBar;//工具栏
IMAGE imgCards[ZHI_WU_COUNT];//植物卡牌
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//植物

int curX, curY;//当前选中的植物，在移动过程的位置
int curZhiWu;  // 0表示没有选择，1表示选择第一种

struct zhiwu
{
	int type;  //0表示没有选择，1表示选择第一种
	int frameIndex;  //序列帧的序号

	bool catched;//是否被僵尸捕获
	int deadTime;//死亡计数器

	int timer;
	int x, y;

	int shootTime;
};

struct zhiwu map[3][9];

enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };

struct sunshineBall
{
	int x, y;  //阳光球在飘落过程中的坐标(x不变)
	int frameIndex;  // 当前显示的图片帧
	int destY;  // 飘落的目标位置的y坐标
	bool used;//是否使用
	int timer;

	float xoff;
	float yoff; //偏移量

	float t;//贝塞尔曲线的时间点 0..1
	vector2 p1, p2, p3, p4;
	vector2 pCur;//当前时刻阳光球的位置
	float speed;
	int status;
};

struct sunshineBall balls[10]; //阳光池
IMAGE imgSunshineBall[29];
int sunshine;

//僵尸
struct zm {
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;
	bool dead;

	bool eating;//正在吃植物
};

struct zm zms[10];
IMAGE imgZM[22];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];
IMAGE imgZMStand[11];

//子弹的数据类型
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//是否爆炸
	int frameIndex;//帧序号
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];




//判断文件存在
bool fileExist(const char* name)
{
	FILE* fp = fopen(name, "r");
	if (fp == NULL)
	{
		return false;
	}
	else
	{
		fclose(fp);
		return true;
	}
}

//初始化游戏
void gameInit()
{
	//加载背景图片到内存
	//需要把字符集改为 多字节字符集	
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));//指针设置为空
	memset(map, 0, sizeof(map));

	killCount = 0;
	zmCount = 0;
	gameStatus = Going;

	//初始化植物卡牌
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		//生成植物卡牌的文件名
		//sprintf_s为打印到字符串里
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		//加载植物卡牌
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//先判断文件是否存在
			if (fileExist(name))
			{
				imgZhiWu[i][j] = new IMAGE;//由于easyx底层是c++，所以用new分配内存
				//加载植物
				loadimage(imgZhiWu[i][j], name);
			}
			else
			{
				break;
			}

		}
	}
	curZhiWu = 0;

	sunshine = 50;
	//初始化阳光球
	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}
	//配置随机种子
	srand(time(NULL));

	//创建图像窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);    //1表示命令提示符窗口

	//设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 40;
	f.lfWeight = 25;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;  //抗锯齿效果
	settextstyle(&f);
	setbkmode(TRANSPARENT);  //设置背景透明
	setcolor(BLACK);  //设置字体颜色

	//初始化僵尸数据
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i);
		loadimage(&imgZM[i], name);
	}

	//初始化子弹
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//初始化子弹的帧图片数组
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
			imgBullBlast[3].getwidth() * k,
			imgBullBlast[3].getheight() * k, true);
	}//子弹爆炸图片，改变大小比例

	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZMDead[i], name);
	}//僵尸死亡

	for (int i = 0; i < 21; i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}//僵尸吃

	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZMStand[i], name);
	}//僵尸站立
}

void drawZM() {
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used) {
			//IMAGE* img = &imgZM[zms[i].frameIndex];
			//IMAGE* img = (zms[i].dead) ? imgZMDead : imgZM;
			IMAGE* img = NULL;
			if (zms[i].dead) img = imgZMDead;
			else if (zms[i].eating) img = imgZMEat;
			else img = imgZM;

			img += zms[i].frameIndex;

			putimagePNG(
				zms[i].x,
				zms[i].y - img->getheight(),
				img);
		}
	}
}

void drawSunshine() {
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++)
	{
		//if (balls[i].used || balls[i].xoff){
		if(balls[i].used){
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			//putimagePNG(balls[i].x, balls[i].y, img);
			putimagePNG(balls[i].pCur.x, balls[i].pCur.y, img);
		}
	}

	char scoreText[8];
	sprintf_s(scoreText, sizeof(scoreText), "%d", sunshine);
	outtextxy(278, 64, scoreText);   //输出太阳数量

	outtextxy(631, 481, "剩余僵尸：");
	char zmText[8];
	sprintf_s(zmText, sizeof(zmText), "%d", 10 - zmCount);
	outtextxy(780, 481, zmText);   //输出剩余僵尸
}

void drawCards()
{
	// 渲染工具栏中的植物卡牌
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}

void drawZhiWu()
{
	//渲染种下的植物
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				//int x = 256 + j * 81;
				//int y = 179 + i * 102 + 14;
				int ZhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				//putimagePNG(x, y, imgZhiWu[ZhiWuType][index]);
				putimagePNG(map[i][j].x, map[i][j].y, imgZhiWu[ZhiWuType][index]);
			}
		}
	}

	//渲染拖动过程中的植物
	if (curZhiWu > 0)
	{
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth() / 2, curY - img->getheight() / 2, img);

	}
}

void drawBullets()
{
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < bulletMax; i++) {
		if (bullets[i].used) {
			if (bullets[i].blast) {
				IMAGE* img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}
			else
			{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}

}

//渲染
void updateWindow()
{
	//使用双缓冲防止闪烁   先将要绘制的内容一次性绘制在图片上，再把图片输出，避免不断从内存读取数据而导致的屏幕闪烁
	BeginBatchDraw();//开始缓冲

	//显示图片
	putimage(-112, 0, &imgBg);   //坐标，图片
	putimagePNG(250, 0, &imgBar);
	
	drawCards();//画卡牌

	drawZhiWu();//画植物

	drawSunshine(); //阳光

	drawZM();//僵尸

	drawBullets();//子弹

	EndBatchDraw();//结束双缓冲
}

void collectSunshine(ExMessage* msg)
{
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for (int i = 0; i < count; i++)
	{
		if (balls[i].used)
		{
			//int x = balls[i].x;
			//int y = balls[i].y;
			int x = balls[i].pCur.x;
			int y = balls[i].pCur.y;
			if (msg->x > x && msg->x < x + w &&
				msg->y >y && msg->y < y + h)
			{
				// balls[i].used = false;
				balls[i].status = SUNSHINE_COLLECT;
				sunshine += 25;
			/*	mciSendString("play res/sunshine.mp3", 0, 0, 0);*/
				PlaySound("res/sunshine.wav", NULL, SND_FILENAME | SND_ASYNC);

				//设置阳光球的偏移量
				//float destY = 0;
				//float destX = 262;
				//float angle = atan((y - destY) / (x - destX));//角度
				//balls[i].xoff = 4 * cos(angle);
				//balls[i].yoff = 4 * sin(angle);
				balls[i].p1 = balls[i].pCur;
				balls[i].p4 = vector2(262, 0);
				balls[i].t = 0;
				float distance = dis(balls[i].p1 - balls[i] .p4);
				float off = 8;
				balls[i].speed = 1.0 / (distance / off);
				break;
			}
		}
	}
}

//用户点击
void userClick()
{
	ExMessage msg;   //定义一个消息
	static int status = 0;
	if (peekmessage(&msg))//判断有没有消息
	{
		if (msg.message == WM_LBUTTONDOWN)//左键按下
		{
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96)//是否按到植物卡牌
			{
				int index = (msg.x - 338) / 65;  //计算索引
				if (index == 0 && sunshine >= 100)
				{
					status = 1;//先选定
					curZhiWu = index + 1;
				}
				else if (index == 1 && sunshine >= 50)
				{
					status = 1;//先选定
					curZhiWu = index + 1;
				}
			}
			else //判断是否收集阳光
			{
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)//鼠标移动
		{
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP && status == 1)//左键抬起
		{
			if (msg.x > 144 && msg.y > 179 && msg.y < 489)
			{
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 144) / 81;
				/*printf("%d ,%d\n", row, col);*/
				//消耗阳光
				if (curZhiWu == 1)
				{
					sunshine -= 100;
				}
				else if (curZhiWu == 2)
				{
					sunshine -= 50;
				}
				if (map[row][col].type == 0)
				{
					map[row][col].type = curZhiWu;
					map[row][col].frameIndex = 0;
					map[row][col].shootTime = 0;
					//int x = 256 + j * 81;
				    //int y = 179 + i * 102 + 14;
					map[row][col].x = 144 + col * 81;
					map[row][col].y = 179 + row * 102 + 14;
				}
			}
			

			curZhiWu = 0;
			status = 0;
		}
	}
}

void createSunshine()
{
	static int count = 0;
	static int fre = 200;
	count++;
	if (count >= fre)
	{
		fre = 200 + rand() % 200; // 随机时间生成阳光
		count = 0;

		//从阳光池中取一个可以使用的
		int ballMax = sizeof(balls) / sizeof(balls[0]);

		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax) return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		//balls[i].x = 260 + rand() % (700 - 260);  //260到700
		//balls[i].y = 60;
		//balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;  //计时器

		//balls[i].xoff = 0;
		//balls[i].yoff = 0;
		
		balls[i].status = SUNSHINE_DOWN;
		balls[i].t = 0;
		balls[i].p1 = vector2(260 - 112 + rand() % (700 - (260 - 112)), 60);
		balls[i].p4 = vector2(balls[i].p1.x, 200 + (rand() % 4) * 90);
		int off = 2;
		float distance = balls[i].p4.y - balls[i].p1.y;
		balls[i].speed = 1.0 / (distance / off);
	}

	//向日葵生产阳光
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++){
			if (map[i][j].type == XIANG_RI_KUI + 1) {
				map[i][j].timer++;
				if (map[i][j].timer > 200) {
					map[i][j].timer = 0;

					int k;
					for (k = 0; k < ballMax && balls[k].used; k++);
					if (k >= ballMax) return;

					balls[k].used = true;
					balls[k].p1 = vector2(map[i][j].x, map[i][j].y);
					int w = (100 + rand() % 50) * (rand() % 2 ? 1 : -1);
					balls[k].p4 = vector2(map[i][j].x + w,
						map[i][j].y + imgZhiWu[XIANG_RI_KUI][0]->getheight() -
						imgSunshineBall[0].getheight());
					balls[k].p2 = vector2(balls[k].p1.x + w * 0.3, balls[k].p1.y - 100);
					balls[k].p3 = vector2(balls[k].p1.x + w * 0.7, balls[k].p1.y - 100);
					balls[k].status = SUNSHINE_PRODUCT;
					balls[k].speed = 0.05;
					balls[k].t = 0;
				}
			}
		}
	}

}

void undateSunshine()
{
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for (int i = 0; i < ballMax; i++){
		if (balls[i].used) {
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if (balls[i].status == SUNSHINE_DOWN) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t >= 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_GROUND) {
				balls[i].timer++;
				if (balls[i].timer > 100) {
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
			else if (balls[i].status == SUNSHINE_COLLECT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = sun->p1 + sun->t * (sun->p4 - sun->p1);
				if (sun->t > 1) {
					sun->used = false;
					/*sunshine += 25;*/
				}
			}
			else if (balls[i].status == SUNSHINE_PRODUCT) {
				struct sunshineBall* sun = &balls[i];
				sun->t += sun->speed;
				sun->pCur = calcBezierPoint(sun->t, sun->p1, sun->p2, sun->p3, sun->p4);
				if (sun->t > 1) {
					sun->status = SUNSHINE_GROUND;
					sun->timer = 0;
				}
			}

		}
	}
}

void createZM() {
	if (zmCount >= ZM_MAX) {
		return;
	}

	static int zmFre = 100;//出现时间
	static int count = 0;
	count++;
	if (count > zmFre) {
		count = 0;
		zmFre = rand() % 200 + 300;

		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for (i = 0; i < zmMax && zms[i].used; i++);
		if (i < zmMax) {
			memset(&zms[i], 0, sizeof(zms[i]));
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;//0..2行
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;  //僵尸速度
			zms[i].blood = 160;   //僵尸血量
			zms[i].dead = false;

			zmCount++;
		}
		else {
			printf("创建僵尸失败！\n");
		}
	}
}

void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
		//僵尸位置 更新
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 50) {
					printf("GAME OVER\n");
					/*MessageBox(NULL, "over", "over", 0);*/
					//exit(0);//游戏结束
					gameStatus = FAIL;
				}
			}
		}
	}

	static int count2 = 0;
	count2++;
	if (count2 > 4) {
		count2 = 0;
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				if (zms[i].dead) {
					zms[i].frameIndex++;
					if (zms[i].frameIndex >= 20) {
						zms[i].used = false;

						killCount++;
						if (killCount == ZM_MAX)
						{
							gameStatus = Win;
						}
					}
				}
				else if (zms[i].eating) {
					zms[i].frameIndex = (zms[1].frameIndex + 1) % 21;
				}
				else {
					zms[i].frameIndex = (zms[i].frameIndex + 1) % 22;
				}
			}
		}
	}
}
//发射子弹
void shoot() {
	int lines[3] = { 0 };
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();//开始发射
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}//发射条件
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
				map[i][j].shootTime++;
				//吐子弹速度
				if (map[i][j].shootTime > 50) {
					map[i][j].shootTime = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 7;//子弹飞行速度

						bullets[k].blast = false;
						bullets[k].frameIndex = 0;

						int zwX = 144 + j * 81;
						int zwY = 179 + i * 102 + 14;
						bullets[k].x = zwX + imgZhiWu[map[i][j].type - 1][0]->getwidth() - 10;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}
//更新子弹
void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}
			//子弹碰撞检测
			if (bullets[i].blast) {
				bullets[i].frameIndex++;
				if (bullets[i].frameIndex >= 4) {
					bullets[i].used = false;
				}
			}
		}
	}
}

void checkBullet2Zm() {
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < bCount; i++) {
		if (bullets[i].used == false || bullets[i].blast)continue;

		for (int k = 0; k < zCount; k++) {
			if (zms[k].used == false) continue;
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if (zms[k].dead == false && bullets[i].row == zms[k].row && x > x1 && x < x2) {
				zms[k].blood -= 20;
				bullets[i].blast = true;
				bullets[i].speed = 0;

				if (zms[k].blood <= 0) {
					zms[k].dead = true;
					zms[k].speed = 0;
					zms[k].frameIndex = 0;
				}
				break;
			}
		}
	}
}

void checkZm2ZhiWu() {
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for (int i = 0; i < zCount; i++) {
		if (zms[i].dead)continue;

		int row = zms[i].row;
		for (int k = 0; k < 9; k++) {
			if (map[row][k].type == 0) continue;

			int zhiWuX = 144 + k * 81;
			int x1 = zhiWuX + 10;
			int x2 = zhiWuX + 60;
			int x3 = zms[i].x + 80;
			if (x3 > x1 && x3 < x2) {
				if (map[row][k].catched) {
					//zms[i].frameIndex++;
					map[row][k].deadTime++;
					//if (zms[i].frameIndex > 100) {
					if(map[row][k].deadTime > 100) {
						map[row][k].deadTime = 0;
						map[row][k].type = 0;
						zms[i].eating = false;
						zms[i].frameIndex = 0;
						zms[i].speed = 1;
					}
				}
				else {
					map[row][k].catched = true;
					map[row][k].deadTime = 0;
					zms[i].eating = true;
					zms[i].speed = 0;
					zms[i].frameIndex = 0;

				}
			}
		}
	}
}

//更新游戏

void collisionCheck() {
	checkBullet2Zm();//子弹对僵尸的碰撞检测
	checkZm2ZhiWu();//僵尸对植物的碰撞检测
}

void updateZhiWu()
{
	static int count = 0;
	if (++count < 3) return;
	count = 0;
	//更新植物的序列帧
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (map[i][j].type > 0)
			{
				map[i][j].frameIndex++;
				int ZhiWuType = map[i][j].type - 1;
				int index = map[i][j].frameIndex;
				if (imgZhiWu[ZhiWuType][index] == NULL)
				{
					map[i][j].frameIndex = 0;
				}
			}
		}
	}
}

void updateGame() {

	updateZhiWu();

	createSunshine();  //创建阳光
	undateSunshine();  //更新阳光状态

	createZM();//创建僵尸
	updateZM();//更新僵尸状态

	shoot();//发射豌豆
	updateBullets();//更新子弹

	collisionCheck();//实现碰撞
}
//启动菜单
void startUI()
{
	//加载菜单图片
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");

	int flag = 0;


	while (1)
	{
		BeginBatchDraw();//缓冲

		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);



		ExMessage msg;
		if (peekmessage(&msg))
		{
			if (msg.message == WM_LBUTTONDOWN &&
				msg.x > 474 && msg.x < 474 + 300 &&
				msg.y >75 && msg.y < 75 + 140)
			{
				flag = 1;
			}
			else if (msg.message == WM_LBUTTONUP && flag)
			{
				EndBatchDraw();
				break;
			}
		}

		EndBatchDraw();
	}

}

//场景巡场
void viewScence()
{
	int xMin = WIN_WIDTH - imgBg.getwidth();//900-1400=-500
	vector2 points[9] = { {550,80},{530,160},{630,170},{530,200},{525,270},		//9个僵尸站位
		{565,370},{605,340},{705,280},{680,340} };
	int index[9];
	for (int i = 0; i < 9; i++)
	{
		index[i] = rand() % 11;
	}
	int count = 0;
	for (int x = 0; x >=xMin ; x-=4)
	{
		BeginBatchDraw();
		putimage(x, 0, &imgBg);

		count++;

		for (int k = 0; k < 9; k++)
		{
			putimagePNG(points[k].x - xMin + x, points[k].y,
				&imgZMStand[index[k]]);
			if (count>=10)//计数
			{
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10) count = 0;

		EndBatchDraw();
		Sleep(3);
	}

	//停留1s
	for (int i = 0; i < 50; i++)
	{
		BeginBatchDraw();

		putimage(xMin, 0, &imgBg);
		for (int j = 0; j < 9; j++) {
			putimagePNG(points[j].x, points[j].y, &imgZMStand[index[j]]);
			index[j] = (index[j] + 1) % 11;
		}

		EndBatchDraw();
		Sleep(30);
	}

	//往回移
	for (int x = xMin; x <= -112; x += 4) {	//每帧移动4像素
		BeginBatchDraw();

		putimage(x, 0, &imgBg);

		count++;
		for (int k = 0; k < 9; k++) {
			putimagePNG(points[k].x - xMin + x, points[k].y, &imgZMStand[index[k]]);
			if (count >= 10) {
				index[k] = (index[k] + 1) % 11;
			}
			if (count >= 10)count = 0;
		}

		EndBatchDraw();
		Sleep(5);
	}
}

void barsDown()
{
	int height = imgBar.getheight();
	for (int y = -height; y <= 0; y+=4)
	{
		BeginBatchDraw();

		putimage(-112, 0, &imgBg);
		putimagePNG(250, y, &imgBar);
			
		for (int i = 0; i < ZHI_WU_COUNT; i++)
		{
			int x = 338 + i * 65;
			putimagePNG(x, 6 + y, &imgCards[i]);
		}

		EndBatchDraw();
		Sleep(3);
		mciSendString("play res/awooga.mp3", 0, 0, 0);
	}
}

bool checkOver()//判断游戏结束
{
	mciSendString("play res/bg.mp3", 0, 0, 0);//背景音乐

	BeginBatchDraw();
	int ret = false;
	if (gameStatus==Win)//游戏胜利
	{
		mciSendString("close res/bg.mp3", 0, 0, 0);//关闭背景音乐
		Sleep(500);
		loadimage(0, "res/gameWin.png");
		mciSendString("play res/win.mp3", 0, 0, 0);
		ret = true;
	}
	else if (gameStatus == FAIL)//游戏失败
	{
		mciSendString("close res/bg.mp3", 0, 0, 0);//关闭背景音乐

		mciSendString("play res/chomp.mp3", 0, 0, 0);
		Sleep(1500);
		mciSendString("play res/lose.mp3", 0, 0, 0);
		
		loadimage(0, "res/gameFail.png");
		
		ret = true;
	}
	EndBatchDraw();
	return ret;
}

int main()
{
	gameInit(); //初始化

	startUI();//开始菜单

	viewScence();//开始动画

	barsDown();//卡牌槽下降

	int timer = 0;
	bool flag = true;
	while (1)
	{
		userClick();
		timer += getDelay();//一次调用与上一次调用的时间间隔
		if (timer > 20)
		{
			flag = true;
			timer = 0;
		}
		if (flag)
		{
			flag = false;
			updateWindow();

			updateGame();

			if (checkOver())break;
		}

	}


	system("pause");   //暂停
	return 0;
}