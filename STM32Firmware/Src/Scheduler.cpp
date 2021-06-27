/*
 * Scheduler.h
 *
 *  Created on: Oct 31, 2020
 *      Author: QiYang
 */

#include "Scheduler.h"
#include "SysTime.h"
#include "usart.h"
#include "DSP.h"
#include "process.h"

Task scheduler_tasks[] = {
    SCHED_TASK(USART_20Hz_Routine, 20),
    SCHED_TASK(PrintLoop, PRINTLOOP_FREQ),
};

void run_scheduler(void)
{
    uint64_t now;
    for (unsigned int i = 0; i < (sizeof(scheduler_tasks) / sizeof(Task)); i++)
    {
        now = GetSysTicks();
        if (now > scheduler_tasks[i].next_due_ticks)
        {
            //USART_Printf(&huart2,"Invoke %s, time:%llu\n", scheduler_tasks[i].name, now);
            scheduler_tasks[i].function();
            scheduler_tasks[i].next_due_ticks += scheduler_tasks[i].period_ticks;
        }
    }
}