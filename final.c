#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

// board information
#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2

// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0

// option
typedef int OPTION;
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3
#define UP_LEFT 4
#define UP_RIGHT 5
#define DOWN_LEFT 6
#define DOWN_RIGHT 7

#define MAX_BYTE 10000

#define START "START"
#define PLACE "PLACE"
#define TURN  "TURN"
#define END   "END"

struct Command
{
    int x;
    int y;
    OPTION option;
};

char buffer[MAX_BYTE] = {0};
char board[BOARD_SIZE][BOARD_SIZE] = {0};
int me_flag;
int other_flag;

const int DIR[8][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1} };

void debug(const char *str) {
    printf("DEBUG %s\n", str);
    fflush(stdout);
}

void printBoard() {
    char visual_board[BOARD_SIZE][BOARD_SIZE] = {0};
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == EMPTY) {
                visual_board[i][j] = '.';
            } else if (board[i][j] == BLACK) {
                visual_board[i][j] = 'O';
            } else if (board[i][j] == WHITE) {
                visual_board[i][j] = 'X';
            }
        }
        printf("%s\n", visual_board[i]);
    }
}

BOOL isInBound(int x, int y) {
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

void initAI(int me) {
    
}
char myboard[BOARD_SIZE][BOARD_SIZE] = {0};//真正移动棋子的棋盘
int best_x = 0;
int best_y = 0;
OPTION best_opt = 0;
clock_t begin, finish;//计时
double  duration;//所用时间
BOOL place(int x,int y,OPTION option,int cur_flag);

void  Reverse(int new_x,int new_y,char myboard[BOARD_SIZE][BOARD_SIZE],int flag)//挑夹判断吃子（直接copy了助教大大的代码）
{
    int other_flag = 3 - flag;
    int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
    for (int i = 0; i < 4; i++)
    {
        int x1 = new_x + intervention_dir[i][0];
        int y1 = new_y + intervention_dir[i][1];
        int x2 = new_x - intervention_dir[i][0];
        int y2 = new_y - intervention_dir[i][1];
        if (isInBound(x1, y1) && isInBound(x2, y2) && myboard[x1][y1] == other_flag && myboard[x2][y2] == other_flag)
        {
            myboard[x1][y1] = flag;
            myboard[x2][y2] = flag;
        }
    }
    int custodian_dir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    for (int i = 0; i < 8; i++)
    {
        int x1 = new_x + custodian_dir[i][0];
        int y1 = new_y + custodian_dir[i][1];
        int x2 = new_x + custodian_dir[i][0] * 2;
        int y2 = new_y + custodian_dir[i][1] * 2;
        if (isInBound(x1, y1) && isInBound(x2, y2) && myboard[x2][y2] == flag && myboard[x1][y1] == other_flag)
        {
            myboard[x1][y1] = flag;
        }
    }
}

//
double variance(char myboard[BOARD_SIZE][BOARD_SIZE],int flag)
{
    double x_average;//x平均值
    double y_average;//y平均值
    double s1,s2,s;//分别是x，y，xy的方差
    int x_sum = 0;//x坐标的和
    int y_sum = 0;//y坐标的和
    int count = 0;//棋子数目
    double x_square = 0;//x平方和
    double y_square = 0;//y平方和
    for(int x = 0; x < BOARD_SIZE; x++)
    {
        for(int y = 0; y < BOARD_SIZE; y++)
        {
            if(myboard[x][y] == flag)
            {
                x_sum = x_sum + x;
                y_sum = y_sum + y;
                count = count + 1;
            }
        }
    }
    x_average = x_sum / count;
    y_average = y_sum / count;
    for(int x = 0; x < BOARD_SIZE; x++)
    {
        for(int y = 0; y < BOARD_SIZE; y++)
        {
            if(myboard[x][y] == flag)
            {
                x_square = x_square + (x - x_average) * (x - x_average);
                y_square = y_square + (y - y_average) * (y - y_average);
            }
        }
    }
    s1 = sqrt(x_square/(count-1));
    s2 = sqrt(y_square/(count-1));
    s = sqrt(s1*s1+ s2 * s2);
    //printf("%.2lf %.2lf %.2lf",s,x_average,y_average);
    return s;
}


int evaluate(char myboard[BOARD_SIZE][BOARD_SIZE],int flag)
{
    int value = 0;//估值
    double s = 0.00;//方差
    int countt = 0;
    int cur_flag = 3 - flag;
    for(int x = 0; x < BOARD_SIZE; x++)
    {
        for(int y = 0; y < BOARD_SIZE; y++)
        {
            if(myboard[x][y] == flag)
            {
                countt += 1;
                value = value + 400;//每个棋子400分
                /*                    出现2*3局面
                                      类似以下形状              */
                /*                        O.                  */
                /*                        ..                  */
                /*                        .O                  */
                if((isInBound(x - 2,y - 1) && myboard[x - 2][y - 1] == flag) ||
                   (isInBound(x - 2,y + 1) && myboard[x - 2][y + 1] == flag) ||
                   (isInBound(x + 2,y - 1) && myboard[x + 2][y - 1] == flag) ||
                   (isInBound(x + 2,y + 1) && myboard[x + 2][y + 1] == flag) ||
                   (isInBound(x - 1,y - 2) && myboard[x - 1][y - 2] == flag) ||
                   (isInBound(x - 1,y + 2) && myboard[x - 1][y + 2] == flag) ||
                   (isInBound(x + 1,y - 2) && myboard[x + 1][y - 2] == flag) ||
                   (isInBound(x + 1,y + 2) && myboard[x + 1][y + 2] == flag))
                {
                    value = value + 20;
                }
                /*                    出现2*4局面
                                      类似以下形状              */
                /*                        O.                  */
                /*                        ..                  */
                /*                        ..                  */
                /*                        .O                  */
                if((isInBound(x - 3,y - 1) && myboard[x - 3][y - 1] == flag) ||
                   (isInBound(x - 3,y + 1) && myboard[x - 3][y + 1] == flag) ||
                   (isInBound(x + 3,y - 1) && myboard[x + 3][y - 1] == flag) ||
                   (isInBound(x + 1,y + 3) && myboard[x + 1][y + 3] == flag) ||
                   (isInBound(x - 1,y - 3) && myboard[x - 1][y - 3] == flag) ||
                   (isInBound(x - 1,y + 3) && myboard[x - 1][y + 3] == flag) ||
                   (isInBound(x + 1,y - 3) && myboard[x + 1][y - 3] == flag) ||
                   (isInBound(x + 1,y + 3) && myboard[x + 1][y + 3] == flag))
                {
                    value = value + 20;
                }
                if(
                   (myboard[x][y-2] == flag && myboard[x][y-1] == EMPTY &&
                    (myboard[x-1][y]==cur_flag||myboard[x-1][y-1]==cur_flag||myboard[x-1][y-2]==cur_flag||
                     myboard[x+1][y]==cur_flag||myboard[x+1][y-1]==cur_flag||myboard[x+1][y-2]==cur_flag))||
                   (myboard[x][y+2] == flag && myboard[x][y+1] == EMPTY &&
                    (myboard[x-1][y]==cur_flag||myboard[x-1][y+1]==cur_flag||myboard[x-1][y+2]==cur_flag||
                     myboard[x+1][y]==cur_flag||myboard[x+1][y+1]==cur_flag||myboard[x+1][y+2]==cur_flag))||
                   (myboard[x-2][y] == flag && myboard[x-1][y] == EMPTY &&
                    (myboard[x][y-1]==cur_flag||myboard[x-1][y-1]==cur_flag||myboard[x-2][y-1]==cur_flag||
                     myboard[x][y+1]==cur_flag||myboard[x-1][y+1]==cur_flag||myboard[x-1][y+1]==cur_flag))||
                   (myboard[x+2][y] == flag && myboard[x+1][y] == EMPTY &&
                    (myboard[x][y-1]==cur_flag||myboard[x+1][y-1]==cur_flag||myboard[x+2][y-1]==cur_flag||
                     myboard[x][y+1]==cur_flag||myboard[x+1][y+1]==cur_flag||myboard[x+1][y+1]==cur_flag))||
                   (myboard[x-2][y-2] == flag && myboard[x-1][y-1] == EMPTY &&
                    (myboard[x][y-1]==cur_flag||myboard[x][y-2]==cur_flag||myboard[x-1][y-2]==cur_flag||
                     myboard[x-2][y]==cur_flag||myboard[x-1][y]==cur_flag||myboard[x-2][y-1]==cur_flag))||
                   (myboard[x-2][y+2] == flag && myboard[x-1][y+1] == EMPTY &&
                    (myboard[x][y+1]==cur_flag||myboard[x][y+2]==cur_flag||myboard[x-1][y+2]==cur_flag||
                     myboard[x-2][y]==cur_flag||myboard[x-1][y]==cur_flag||myboard[x-2][y+1]==cur_flag))||
                   (myboard[x+2][y-2] == flag && myboard[x+1][y-1] == EMPTY &&
                    (myboard[x][y-1]==cur_flag||myboard[x][y-2]==cur_flag||myboard[x+1][y-2]==cur_flag||
                     myboard[x+2][y]==cur_flag||myboard[x+1][y]==cur_flag||myboard[x+2][y-1]==cur_flag))||
                   (myboard[x+2][y+2] == flag && myboard[x+1][y+1] == EMPTY &&
                    (myboard[x][y+1]==cur_flag||myboard[x][y+2]==cur_flag||myboard[x+1][y+2]==cur_flag||
                     myboard[x+2][y]==cur_flag||myboard[x+1][y]==cur_flag||myboard[x+2][y+1]==cur_flag))
                   )
                {
                    value = value - 90;
                }
                /*                   所有可以夹的局面               */
                /*          如                O                   */
                /*                          OX                    */
                if(
                   (myboard[x-1][y] == cur_flag && myboard[x+1][y] == EMPTY &&
                    (myboard[x+1][y-1] == cur_flag || myboard[x+1][y+1] == cur_flag || myboard[x+2][y+1] == cur_flag||
                     myboard[x+2][y] == cur_flag || myboard[x+2][y-1] == cur_flag || myboard[x][y-1] == cur_flag||
                     myboard[x][y+1] == cur_flag))||
                   (myboard[x-1][y+1] == cur_flag && myboard[x+1][y-1] == EMPTY &&
                    (myboard[x+1][y] == cur_flag || myboard[x+2][y] == cur_flag || myboard[x][y-1] == cur_flag ||
                     myboard[x+2][y-1] == cur_flag|| myboard[x+2][y-2] == cur_flag || myboard[x+1][y-2] == cur_flag||
                     myboard[x][y-2] == cur_flag))||
                   (myboard[x][y+1] == cur_flag && myboard[x][y-1] == EMPTY &&
                    (myboard[x-1][y-1] == cur_flag || myboard[x+1][y-1] == cur_flag || myboard[x+1][y-2] == cur_flag ||
                     myboard[x][y-2] == cur_flag || myboard[x+1][y-2] == cur_flag || myboard[x+1][y] == cur_flag ||
                     myboard[x-1][y]==cur_flag))||
                   (myboard[x+1][y-1] == cur_flag && myboard [x-1][y+1] == EMPTY &&
                    (myboard[x+1][y] == cur_flag || myboard[x-2][y] == cur_flag || myboard[x][y+1] == cur_flag ||
                     myboard[x-2][y+1] == cur_flag || myboard[x-2][y+2] == cur_flag || myboard[x-1][y+2] == cur_flag ||
                     myboard[x][y+2] == cur_flag))||
                   (myboard[x][y-1] == cur_flag && myboard[x][y+1] == EMPTY &&
                    (myboard[x+1][y+1] == cur_flag || myboard[x-1][y+1] == cur_flag || myboard[x-1][y+2] == cur_flag ||
                     myboard[x][y+2] == cur_flag || myboard[x-1][y+2] == cur_flag || myboard[x-1][y] == cur_flag ||
                     myboard[x+1][y] == cur_flag))||
                   (myboard[x+1][y+1] == cur_flag && myboard[x-1][y-1] == EMPTY &&
                    (myboard[x-1][y] == cur_flag || myboard[x-1][y-2] == cur_flag || myboard[x][y-1] == cur_flag ||
                     myboard[x][y-2] == cur_flag || myboard[x-2][y-1] == cur_flag || myboard[x-2][y] == cur_flag ||
                     myboard[x-2][y-2] == cur_flag))||
                   (myboard[x+1][y] == cur_flag && myboard[x-1][y] == EMPTY &&
                    (myboard[x-1][y+1] == cur_flag || myboard[x-1][y-1] == cur_flag || myboard[x-2][y-1] == cur_flag ||
                     myboard[x-2][y] == cur_flag || myboard[x-2][y+1] == cur_flag || myboard[x][y+1] == cur_flag ||
                     myboard[x][y-1] == cur_flag))||
                   (myboard[x-1][y-1] == cur_flag && myboard[x+1][y+1] == EMPTY &&
                    (myboard[x+1][y] == cur_flag || myboard[x+1][y+2] == cur_flag || myboard[x][y+1] == cur_flag ||
                     myboard[x][y+2] == cur_flag || myboard[x+2][y+1] == cur_flag || myboard[x+2][y] == cur_flag ||
                     myboard[x+2][y+2] == cur_flag))
                   )
                {
                    value = value - 65;
                }
            }
            if(myboard[x][y] == cur_flag)
            {
                if(x < 11 && x > 0 && y < 11 && y > 0)//±ß½çÇé¿ö²»×÷¿¼ÂÇ Ö±½ÓÏÞÖÆxy·¶Î§
                {
                    /*                 出现至少一对一             */
                    /*          在敌方棋子的对角存在我方棋子        */
                    if(myboard[x - 1][y - 1] == flag || myboard[x - 1][y + 1] == flag || myboard[x + 1][y - 1] == flag || myboard[x + 1][y + 1] == flag)
                    {
                        value = value + 40;
                    }
                    /*                   出现三夹一               */
                    /*                  类似以下局面              */
                    /*                       O                  */
                    /*                      OX                  */
                    /*                        O                 */
                    /*                限制敌方棋子走位            */
                    if((myboard[x][y - 1] == flag && myboard[x - 1][y] == flag && myboard[x + 1][y + 1] == flag)||
                       (myboard[x][y + 1] == flag && myboard[x - 1][y] == flag && myboard[x - 1][y + 1] == flag)||
                       (myboard[x][y - 1] == flag && myboard[x + 1][y] == flag && myboard[x - 1][y + 1] == flag)||
                       (myboard[x][y + 1] == flag && myboard[x + 1][y] == flag && myboard[x - 1][y - 1] == flag))
                    {
                        value = value + 66;
                    }
                }
                /*                   所有可以挑的局面              */
                /*         如              O O                   */
                /*                          X                    */
                if(myboard[x][y-2] == cur_flag && myboard[x][y-1] == EMPTY &&
                    (myboard[x-1][y]==flag||myboard[x-1][y-1]==flag||myboard[x-1][y-2]==flag||
                     myboard[x+1][y]==flag||myboard[x+1][y-1]==flag||myboard[x+1][y-2]==flag))
                {
                    value = value + 139;
                }
                if(myboard[x-2][y] == cur_flag && myboard[x-1][y] == EMPTY &&
                    (myboard[x][y-1]==flag||myboard[x-1][y-1]==flag||myboard[x-2][y-1]==flag||
                     myboard[x][y+1]==flag||myboard[x-1][y+1]==flag||myboard[x-1][y+1]==flag))
                {
                    value = value + 139;
                }
                if(myboard[x+2][y] == cur_flag && myboard[x+1][y] == EMPTY &&
                    (myboard[x][y-1]==flag||myboard[x+1][y-1]==flag||myboard[x+2][y-1]==flag||
                     myboard[x][y+1]==flag||myboard[x+1][y+1]==flag||myboard[x+1][y+1]==flag))
                {
                    value = value + 139;
                }
                if(myboard[x-2][y-2] == cur_flag && myboard[x-1][y-1] == EMPTY &&
                    (myboard[x][y-1]==flag||myboard[x][y-2]==flag||myboard[x-1][y-2]==flag||
                     myboard[x-2][y]==flag||myboard[x-1][y]==flag||myboard[x-2][y-1]==flag))
                {
                    value = value + 139;
                }
                if(myboard[x-2][y+2] == cur_flag && myboard[x-1][y+1] == EMPTY &&
                    (myboard[x][y+1]==flag||myboard[x][y+2]==flag||myboard[x-1][y+2]==flag||
                     myboard[x-2][y]==flag||myboard[x-1][y]==flag||myboard[x-2][y+1]==flag))
                {
                    value = value + 139;
                }
                if(myboard[x+2][y-2] == cur_flag && myboard[x+1][y-1] == EMPTY &&
                    (myboard[x][y-1]==flag||myboard[x][y-2]==flag||myboard[x+1][y-2]==flag||
                     myboard[x+2][y]==flag||myboard[x+1][y]==flag||myboard[x+2][y-1]==flag))
                {
                    value = value + 139;
                }                
                if(myboard[x+2][y+2] == cur_flag && myboard[x+1][y+1] == EMPTY &&
                    (myboard[x][y+1]==flag||myboard[x][y+2]==flag||myboard[x+1][y+2]==flag||
                     myboard[x+2][y]==flag||myboard[x+1][y]==flag||myboard[x+2][y+1]==flag))
                {
                    value = value + 139;
                }
                if(myboard[x][y+2] == cur_flag && myboard[x][y+1] == EMPTY &&
                    (myboard[x-1][y]==flag||myboard[x-1][y+1]==flag||myboard[x-1][y+2]==flag||
                     myboard[x+1][y]==flag||myboard[x+1][y+1]==flag||myboard[x+1][y+2]==flag))
                {
                    value = value + 139;
                }
                /*                   所有可以夹的局面               */
                /*          如                O                   */
                /*                          OX                    */
                if(myboard[x-1][y] == flag && myboard[x+1][y] == EMPTY &&
                    (myboard[x+1][y-1] == flag || myboard[x+1][y+1] == flag || myboard[x+2][y+1] == flag||
                     myboard[x+2][y] == flag || myboard[x+2][y-1] == flag || myboard[x][y-1] == flag||
                     myboard[x][y+1] == flag))
                {
                    value = value + 93;
                }
                if(myboard[x-1][y+1] == flag && myboard[x+1][y-1] == EMPTY &&
                    (myboard[x+1][y] == flag || myboard[x+2][y] == flag || myboard[x][y-1] == flag ||
                     myboard[x+2][y-1] == flag|| myboard[x+2][y-2] == flag || myboard[x+1][y-2] == flag||
                     myboard[x][y-2] == flag))
                {
                    value = value + 93;
                }
                if(myboard[x][y+1] == flag && myboard[x][y-1] == EMPTY &&
                    (myboard[x-1][y-1] == flag || myboard[x+1][y-1] == flag || myboard[x+1][y-2] == flag ||
                     myboard[x][y-2] == flag || myboard[x+1][y-2] == flag || myboard[x+1][y] == flag ||
                     myboard[x-1][y]==flag))
                {
                    value = value + 93;
                }
                if(myboard[x+1][y-1] == flag && myboard [x-1][y+1] == EMPTY &&
                    (myboard[x+1][y] == flag || myboard[x-2][y] == flag || myboard[x][y+1] == flag ||
                     myboard[x-2][y+1] == flag || myboard[x-2][y+2] == flag || myboard[x-1][y+2] == flag ||
                     myboard[x][y+2] == flag))
                {
                    value = value + 93;
                }
                if(myboard[x][y-1] == flag && myboard[x][y+1] == EMPTY &&
                    (myboard[x+1][y+1] == flag || myboard[x-1][y+1] == flag || myboard[x-1][y+2] == flag ||
                     myboard[x][y+2] == flag || myboard[x-1][y+2] == flag || myboard[x-1][y] == flag ||
                     myboard[x+1][y] == flag))
                {
                    value = value + 93;
                }
                if(myboard[x+1][y+1] == flag && myboard[x-1][y-1] == EMPTY &&
                    (myboard[x-1][y] == flag || myboard[x-1][y-2] == flag || myboard[x][y-1] == flag ||
                     myboard[x][y-2] == flag || myboard[x-2][y-1] == flag || myboard[x-2][y] == flag ||
                     myboard[x-2][y-2] == flag))
                {
                    value = value + 93;
                }
                if(myboard[x+1][y] == flag && myboard[x-1][y] == EMPTY &&
                    (myboard[x-1][y+1] == flag || myboard[x-1][y-1] == flag || myboard[x-2][y-1] == flag ||
                     myboard[x-2][y] == flag || myboard[x-2][y+1] == flag || myboard[x][y+1] == flag ||
                     myboard[x][y-1] == flag))
                {
                    value = value + 93;
                }
                if(myboard[x-1][y-1] == flag && myboard[x+1][y+1] == EMPTY &&
                    (myboard[x+1][y] == flag || myboard[x+1][y+2] == flag || myboard[x][y+1] == flag ||
                     myboard[x][y+2] == flag || myboard[x+2][y+1] == flag || myboard[x+2][y] == flag ||
                     myboard[x+2][y+2] == flag))
                {
                    value = value + 93;
                }
            }
        }
    }
/*                   方差反映离散程度              */
/*               让棋子汇聚起来更好得防止被吃        */
    s = variance(myboard,flag);
    if(countt > 8)
    {
        if(s < 4.25)
        {
            value = value + 10;
        }
        if(s < 4.20)
        {
            value = value + 10;
        }
        if(s < 4.13)
        {
            value = value + 10;
        }
        if(s < 4.10)
        {
            value = value + 10;
        }
        if(s < 4.08)
        {
            value = value + 10;
        }
        if(s < 3.96)
        {
            value = value + 10;
        }
        if(s < 3.92)
        {
            value = value + 10;
        }
        if(s< 3.83)
        {
            value = value + 10;
        }
        if(s< 3.73)
        {
            value = value + 10;
        }
        if(s< 3.69)
        {
            value = value + 10;
        }
        if(s< 3.65)
        {
            value = value + 10;
        }
        if(s< 3.60)
        {
            value = value + 10;
        }
        if(s< 3.54)
        {
            value = value + 10;
        }
        if(s< 3.49)
        {
            value = value + 10;
        }
        if(s<= 3.42)
        {
            value = value + 10;
        }
        if(s< 3.33)
        {
            value = value + 10;
        }
        if(s< 3.26)
        {
            value = value + 10;
        }
        if(s< 3.15)
        {
            value = value + 20;
        }
        if(s< 3.05)
        {
            value = value + 20;
        }
        if(s< 2.85)
        {
            value = value + 10;
        }
        if(s< 2.75)
        {
            value = value + 20;
        }
        if(s< 2.65)
        {
            value = value + 20;
        }
        if(s< 2.55)
        {
            value = value + 3;
        }
        if(s< 2.25)
        {
            value = value + 1;
        }
        if(s< 2.15)
        {
            value = value + 1;
        }
        if(s< 2.05)
        {
            value = value + 1;
        }
        if(s< 1.95)
        {
            value = value + 1;
        }
        if(s< 1.55)
        {
            value = value + 1;
        }
        if(s< 1.45)
        {
            value = value + 1;
        }
        if(s< 1.35)
        {
            value = value + 1;
        }
        if(s< 1.25)
        {
            value = value + 1;
        }
        if(s< 1.15)
        {
            value = value + 1;
        }
        if(s< 1.05)
        {
            value = value + 1;
        }
    }
    if(countt <= 8)
    {
        if(s < 4.25)
        {
            value = value + 1;
        }
        if(s < 4.20)
        {
            value = value + 1;
        }
        if(s < 4.13)
        {
            value = value + 1;
        }
        if(s < 4.10)
        {
            value = value + 1;
        }
        if(s < 4.08)
        {
            value = value + 1;
        }
        if(s < 3.96)
        {
            value = value + 1;
        }
        if(s < 3.92)
        {
            value = value + 1;
        }
        if(s< 3.83)
        {
            value = value + 1;
        }
        if(s< 3.73)
        {
            value = value + 1;
        }
        if(s< 3.69)
        {
            value = value + 1;
        }
        if(s< 3.65)
        {
            value = value + 1;
        }
        if(s< 3.60)
        {
            value = value + 1;
        }
        if(s< 3.54)
        {
            value = value + 1;
        }
        if(s< 3.49)
        {
            value = value + 1;
        }
        if(s<= 3.42)
        {
            value = value + 1;
        }
        if(s< 3.33)
        {
            value = value + 1;
        }
        if(s< 3.26)
        {
            value = value + 1;
        }
        if(s< 3.15)
        {
            value = value + 2;
        }
        if(s< 3.05)
        {
            value = value + 2;
        }
        if(s< 2.85)
        {
            value = value + 1;
        }
        if(s< 2.75)
        {
            value = value + 2;
        }
        if(s< 2.65)
        {
            value = value + 2;
        }
        if(s< 2.55)
        {
            value = value + 3;
        }
        if(s< 2.25)
        {
            value = value + 1;
        }
        if(s< 2.15)
        {
            value = value + 1;
        }
        if(s< 2.05)
        {
            value = value + 1;
        }
        if(s< 1.95)
        {
            value = value + 1;
        }
        if(s< 1.55)
        {
            value = value + 1;
        }
        if(s< 1.45)
        {
            value = value + 1;
        }
        if(s< 1.35)
        {
            value = value + 1;
        }
        if(s< 1.25)
        {
            value = value + 1;
        }
        if(s< 1.15)
        {
            value = value + 1;
        }
        if(s< 1.05)
        {
            value = value + 1;
        }
    }
    // printf("%.2lf\n",s);
    return value;
}
int alphabeta(int depth,int alpha,int beta,char myboard[BOARD_SIZE][BOARD_SIZE],int flag)
{
    //printf("depth %d with flag %d\n",depth,flag);
    char mymyboard[BOARD_SIZE][BOARD_SIZE] = {0};
    int cur_flag = 3 - flag;
    if(depth == 0)//层数为0 进行估值
    {
        return evaluate(myboard,flag);
    }
    memcpy(mymyboard,myboard,BOARD_SIZE * BOARD_SIZE * sizeof(char));//拷贝当前棋局以备用，每层要复原
    for(int i = 0;i < 8; i++)
    {
        for(int x = 0;x < BOARD_SIZE; x++)
        {
            for(int y = 0;y < BOARD_SIZE; y++)
            {
                if(myboard[x][y]!= flag)
                {
                    continue;
                }
                int new_x = x + DIR[i][0];
                int new_y = y + DIR[i][1];
                int value;
                if(isInBound(new_x,new_y) && myboard[new_x][new_y] == EMPTY)
                {
                    myboard[new_x][new_y] = flag;//每种可能的着法
                    myboard[x][y] = EMPTY;//找到我的子并移动
                    Reverse(new_x,new_y,myboard,flag);//进行挑夹转换判断
                    value = - alphabeta(depth - 1,- beta,- alpha,myboard,cur_flag);//ab剪枝递归 <注意交换ab位置且为负值 基于负极大值搜索>
                    for(int xx = 0;xx < BOARD_SIZE; xx++)
                    {
                        for(int yy = 0;yy < BOARD_SIZE; yy++)
                        {
                            myboard[xx][yy] = mymyboard[xx][yy];//复原刚刚移动的棋盘 即撤销着法
                        }
                    }
                    if(value > alpha)
                    {
                        alpha = value;//如果出现比a大的值 更新并记录着法
                        //printf("%d\n",alpha);
                        if(depth == 4)
                        {
                            best_x = x;
                            best_y = y;
                            best_opt = i;
                            //  printf("%d %d %d\n",x,y,i);
                        }
                    }
                    if(value >= beta)
                    {
                        return beta;
                    }
                }
                
            }
        }
    }
    return alpha;
}





struct Command aiTurn(char board[BOARD_SIZE][BOARD_SIZE], int me) {
    
    struct Command preferedPos;
    memcpy(myboard,board,BOARD_SIZE * BOARD_SIZE * sizeof(char));
    //memcpy(mymyboard,board,BOARD_SIZE * BOARD_SIZE * sizeof(char));
    //begin = clock();
    alphabeta(4,-1000000,1000000,myboard,me);
    //finish = clock();
    //duration = (double)(finish - begin);  //显示运行时间
    //printf( "%f ms\n", duration);
    preferedPos.x = best_x;
    preferedPos.y = best_y;
    preferedPos.option = best_opt;
    return preferedPos;
}

BOOL place(int x, int y, OPTION option, int cur_flag) {
    if (board[x][y] != cur_flag) {
        return FALSE;
    }
    
    int new_x = x + DIR[option][0];
    int new_y = y + DIR[option][1];
    if (!isInBound(new_x, new_y) || board[new_x][new_y] != EMPTY) {
        return FALSE;
    }
    
    board[x][y] = EMPTY;
    board[new_x][new_y] = cur_flag;
    
    int cur_other_flag = 3 - cur_flag;
    
    //
    int intervention_dir[4][2] = { {1, 0}, {0, 1}, {1, 1}, {1, -1} };
    for (int i = 0; i < 4; i++) {
        int x1 = new_x + intervention_dir[i][0];
        int y1 = new_y + intervention_dir[i][1];
        int x2 = new_x - intervention_dir[i][0];
        int y2 = new_y - intervention_dir[i][1];
        if (isInBound(x1, y1) && isInBound(x2, y2) && board[x1][y1] == cur_other_flag && board[x2][y2] == cur_other_flag) {
            board[x1][y1] = cur_flag;
            board[x2][y2] = cur_flag;
        }
    }
    
    int custodian_dir[8][2] = { {1, 0}, {-1, 0}, {0, 1}, {0, -1}, {1, 1}, {1, -1}, {-1, 1}, {-1, -1} };
    for (int i = 0; i < 8; i++) {
        int x1 = new_x + custodian_dir[i][0];
        int y1 = new_y + custodian_dir[i][1];
        int x2 = new_x + custodian_dir[i][0] * 2;
        int y2 = new_y + custodian_dir[i][1] * 2;
        if (isInBound(x1, y1) && isInBound(x2, y2) && board[x2][y2] == cur_flag && board[x1][y1] == cur_other_flag) {
            board[x1][y1] = cur_flag;
        }
    }
    return TRUE;
}


void start(int flag) {
    memset(board, 0, sizeof(board));
    
    for (int i = 0; i < 3; i++) {
        board[2][2 + i] = WHITE;
        board[6][6 + i] = WHITE;
        board[5][3 + i] = BLACK;
        board[9][7 + i] = BLACK;
    }
    
    for (int i = 0; i < 2; i++) {
        board[8 + i][2] = WHITE;
        board[2 + i][9] = BLACK;
    }
    
    initAI(flag);
}

void turn() {
    // AI
    struct Command command = aiTurn(board, me_flag);
    place(command.x, command.y, command.option, me_flag);
    printf("%d %d %d\n", command.x, command.y, command.option);
    fflush(stdout);
}

void end(int x) {
    
}

void loop() {
    //  freopen("../input", "r", stdin);
    while (TRUE)
    {
        memset(buffer, 0, sizeof(buffer));
        gets(buffer);
        
        if (strstr(buffer, START))
        {
            char tmp[MAX_BYTE] = {0};
            sscanf(buffer, "%s %d", tmp, &me_flag);
            other_flag = 3 - me_flag;
            start(me_flag);
            printf("OK\n");
            fflush(stdout);
        }
        else if (strstr(buffer, PLACE))
        {
            char tmp[MAX_BYTE] = {0};
            int x, y;
            OPTION option;
            sscanf(buffer, "%s %d %d %d", tmp, &x, &y, &option);
            place(x, y, option, other_flag);
            // printBoard();
        }
        else if (strstr(buffer, TURN))
        {
            turn();
            // printBoard();
        }
        else if (strstr(buffer, END))
        {
            char tmp[MAX_BYTE] = {0};
            int x;
            sscanf(buffer, "%s %d", tmp, &x);
            end(x);
        }
        
    }
}
void Sonnet18_William_Shakespeare()
{
    int LINE14;/*    So long lives this, and this gives life to thee.    */
    int LINE13;/*    So long as men can breathe, or eyes can see,    */
    int LINE12;/*    When in eternal lines to time thou growest:    */
    int LINE11;/*    Nor shall Death brag thou wanderest in his shade.   */
    int LINE10;/*    Nor lose possession of that fair thou owest;    */
    int LINE9; /*    But thy eternal Summer shall not fade£¬    */
    int LINE8; /*    By chance or nature's changing course untrimm'd:    */
    int LINE7; /*    And every fair from fair sometime declines,   */
    int LINE6; /*    And oft' is his gold complexion dimm'd;    */
    int LINE5; /*    Sometime too hot the eye of heaven shines,    */
    int LINE4; /*    And Summer's lease hath all too short a date:    */
    int LINE3; /*    Rough winds do shake the darling buds of May,    */
    int LINE2; /*    Thou art more lovely and more temperate:    */
    int LINE1; /*    Shall I compare thee to a Summer's day?    */
    int Sonnet_begin;/*           十四行诗从这里开始……             */
    int Sonnet18_william_shakespeare;/*十四行诗第十八号*/
}
int main(int argc, char *argv[]) {
    loop();
    return 0;
}
