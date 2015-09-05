#ifndef __DEADLINE_BIG_H__
#define __DEADLINE_BIG_H__
#include "my_common.h"
#if CORE
    #if _pocketsphinx_
        #define DEADLINE_TIME (int)((4000000*SWEEP)/100) //max_exec * sweep / 100
    #else
        #define DEADLINE_TIME (int)((50000*SWEEP)/100) //max_exec * sweep / 100
    #endif
#else //LITTLE
    #if _pocketsphinx_
        #define DEADLINE_TIME (int)((4000000*SWEEP)/100) //max_exec * sweep / 100
    #else
        #define DEADLINE_TIME (int)((50000*SWEEP)/100) //max_exec * sweep / 100
    #endif
#endif //CORE 
#endif //__DEADLIN_H_
