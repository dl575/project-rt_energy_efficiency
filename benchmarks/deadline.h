#ifndef __DEADLINE_BIG_H__
#define __DEADLINE_BIG_H__
#include "my_common.h"
#if CORE
//#if _pocketsphinx_
//    #define DEADLINE_TIME (int)((1754579*SWEEP)/100) //max_exec * sweep / 100
//#endif
#if _pocketsphinx_
    //#define DEADLINE_TIME (int)((2051189*SWEEP)/100) //max_exec * sweep / 100
    #define DEADLINE_TIME (int)(SWEEP) //max_exec * sweep / 100
#endif

//#if _pocketsphinx_
//    #define DEADLINE_TIME (int)((2035026*SWEEP)/100) //max_exec * sweep / 100
//#endif
//#if _pocketsphinx_
//    #define DEADLINE_TIME (int)((1824625*SWEEP)/100) //max_exec * sweep / 100
//#endif
//#if _pocketsphinx_
//    #define DEADLINE_TIME (int)((1839828*SWEEP)/100) //max_exec * sweep / 100
//#endif

    #if _stringsearch_
        #define DEADLINE_TIME (int)((3718*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _sha_preread_
        #define DEADLINE_TIME (int)((45571*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _rijndael_preread_
        #define DEADLINE_TIME (int)((21040*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _2048_slice_
        #define DEADLINE_TIME (int)((1608*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _curseofwar_slice_
        #define DEADLINE_TIME (int)((5083*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _xpilot_slice_
        #define DEADLINE_TIME (int)((766*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _uzbl_
        #define DEADLINE_TIME (int)((26860*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _ldecode_
        #define DEADLINE_TIME (int)((36243*SWEEP)/100) //max_exec * sweep / 100
    #endif
#else //LITTLE
/*    #if _pocketsphinx_
        #define DEADLINE_TIME (int)((3088226*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _stringsearch_
        #define DEADLINE_TIME (int)((9353*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _sha_preread_
        #define DEADLINE_TIME (int)((49135*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _rijndael_preread_
        #define DEADLINE_TIME (int)((50000*SWEEP)) //max_exec * sweep / 100
//        #define DEADLINE_TIME (int)((57863*SWEEP)/100) //max_exec * sweep / 100
    #endif
    #if _xpilot_slice_
        #define DEADLINE_TIME (int)(3835) //max_exec * sweep / 100
        //#define DEADLINE_TIME (int)((3335*SWEEP)/100) //max_exec * sweep / 100
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
        #define DEADLINE_TIME (int)((39287*SWEEP)/100)
    //    #define DEADLINE_TIME (int)((50000)) //max_exec * sweep / 100
    //    #define DEADLINE_TIME (int)(34287) //max_exec * sweep / 100
    //    #define DEADLINE_TIME (int)((34287*SWEEP)/100) //max_exec * sweep / 100
    #endif
*/
//for all
#if _pocketsphinx_
    #define DEADLINE_TIME (int)((4000000*SWEEP)) //max_exec * sweep / 100
#else
    #define DEADLINE_TIME (int)(45167) //max_exec * sweep / 100

//    #define DEADLINE_TIME (int)((50000)) //max_exec * sweep / 100
#endif

#endif //CORE 
#endif //__DEADLIN_H_
