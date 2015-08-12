#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <climits>

#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

#include <list>
#include <queue>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>>

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

int total;
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
int dangerous[MAX_N][MAX_N] = {0};

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
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			debug << setw(12) << dangerous[i][j];
		}
		debug << endl;
	}
	debug << endl;
	debug.close();
}
#endif

void MaintainGrid()
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

bool isInBody(const int &x, const int &y, list<Point> snake[2])   //判断(x,y)位置是否有蛇
{
	for (int id = 0; id <= 1; id++)
		for (list<Point>::iterator iter = snake[id].begin(); iter != snake[id].end(); ++iter)
			if (x == iter->x && y == iter->y)
				return true;
	return false;
}

void CalcCanPass()
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

void CalcDangerous()
{
	int status[MAX_N][MAX_N] = {0};
	queue<Point> que;
	for (int i = 1; i <= n; ++i) {
		for (int j = 1; j <= m; ++j) {
			if (!canPass[i][j]) {
				dangerous[i][j] = 10000;
				status[i][j] = 2;
				continue;
			}
			int validDireCnt = 0;
			for (int k = 0; k < 4; ++k) {
				int X = i + dx[k];
				int Y = j + dy[k];
				if (X < 1 || Y < 1 || X > n || Y > m) {
					continue;
				}
				if (canPass[X][Y]) {
					validDireCnt++;
				} else {
					if (snake[0].front().x == X && snake[0].front().y == Y) {
						validDireCnt++;
					}
				}
			}
			if (validDireCnt <= 1) {
				dangerous[i][j] = 1000;
				que.push(Point(i, j));
				status[i][j] = 1;
			}
		}
	}
	Point head1 = snake[1].front();
	for (int k = 0; k < 4; ++k) {
		int X = head1.x + dx[k];
		int Y = head1.y + dy[k];
		if (X < 1 || Y < 1 || X > n || Y > m) {
			continue;
		}
		if (canPass[X][Y]) {
			dangerous[X][Y] = 1000;
			que.push(Point(X, Y));
			status[X][Y] = 1;
		}
	}
	while (!que.empty()) {
		int uX = que.front().x;
		int uY = que.front().y;
		status[uX][uY] = 2;
		que.pop();
		for (int i = 0; i < 4; ++i) {
			int X = uX + dx[i];
			int Y = uY + dy[i];
			if (X < 1 || Y < 1 || X > n || Y > m || status[X][Y] == 2) {
				continue;
			}
			dangerous[X][Y] += dangerous[uX][uY] >> 1;
			if (status[X][Y] == 0) {
				status[X][Y] = 1;
				que.push(Point(X, Y));
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

struct DecisionType {
	int dire, dist, dang;
	DecisionType(const int &_dire = 0, const int &_dist = 0, const int &_dang = 0)
		: dire(_dire), dist(_dist), dang(_dang)
	{}
};

bool operator<(const DecisionType &lhs, const DecisionType &rhs)
{
	if (lhs.dang <= 125 && rhs.dang <= 125) {
		return lhs.dist > rhs.dist;
	} else if (lhs.dang > 125 && rhs.dang > 125) {
		return lhs.dang < rhs.dang;
	} else {
		return lhs.dang <= 125;
	}
}

int Choose(vector<DecisionType> choice)
{
	sort(choice.begin(), choice.end());
	const int rate = 850;
	for (int i = 0; i < choice.size() - 1; ++i) {
		if (rand() % 1000 < rate) {
			return choice[i].dire;
		}
	}
	return choice.back().dire;
}

int SearchDecision();
int RandomDecision();

int MakeDecision()
{
	srand((unsigned)time(NULL));
	MaintainGrid();
	CalcCanPass();
	CalcDangerous();
#ifdef _DEBUG
	void Debug(string);
	Debug("find-can-pass-debug.txt");
#endif
	Point head = GetHead(0);
	Point tail = GetTail(0);
	//BFS from the tail of snake No.0
	BFS(tail);
#ifdef _DEBUG
	void Debug(string);
	Debug("find-tail-debug.txt");
#endif
	if (dist[head.x][head.y] < 0x7f7f7f7f) {
		vector<DecisionType> choice;
		for (int i = 0; i < 4; ++i) {
			if (validDirection(0, i)) {
				if (dist[head.x + dx[i]][head.y + dy[i]] < 0x7f7f7f7f) {
					choice.push_back(DecisionType(i, dist[head.x + dx[i]][head.y + dy[i]], dangerous[head.x + dx[i]][head.y + dy[i]]));
				}
			}
		}
		return Choose(choice);
	} else {
		Point tai = GetTail(1);
		BFS(tail);
		if (dist[head.x][head.y] < 0x7f7f7f7f) {
			vector<DecisionType> choice;
			for (int i = 0; i < 4; ++i) {
				if (validDirection(0, i)) {
					if (dist[head.x + dx[i]][head.y + dy[i]] < 0x7f7f7f7f) {
						choice.push_back(DecisionType(i, dist[head.x + dx[i]][head.y + dy[i]], dangerous[head.x + dx[i]][head.y + dy[i]]));
					}
				}
			}
			return Choose(choice);
		} else {
			return SearchDecision();
		}
	}
}

int BlockSize(Point S)
{
	memset(dist, 0x7f, sizeof(dist));
	queue<Point> que;
	que.push(S);
	dist[S.x][S.y] = 0;
	int cnt = 1;
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
				cnt++;
			}
		}
		que.pop();
	}
	return cnt;
}

int SearchDecision()
{
	vector<DecisionType> choice;
	for (int i = 0; i < 4; ++i) {
		if (validDirection(0, i)) {
			int w = BlockSize(Point(snake[0].front().x + dx[i], snake[0].front().y + dy[i]));
			choice.push_back(DecisionType(i, w, dangerous[snake[0].front().x + dx[i]][snake[0].front().y + dy[i]]));
		}
	}
	return Choose(choice);
}

int RandomDecision()
{
	vector<int> choice;
	for (int i = 0; i < 4; ++i) {
		if (validDirection(0, i)) {
			choice.push_back(i);
		}
	}
	return choice[rand() % choice.size()];
}