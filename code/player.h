#include "../include/playerbase.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <iostream>
#include <queue>
#include <string>
#include <stack>
#include <list>
#define MAX 11

// #define DEBUG
// #define DEBUG_SNAKE

using namespace std;

typedef struct _point {
    int x, y;
    string path;
} Node;

typedef pair<int, int> Snake;

list<Snake> snake;
string path;
queue<Node> q1;
stack<Node> st;
Node possibleFood;

int row, col;       // copy of row_cnt and col_cnt (ignore map shrinking!!!)
char** map;  // copy of player->mat
int currentDircetion;
int vis[MAX][MAX];
struct Player* pl;
int shrinkTime;
int mapTwo[MAX][MAX];
int visTwo[MAX][MAX];
int adjust;
char body;
char opBody;

int step[4][2] = { {0, 1}, {1, 0}, {0, -1}, {-1, 0} };  // E, S, W, N
string dir[4] = { "0", "1", "2", "3" };

void init(struct Player* player) {}

void findAllFoodAndShield(struct Player* player) {
    for (int i = 0 + shrinkTime; i < row - shrinkTime; i++) {
        for (int j = 0 + shrinkTime; j < col - shrinkTime; j++) {
            if (map[i][j] == 'o' || map[i][j] == 'O') {
                Node food = { i, j, "" };
                st.push(food);
            }
        }
    }
}

bool inBound(Node p) {
    return p.x >= 0 + shrinkTime && p.x < row - shrinkTime && p.y >= 0 + shrinkTime && p.y < col - shrinkTime;
}

void clear(queue<Node>& q) {
    queue<Node> empty;
    swap(empty, q);
}

bool bfs(Node begin, Node dest) {
    clear(q1);
    vis[begin.x][begin.y] = 1;
    q1.push(begin);
    while (q1.size() > 0) {
        Node now = q1.front();
        q1.pop();
        for (int i = 0; i < 4; i++) {
            Node next = { now.x + step[i][0], now.y + step[i][1], now.path };
            next.path.append(dir[i]);
            if (next.x == dest.x && next.y == dest.y) {
                string tempPath = next.path;
                if (map[dest.x][dest.y] != body && tempPath.size() < path.size()) {  // search from head to food
                    path.assign(tempPath);      // update path
                    possibleFood.x = dest.x;
                    possibleFood.y = dest.y;
                }
                else if (map[begin.x][begin.y] == body && map[dest.x][dest.y] == body) {  // search from head to tail
                    path.assign(tempPath);
                }
                #ifdef DEBUG
                printf("final: (%d, %d)\n", next.x, next.y);
                #endif
                return true;
            }

            if (!inBound(next) || map[next.x][next.y] == '#' || map[next.x][next.y] == opBody) {
                continue;
            }
            if (begin.x != pl->your_posx && begin.y != pl->your_posy) {     // search from food
                if (next.x != snake.back().first && next.y != snake.back().second) {    // not tail
                    if (map[next.x][next.y] == body) {
                        continue;
                    }
                }
            }
            else if (map[next.x][next.y] == body || map[next.x][next.y] == opBody) {      // search from head, next node is body
                continue;
            }

            if (!vis[next.x][next.y]) {     // find a valid Node
                vis[next.x][next.y] = 1;
                q1.push(next);
            }
        }
    }
    return false;
}

void updateSnake(int x, int y) {
    snake.push_front(make_pair(x, y));
    int basicLen = (int)ceil(3.0 * (row + col) / 8);
    if ((int)snake.size() > basicLen + adjust)      // link new head and delete tail
        if (map[x][y] == 'o' || map[x][y] == 'O');     // is food
        else snake.pop_back();
    else
        if (map[x][y] == 'o' || map[x][y] == 'O') ++adjust;     // increase basic length

    #ifdef DEBUG_SNAKE
    list<Snake>::iterator it = snake.begin();
    for (; it != snake.end(); ++it) {
        printf("(%d, %d)\n", (*it).first, (*it).second);
    }
    #endif
}

void findPath() {
    path.assign(100, '0');  // initial path with max length
    findAllFoodAndShield(pl);
    int cnt = st.size();
    for (int i = 0; i < cnt; i++) {       // bfs from head to each food or shield
        Node dest = st.top();           // get a destinate point
        st.pop();       // remove top element from stack
        Node begin = { pl->your_posx, pl->your_posy, "" };
        memset(vis, 0, sizeof(vis));
        bfs(begin, dest);
    }
    string test(100, '0');
    if (path == test)
        path.clear();
}

int findAVacantBlock(int sx, int sy) {
    Node qu[100];
    Node w;
    w.x = sx;
    w.y = sy;
    int front = -1;
    int rear = -1;
    front = front + 1;
    qu[front] = w;
    visTwo[sx][sy] = 1;
    while (front > rear) {
        rear += 1;
        Node now = qu[rear];
        for (int i = 0; i < 4; i++) {
            Node t = { now.x + step[i][0], now.y + step[i][1], now.path };\
                if (inBound(t) && !visTwo[t.x][t.y] && mapTwo[t.x][t.y] == 1) {
                    visTwo[t.x][t.y] = 1;
                    w.x = t.x;
                    w.y = t.y;
                    front += 1;
                    qu[front] = w;
                }
        }
    }
    return front + 1;
}

string findMaxUnion(int sx, int sy) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            if (map[i][j] == '#' || map[i][j] == body || map[i][j] == opBody || map[i][j] == '3')
                mapTwo[i][j] = 0;
            else
                mapTwo[i][j] = 1;
        }
    }
    memset(visTwo, 0, sizeof(visTwo));
    int max = 0, iNext = -1;
    bool flag = false;
    for (int i = 0; i < 4; i++) {
        int nx = sx + step[i][0];
        int ny = sy + step[i][1];
        int tmp = 0;
        if (mapTwo[nx][ny] == 1 && !visTwo[nx][ny]) {
            printf("tmp2 is %d\n", tmp);
            tmp = findAVacantBlock(nx, ny);
            printf("tmp1 is %d\n", tmp);
            if (tmp > max) {
                iNext = i;
                max = tmp;
                flag = true;
            }
        }
    }
    printf("flag: %d, iNext: %d\n", flag, iNext/*, dir[iNext].c_str()*/);
    if (flag)
        return dir[iNext];
    else
        return "";
        }

void checkSecurity() {
    Node tail = { snake.back().first, snake.back().second, "" };
    memset(vis, 0, sizeof(vis));
    if (!path.size()) goto out;
    if (!bfs(possibleFood, tail)) {    // cannot find tail from food
    out:
        memset(vis, 0, sizeof(vis));
        Node begin = { pl->your_posx, pl->your_posy, "" };
        #ifdef DEBUG
        printf("food->tail: fail.\n");
        #endif
        if (bfs(begin, tail)) {
        #ifdef DEBUG
            printf("head->tail: %s\n", path.c_str());
        #endif
        }
        else {      // try to find the largest vacant block, if cannot find it, the snake will commit suicide
        #ifdef DEBUG
            printf("head->tail: fail.\n");
        #endif
            path.assign(findMaxUnion(begin.x, begin.y));
        }
    }
    #ifdef DEBUG
    else {
        printf("food->tail: success.\n");
    }
    #endif
}

void move() {
    // if path is null, snake remain unmoved(suicide)
    if (path.size()) {
        currentDircetion = path.at(0) - '0';
        pl->your_posx += step[currentDircetion][0];
        pl->your_posy += step[currentDircetion][1];
        path.erase(0, 1);       // update path
        path.clear();           // timely update
    }
}

void roundShrink() {
    if (pl->round_to_shrink == 4)
        shrinkTime += 1;
}

struct Point walk(struct Player* player) {
    #ifdef DEBUG
    for (int i = 0; i < player->row_cnt; i++) {
        for (int j = 0; j < player->col_cnt; j++) {
            printf("%c", player->mat[i][j]);
        }
        printf("\n");
    }
    printf("\n");
    #endif

    // initialize variables
    pl = player;
    map = pl->mat;
    row = pl->row_cnt;
    col = pl->col_cnt;

    // initalize a virtual snake
    if (snake.size() == 0) {
        body = map[pl->your_posx][pl->your_posy];
        opBody = '2';
        if (body == '2')
            opBody = '1';
        snake.push_back(make_pair(player->your_posx, player->your_posy));
        adjust = 0;
    }

    roundShrink();

    if (!path.size()) {
        findPath();
        #ifdef DEBUG
        if (path.size())
            printf("head->food: success.\n");
        else
            printf("head->food: fail.\n");
        printf("possibleFood: (%d, %d)\n", possibleFood.x, possibleFood.y);
        printf("tail: (%d, %d)\n", snake.back().first, snake.back().second);
        printf("path: %s\n", path.c_str());
        printf("round_to_shrink: %d\nshrinkTime: %d\n", pl->round_to_shrink, shrinkTime);
        #endif

        // now, 'path' is the shortest path between snake and food, bfs from food to tail
        checkSecurity();
    }
    move();
    updateSnake(pl->your_posx, pl->your_posy);

    return initPoint(player->your_posx, player->your_posy);
}
