
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    
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


void initializeAllPro()	//init all processes
{
	PROCESS* p;
	firstLen=0;
	secondLen=0;
	secondHead=0;
	firstHead=0;
	lastLen=0;
	firstTick=2;
	secondTick=4;
	int i;
	for (i=0; i<NR_TASKS+NR_PROCS; i++)
	{
		p=&proc_table[i]; 
		if (p->state!=kRUNNABLE) continue;
		if (p->priority>=10)
		{
			firstQueue[firstLen]=p;
			firstLen++;
			p->ticks=firstTick;
			p->whichQueue=1;
			
		}
		else if (p->priority>=5)
		{
			secondQueue[secondLen]=p;
			secondLen++;
			p->ticks=secondTick;
			p->whichQueue=2;
			
		}
		else
		{
			lastQueue[lastLen]=p;
			lastLen++;
			p->ticks=p->priority;
			p->whichQueue=3;    
		}

	}
	

}

/*======================================================================*
                              schedule
 *======================================================================*/


PUBLIC void schedule()
{
	PROCESS*	p;
	int 		greatest_priority=0;
	int 		i;
	
	while (!greatest_priority) 
	{
		if (firstLen-firstHead>0)
		{		
			p_proc_ready=firstQueue[firstHead];	
			//greatest_priority=p_proc_ready->ticks;
			greatest_priority=p_proc_ready->priority;
			break;
		}
		else if(secondLen-secondHead>0)						
		{
			for (i=0; i<secondLen; i++)		
			{
				p=secondQueue[i];
				if (p->state!=kRUNNABLE || p->ticks==0) continue;
				if (p->ticks > greatest_priority) 
				{
					greatest_priority = p->ticks;
					p_proc_ready = p;
				}
				/*{	
					if (p->priority>greatest_priority && p->ticks!=0)
					{
						greatest_priority=p->priority;
						p_proc_ready=p;
					}
				}*/
			}
		}
		else
		{
			for(i=0;i<lastLen;i++)
			{
				p=lastQueue[i];
				if (p->ticks > greatest_priority) 
				{
					greatest_priority = p->ticks;
					p_proc_ready = p;
				}
			}
		}
		if (!greatest_priority)	initializeAllPro();
	}
	p_proc_ready->state=kRUNNING;
}


/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

