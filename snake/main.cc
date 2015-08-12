#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <climits>

#include <iostream>
#include <sstream>
#include <fstream>

#include <list>
#include <vector>
#include <queue>
#include <deque>
#include <string>
#include <ctime>
#include "jsoncpp/json.h"

using namespace std;
int n, m;
const int MAX_N = 25;
const int dx[4] = {-1,0,1,0};
const int dy[4] = {0,1,0,-1};
bool invalid[MAX_N][MAX_N];

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

void Move(list<Point> *snake, const int &id, const int &dire, const int &num)  //编号为id的蛇朝向dire方向移动一步
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

int total = 0;

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
		Move(snake, 0, dire, i);

		dire = input["requests"][i + 1]["direction"].asInt();
		Move(snake, 1, dire, i);
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

pair<int, int> Search(list<Point> *snake, const int &step, const int &round);

int MakeDecision()
{
	pair<int, int> result = Search(snake, 4, total);
	return result.first;
}

int grid[MAX_N][MAX_N] = {0};
int canPass[MAX_N][MAX_N] = {0};
int dist[MAX_N][MAX_N] = {0};

#ifdef _DEBUG
void Debug(string filename)
{
	ofstream debug(filename);
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			debug << setw(12) << grid[i][j];
		}
		debug << endl;
	}
	debug << endl;
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			debug << setw(12) << canPass[i][j];
		}
		debug << endl;
	}
	debug << endl;
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			debug << setw(12) << dist[i][j];
		}
		debug << endl;
	}
	debug << endl;
	debug.close();
}
#endif

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

Point que[MAX_N * MAX_N];

int Evaluate(list<Point> *snake)
{
	MaintainGrid(snake);
	CalcCanPass(snake);
	memset(dist, 0x7f, sizeof(dist));
	int tail = 0;
	que[tail++] = snake[0].front();
	for (int i = 0; i < tail; ++i) {
		Point u = que[i];
		for (int i = 0; i < 4; ++i) {
			int X = u.x + dx[i];
			int Y = u.y + dy[i];
			if (X < 1 || Y < 1 || X > n || Y > m) {
				continue;
			}
			if (dist[X][Y] > dist[u.x][u.y] + 1) {
				dist[X][Y] = dist[u.x][u.y] + 1;
				que[tail++] = Point(X, Y);
			}
		}
	}
	return tail;
}

pair<int, int> Search(list<Point> *snake, const int &step, const int &round)
{
	if (step == 0) {
		return make_pair(-1, Evaluate(snake));
	} else {
		pair<int, int> best = make_pair(-1, -1);
		for (int i = 0; i < 4; ++i) {
			if (validDirection(snake, 0, i)) {
				bool flag = false;
				for (int j = 0; j < 4; ++j) {
					if (validDirection(snake, 1, j)) {
						flag = true;
						list<Point> newSnake[2];
						for (int id = 0; id <= 1; ++id) {
							for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter) {
								newSnake[id].push_back(*iter);
							}
						}
						Move(newSnake, 0, i, round);
						Move(newSnake, 1, j, round);
						pair<int, int> result = Search(newSnake, step - 1, round + 1);
						if (result.second > best.second) {
							best = result;
							best.first = i;
						} else if (result.second == best.second) {
							int rate = rand() % 10000;
							if (rate > 6180) {
								best.first = i;
							}
						}
					}
				}
				if (!flag) {
					return make_pair(i, INT_MAX);
				}
			}
		}
		return best;
	}
}