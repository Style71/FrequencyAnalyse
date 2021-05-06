/*
 * DataStructure.h
 *
 *  Created on: Oct 31, 2020
 *      Author: QiYang
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define SYSTICK_CLOCK_FLOAT (2.16e8) //216MHz
typedef struct
{
    void (*function)(void);
    const uint64_t period_ticks;
    uint64_t next_due_ticks;
} Task;

#define SCHED_TASK(func, _rate_hz)                                  \
    {                                                               \
        .function = func,                                           \
        .period_ticks = (uint64_t)(SYSTICK_CLOCK_FLOAT / _rate_hz), \
        .next_due_ticks = 0,                                        \
    }

void run_scheduler(void);

#endif