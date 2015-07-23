
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                clock.c
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



/*======================================================================*
                           clock_handler
 *======================================================================*/
PUBLIC void clock_handler(int irq)
{
	//disp_str("#");
	ticks++;
	p_proc_ready->ticks--;

	if (k_reenter != 0) {
		//disp_str("!");
		return;
	}

	if (p_proc_ready->ticks > 0) {
		return;

	}
	if (p_proc_ready->whichQueue==1)	//from queue 1 to queue 2
	{	
		p_proc_ready->whichQueue=2;
		p_proc_ready->ticks=4;
		secondQueue[secondLen]=p_proc_ready;
		secondLen++;
		firstHead++;
	}
	else if(p_proc_ready->whichQueue==2)      //from queue 2 to queue 3
	{
		p_proc_ready->whichQueue=3;
		p_proc_ready->ticks=p_proc_ready->priority;
		lastQueue[lastLen]=p_proc_ready;
		lastLen++;
		secondHead++;
	}
	else					//last queue
	{
		
	}
	p_proc_ready->state=kRUNNABLE;
	
	schedule();
}

/*======================================================================*
                              milli_delay
 *======================================================================*/
PUBLIC void milli_delay(int milli_sec)
{
	int t = get_ticks();

	while(((get_ticks() - t) * 1000 / HZ) < milli_sec) {}
}

/*======================================================================*
                           init_clock
 *======================================================================*/
PUBLIC void init_clock()
{
	/* ³õÊ¼»¯ 8253 PIT */
	out_byte(TIMER_MODE, RATE_GENERATOR);
	out_byte(TIMER0, (t_8) (TIMER_FREQ/HZ) );
	out_byte(TIMER0, (t_8) ((TIMER_FREQ/HZ) >> 8));

	put_irq_handler(CLOCK_IRQ, clock_handler);	
	enable_irq(CLOCK_IRQ);				
}

