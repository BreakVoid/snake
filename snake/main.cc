#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <list>
#include <queue>
#include <string>
#include <vector>
#include <ctime>
#include <climits>
#include "jsoncpp/json.h"

using namespace std;

const int MAX_N = 25;
const int dx[4] = {-1, 0, 1, 0};
const int dy[4] = {0, 1, 0, -1};

bool invalid[MAX_N][MAX_N];
int n, m;

struct Point {
	int x, y;
	Point(const int &_x = 0, const int &_y = 0)
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

int total = 0;
int MakeDecision();

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
	total = input["responses"].size();

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
	ret["response"]["direction"] = MakeDecision();

	Json::FastWriter writer;
	cout << writer.write(ret) << endl;

	return 0;
}

int grid[MAX_N][MAX_N] = {0};
int canPass[MAX_N][MAX_N] = {0};
int dist[MAX_N][MAX_N] = {0};


void MaintainGrid(list<Point> *snake)
{
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			if (invalid[i][j]) {
				grid[i][j] = INT_MAX;
			} else {
				grid[i][j] = 0;
			}
		}
	}
	for (int id = 0; id <= 1; ++id) {
		int roundCnt = total + 1;
		for (list<Point>::reverse_iterator iter = snake[id].rbegin(); iter != snake[id].rend(); ++iter) {
			while (whetherGrow(roundCnt)) {
				++roundCnt;
			}
			grid[iter->x][iter->y] = (roundCnt++) - total;
		}
	}
}

void CalcCanPass(list<Point> *snake)
{
	int uX = snake[0].front().x;
	int uY = snake[0].front().y;
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			dist[i][j] = abs(uX - i) + abs(uY - j);
			if (dist[i][j] >= grid[i][j]) {
				canPass[i][j] = true;
			} else {
				canPass[i][j] = false;
			}
		}
	}
}

void BFS(const Point &S)
{
	memset(dist, 0x7f, sizeof(dist));
	queue<Point> que;
	que.push(S);
	dist[S.x][S.y] = 0;
	while (!que.empty()) {
		for (int i = 0; i < 4; ++i) {
			int X = que.front().x + dx[i];
			int Y = que.front().y + dy[i];
			if (X < 1 || Y < 1 || X > n || Y > m || !canPass[X][Y]) {
				continue;
			}
			if (dist[X][Y] > dist[que.front().x][que.front().y] + 1) {
				dist[X][Y] = dist[que.front().x][que.front().y] + 1;
				que.push(Point(X, Y));
			}
		}
		que.pop();
	}
}

inline Point GetHead(const int &id)
{
	return snake[id].front();
}

inline Point GetTail(const int &id)
{
	return snake[id].back();
}

int MakeDecision()
{
	srand((unsigned)time(NULL));
	MaintainGrid(snake);
	CalcCanPass(snake);
	Point head = GetHead(0);
	BFS(head);
	Point target(-1, -1);
	for (list<Point>::iterator iter = snake[1].begin(); iter != snake[1].end(); iter++) {
		if (dist[iter->x][iter->y] < 0x7f7f7f7f) {
			target = Point(iter->x, iter->y);
		}
	}
	if (target.x == -1 && target.y == -1) {
		int maxDist = -1;
		for (int i = 1; i <= n; ++i) {
			for (int j = 1; j <= m; ++j) {
				if (dist[i][j] < 0x7f7f7f7f) {
					maxDist = max(maxDist, dist[i][j]);
				}
			}
		}
		vector<Point> choice;
		for (int i = 1; i <= n; ++i) {
			for (int j = 1; j <= m; ++j) {
				if (dist[i][j] == maxDist) {
					choice.push_back(Point(i, j));
				}
			}
		}
		target = choice[rand() % choice.size()];
	}
	BFS(target);
MAKE_DECISION:
	{
		deque<int> choice;
		int farthest = 0;
		for (int i = 0; i < 4; ++i) {
			int X = head.x + dx[i];
			int Y = head.y + dy[i];
			if (validDirection(0, i)) {
				if (dist[X][Y] < 0x7f7f7f7f && farthest <= dist[X][Y]) {
					farthest = dist[X][Y];
					choice.push_front(i);
				} else if (dist[X][Y] < 0x7f7f7f7f) {
					choice.push_back(i);
				}
			}
		}
		if (farthest > snake[0].size() * 2 / 3) {
			return choice.back();
		} else {
			return choice.front();
		}
	}
RANDOM_DECISION:
	{
		vector<int> choice;
		for (int i = 0; i < 4; ++i) {
			if (validDirection(0, i)) {
				choice.push_back(i);
			}
		}
		return choice[rand() % choice.size()];
	}
}