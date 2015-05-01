#ifndef __DEADLINE_BIG_H__
#define __DEADLINE_BIG_H__
#include "my_common.h"
#if CORE
    #if _stringsearch_
        #define OVERHEAD_TIME 1311 //overhead deadline
        #define AVG_OVERHEAD_TIME 461 //avg overhead deadline
        #define DEADLINE_TIME (int)((3718*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1281 //max dvfs time
        #define AVG_DVFS_TIME 460 //average dvfs time
    #endif
    #if _sha_preread_
        #define OVERHEAD_TIME 4126 //overhead deadline
        #define AVG_OVERHEAD_TIME 1607 //avg overhead deadline
        #define DEADLINE_TIME (int)((45571*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 3393 //max dvfs time
        #define AVG_DVFS_TIME 1490 //average dvfs time
    #endif
    #if _rijndael_preread_
        #define OVERHEAD_TIME 2965 //overhead deadline
        #define AVG_OVERHEAD_TIME 739 //avg overhead deadline
        #define DEADLINE_TIME (int)((21040*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2522 //max dvfs time
        #define AVG_DVFS_TIME 684 //average dvfs time
    #endif
    #if _2048_slice_
        #define OVERHEAD_TIME 1857 //overhead deadline
        #define AVG_OVERHEAD_TIME 786 //avg overhead deadline
        #define DEADLINE_TIME (int)((40313*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1737 //max dvfs time
        #define AVG_DVFS_TIME 778 //average dvfs time
    #endif
    #if _curseofwar_slice_
        #define OVERHEAD_TIME 8045 //overhead deadline
        #define AVG_OVERHEAD_TIME 360 //avg overhead deadline
        #define DEADLINE_TIME (int)((5083*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1415 //max dvfs time
        #define AVG_DVFS_TIME 275 //average dvfs time
    #endif
    #if _pocketsphinx_
        #define OVERHEAD_TIME 56827 //overhead deadline
        #define AVG_OVERHEAD_TIME 23532 //avg overhead deadline
        #define DEADLINE_TIME (int)((3732440*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2694 //max dvfs time
        #define AVG_DVFS_TIME 1439 //average dvfs time
    #endif
    #if _xpilot_slice_
        #define OVERHEAD_TIME 1228 //overhead deadline
        #define AVG_OVERHEAD_TIME 56 //avg overhead deadline
        #define DEADLINE_TIME (int)((766*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1155 //max dvfs time
        #define AVG_DVFS_TIME 24 //average dvfs time
    #endif
    #if _uzbl_
        #define OVERHEAD_TIME 3459 //overhead deadline
        #define AVG_OVERHEAD_TIME 1241 //avg overhead deadline
        #define DEADLINE_TIME (int)((26860*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 3263 //max dvfs time
        #define AVG_DVFS_TIME 1233 //average dvfs time
    #endif
#else //LITTLE
    #if _xpilot_slice_
        #define OVERHEAD_TIME 1771 //overhead deadline
        #define AVG_OVERHEAD_TIME 413 //avg overhead deadline
        #define DEADLINE_TIME (int)((780*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1708 //max dvfs time
        #define AVG_DVFS_TIME 388 //average dvfs time
    #endif
#endif //CORE 
#endif //__DEADLIN_H_
