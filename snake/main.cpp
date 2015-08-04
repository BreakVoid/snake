#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <ctime>
#include "jsoncpp/json.h"
using namespace std;
int n, m;
const int maxn = 25;
const int dx[4] = { -1,0,1,0 };
const int dy[4] = { 0,1,0,-1 };
bool invalid[maxn][maxn];

struct point {
	int x, y;
	point(const int &_x, const int &_y)
		: x(_x), y(_y)
	{}
};

list<point> snake[2]; // 0��ʾ�Լ����ߣ�1��ʾ�Է�����
int possibleDire[10];
int posCount;

bool whetherGrow(int num)  //���غ��Ƿ�����
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}

void deleteEnd(int id)     //ɾ����β
{
	snake[id].pop_back();
}

void move(int id, int dire, int num)  //���Ϊid���߳���dire�����ƶ�һ��
{
	point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(point(x, y));
	if (!whetherGrow(num))
		deleteEnd(id);
}
void outputSnakeBody(int id)    //�������
{
	cout << "Snake No." << id << endl;
	for (list<point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
		cout << iter->x << " " << iter->y << endl;
	cout << endl;
}

bool isInBody(int x, int y)   //�ж�(x,y)λ���Ƿ�����
{
	for (int id = 0; id <= 1; id++)
		for (list<point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
			if (x == iter->x && y == iter->y)
				return true;
	return false;
}

bool validDirection(int id, int k)  //�жϵ�ǰ�ƶ��������һ���Ƿ�Ϸ�
{
	point p = *(snake[id].begin());
	int x = p.x + dx[k];
	int y = p.y + dy[k];
	if (x > n || y > m || x < 1 || y < 1) return false;
	if (invalid[x][y]) return false;
	if (isInBody(x, y)) return false;
	return true;
}

int Rand(int p)   //�������һ��0��p������
{
	return rand()*rand()*rand() % p;
}

int TurnLeft(int id)
{
	list<point>::iterator iter = snake[id].begin();
	point first(iter->x, iter->y);
	iter++;
	point second(iter->x, iter->y);
	if (first.x == second.x) {
		if (first.y == second.y + 1) {
			return 2;
		} else {
			return 0;
		}
	} else {
		if (first.x == second.x + 1) {
			return 3;
		} else {
			return 1;
		}
	}
}

int TurnRight(int id)
{
	list<point>::iterator iter = snake[id].begin();
	point first(iter->x, iter->y);
	iter++;
	point second(iter->x, iter->y);
	if (first.x == second.x) {
		if (first.y == second.y + 1) {
			return 0;
		} else {
			return 2;
		}
	} else {
		if (first.x == second.x + 1) {
			return 1;
		} else {
			return 3;
		}
	}
}

int GoStraight(int id)
{
	list<point>::iterator iter = snake[id].begin();
	point first(iter->x, iter->y);
	iter++;
	point second(iter->x, iter->y);
	if (first.x == second.x) {
		if (first.y == second.y + 1) {
			return 3;
		} else {
			return 1;
		}
	} else {
		if (first.x == second.x + 1) {
			return 2;
		} else {
			return 0;
		}
	}
}

int main()
{
	memset(invalid, 0, sizeof(invalid));
	string str;
	string temp;
	while (getline(cin, temp))
		str += temp;

	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);

	n = input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //���̸߶�
	m = input["requests"][(Json::Value::UInt) 0]["width"].asInt();   //���̿��

	int x = input["requests"][(Json::Value::UInt) 0]["x"].asInt();  //���߳�ʼ������Ϣ
	if (x == 1) {
		snake[0].push_front(point(1, 1));
		snake[1].push_front(point(n, m));
	} else {
		snake[1].push_front(point(1, 1));
		snake[0].push_front(point(n, m));
	}
	//�����ͼ�е��ϰ���
	int obsCount = input["requests"][(Json::Value::UInt) 0]["obstacle"].size();

	for (int i = 0; i < obsCount; i++) {
		int ox = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["x"].asInt();
		int oy = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["y"].asInt();
		invalid[ox][oy] = 1;
	}

	//������ʷ��Ϣ�ָ��ֳ�
	int total = input["responses"].size();

	int dire;
	for (int i = 0; i < total; i++) {
		dire = input["responses"][i]["direction"].asInt();
		move(0, dire, i);

		dire = input["requests"][i + 1]["direction"].asInt();
		move(1, dire, i);
	}

	if (!whetherGrow(total)) // ���غ�����������
	{
		deleteEnd(0);
		deleteEnd(1);
	}
	int dirCnt = 0;
	if (validDirection(0, TurnLeft(0))) {
		possibleDire[dirCnt++] = TurnLeft(0);
	}
	if (validDirection(0, TurnRight(0))) {
		possibleDire[dirCnt++] = TurnRight(0);
	}
	if (validDirection(0, GoStraight(0))) {
		possibleDire[dirCnt++] = GoStraight(0);
	}

	Json::Value ret;
	ret["response"]["direction"] = possibleDire[0];

	Json::FastWriter writer;
	cout << writer.write(ret) << endl;

	return 0;
}