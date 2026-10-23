#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ===================== 1. 平台适配层（先照抄，不用看懂） ===================== */
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
    static void plat_init(void) {
        HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode;
        GetConsoleMode(h, &mode);
        SetConsoleMode(h, mode | 0x0004);   /* 开启 ANSI 转义支持 */
    }
    static int  key_pressed(void) { return _kbhit(); }
    static int  read_key(void)    { return _getch(); }
    static void sleep_ms(int ms)  { Sleep(ms); }
#else
    #include <unistd.h>
    #include <termios.h>
    static struct termios g_orig;
    static int  g_pushback = -1;

    static void plat_restore(void) { tcsetattr(STDIN_FILENO, TCSANOW, &g_orig); }

    static void plat_init(void) {
        struct termios raw;
        tcgetattr(STDIN_FILENO, &g_orig);
        atexit(plat_restore);
        raw = g_orig;
        raw.c_lflag &= ~(ICANON | ECHO);   /* 关掉行缓冲和回显 */
        raw.c_cc[VMIN]  = 0;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &raw);
    }
    static int key_pressed(void) {
        unsigned char c;
        if (g_pushback != -1) return 1;
        if (read(STDIN_FILENO, &c, 1) == 1) { g_pushback = c; return 1; }
        return 0;
    }
    static int read_key(void) {
        unsigned char c;
        if (g_pushback != -1) { int t = g_pushback; g_pushback = -1; return t; }
        if (read(STDIN_FILENO, &c, 1) == 1) return c;
        return -1;
    }
    static void sleep_ms(int ms) { usleep(ms * 1000); }
#endif

/* ===================== 2. 常量与全局状态 ===================== */
#define WIDTH 30
#define HEIGHT 20
#define MAX_LEN (WIDTH*HEIGHT)

typedef struct {int x,y;} Point;

Point snake[MAX_LEN];
int snake_len;
int dir_x,dir_y;
Point food;
int score;
int speed_ms;
int game_over;

/* ===================== 3. 用指针操作蛇身 ===================== */
static int snake_contains(int x,int y)//遍历 判断该坐标是否被蛇占了
{
    Point *p;
    for (p=snake;p<snake+snake_len;p++)
    {
        if (p->x==x && p->y==y)
        {
            return 1;//如果被占 返回1
        }
    }
    return 0;
}

static void spawn_food(void)//生成食物
{
    if (snake_len>=MAX_LEN)return;//已通关
    do
    {
        food.x=rand()%WIDTH;
        food.y=rand()%HEIGHT;
    } while (snake_contains(food.x,food.y));
    
}

static void init_game(void)//初始蛇头 长度 位置
{
    snake_len=3;
    snake[0].x = WIDTH / 2;       snake[0].y = HEIGHT / 2;
    snake[1].x = WIDTH / 2 - 1;   snake[1].y = HEIGHT / 2;
    snake[2].x = WIDTH / 2 - 2;   snake[2].y = HEIGHT / 2;
    dir_x=1;
    dir_y=0;
    speed_ms=150;
    game_over=0;
    spawn_food();
}

/* ===================== 4. 输入 ===================== */
static void handle_input(void)
{
    int c;
    if(!key_pressed()) return;//检测当前是1否0有按键被按下
    c=read_key();//读取键盘输入
    switch (c)//方向：上-1下1 左-1右1
    {
    case 'w':case 'W':if(dir_y!=1){dir_x=0;dir_y=-1;}break;//仅仅判断无法180度拐弯
    case 's':case 'S':if(dir_y!=-1){dir_x=0;dir_y=1;}break;
    case 'a':case 'A':if(dir_x!=1){dir_x=-1;dir_y=0;}break;
    case 'd':case 'D':if(dir_x!=-1){dir_x=1;dir_y=0;}break;
    case 'q':case 'Q':game_over=1;break;
    }
}

/* ===================== 5. 移动（指针核心） ===================== */
//简单示例：
// void move_point(Point *p,int dx,int dy)//点移动 需要传&head
// {
//     p->x += dx;//等价于(*p).x += dx
//     p->y += dy;
// }

static void move_snake(void)//移动蛇 注意算法是从尾往头遍历
{
    Point *p;
    Point old_tail=snake[snake_len-1];
    for (p=snake+snake_len-1;p>snake;p--)
    {
        *p=*(p-1);
    }
    snake[0].x += dir_x;//蛇头单独往运动方向移动一格
    snake[0].y += dir_y;

    if (snake[0].x==food.x && snake[0].y==food.y)//如果碰到食物
    {
        if (snake_len<MAX_LEN)
        {
            snake[snake_len]=old_tail;
            snake_len++;
        }
        score+=10;
        if(speed_ms>60) speed_ms-=3;
        spawn_food();
    }
}

/* ===================== 6. 碰撞检测 ===================== */
static int check_collision(void)
{
    Point *p;
    if (snake[0].x<0 || snake[0].x>=WIDTH || snake[0].y<0 || snake[0].y>=HEIGHT)
    return 1;
    for (p=snake+1;p<snake+snake_len;p++)
    {
        if(p->x == snake[0].x && p->y == snake[0].y) return 1;
    }
    return 0;
}

/* ===================== 7. 绘制 ===================== */
static void draw(void)
{
    int x,y;
    Point *p;
    printf("\033[H");//光标返回左上角
    for (x=0;x<WIDTH+2;x++)putchar('#');//开始逐帧打印整个画面
    putchar('\n');
    for (y=0;y<HEIGHT;y++)
    {
        putchar('#');
        for (x=0;x<WIDTH;x++)//如果坐标打印到食物或蛇的位置 则相应输出 否则打印空格
        {
            char c=' ';
            if (food.x==x && food.y==y)
            {
                c='$';//'$'表示食物
            }
            else
            {
                for (p=snake;p<snake+snake_len;p++)
                {
                    if (p->x==x && p->y==y)
                    {
                        c= (p==snake)? '@':'O';//判断蛇头@
                        break;
                    }
                }
            }
            putchar(c);
        }
        putchar('#');
        putchar('\n');
    }
    for (x=0;x<WIDTH+2;x++) putchar('#');
    printf("\nScore:%d  Length:%d WASD移动 Q退出 \n",score,snake_len);
}

/* ===================== 8. 主循环 ===================== */
int main(void)
{
    plat_init();
    srand((unsigned)time(NULL));
    printf("\033[2J");
    init_game();
    while (!game_over)
    {
        draw();
        handle_input();
        move_snake();
        if (check_collision()) game_over=1;
        sleep_ms(speed_ms);
    }
    printf("\033[2J\033[H");
    printf("Game Over! Score = %d, Length = %d\n",score,snake_len);
    return 0;
}