#ifndef _TIMER_H_
#define _TIMER_H_
#include "config.h"
#include "type.h"

typedef void (*TM_DONE_CB) (U32 time);

extern volatile U32 realtime;


void tm_start_rt (void);

void tm_start(unsigned freq,TM_DONE_CB cb);
void tm_stop(void);
BOOL tm_task(void);



#endif
