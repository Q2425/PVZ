#include <stdio.h>
#include <graphics.h> //easyxͼ�ο�
#include <time.h>
#include <math.h>
#include "tools.h"
#include "vector2.h"

#include<mmsystem.h>
#pragma comment(lib,"winmm.lib")

#define _CRT_SECURE_NO_DEPRECATE  //�رհ�ȫ���

//�����С
#define WIN_WIDTH 900
#define WIN_HEIGHT 600
//��ʬ�������
#define ZM_MAX 9

//ö�����ͣ�������0,1,2,n
enum  //ֲ������
{
	WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT
};

enum //��Ϸ״̬
{
	Going,Win, FAIL
};
int killCount;//�����Ľ�ʬ����
int zmCount;//�Ѿ����ֵĽ�ʬ����
int gameStatus;

IMAGE imgBg;//  ��ʾ����ͼƬ
IMAGE imgBar;//������
IMAGE imgCards[ZHI_WU_COUNT];//ֲ�￨��
IMAGE* imgZhiWu[ZHI_WU_COUNT][20];//ֲ��

int curX, curY;//��ǰѡ�е�ֲ����ƶ����̵�λ��
int curZhiWu;  // 0��ʾû��ѡ��1��ʾѡ���һ��

struct zhiwu
{
	int type;  //0��ʾû��ѡ��1��ʾѡ���һ��
	int frameIndex;  //����֡�����

	bool catched;//�Ƿ񱻽�ʬ����
	int deadTime;//����������

	int timer;
	int x, y;

	int shootTime;
};

struct zhiwu map[3][9];

enum { SUNSHINE_DOWN, SUNSHINE_GROUND, SUNSHINE_COLLECT, SUNSHINE_PRODUCT };

struct sunshineBall
{
	int x, y;  //��������Ʈ������е�����(x����)
	int frameIndex;  // ��ǰ��ʾ��ͼƬ֡
	int destY;  // Ʈ���Ŀ��λ�õ�y����
	bool used;//�Ƿ�ʹ��
	int timer;

	float xoff;
	float yoff; //ƫ����

	float t;//���������ߵ�ʱ��� 0..1
	vector2 p1, p2, p3, p4;
	vector2 pCur;//��ǰʱ���������λ��
	float speed;
	int status;
};

struct sunshineBall balls[10]; //�����
IMAGE imgSunshineBall[29];
int sunshine;

//��ʬ
struct zm {
	int x, y;
	int frameIndex;
	bool used;
	int speed;
	int row;
	int blood;
	bool dead;

	bool eating;//���ڳ�ֲ��
};

struct zm zms[10];
IMAGE imgZM[22];
IMAGE imgZMDead[20];
IMAGE imgZMEat[21];
IMAGE imgZMStand[11];

//�ӵ�����������
struct bullet {
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//�Ƿ�ը
	int frameIndex;//֡���
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];




//�ж��ļ�����
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

//��ʼ����Ϸ
void gameInit()
{
	//���ر���ͼƬ���ڴ�
	//��Ҫ���ַ�����Ϊ ���ֽ��ַ���	
	loadimage(&imgBg, "res/bg.jpg");
	loadimage(&imgBar, "res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));//ָ������Ϊ��
	memset(map, 0, sizeof(map));

	killCount = 0;
	zmCount = 0;
	gameStatus = Going;

	//��ʼ��ֲ�￨��
	char name[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		//����ֲ�￨�Ƶ��ļ���
		//sprintf_sΪ��ӡ���ַ�����
		sprintf_s(name, sizeof(name), "res/Cards/card_%d.png", i + 1);
		//����ֲ�￨��
		loadimage(&imgCards[i], name);

		for (int j = 0; j < 20; j++)
		{
			sprintf_s(name, sizeof(name), "res/zhiwu/%d/%d.png", i, j + 1);
			//���ж��ļ��Ƿ����
			if (fileExist(name))
			{
				imgZhiWu[i][j] = new IMAGE;//����easyx�ײ���c++��������new�����ڴ�
				//����ֲ��
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
	//��ʼ��������
	memset(balls, 0, sizeof(balls));
	for (int i = 0; i < 29; i++)
	{
		sprintf_s(name, sizeof(name), "res/sunshine/%d.png", i + 1);
		loadimage(&imgSunshineBall[i], name);
	}
	//�����������
	srand(time(NULL));

	//����ͼ�񴰿�
	initgraph(WIN_WIDTH, WIN_HEIGHT, 1);    //1��ʾ������ʾ������

	//��������
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 40;
	f.lfWeight = 25;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;  //�����Ч��
	settextstyle(&f);
	setbkmode(TRANSPARENT);  //���ñ���͸��
	setcolor(BLACK);  //����������ɫ

	//��ʼ����ʬ����
	memset(zms, 0, sizeof(zms));
	for (int i = 0; i < 22; i++) {
		sprintf_s(name, sizeof(name), "res/zm/%d.png", i);
		loadimage(&imgZM[i], name);
	}

	//��ʼ���ӵ�
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));

	//��ʼ���ӵ���֡ͼƬ����
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	for (int i = 0; i < 3; i++) {
		float k = (i + 1) * 0.2;
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
			imgBullBlast[3].getwidth() * k,
			imgBullBlast[3].getheight() * k, true);
	}//�ӵ���ըͼƬ���ı��С����

	for (int i = 0; i < 20; i++) {
		sprintf_s(name, sizeof(name), "res/zm_dead/%d.png", i + 1);
		loadimage(&imgZMDead[i], name);
	}//��ʬ����

	for (int i = 0; i < 21; i++) {
		sprintf_s(name, "res/zm_eat/%d.png", i + 1);
		loadimage(&imgZMEat[i], name);
	}//��ʬ��

	for (int i = 0; i < 11; i++)
	{
		sprintf_s(name, sizeof(name), "res/zm_stand/%d.png", i + 1);
		loadimage(&imgZMStand[i], name);
	}//��ʬվ��
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
	outtextxy(278, 64, scoreText);   //���̫������

	outtextxy(631, 481, "ʣ�ཀྵʬ��");
	char zmText[8];
	sprintf_s(zmText, sizeof(zmText), "%d", 10 - zmCount);
	outtextxy(780, 481, zmText);   //���ʣ�ཀྵʬ
}

void drawCards()
{
	// ��Ⱦ�������е�ֲ�￨��
	for (int i = 0; i < ZHI_WU_COUNT; i++)
	{
		int x = 338 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}
}

void drawZhiWu()
{
	//��Ⱦ���µ�ֲ��
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

	//��Ⱦ�϶������е�ֲ��
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

//��Ⱦ
void updateWindow()
{
	//ʹ��˫�����ֹ��˸   �Ƚ�Ҫ���Ƶ�����һ���Ի�����ͼƬ�ϣ��ٰ�ͼƬ��������ⲻ�ϴ��ڴ��ȡ���ݶ����µ���Ļ��˸
	BeginBatchDraw();//��ʼ����

	//��ʾͼƬ
	putimage(-112, 0, &imgBg);   //���꣬ͼƬ
	putimagePNG(250, 0, &imgBar);
	
	drawCards();//������

	drawZhiWu();//��ֲ��

	drawSunshine(); //����

	drawZM();//��ʬ

	drawBullets();//�ӵ�

	EndBatchDraw();//����˫����
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

				//�����������ƫ����
				//float destY = 0;
				//float destX = 262;
				//float angle = atan((y - destY) / (x - destX));//�Ƕ�
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

//�û����
void userClick()
{
	ExMessage msg;   //����һ����Ϣ
	static int status = 0;
	if (peekmessage(&msg))//�ж���û����Ϣ
	{
		if (msg.message == WM_LBUTTONDOWN)//�������
		{
			if (msg.x > 338 && msg.x < 338 + 65 * ZHI_WU_COUNT && msg.y < 96)//�Ƿ񰴵�ֲ�￨��
			{
				int index = (msg.x - 338) / 65;  //��������
				if (index == 0 && sunshine >= 100)
				{
					status = 1;//��ѡ��
					curZhiWu = index + 1;
				}
				else if (index == 1 && sunshine >= 50)
				{
					status = 1;//��ѡ��
					curZhiWu = index + 1;
				}
			}
			else //�ж��Ƿ��ռ�����
			{
				collectSunshine(&msg);
			}
		}
		else if (msg.message == WM_MOUSEMOVE && status == 1)//����ƶ�
		{
			curX = msg.x;
			curY = msg.y;
		}
		else if (msg.message == WM_LBUTTONUP && status == 1)//���̧��
		{
			if (msg.x > 144 && msg.y > 179 && msg.y < 489)
			{
				int row = (msg.y - 179) / 102;
				int col = (msg.x - 144) / 81;
				/*printf("%d ,%d\n", row, col);*/
				//��������
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
		fre = 200 + rand() % 200; // ���ʱ����������
		count = 0;

		//���������ȡһ������ʹ�õ�
		int ballMax = sizeof(balls) / sizeof(balls[0]);

		int i;
		for (i = 0; i < ballMax && balls[i].used; i++);
		if (i >= ballMax) return;

		balls[i].used = true;
		balls[i].frameIndex = 0;
		//balls[i].x = 260 + rand() % (700 - 260);  //260��700
		//balls[i].y = 60;
		//balls[i].destY = 200 + (rand() % 4) * 90;
		balls[i].timer = 0;  //��ʱ��

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

	//���տ���������
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

	static int zmFre = 100;//����ʱ��
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
			zms[i].row = rand() % 3;//0..2��
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 1;  //��ʬ�ٶ�
			zms[i].blood = 160;   //��ʬѪ��
			zms[i].dead = false;

			zmCount++;
		}
		else {
			printf("������ʬʧ�ܣ�\n");
		}
	}
}

void updateZM() {
	int zmMax = sizeof(zms) / sizeof(zms[0]);

	static int count = 0;
	count++;
	if (count > 2) {
		count = 0;
		//��ʬλ�� ����
		for (int i = 0; i < zmMax; i++) {
			if (zms[i].used) {
				zms[i].x -= zms[i].speed;
				if (zms[i].x < 50) {
					printf("GAME OVER\n");
					/*MessageBox(NULL, "over", "over", 0);*/
					//exit(0);//��Ϸ����
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
//�����ӵ�
void shoot() {
	int lines[3] = { 0 };
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - imgZM[0].getwidth();//��ʼ����
	for (int i = 0; i < zmCount; i++) {
		if (zms[i].used && zms[i].x < dangerX) {
			lines[zms[i].row] = 1;
		}//��������
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 9; j++) {
			if (map[i][j].type == WAN_DOU + 1 && lines[i]) {
				map[i][j].shootTime++;
				//���ӵ��ٶ�
				if (map[i][j].shootTime > 50) {
					map[i][j].shootTime = 0;

					int k;
					for (k = 0; k < bulletMax && bullets[k].used; k++);
					if (k < bulletMax) {
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 7;//�ӵ������ٶ�

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
//�����ӵ�
void updateBullets() {
	int countMax = sizeof(bullets) / sizeof(bullets[0]);
	for (int i = 0; i < countMax; i++) {
		if (bullets[i].used) {
			bullets[i].x += bullets[i].speed;
			if (bullets[i].x > WIN_WIDTH) {
				bullets[i].used = false;
			}
			//�ӵ���ײ���
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

//������Ϸ

void collisionCheck() {
	checkBullet2Zm();//�ӵ��Խ�ʬ����ײ���
	checkZm2ZhiWu();//��ʬ��ֲ�����ײ���
}

void updateZhiWu()
{
	static int count = 0;
	if (++count < 3) return;
	count = 0;
	//����ֲ�������֡
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

	createSunshine();  //��������
	undateSunshine();  //��������״̬

	createZM();//������ʬ
	updateZM();//���½�ʬ״̬

	shoot();//�����㶹
	updateBullets();//�����ӵ�

	collisionCheck();//ʵ����ײ
}
//�����˵�
void startUI()
{
	//���ز˵�ͼƬ
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu1.png");
	loadimage(&imgMenu2, "res/menu2.png");

	int flag = 0;


	while (1)
	{
		BeginBatchDraw();//����

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

//����Ѳ��
void viewScence()
{
	int xMin = WIN_WIDTH - imgBg.getwidth();//900-1400=-500
	vector2 points[9] = { {550,80},{530,160},{630,170},{530,200},{525,270},		//9����ʬվλ
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
			if (count>=10)//����
			{
				index[k] = (index[k] + 1) % 11;
			}
		}
		if (count >= 10) count = 0;

		EndBatchDraw();
		Sleep(3);
	}

	//ͣ��1s
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

	//������
	for (int x = xMin; x <= -112; x += 4) {	//ÿ֡�ƶ�4����
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

bool checkOver()//�ж���Ϸ����
{
	mciSendString("play res/bg.mp3", 0, 0, 0);//��������

	BeginBatchDraw();
	int ret = false;
	if (gameStatus==Win)//��Ϸʤ��
	{
		mciSendString("close res/bg.mp3", 0, 0, 0);//�رձ�������
		Sleep(500);
		loadimage(0, "res/gameWin.png");
		mciSendString("play res/win.mp3", 0, 0, 0);
		ret = true;
	}
	else if (gameStatus == FAIL)//��Ϸʧ��
	{
		mciSendString("close res/bg.mp3", 0, 0, 0);//�رձ�������

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
	gameInit(); //��ʼ��

	startUI();//��ʼ�˵�

	viewScence();//��ʼ����

	barsDown();//���Ʋ��½�

	int timer = 0;
	bool flag = true;
	while (1)
	{
		userClick();
		timer += getDelay();//һ�ε�������һ�ε��õ�ʱ����
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


	system("pause");   //��ͣ
	return 0;
}