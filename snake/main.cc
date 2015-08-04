#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <string>
#include <ctime>
#include "jsoncpp/json.h"

using namespace std;

const int MAX_N = 25;
const int dx[4] = {-1, 0, 1, 0};
const int dy[4] = {0, 1, 0, -1};

bool invalid[MAX_N][MAX_N];
int n, m;

struct Point {
	int x, y;
	Point(const int &_x, const int &_y)
		: x(_x), y(_y)
	{}
};

list<Point> snake[2]; // 0表示自己的蛇，1表示对方的蛇
int possibleDire[10];
int posCount;

bool whetherGrow(int num)  //本回合是否生长
{
	if (num <= 9) return true;
	if ((num - 9) % 3 == 0) return true;
	return false;
}

void deleteEnd(int id)     //删除蛇尾
{
	snake[id].pop_back();
}

void move(int id, int dire, int num)  //编号为id的蛇朝向dire方向移动一步
{
	Point p = *(snake[id].begin());
	int x = p.x + dx[dire];
	int y = p.y + dy[dire];
	snake[id].push_front(Point(x, y));
	if (!whetherGrow(num))
		deleteEnd(id);
}
void outputSnakeBody(int id)    //调试语句
{
	cout << "Snake No." << id << endl;
	for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
		cout << iter->x << " " << iter->y << endl;
	cout << endl;
}

bool isInBody(int x, int y)   //判断(x,y)位置是否有蛇
{
	for (int id = 0; id <= 1; id++)
		for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
			if (x == iter->x && y == iter->y)
				return true;
	return false;
}

bool validDirection(int id, int k)  //判断当前移动方向的下一格是否合法
{
	Point p = *(snake[id].begin());
	int x = p.x + dx[k];
	int y = p.y + dy[k];
	if (x > n || y > m || x < 1 || y < 1) return false;
	if (invalid[x][y]) return false;
	if (isInBody(x, y)) return false;
	return true;
}

int canPass[MAX_N][MAX_N] = {0};

void MaintainMap()
{
	
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			if (invalid[i][j]) {
				canPass[i][j] = 100000;
			} else {
				canPass[i][j] = 0;
			}
		}
	}
	for (int id = 0; id <= 1; id++) {
		int len = 1;
		for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter, ++len) {
			canPass[iter->x][iter->y] = len;
		}
	}
}

int BFS(const int &sX, const int &sY)
{
	bool vis[MAX_N][MAX_N] = {0};
	vector<Point> que;
	que.push_back(Point(sX, sY));
	vis[sX][sY] = true;
	for (int i = 0; i < que.size(); ++i) {
		int uX = que[i].x;
		int uY = que[i].y;
		for (int i = 0; i < 4; ++i) {
			int tX = uX + dx[i];
			int tY = uY + dy[i];
			if (tX > n || tY > m || tX < 1 || tY < 1) {
				continue;
			}
			if (canPass[tX][tY]) {
				continue;
			}
			if (!vis[tX][tY]) {
				vis[tX][tY] = true;
				que.push_back(Point(tX, tY));
			}
		}
	}
	return que.size();
}

int CurHeadDirection(int id)
{
	list<Point>::iterator iter = snake[id].begin();
	Point head = *iter++;
	Point next = *iter;
	if (head.x == next.x) {
		if (head.y == next.y + 1) {
			return 1;
		} else {
			return 3;
		}
	} else {
		if (head.x == next.y + 1) {
			return 2;
		} else {
			return 0;
		}
	}
}

int MakeDecision(Json::Value &ret)
{
	MaintainMap();
	vector<int> consider;
	int bestCnt = 0;
	int headX = snake[0].begin()->x;
	int headY = snake[0].begin()->y;
	for (int dir = 0; dir < 4; ++dir) {
		if (validDirection(0, dir)) {
			int sX = headX + dx[dir];
			int sY = headY + dy[dir];
			int result = BFS(sX, sY);
			ret["response"]["debug"][char(dir + '0')]["result"] = result;
			if (bestCnt < result) {
				consider.clear();
				consider.push_back(dir);
				bestCnt = result;
			} else if (bestCnt == result) {
				consider.push_back(dir);
			}
		}
	}
	int cur = CurHeadDirection(0);
	if (consider.size() > 1) {
		for (int i = 0; i < consider.size(); ++i) {
			if (consider[i] == cur) {
				continue;
			}
			return consider[i];
		}
	}
	return consider.front();
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
		deleteEnd(0);
		deleteEnd(1);
	}

	Json::Value ret;
	ret["response"]["direction"] = MakeDecision(ret);

	Json::FastWriter writer;
	cout << writer.write(ret) << endl;

	return 0;
}