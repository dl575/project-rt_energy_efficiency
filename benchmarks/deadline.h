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
        #define OVERHEAD_TIME 70741 //overhead deadline
        #define AVG_OVERHEAD_TIME 41526 //avg overhead deadline
        #define DEADLINE_TIME (int)((13907437*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 328 //max dvfs time
        #define AVG_DVFS_TIME 201 //average dvfs time
    #endif
    #if _stringsearch_
        #define OVERHEAD_TIME 2834 //overhead deadline
        #define AVG_OVERHEAD_TIME 1683 //avg overhead deadline
        #define DEADLINE_TIME (int)((9353*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2270 //max dvfs time
        #define AVG_DVFS_TIME 1679 //average dvfs time
    #endif
    #if _sha_preread_
        #define OVERHEAD_TIME 2201 //overhead deadline
        #define AVG_OVERHEAD_TIME 327 //avg overhead deadline
        #define DEADLINE_TIME (int)((118798*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1809 //max dvfs time
        #define AVG_DVFS_TIME 225 //average dvfs time
    #endif
    #if _rijndael_preread_
        #define OVERHEAD_TIME 2708 //overhead deadline
        #define AVG_OVERHEAD_TIME 1892 //avg overhead deadline
        #define DEADLINE_TIME (int)((45641*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2200 //max dvfs time
        #define AVG_DVFS_TIME 1705 //average dvfs time
    #endif
    #if _xpilot_slice_
        #define OVERHEAD_TIME 1556 //overhead deadline
        #define AVG_OVERHEAD_TIME 216 //avg overhead deadline
        #define DEADLINE_TIME (int)((3335*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 1349 //max dvfs time
        #define AVG_DVFS_TIME 84 //average dvfs time
    #endif
    #if _2048_slice_
        #define OVERHEAD_TIME 3813 //overhead deadline
        #define AVG_OVERHEAD_TIME 1941 //avg overhead deadline
        #define DEADLINE_TIME (int)((2719*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 3497 //max dvfs time
        #define AVG_DVFS_TIME 1901 //average dvfs time
    #endif
    #if _curseofwar_slice_
        #define OVERHEAD_TIME 6222 //overhead deadline
        #define AVG_OVERHEAD_TIME 1488 //avg overhead deadline
        #define DEADLINE_TIME (int)((10592*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2100 //max dvfs time
        #define AVG_DVFS_TIME 1417 //average dvfs time
    #endif

    #if _uzbl_
        #define OVERHEAD_TIME 2123 //overhead deadline
        #define AVG_OVERHEAD_TIME 358 //avg overhead deadline
        #define DEADLINE_TIME (int)((61381*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2097 //max dvfs time
        #define AVG_DVFS_TIME 355 //average dvfs time
    #endif
    #if _ldecode_
        #define OVERHEAD_TIME 308678 //overhead deadline
        #define AVG_OVERHEAD_TIME 50129 //avg overhead deadline
        #define DEADLINE_TIME (int)((70975*SWEEP)/100) //max_exec * sweep / 100
        #define MAX_DVFS_TIME 2221 //max dvfs time
        #define AVG_DVFS_TIME 429 //average dvfs time
    #endif
#endif //CORE 
#endif //__DEADLIN_H_
