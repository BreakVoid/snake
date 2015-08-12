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

struct Point {
	int x, y;
	Point(const int &_x = 0, const int &_y = 0)
		: x(_x), y(_y)
	{}
};

list<Point> snake[2]; // 0表示自己的蛇，1表示对方的蛇
int possibleDire[10];
int posCount;

bool whetherGrow(const int &num)  //本回合是否生长
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}

void deleteEnd(list<Point> *snake, const int &id)     //删除蛇尾
{
	snake[id].pop_back();
}

void move(list<Point> *snake, const int &id, const int &dire, const int &num)  //编号为id的蛇朝向dire方向移动一步
{
	Point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(Point(x, y));
	if (!whetherGrow(num))
		deleteEnd(snake, id);
}
void outputSnakeBody(list<Point> *snake, const int &id)    //调试语句
{
	cout << "Snake No." << id << endl;
	for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
		cout << iter->x << " " << iter->y << endl;
	cout << endl;
}

bool isInBody(list<Point> *snake, const int &x, const int &y)   //判断(x,y)位置是否有蛇
{
	for (int id = 0; id <= 1; id++)
		for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
			if (x == iter->x && y == iter->y)
				return true;
	return false;
}

bool validDirection(list<Point> *snake, const int &id, const int &k)  //判断当前移动方向的下一格是否合法
{
	Point p = *(snake[id].begin());
	int x = p.x + dx[k];
	int y = p.y + dy[k];
	if (x > n || y > m || x < 1 || y < 1) return false;
	if (invalid[x][y]) return false;
	if (isInBody(snake, x, y)) return false;
	return true;
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

	n = input["requests"][(Json::Value::UInt) 0]["height"].asInt();  //棋盘高度
	m = input["requests"][(Json::Value::UInt) 0]["width"].asInt();   //棋盘宽度

	int x = input["requests"][(Json::Value::UInt) 0]["x"].asInt();  //读蛇初始化的信息
	if (x == 1) {
		snake[0].push_front(Point(1, 1));
		snake[1].push_front(Point(n, m));
	} else {
		snake[1].push_front(Point(1, 1));
		snake[0].push_front(Point(n, m));
	}
	//处理地图中的障碍物
	int obsCount = input["requests"][(Json::Value::UInt) 0]["obstacle"].size();

	for (int i = 0; i < obsCount; i++) {
		int ox = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["x"].asInt();
		int oy = input["requests"][(Json::Value::UInt) 0]["obstacle"][(Json::Value::UInt) i]["y"].asInt();
		invalid[ox][oy] = 1;
	}

	//根据历史信息恢复现场
	int total = input["responses"].size();

	int dire;
	for (int i = 0; i < total; i++) {
		dire = input["responses"][i]["direction"].asInt();
		move(0, dire, i);

		dire = input["requests"][i + 1]["direction"].asInt();
		move(1, dire, i);
	}

	if (!whetherGrow(total)) // 本回合两条蛇生长
	{
		deleteEnd(snake, 0);
		deleteEnd(snake, 1);
	}
	
	int MakeDecision();
	Json::Value ret;
	ret["response"]["direction"] = MakeDecision();

	Json::FastWriter writer;
	cout << writer.write(ret) << endl;

	return 0;
}



int MakeDecision()
{

}