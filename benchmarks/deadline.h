#ifndef __DEADLINE_BIG_H__
#define __DEADLINE_BIG_H__
#include "my_common.h"
#if CORE
    #if _pocketsphinx_
        #define OVERHEAD_TIME 56827 //overhead deadline
        #define AVG_OVERHEAD_TIME 23532 //avg overhead deadline
        #define DEADLINE_TIME (int)((3732440*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2694 //max dvfs time
        #define AVG_DVFS_TIME 1439 //average dvfs time
    #endif
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
        #define OVERHEAD_TIME 2592 //overhead deadline
        #define AVG_OVERHEAD_TIME 1292 //avg overhead deadline
        #define DEADLINE_TIME (int)((1608*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2532 //max dvfs time
        #define AVG_DVFS_TIME 1274 //average dvfs time
    #endif
    #if _curseofwar_slice_
        #define OVERHEAD_TIME 8045 //overhead deadline
        #define AVG_OVERHEAD_TIME 360 //avg overhead deadline
        #define DEADLINE_TIME (int)((5083*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1415 //max dvfs time
        #define AVG_DVFS_TIME 275 //average dvfs time
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
    #if _ldecode_
        #define OVERHEAD_TIME 211014 //overhead deadline
        #define AVG_OVERHEAD_TIME 41391 //avg overhead deadline
        #define DEADLINE_TIME (int)((36243*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2405 //max dvfs time
        #define AVG_DVFS_TIME 350 //average dvfs time
    #endif
#else //LITTLE
    #if _pocketsphinx_
        #define DEADLINE_TIME (int)((4631866*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _stringsearch_
        #define DEADLINE_TIME (int)((9353*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _sha_preread_
        #define DEADLINE_TIME (int)((102139*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _rijndael_preread_
        #define DEADLINE_TIME (int)((104819*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _xpilot_slice_
        #define DEADLINE_TIME (int)((3335*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _2048_slice_
        #define DEADLINE_TIME (int)((2719*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _curseofwar_slice_sdl_
        #define DEADLINE_TIME (int)((35909*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _curseofwar_slice_no_sdl_
        #define DEADLINE_TIME (int)((10592*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _uzbl_
        #define DEADLINE_TIME (int)((61381*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _ldecode_
        #define DEADLINE_TIME (int)((70975*SWEEP)/100) //max_exec * sweep / 100
    #endif
#endif //CORE 
#endif //__DEADLIN_H_
