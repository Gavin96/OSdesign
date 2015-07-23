
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"
#include "headfile.h"

int strcmp(char *str1,char *str2)
{
	int i;
	for (i=0; i<strlen(str1); i++)
	{
		if (i==strlen(str2)) return 1;
		if (str1[i]>str2[i]) return 1;
		else if (str1[i]<str2[i]) return -1;
	}
	return 0;
}

void strlwr(char *str)
{
	int i;
	for (i=0; i<strlen(str); i++)
	{
		if ('A'<=str[i] && str[i]<='Z') str[i]=str[i]+'a'-'A';
	}
}

void addToQueue(PROCESS* p)
{
	p->state=kRUNNABLE;
//	if (p->priority>=10)
//	{
		firstQueue[firstLen]=p;
		firstLen++;
		p->ticks=2;
		p->whichQueue=1;
//	}
/*	else if(p->priority>=5)
	{
		secondQueue[secondLen]=p;
		secondLen++;
		p->ticks=4;
		p->whichQueue=2;
	}
	else
	{
		lastQueue[lastLen]=p;
		lastLen++;
		p->ticks=p->priority;
		p->whichQueue=3;
	}
*/}

/*======================================================================*
                            tinix_main
 *======================================================================*/
PUBLIC int tinix_main()
{
	//disp_str("-----\"tinix_main\" begins-----\n");
	clearScreen();
	disp_str("*************************************************************\n");
	disp_str("**********               TINIX v1.0.1              **********\n");
	disp_str("*********  1352911 Jasmine 1352913 Picses 1352873 Gavin  ********\n");
	disp_str("**********   Welcome to Our Operating System ^_^   **********\n");
	disp_str("                                                             \n");
	disp_str("******* You Can Input HELP First to Know Our System  *********\n");
	disp_str("*************************************************************\n");
	TASK*		p_task;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	t_16		selector_ldt	= SELECTOR_LDT_FIRST;
	int		i;
	t_8		privilege;
	t_8		rpl;
	int		eflags;
	for(i=0;i<NR_TASKS+NR_PROCS;i++){
		if (i < NR_TASKS) {	/* ÈÎÎñ */
			p_task		= task_table + i;
			privilege	= PRIVILEGE_TASK;
			rpl		= RPL_TASK;
			eflags		= 0x1202;	/* IF=1, IOPL=1, bit 2 is always 1 */
		}
		else {			/* ÓÃ»§œø³Ì */
			p_task		= user_proc_table + (i - NR_TASKS);
			privilege	= PRIVILEGE_USER;
			rpl		= RPL_USER;
			eflags		= 0x202;	/* IF=1, bit 2 is always 1 */
		}

		strcpy(p_proc->name, p_task->name);	/* name of the process */
		p_proc->pid	= i;			/* pid */

		p_proc->ldt_sel	= selector_ldt;
		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | privilege << 5;	/* change the DPL */
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3], sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | privilege << 5;/* change the DPL */
		p_proc->regs.cs		= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ds		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.es		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.fs		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.ss		= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK) | SA_TIL | rpl;
		p_proc->regs.gs		= (SELECTOR_KERNEL_GS & SA_RPL_MASK) | rpl;
		p_proc->regs.eip	= (t_32)p_task->initial_eip;
		p_proc->regs.esp	= (t_32)p_task_stack;
		p_proc->regs.eflags	= eflags;

		p_proc->nr_tty		= 0;

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	//init priority
	proc_table[0].priority = 15;
	proc_table[1].priority =  15;
	proc_table[2].priority =  12;
	proc_table[3].priority =  4;
	proc_table[4].priority =  7;
	proc_table[5].priority =  12;
	proc_table[6].priority =  10;

	//init queue
	firstLen=firstHead=secondLen=lastLen=0;
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		addToQueue(proc_table+i);
	}
	//assign console
	proc_table[1].nr_tty = 0;
	proc_table[2].nr_tty = 1;
	proc_table[3].nr_tty = 1;
	proc_table[4].nr_tty = 1;
	proc_table[5].nr_tty = 2;
	proc_table[6].nr_tty = 3;

	k_reenter	= 0;
	ticks		= 0;

	p_proc_ready	= proc_table;

	init_clock();

	restart();

	while(1){}
}

void clearScreen()
{
	int i;
	disp_pos=0;
	for(i=0;i<80*25;i++)
	{
		disp_str(" ");
	}
	disp_pos=0;
}


void help()
{
	printf("           ***********************************************\n");
	printf("                          Welcome to Our OS System ^_^ \n");
	printf("           ***********************************************\n");
	printf("\n");
	printf("      *****  help         --------  show the help menu    *****\n");
	printf("      *****  clear        --------  clear screen          *****\n");
	printf("      *****  alt+F2       --------  show the process run  *****\n");
	printf("      *****  alt+F3       --------  goBang game           *****\n");
	printf("      *****  kill 2~5     --------  kill the process 2~5  *****\n");
	printf("      *****  start 2~5    --------  start the process 2~5 *****\n");
	printf("      *****  ready 2~5    --------  make the process 2~5 ready \n");
	printf("      *****  show         --------  show the process state*****\n");
	printf("      *********************************************************\n");
	printf("\n");
}

void show()
{
	printf("Process      State    Priority\n");
	PROCESS* p;
	int i;
	for (i=0; i<NR_TASKS+NR_PROCS;i++)
	{
		p=&proc_table[i];
		printf("process%d",p->pid);
		switch (p->state)
		{
		case kRUNNABLE:
			printf("    Runnable      ");
			break;
		case kRUNNING:
			printf("    Running       ");
			break;
		case kREADY:
			printf("    Finish        ");
			break;
		}
		printf("%d\n",p->priority);
	}
}

void readOneStringAndOneNumber(char* command,char* str,int* number)
{
	int i;
	int j=0;
	for (i=0; i<strlen(command); i++)
	{
		if (command[i]!=' ') break;
	}
	for (; i<strlen(command); i++)
	{
		if (command[i]==' ') break;
		str[j]=command[i];
		j++;
	}
	for (; i<strlen(command); i++)
	{
		if (command[i]!=' ') break;
	}

	*number=0;
	for (; i<strlen(command) && '0'<=command[i] && command[i]<='9'; i++)
	{
		*number=*number*10+(int) command[i]-'0';
	}
}

void dealWithCommand(char* command)
{
	strlwr(command);
	if (strcmp(command,"clear")==0)
	{
		clearScreen();
		sys_clear(tty_table);
		return ;
	}
	if (strcmp(command,"help")==0)
	{
		help();
		return ;
	}
	if (strcmp(command,"show")==0)
	{
		show();
		return ;
	}


	char str[100];
	int i=0;
	for(; i<100;i++)
		str[i]=0;
	printf("%s",command);
	int number;
	readOneStringAndOneNumber(command,str,& number);
	if (strcmp(str,"start")==0)
	{
		char* state;
		state= (char *)proc_table[number].state;
		if(strcmp(state,"kRUNNABLE")!=0)
		{
			printf("The process is blocked");
		}
		else if (number<0 || number>NR_TASKS+NR_PROCS)
		{
			printf("No found this process!!");
		}
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
		{
			proc_table[number].state=kRUNNING;
			printf("start process %d successful\n",number);
		}
		return ;
	}
	if (strcmp(str,"kill")==0)
	{
		char* state;
		state= (char *)proc_table[number].state;
		if(strcmp(state,"kRUNNING")==0)
		{
			printf("The process hasn't run\n");
		}
		else if (number<0 || number>NR_TASKS+NR_PROCS)
		{
			printf("No found this process!!\n");
		}
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
		{
			proc_table[number].state=kREADY;
			printf("kill process %d successful\n",number);
		}
		return ;
	}
	if (strcmp(str,"ready")==0)
	{
		char* state;
		state= (char *)proc_table[number].state;
		if(strcmp(state,"kREADY")==0)
		{
			printf("The process hasn't finish\n");
		}
		else if (number<0 || number>NR_TASKS+NR_PROCS)
		{
			printf("No found this process!!\n");
		}
		else if (number==0 || number==6)
		{
			printf("You do not have sufficient privileges\n");
		}
		else if (2<=number && number <=5)
		{
			proc_table[number].state=kRUNNABLE;
			printf("make process %d ready\n",number);
		}
		return ;
	}
	printf("%s", str);
	printf("can not find this command\n");
}

/*======================================================================*
                               Terminal
 *======================================================================*/
void Terminal()
{
	TTY *p_tty=tty_table;
	p_tty->startScanf=0;
	while(1)
	{
		printf("DB=>");
		openStartScanf(p_tty);
		while (p_tty->startScanf) ;
		dealWithCommand(p_tty->str);
	}
}


/*======================================================================*
                               TestB
 *======================================================================*/
void TestB()
{
	char* state;
	state= (char *)proc_table[2].state;
	if(strcmp(state,"kREADY")!=0)
	{
		int i = 0;
		while(1){
			printf("2   ");
			milli_delay(1000);
		}

	}
}



/*======================================================================*
                               TestC
 *======================================================================*/
void TestC()
{
	char* state;
	state= (char *)proc_table[3].state;
	if(strcmp(state,"kREADY")!=0)
	{
		int i = 0;
		while(1){
			printf("3   ");
			milli_delay(1000);
		}

	}
}

void TestD()
{
	char* state;
	state= (char *)proc_table[4].state;
	if(strcmp(state,"kREADY")!=0)
	{
		int i = 0;
		while(1){
			printf("4   ");
			milli_delay(1000);
		}

	}
}




/*======================================================================*
                               2048
 *======================================================================*/

int same[SIZE][SIZE]= {};

void line(void)
{
    int i = 0;
    for (i =0; i <SIZE; i++)
    {
        printf("-----");
    }
}


void set_number(int row, int vol)
{
    switch(arr[row][vol])
    {
        case 0:
            printf("|[0]");
            break;
        case 2:
            printf("|[2]");
            break;
        case 4:
            printf("|[4]");
            break;
        case 8:
            printf("|[8]");
            break;
        case 16:
            printf("|[16]");
            break;
        case 32:
            printf("|[32]");
            break;
        case 64:
            printf("|[64]");
            break;
        case 128:
            printf("|[128]");
            break;
        case 256:
            printf("|[256]");
            break;
        case 512:
            printf("|[512]");
            break;
        case 1024:
            printf("|[1024]");
            break;
        case 2048:
            printf("|[2048]");
            break;
        default:
            break;
    }
}


void refresh_show(void)
{
    int i =0, j =0;
    //clearScreen();
    printf("\n\n");
    printf("    GAME: 2048\n");
    //printf(" SCORE        \n");
    //printf("  %d        \n", score);
    line();
    printf("\n");
    for(i =0; i <4; i++)
    {
        for(j =0; j <4; j++)
        {
            set_number(i, j);
	    printf("|  ");
        }
        
        printf("\n");
        line();
        printf("\n");
    }
    printf("Q(exit)   R(restart_2048)\n");
}

void move_up(void)
{
    int i =0, j =0;		//	循环变量
    int point = 0;		//	定位作用
    
    for (i =0; i <SIZE; i++)
    {
        point = 0;
        for(j =1; j <SIZE; j++)
        {
            //如果没这个if，则要在两者相等处做个同为O的分支
            if (arr[j][i] > 0)
            {
                // 定位点要不为0,不为0则有可能与对比数相等或不相等`
                if (arr[point][i] == arr[j][i])
                {
                    arr[point][i] *= 2;
                    score += arr[point][i];
                    arr[j][i] = 0;
                    point++;
                }
                else if(arr[point][i] == 0)
                {
                    arr[point][i] = arr[j][i];
                    arr[j][i] = 0;
                }
                else
                {
                    arr[++point][i] = arr[j][i];
                    if (point != j)
                    {
                        arr[j][i] = 0;
                    }
                }
            }
            
        }
    }
}

void move_down(void)
{
    int i =0, j =0;		//	循环变量
    int point = 0;		//	定位作用
    
    for (i =0; i <SIZE; i++)
    {
        point = SIZE-1;
        for(j =SIZE-2; j >=0; j--)
        {
            //如果没这个if，则要在两者相等处做个同为O的分支
            if (arr[j][i] > 0)
            {
                // 定位点要不为0,不为0则有可能与对比数相等或不相等`
                if (arr[point][i] == arr[j][i])
                {
                    arr[point][i] *= 2;
                    score += arr[point][i];
                    arr[j][i] = 0;
                    point--;
                }
                else if(arr[point][i] == 0)
                {
                    arr[point][i] = arr[j][i];
                    arr[j][i] = 0;
                }
                else
                {
                    arr[--point][i] = arr[j][i];
                    if (point != j)
                    {
                        arr[j][i] = 0;
                    }
                }
            }
            
        }
    }
}

void move_left(void)
{
    int i =0, j =0;		//	循环变量
    int point = 0;		//	定位作用
    
    for (i =0; i <SIZE; i++)
    {
        point = 0;
        for(j =1; j <SIZE; j++)
        {
            //如果没这个if，则要在两者相等处做个同为O的分支
            if (arr[i][j] > 0)
            {
                // 定位点要不为0,不为0则有可能与对比数相等或不相等`
                if (arr[i][point] == arr[i][j])
                {
                    arr[i][point] *= 2;
                    score += arr[i][point];
                    arr[i][j] = 0;
                    point++;
                }
                else if(arr[i][point] == 0)
                {
                    arr[i][point] = arr[i][j];
                    arr[i][j] = 0;
                }
                else
                {
                    arr[i][++point] = arr[i][j];
                    if (point != j)
                    {
                        arr[i][j] = 0;
                    }
                }
            }
            
        }
    }
}

void move_right(void)
{
    int i =0, j =0;		//	循环变量
    int point = 0;		//	定位作用
    
    for (i =0; i <SIZE; i++)
    {
        point = SIZE-1;
        for(j =SIZE-2; j >=0; j--)
        {
            //如果没这个if，则要在两者相等处做个同为O的分支
            if (arr[i][j] > 0)
            {
                // 定位点要不为0,不为0则有可能与对比数相等或不相等`
                if (arr[i][point] == arr[i][j])
                {
                    arr[i][point] *= 2;
                    score += arr[i][point];
                    arr[i][j] = 0;
                    point--;
                }
                else if(arr[i][point] == 0)
                {
                    arr[i][point] = arr[i][j];
                    arr[i][j] = 0;
                }
                else
                {
                    arr[i][--point] = arr[i][j];
                    if (point != j)
                    {
                        arr[i][j] = 0;
                    }
                }
            }
            
        }
    }
}

int game_judge(void)
{
    int i = 0, j = 0;
    // 判断是否胜利
    for (i =0; i <SIZE; i++)
        for(j =0; j <SIZE; j++)
        {
            if (arr[i][j] == 2048)
            {
                return 2;
            }
        }
    //判断是否有空格，有则继续
    for (i =0; i <SIZE; i++)
        for(j =0; j <SIZE; j++)
        {
            if(arr[i][j] == 0)
            {
                return 1;
            }
        }
    //判断是否相邻有重复，有则游戏继续
    for (i =0; i <SIZE; i++)
        for(j =0; j <SIZE-1; j++)
        {
            if ((arr[i][j] == arr[i][j+1]) || (arr[j][i] == arr[j+1][i]))
            {
                return 1;
            }
        }
    
    //没地可走，则game over
    return 0;
}

void add_number(void)
{
    int num = 10 % 3 ? 2 : 4;	// 生成2的概率是4的2倍
    int un[SIZE * SIZE] = {};		// 记录没有被占据的位置
    int i = 0, j =0, cnt = 0;
    int loc = 0;
    for (i =0; i <SIZE; i++)
        for (j =0; j <SIZE; j++)
        {
            //if (arr[i][j] == 0 && loc-- == 0 )
            if (arr[i][j] == 0)
            {
                un[cnt] = i * 10 + j;
                cnt++;
            }
        }
    loc = 20 % ++cnt;				//随机挑选未被占据的位置
    
    // 复原数组存放数字对应的序列
    i = un[loc] / 10;
    j = un[loc] % 10;
    arr[i][j] = num;

}

void restart_2048(void)
{
    int i = 0, j = 0;
    int row = 0, vol = 0;
    for (i =0; i <SIZE; i++)
        for(j =0; j <SIZE; j++)
        {
            arr[i][j] = 0;
        }
    row = 35 % SIZE;
    vol = 23 % SIZE;
    arr[row][vol] = 2;
    add_number();
}

void assign(void)
{
    int i =0, j =0;
    
    for (i =0; i <SIZE; i++)
        for (j =0; j <SIZE; j++)
        {
            same[i][j] = arr[i][j];
        }
}

int same_judge()
{
    int i = 0, j =0;
    
    for (i =0; i <SIZE; i++)
        for (j =0; j <SIZE; j++)
        {
            if (same[i][j] != arr[i][j])
            {
                return 0;
            }
        }
    return 1;
}





void start_game()
{
    int judge = 0;
   // char quit =0;
   // char move =0;
	TTY *moveTty=tty_table;
	moveTty->startScanf =0;
	
	
	TTY *quitTty=tty_table;
	quitTty->startScanf =0;
	
	
    while(1)
    {
        add_number();
        refresh_show();
        assign();          //记录数组，对比可知操作后数字是否有移动
        openStartScanf(moveTty);
	

	while (moveTty->startScanf); 
		strlwr(moveTty->str);
		if (strcmp(moveTty->str,"w")==0 || strcmp(moveTty->str,"W")==0){
       			move_up();
                	break;}
            	else if (strcmp(moveTty->str,"a")==0 || strcmp(moveTty->str,"A")==0){
       			move_up();
                	break;}
		else if (strcmp(moveTty->str,"s")==0 || strcmp(moveTty->str,"S")==0){
       			move_up();
                	break;}
		else if (strcmp(moveTty->str,"d")==0 || strcmp(moveTty->str,"D")==0){
       			move_up();
                	break;}
		else if (strcmp(moveTty->str,"q")==0 || strcmp(moveTty->str,"Q")==0){
       			printf("Are you sure? Y/N\n");
			openStartScanf(quitTty);
			while (quitTty->startScanf); 
				strlwr(quitTty->str);
				if (strcmp(quitTty->str,"y")==0 || strcmp(quitTty->str,"Y")==0){
               				return;	
               			}
               			else
                    			{continue;
				}
			
		}
           	else if (strcmp(moveTty->str,"r")==0 || strcmp(moveTty->str,"R")==0){
                	restart_2048();
                	score = 0;
                	continue;}

		else {
                	continue;}
        
        
        
        switch (game_judge())
        {
            case 2:
                printf("YOU WIN !!!\n");
                printf("Do you want to play again (Y/N)\n");
                restart_2048();
                score = 0;
                
                while (quitTty->startScanf) {
				strlwr(quitTty->str);
				if (strcmp(quitTty->str,"y")==0 || strcmp(quitTty->str,"Y")==0){
               				continue;	
               			}
               			else
                    			{return;
				}
			}	
                
            case 1:
                break;
            default:
                printf("YOU FAIL !!!\n");
                printf("Do you want to restart_2048 (Y/N)\n");
                restart_2048();
                score = 0;
                
                while (quitTty->startScanf) {
				strlwr(quitTty->str);
				if (strcmp(quitTty->str,"y")==0 || strcmp(quitTty->str,"Y")==0){
               				continue;	
               			}
               			else
                    			{return;
				}
			}
        }
        
        if (same_judge())
        {
            continue;
        }
        
        add_number();
        
    }
    
}

/*int main(void)
{
    srand((unsigned)time(0));
    
    start_game();
    return 0;
}*/

void TestE()
{

	int i = 0;
		while(1){
			printf("4   ");
			milli_delay(1000);
		}


}

/*======================================================================*
				goBangGame
*=======================================================================*/
char gameMap[15][15];
TTY *goBangGameTty=tty_table+2;

void readTwoNumber(int* x,int* y)
{
	int i=0;
	*x=0;
	*y=0;
	for (i=0; i<goBangGameTty->len && goBangGameTty->str[i]==' '; i++);
	for (; i<goBangGameTty->len && goBangGameTty->str[i]!=' '  && goBangGameTty->str[i]!='\n'; i++)
	{
		*x=(*x)*10+(int) goBangGameTty->str[i]-48;
	}
	for (i; i<goBangGameTty->len && goBangGameTty->str[i]==' '; i++);
	for (; i<goBangGameTty->len && goBangGameTty->str[i]!=' ' && goBangGameTty->str[i]!='\n'; i++)
	{
		*y=(*y)*10+(int) goBangGameTty->str[i]-48;
	}
}

int max(int x,int y)
{
	return x>y?x:y;
}

int selectPlayerOrder()
{
	printf("o player\n");
	printf("* computer\n");
	printf("who play first?[1/user  other/computer]");
	openStartScanf(goBangGameTty);
	while (goBangGameTty->startScanf) ;
	if (strcmp(goBangGameTty->str,"1")==0) return 1;
	else return 0;
}

void displayGameState()
{
	sys_clear(goBangGameTty);
	int n=15;
	int i,j;
	for (i=0; i<=n; i++)
	{
		if (i<10) printf("%d   ",i);
		else printf("%d  ",i);
	}
	printf("\n");
	for (i=0; i<n; i++)
	{
		if (i<9) printf("%d   ",i+1);
		else printf("%d  ",i+1);
		for (j=0; j<n; j++)
		{
			if (j<10) printf("%c   ",gameMap[i][j]);
			else printf("%c   ",gameMap[i][j]);
		}
		printf("\n");
	}

}

int checkParameter(int x, int y)	//Œì²éÍæŒÒÊäÈëµÄ²ÎÊýÊÇ·ñÕýÈ·
{
	int n=15;
	if (x<0 || y<0 || x>=n || y>=n) return 0;
	if (gameMap[x][y]!='_') return 0;
	return 1;
}

//žüÐÂµÄÎ»ÖÃÎªx£¬y£¬ÒòŽË Ö»ÒªŒì²é×ø±êÎªx£¬yµÄÎ»ÖÃ
int win(int x,int y)		//Ê€Àû·µ»Ø1    ·ñÔò0£šÄ¿Ç°ÎÞÈË»ñÊ€£©
{
	int n=15;
	int i,j;
	int gameCount;
	//×óÓÒÀ©Õ¹
	gameCount=1;
	for (j=y+1; j<n; j++)
	{
		if (gameMap[x][j]==gameMap[x][y]) gameCount++;
		else break;
	}
	for (j=y-1; j>=0; j--)
	{
		if (gameMap[x][j]==gameMap[x][y]) gameCount++;
		else break;
	}
	if (gameCount>=5) return 1;

	//ÉÏÏÂÀ©Õ¹
	gameCount=1;
	for (i=x-1; i>0; i--)
	{
		if (gameMap[i][y]==gameMap[x][y]) gameCount++;
		else break;
	}
	for (i=x+1; i<n; i++)
	{
		if (gameMap[i][y]==gameMap[x][y]) gameCount++;
		else break;
	}
	if (gameCount>=5) return 1;

	//Õý¶ÔœÇÏßÀ©Õ¹
	gameCount=1;
	for (i=x-1,j=y-1; i>=0 && j>=0; i--,j--)
	{
		if (gameMap[i][j]==gameMap[x][y]) gameCount++;
		else break;
	}
	for (i=x+1,j=y+1; i<n && j<n; i++,j++)
	{
		if (gameMap[i][j]==gameMap[x][y]) gameCount++;
		else break;
	}
	if (gameCount>=5) return 1;

	//žº¶ÔœÇÏßÀ©Õ¹
	gameCount=1;
	for (i=x-1,j=y+1; i>=0 && j<n; i--,j++)
	{
		if (gameMap[i][j]==gameMap[x][y]) gameCount++;
		else break;
	}
	for (i=x+1,j=y-1; i<n && j>=0; i++,j--)
	{
		if (gameMap[i][j]==gameMap[x][y]) gameCount++;
		else break;
	}
	if (gameCount>=5) return 1;

	return 0;
}

void free1(int x,int y1,int y2,int* ff1,int* ff2)
{
	int n=15;
	int i;
	int f1=0,f2=0;
	for (i=y1; i>=0; i++)
	{
		if (gameMap[x][i]=='_') f1++;
		else break;
	}
	for (i=y2; i<n; i++)
	{
		if (gameMap[x][i]=='_') f2++;
		else break;
	}
	*ff1=f1;
	*ff2=f2;
}

void free2(int x1,int x2,int y,int *ff1,int *ff2)
{
	int n=15;
	int i;
	int f1=0,f2=0;
	for (i=x1; i>=0; i--)
	{
		if (gameMap[i][y]=='_') f1++;
		else break;
	}
	for (i=x2; i<n; i++)
	{
		if (gameMap[i][y]=='_') f2++;
		else break;
	}
	*ff1=f1;
	*ff2=f2;
}

void free3(int x1,int y1,int x2,int y2,int *ff1,int *ff2)
{
	int n=15;
	int x,y;
	int f1=0;
	int f2=0;
	for (x=x1,y=y1; 0<=x && 0<=y; x--,y--)
	{
		if (gameMap[x][y]=='_') f1++;
		else break;
	}
	for (x=x2,y=y2; x<n &&  y<n; x++,y++)
	{
		if (gameMap[x][y]=='_') f2++;
		else break;
	}
	*ff1=f1;
	*ff2=f2;
}

void free4(int x1,int y1,int x2,int y2,int *ff1,int *ff2)
{
	int n=15;
	int x,y;
	int f1=0,f2=0;
	for (x=x1,y=y1; x>=0 && y<n; x--,y++)
	{
		if (gameMap[x][y]=='_') f1++;
		else break;
	}
	for (x=x2,y=y2; x<n && y>=0; x++,y--)
	{
		if (gameMap[x][y]=='_') f2++;
		else break;
	}
	*ff1=f1;
	*ff2=f2;
}

int getPossibleByAD(int attack,int defence,int attackFree1,int attackFree2,int defenceFree1,int defenceFree2)
{
	if (attack>=5) return 20;						//5¹¥»÷
	if (defence>=5) return 19;						//5·ÀÓù
	if (attack==4 && (attackFree1>=1 && attackFree2>=1)) return 18;		//4¹¥»÷ 2±ß
	if (attack==4 && (attackFree1>=1 || attackFree2>=1)) return 17;		//4¹¥»÷ 1±ß
	if (defence==4 && (defenceFree1>=1 || defenceFree2>=1)) return 16;	//4·ÀÓù
	if (attack==3 && (attackFree1>=2 && attackFree2>=2)) return 15;		//3¹¥»÷ 2±ß
	if (defence==3 && (defenceFree1>=2 && defenceFree2>=2)) return 14;	//3·ÀÓù 2±ß
	if (defence==3 && (defenceFree1>=2 || defenceFree2>=2)) return 13;	//3·ÀÓù 1±ß
	if (attack==3 && (attackFree1>=2 || attackFree2>=2)) return 12;		//3¹¥»÷ 1±ß
	if (attack==2 && (attackFree1>=3 && attackFree2>=3)) return 11;		//2¹¥»÷ 2±ß
	if (defence==2 && defenceFree1+defenceFree2>=3) return 10;	//2·ÀÓù 2±ß
	if (defence==2 && defenceFree1+defenceFree2>=3) return 9;		//2·ÀÓù 1±ß
	if (attack==1 && attackFree1+attackFree2>=4) return 8;
	if (defence==1 && defenceFree1+defenceFree2>=4) return 7;
	return 6;
}

int getPossible(int x,int y)
{
	int n=15;
	int attack;
	int defence;
	int attackFree1;
	int defenceFree1;
	int attackFree2;
	int defenceFree2;
	int possible=-100;

	//×óÓÒÀ©Õ¹
	int al,ar;
	int dl,dr;
	//ºáÏò¹¥»÷
	for (al=y-1; al>=0; al--)
	{
		if (gameMap[x][al]!='*') break;
	}
	for (ar=y+1; ar<n; ar++)
	{
		if (gameMap[x][ar]!='*') break;
	}
	//ºáÏò·ÀÊØ
	for (dl=y-1; dl>=0; dl--)
	{
		if (gameMap[x][dl]!='o') break;
	}
	for (dr=y+1; dr<n; dr++)
	{
		if (gameMap[x][dr]!='o') break;
	}
	attack=ar-al-1;
	defence=dr-dl-1;
	free1(x,al,ar,&attackFree1,&attackFree2);
	free1(x,dl,dr,&defenceFree1,&defenceFree2);
	possible=max(possible,getPossibleByAD(attack,defence,attackFree1,attackFree2,defenceFree1,defenceFree2));

	//ÊúÏòœø¹¥
	for (al=x-1; al>=0; al--)
	{
		if (gameMap[al][y]!='*') break;
	}
	for (ar=x+1; ar<n; ar++)
	{
		if (gameMap[ar][y]!='*') break;
	}
	//ÊúÏò·ÀÊØ
	for (dl=x-1; dl>=0; dl--)
	{
		if (gameMap[dl][y]!='o') break;
	}
	for (dr=x+1; dr<n; dr++)
	{
		if (gameMap[dr][y]!='o') break;
	}
	attack=ar-al-1;
	defence=dr-dl-1;
	free2(al,ar,y,&attackFree1,&attackFree2);
	free2(dl,dr,y,&defenceFree1,&defenceFree2);
	possible=max(possible,getPossibleByAD(attack,defence,attackFree1,attackFree2,defenceFree1,defenceFree2));

	//Õý¶ÔœÇÏßœø¹¥
	int al1,al2,ar1,ar2;
	int dl1,dl2,dr1,dr2;
	for (al1=x-1,al2=y-1; al1>=0 && al2>=0; al1--,al2--)
	{
		if (gameMap[al1][al2]!='*') break;
	}
	for (ar1=x+1,ar2=y+1; ar1<n && ar2<n; ar1++,ar2++)
	{
		if (gameMap[ar1][ar2]!='*') break;
	}
	//Õý¶ÔœÇÏß·ÀÊØ
	for (dl1=x-1,dl2=y-1; dl1>=0 && dl2>=0; dl1--,dl2--)
	{
		if (gameMap[dl1][dl2]!='o') break;
	}
	for (dr1=x+1,dr2=y+1; dr1<n && dr2<n; dr1++,dr2++)
	{
		if (gameMap[dr1][dr2]!='o') break;
	}
	attack=ar1-al1-1;
	defence=dr1-dl1-1;
	free3(al1,al2,ar1,ar2,&attackFree1,&attackFree2);
	free3(dl1,dl2,dr1,dr2,&defenceFree1,&defenceFree2);
	possible=max(possible,getPossibleByAD(attack,defence,attackFree1,attackFree1,defenceFree1,defenceFree2));

	//žº¶ÔœÇÏßœø¹¥
	for (al1=x-1,al2=y+1; al1>=0 && al2<n; al1--,al2++)
	{
		if (gameMap[al1][al2]!='*') break;
	}
	for (ar1=x+1,ar2=y-1; ar1<n && ar2>=0; ar1++,ar2--)
	{
		if (gameMap[ar1][ar2]!='*') break;
	}
	//žº¶ÔœÇÏß·ÀÊØ
	for (dl1=x-1,dl2=y+1; dl1>=0 && dl2<n; dl1--,dl2++)
	{
		if (gameMap[dl1][dl2]!='o') break;
	}
	for (dr1=x+1,dr2=y-1; dr1<n && dr2>=0; dr1++,dr2--)
	{
		if (gameMap[dr1][dr2]!='o') break;
	}
	attack=ar1-al1-1;
	defence=dr1-dl1-1;
	free4(al1,al2,ar1,ar2,&attackFree1,&attackFree2);
	free4(dl1,dl2,dr1,dr2,&defenceFree1,&defenceFree2);
	possible=max(possible,getPossibleByAD(attack,defence,attackFree1,attackFree2,defenceFree1,defenceFree2));
	return possible;
}


void goBangGameStart()
{
	int playerStep=0;
	int computerStep=0;
	int n=15;
	int i,j;
	while (1)
	{
	for (i=0; i<n; i++)
		for (j=0; j<n; j++)
			gameMap[i][j]='_';


	if (selectPlayerOrder()==0)
	{
		gameMap[n>>1][n>>1]='*';
		displayGameState();
		printf("[computer step:%d]%d,%d\n",++computerStep,(n>>1)+1,(n>>1)+1);
	}
	else
	{
		displayGameState();
	}

	while (1)
	{
		int x,y;
		while (1)
		{
			printf("[player step:%d]",++playerStep);
			//scanf("%d%d",&x,&y);
			openStartScanf(goBangGameTty);
			while (goBangGameTty->startScanf) ;
			readTwoNumber(&x,&y);
			x--,y--;
			if ( checkParameter(x,y) )
			{
				gameMap[x][y]='o';
				break;
			}
			else
			{
				playerStep--;
				printf("the position you put error\n");
			}
		}
		if (win(x,y))
		{
			displayGameState();
			printf("Congratulation you won the game\n");
			break;
		}
		int willx,willy,winPossible=-100;
		for (i=0; i<n; i++)
			for (j=0; j<n; j++)
			{
				if (gameMap[i][j]=='_')
				{
					int possible=getPossible(i,j);
					if (possible>=winPossible)
					{
						willx=i; willy=j;
						winPossible=possible;
					}
				}
			}
			gameMap[willx][willy]='*';
			displayGameState();
			printf("[computer step:%d]%d,%d\n",++computerStep,willx+1,willy+1);
			if (win(willx,willy))
			{
				printf("Sorry you lost the game\n");
				break;
			}
	}
	}

}

