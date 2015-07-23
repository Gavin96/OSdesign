#ifndef __HEADFILE_H__
#define __HEADFILE_H__

#include <stdio.h>
#include <stdlib.h>
//#include <time.h>
#define SIZE 4

int arr[SIZE][SIZE];
int score;
int best;
int if_game_over;
extern int same[SIZE][SIZE];

// linux_getch.c
//char my_getch();

// show.c
void refresh_show();
void start_game();



// main.c
void game_operate();

// opertate.c
void assign(void);
int same_judge(void);
int game_judge(void);
void add_number(void);
void move_up(void);
void move_left(void);
void move_down(void);
void move_right(void);
void restart(void);



#endif		//__HEADFILE_H__

