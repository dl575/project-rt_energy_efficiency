/**
 * @file   recogmain.c
 * 
 * <JA>
 * @brief  認識メイン関数
 * </JA>
 * 
 * <EN>
 * @brief  Main function of recognition process.
 * </EN>
 * 
 * @author Akinobu Lee
 * @date   Wed Aug  8 14:53:53 2007
 *
 * $Revision: 1.23 $
 * 
 */

/*
 * Copyright (c) 1991-2013 Kawahara Lab., Kyoto University
 * Copyright (c) 1997-2000 Information-technology Promotion Agency, Japan
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2013 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */
/**
 * @mainpage
 *
 * <EN>
 * This is a source code browser of Julius.
 *
 * - Sample code to use JuliusLib: julius-simple.c
 * - JuliusLib API reference: @ref jfunc
 * - List of callbacks: libjulius/include/julius/callback.h
 *
 * You can access documentation for files, functions and structures
 * from the tabs at the top of this page.
 * 
 * </EN>
 * <JA>
 * これは Julius のソースコードのブラウザです．
 *
 * - JuliusLibを使用するサンプルコード: julius-simple/julius-simple.c
 * - JuliusLib API リファレンス： @ref jfunc
 * - コールバック 一覧: libjulius/include/julius/callback.h
 *
 * ページ上部のタブからファイル・関数・構造体等の説明を見ることが出来ます．
 * 
 * </JA>
 * 
 */
/**
 * @defgroup jfunc JuliusLib API
 *
 * <EN>
 * Here is a reference of all Julius library API functions.
 * </EN>
 * <JA>
 * Julius ライブラリ API 関数のリファレンスです. 
 * </JA>
 * 
 */
/**
 * @defgroup engine Basic API
 * @ingroup jfunc
 *
 * <EN>
 * Basic functions to start-up and initialize engines.
 * </EN>
 * <JA>
 * 認識エンジンの設定等
 * </JA>
 * 
 */
/**
 * @defgroup callback Callback API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to add callback to get results and status.
 * </EN>
 * <JA>
 * 認識結果やエンジン状態を知るためのコールバック
 * </JA>
 * 
 */
/**
 * @defgroup pauseresume Pause and Resume API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to pause / resume engine inputs.
 * </EN>
 * <JA>
 * エンジンの一時停止・再開
 * </JA>
 * 
 */
/**
 * @defgroup userfunc User function API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to register user function to be applied inside Julius.
 * </EN>
 * <JA>
 * ユーザ関数の登録
 * </JA>
 * 
 */
/**
 * @defgroup jfunc_process Process API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to create / remove / (de)activate recognition process and models
 * on live.
 * </EN>
 * <JA>
 * モデルおよび認識プロセスの動的追加・削除・有効化・無効化
 * </JA>
 * 
 */
/**
 * @defgroup grammar Grammar / Dictionary API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to manage grammars or word dictionaries at run time.
 * </EN>
 * <JA>
 * 文法・単語辞書の操作
 * </JA>
 * 
 */
/**
 * @defgroup jconf Jconf configuration API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to load / create configuration parameters.
 * </EN>
 * <JA>
 * Jconf 構造体によるパラメータ情報の管理
 * </JA>
 * 
 */
/**
 * @defgroup instance LM/AM/SR instance API
 * @ingroup jfunc
 *
 * <EN>
 * Functions to handle modules and processes directly.
 * </EN>
 * <JA>
 * モデルモジュールやプロセスを直接扱う関数．
 * </JA>
 * 
 */

#define GLOBAL_VARIABLE_DEFINE	///< Actually make global vars in global.h
#include <julius/julius.h>
#include <signal.h>
#if defined(_WIN32) && !defined(__CYGWIN32__)
#include <mbctype.h>
#include <mbstring.h>
#endif

/* ---------- utility functions -----------------------------------------*/
#ifdef REPORT_MEMORY_USAGE
/** 
 * <JA>
 * 通常終了時に使用メモリ量を調べて出力する (Linux, sol2)
 * 
 * </JA>
 * <EN>
 * Get process size and output on normal exit. (Linux, sol2)
 * 
 * </EN>
 */
static void
print_mem()
{
  char buf[200];
  sprintf(buf,"ps -o vsz,rss -p %d",getpid());
  system(buf);
  fflush(stdout);
  fflush(stderr);
}
#endif

#include "timing.h"

//---------------------modified by TJSong----------------------//
//manually set below
#define CORE 1 //0:LITTLE, 1:big
#define PREDICT_EN 1 //0:prediction off, 1:prediction on
#define DELAY_EN 1 //0:delay off, 1:delay on
#define DEADLINE_TIME 42004  //big
//#define DEADLINE_TIME    //LITTLE
//automatically set
#define MAX_FREQ ((CORE)?(2000000):(1400000))

FILE *fp_power; //File pointer of power of A7 (LITTLE) core or A15 (big) core power sensor file
FILE *fp_freq; //File pointer of freq of A7 (LITTLE) core or A15 (big) core power sensor file
float watt; //Value (Watt) at start point.
int khz; //Value (khz) at start point.

FILE *fp_max_freq; //File pointer scaling_max_freq
int predicted_freq = MAX_FREQ;

void fopen_all(void){
    if(NULL == (fp_max_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_max_freq", "w"))){
        printf("ERROR : FILE READ FAILED (SEE IF FILE IS PRIVILEGED)\n");
        return;
    }
    return;
}

void fclose_all(void){
   fclose(fp_max_freq);
    return;
}

void print_power(void){
    if(NULL == (fp_power = fopen("/sys/bus/i2c/drivers/INA231/3-0040/sensor_W", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return;
    }
    if(NULL == (fp_freq = fopen("/sys/devices/system/cpu/cpu4/cpufreq/scaling_cur_freq", "r"))){
        printf("ERROR : FILE READ FAILED\n");
        return;
    }
    fscanf(fp_power, "%f", &watt);
    fscanf(fp_freq, "%d", &khz);
    printf("big core power : %fW, big core freq : %dkhz\n", watt, khz);  
    fclose(fp_power); 
    fclose(fp_freq);
    return;
}

inline void set_freq(float exec_time){
    //calculate predicted freq and round up by adding 99999
    predicted_freq = exec_time * MAX_FREQ / DEADLINE_TIME + 99999;
    //if less then 200000, just set it minimum (200000)
    predicted_freq = (predicted_freq < 200000)?(200000):(predicted_freq);
    //printf("predicted freq %d in set_freq function (rounded up)\n", predicted_freq); 
    //set maximum frequency, because performance governor always use maximum freq.
    fprintf(fp_max_freq, "%d", predicted_freq);
    //start_timing();
    fflush(fp_max_freq);
    //end_timing();
   // print_set_dvfs_timing();
    

    return;
}

//---------------------modified by TJSong----------------------//


/** 
 * <EN>
 * allocate storage of recognition alignment results.
 *
 * @return the new pointer
 * </EN>
 * <JA>
 * アラインメント結果の格納場所を確保
 *
 * @return 確保された領域へのポインタ
 * </JA>
 *
 * @callgraph
 * @callergraph
 * 
 */
SentenceAlign *
result_align_new()
{
  SentenceAlign *new;
  new = (SentenceAlign *)mymalloc(sizeof(SentenceAlign));
  new->w = NULL;
  new->ph = NULL;
  new->loc = NULL;
  new->begin_frame = NULL;
  new->end_frame = NULL;
  new->avgscore = NULL;
  new->is_iwsp = NULL;
  new->next = NULL;
  return new;
}

/** 
 * <EN>
 * free storage of recognition alignment results.
 *
 * @param a [i/o] alignment data to be released
 * </EN>
 * <JA>
 * アラインメント結果の格納場所を確保
 *
 * @param a [i/o] 解放されるアラインメントデータ
 * </JA>
 *
 * @callgraph
 * @callergraph
 * 
 */
void
result_align_free(SentenceAlign *a)
{
  if (a->w) free(a->w);
  if (a->ph) free(a->ph);
  if (a->loc) free(a->loc);
  if (a->begin_frame) free(a->begin_frame);
  if (a->end_frame) free(a->end_frame);
  if (a->avgscore) free(a->avgscore);
  if (a->is_iwsp) free(a->is_iwsp);
  free(a);
}

/** 
 * <EN>
 * Allocate storage of recognition results.
 * </EN>
 * <JA>
 * 認識結果の格納場所を確保する. 
 * </JA>
 * 
 * @param r [out] recognition process instance
 * @param num [in] number of sentences to be output
 *
 * @callgraph
 * @callergraph
 * 
 */
void
result_sentence_malloc(RecogProcess *r, int num)
{
  int i;
  r->result.sent = (Sentence *)mymalloc(sizeof(Sentence) * num);
  for(i=0;i<num;i++) r->result.sent[i].align = NULL;
  r->result.sentnum = 0;
}

/** 
 * <EN>
 * Free storage of recognition results.
 * </EN>
 * <JA>
 * 認識結果の格納場所を解放する. 
 * </JA>
 * 
 * @param r [i/o] recognition process instance
 * 
 * @callgraph
 * @callergraph
 */
void
result_sentence_free(RecogProcess *r)
{  
  int i;
  SentenceAlign *a, *atmp;
  if (r->result.sent) {
    for(i=0;i<r->result.sentnum;i++) {
      a = r->result.sent[i].align;
      while(a) {
	atmp = a->next;
	result_align_free(a);
	a = atmp;
      }
    }
    free(r->result.sent);
    r->result.sent = NULL;
  }
}

/** 
 * <EN>
 * Clear all result storages for next input.
 * </EN>
 * <JA>
 * 認識結果の格納場所を全てクリアする. 
 * </JA>
 * 
 * @param r [in] recognition process instance.
 * 
 * @callgraph
 * @callergraph
 */
void
clear_result(RecogProcess *r)
{
#ifdef WORD_GRAPH
  /* clear 1st pass word graph output */
  wordgraph_clean(&(r->result.wg1));
#endif

  if (r->lmvar == LM_DFA_WORD) {
    if (r->result.status == J_RESULT_STATUS_SUCCESS) {
      /* clear word recog result of first pass as in final result */
      free(r->result.sent);
    }
  } else {
    if (r->graphout) {
      if (r->config->graph.confnet) {
	/* free confusion network clusters */
	cn_free_all(&(r->result.confnet));
      } else if (r->config->graph.lattice) {
      }
      /* clear all wordgraph */
      wordgraph_clean(&(r->result.wg));
    }
    result_sentence_free(r);
  }
}

/* --------------------- speech buffering ------------------ */

/** 
 * <JA>
 * @brief  検出された音をバッファに保存する adin_go() コールバック
 *
 * この関数は，検出された音声入力を順次 recog->speech に記録して
 * いく. バッファ処理モード（＝非リアルタイムモード）で認識を行なう
 * ときに用いられる. 
 * 
 * @param now [in] 検出された音声波形データの断片
 * @param len [in] @a now の長さ(サンプル数)
 * @param recog [i/o] エンジンインスタンス
 * 
 * @return エラー時 -1 (adin_go は即時中断する)，通常時 0 (adin_go は
 * 続行する)，区間終了要求時 1 (adin_go は現在の音声区間を閉じる). 
 * 
 * </JA>
 * <EN>
 * @brief  adin_go() callback to score triggered inputs to buffer.
 *
 * This function records the incomping speech segments detected in adin_go()
 * to recog->speech.  This function will be used when recognition runs
 * in buffered mode (= non-realtime mode).
 * 
 * @param now [in] input speech samples.
 * @param len [in] length of @a now in samples
 * @param recog [i/o] engine instance
 * 
 * @return -1 on error (tell adin_go() to terminate), 0 on success (tell
 * adin_go() to continue recording), or 1 when this function requires
 * input segmentation.
 * </EN>
 */
int
adin_cut_callback_store_buffer(SP16 *now, int len, Recog *recog)
{
  if (recog->speechlen == 0) {		/* first part of a segment */
    if (!recog->process_active) {
      return(1);
    }
  }

  if (recog->speechlen + len > recog->speechalloclen) {
    while (recog->speechlen + len > recog->speechalloclen) {
      recog->speechalloclen += MAX_SPEECH_ALLOC_STEP;
    }
    if (recog->speech == NULL) {
      recog->speech = (SP16 *)mymalloc(sizeof(SP16) * recog->speechalloclen);
    } else {
      if (debug2_flag) {
	jlog("STAT: expanding recog->speech to %d samples\n", recog->speechalloclen);
      }
      recog->speech = (SP16 *)myrealloc(recog->speech, sizeof(SP16) * recog->speechalloclen);
    }
  }

  /* store now[0..len] to recog->speech[recog->speechlen] */
  memcpy(&(recog->speech[recog->speechlen]), now, len * sizeof(SP16));
  recog->speechlen += len;
  return(0);			/* tell adin_go to continue reading */
}


/* --------------------- adin check callback --------------- */
/** 
 * <JA>
 * @brief  音声入力中に定期的に実行されるコールバック. 
 *
 * この関数は，adin_go() にて音声入力待ち，あるいは音声認識中に
 * 定期的に繰り返し呼び出される関数である. ユーザ定義のコールバック
 * (CALLBACK_POLL) の呼び出し，および中断判定を行う. 
 *
 * @param recog [in] エンジンインスタンス
 * 
 * @return 通常時 0, 即時中断を要求時 -2, 認識中止の要求時は -1 を返す. 
 * </JA>
 * <EN>
 * @brief  callback function periodically called while input.
 *
 * This function will be called periodically from adin_go() while
 * waiting input or processing recognition.  It will call user-defined
 * callback registered in CALLBACK_POLL,  check for the process
 * status and issue recognition termination request.
 *
 * @param recog [in] engine instance
 * 
 * @return 0 normally, -2 for immediate termination, and -1 if requesting
 * recognition stop.
 * 
 * </EN>
 */
static int
callback_check_in_adin(Recog *recog)
{
  /* module: check command and terminate recording when requested */
  callback_exec(CALLBACK_POLL, recog);
  /* With audio input via adinnet, TERMINATE command will issue terminate
     command to the adinnet client.  The client then stops recording
     immediately and return end-of-segment ack.  Then it will cause this
     process to stop recognition as normal.  So we need not to
     perform immediate termination at this callback, but just ignore the
     results in the main.c.  */
#if 1
  if (recog->process_want_terminate) { /* TERMINATE ... force termination */
    return(-2);
  }
  if (recog->process_want_reload) {
    return(-1);
  }
#else
  if (recog->process_want_terminate /* TERMINATE ... force termination */
      && recog->jconf->input.speech_input != SP_ADINNET) {
    return(-2);
  }
  if (recog->process_want_reload) {
    return(-1);
  }
#endif
  return(0);
}

/*********************/
/* open input stream */
/*********************/
/** 
 * <EN>
 * Open input stream.
 * </EN>
 * <JA>
 * 音声入力ストリームを開く
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * @param file_or_dev_name [in] file or device name of the device
 * 
 * @return 0 on success, -1 on error, -2 on device initialization error.
 * 
 * @callgraph
 * @callergraph
 * @ingroup engine
 */
int
j_open_stream(Recog *recog, char *file_or_dev_name)
{
  Jconf *jconf;
  RecogProcess *r;
  char *p;

  jconf = recog->jconf;

  if (jconf->input.type == INPUT_WAVEFORM) {
    /* begin A/D input */
    if (adin_begin(recog->adin, file_or_dev_name) == FALSE) {
      return -2;
    }
    /* create A/D-in thread here */
#ifdef HAVE_PTHREAD
    if (recog->adin->enable_thread && ! recog->adin->input_side_segment) {
      if (adin_thread_create(recog) == FALSE) {
	return -2;
      }
    }
#endif
    /* when using adin func, input name should be obtained when called */
  } else {
    switch(jconf->input.speech_input) {
    case SP_MFCMODULE:
      param_init_content(recog->mfcclist->param);
      if (mfc_module_begin(recog->mfcclist) == FALSE) return -2;
      /* when using mfc module func, input name should be obtained when called */
      break;
    case SP_MFCFILE:
    case SP_OUTPROBFILE:
      /* read parameter file */
      param_init_content(recog->mfcclist->param);
      if (rdparam(file_or_dev_name, recog->mfcclist->param) == FALSE) {
	jlog("ERROR: error in reading parameter file: %s\n", file_or_dev_name);
	return -1;
      }
      switch(jconf->input.speech_input) {
      case SP_MFCFILE:
	/* check and strip invalid frames */
	if (jconf->preprocess.strip_zero_sample) {
	  param_strip_zero(recog->mfcclist->param);
	}
	recog->mfcclist->param->is_outprob = FALSE;
	break;
      case SP_OUTPROBFILE:
	/* mark that this is outprob file */
	recog->mfcclist->param->is_outprob = TRUE;
	/* check the size */
	for(r=recog->process_list;r;r=r->next) {
	  if (r->am->hmminfo->totalstatenum != recog->mfcclist->param->veclen) {
	    jlog("ERROR: j_open_stream: outprob vector size != number of states in hmmdefs\n");
	    jlog("ERROR: j_open_stream: outprob size = %d, #state = %d\n", recog->mfcclist->param->veclen, r->am->hmminfo->totalstatenum);
	    return -1;
	  }
	}
	jlog("STAT: outprob vector size = %d, samplenum = %d\n", recog->mfcclist->param->veclen, recog->mfcclist->param->samplenum);
	break;
      }
      /* output frame length */
      callback_exec(CALLBACK_STATUS_PARAM, recog);
      /* store the input filename here */
      strncpy(recog->adin->current_input_name, file_or_dev_name, MAXPATHLEN);
      break;
    default:
      jlog("ERROR: j_open_stream: none of SP_MFC_*??\n");
      return -1;
    }
  }

  if (jconf->input.speech_input != SP_MFCFILE && jconf->input.speech_input != SP_OUTPROBFILE) {
    /* store current input name using input source specific function */
    p = j_get_current_filename(recog);
    if (p) {
      strncpy(recog->adin->current_input_name, p, MAXPATHLEN);
    } else {
      recog->adin->current_input_name[0] = '\0';
    }
  }
      
  return 0;

}

/** 
 * <EN>
 * Close input stream.  The main recognition loop will be stopped after
 * stream has been closed.
 * </EN>
 * <JA>
 * 音声入力ストリームを閉じる．認識のメインループは閉じられた後終了する．
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * 
 * @return 0 on success, -1 on general error, -2 on device error.
 * 
 * @callgraph
 * @callergraph
 * @ingroup engine
 */
int
j_close_stream(Recog *recog)
{
  Jconf *jconf;

  jconf = recog->jconf;

  if (jconf->input.type == INPUT_WAVEFORM) {
#ifdef HAVE_PTHREAD
    /* close A/D-in thread here */
    if (! recog->adin->input_side_segment) {
      if (recog->adin->enable_thread) {
	if (adin_thread_cancel(recog) == FALSE) {
	  return -2;
	}
      } else {
	recog->adin->end_of_stream = TRUE;
      }
    }
#else
    if (! recog->adin->input_side_segment) {
      recog->adin->end_of_stream = TRUE;
    }
#endif
  } else {
    switch(jconf->input.speech_input) {
    case SP_MFCMODULE:
      if (mfc_module_end(recog->mfcclist) == FALSE) return -2;
      break;
    case SP_MFCFILE:
    case SP_OUTPROBFILE:
      /* nothing to do */
      break;
    default:
      jlog("ERROR: j_close_stream: none of SP_MFC_*??\n");
      return -1;
    }
  }
      
  return 0;

}

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

/** 
 * <EN>
 * Recognition error handling.
 * </EN>
 * <JA>
 * エラーによる認識終了時の処理. 
 * </JA>
 * 
 * @param recog [in] engine instance
 * @param status [in] error status to be set
 * 
 */
static void
result_error(Recog *recog, int status)
{
  MFCCCalc *mfcc;
  RecogProcess *r;
  boolean ok_p;

  for(r=recog->process_list;r;r=r->next) r->result.status = status;

  ok_p = FALSE;
  for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
    if (mfcc->f > 0) {
      ok_p = TRUE;
      break;
    }
  }
  if (ok_p) {			/* had some input */
    /* output as rejected */
    callback_exec(CALLBACK_RESULT, recog);
#ifdef ENABLE_PLUGIN
    plugin_exec_process_result(recog);
#endif
  }
}

int call_adin_go(Recog *recog)
{
    start_timing();//TJSong
  int loop_counter[188] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int ret;
  {
    int return_value;
    callback_exec(CALLBACK_EVENT_SPEECH_READY, recog);
    {
      {
        int return_value;
        ADIn *a_rename1;
        int i_rename1;
        int ad_process_ret_rename1;
        int imax_rename1;
        int len_rename1;
        int cnt_rename1;
        int wstep_rename1;
        int end_status_rename1 = 0;
        boolean transfer_online_local_rename1;
        int zc_rename1;
        a_rename1 = recog->adin;
        if (a_rename1->need_init)
        {
          loop_counter[0]++;
          a_rename1->bpmax = MAXSPEECHLEN;
          a_rename1->bp = 0;
          a_rename1->is_valid_data = FALSE;
          if (a_rename1->adin_cut_on)
          {
            loop_counter[1]++;
            reset_count_zc_e(&a_rename1->zc, a_rename1->thres, a_rename1->c_length, a_rename1->c_offset);
          }

          a_rename1->end_of_stream = FALSE;
          a_rename1->nc = 0;
          a_rename1->sblen = 0;
          a_rename1->need_init = FALSE;
        }

        for (;;)
        {
          loop_counter[2]++;
          if (a_rename1->end_of_stream && (a_rename1->bp == 0))
          {
            loop_counter[3]++;
            break;
          }

          if (a_rename1->end_of_stream)
          {
            loop_counter[4]++;
            a_rename1->current_len = a_rename1->bp;
          }
          else
          {
            if (a_rename1->down_sample)
            {
              loop_counter[5]++;
              int ad_read_result_rename1 = (*a_rename1->ad_read)(a_rename1->buffer48, (a_rename1->bpmax - a_rename1->bp) * a_rename1->io_rate);
              cnt_rename1 = ad_read_result_rename1;
            }
            else
            {
              int ad_read_result_rename1 = (*a_rename1->ad_read)(&a_rename1->buffer[a_rename1->bp], a_rename1->bpmax - a_rename1->bp);
              cnt_rename1 = ad_read_result_rename1;
            }

            if (cnt_rename1 < 0)
            {
              loop_counter[6]++;
              switch (cnt_rename1)
              {
                case -1:
                  loop_counter[7]++;
                  a_rename1->input_side_segment = FALSE;
                  end_status_rename1 = 0;
                  break;

                case -2:
                  loop_counter[8]++;
                  a_rename1->input_side_segment = FALSE;
                  end_status_rename1 = -1;
                  break;

                case -3:
                  loop_counter[9]++;
                  a_rename1->input_side_segment = TRUE;
                  end_status_rename1 = 0;

              }

              a_rename1->end_of_stream = TRUE;
              cnt_rename1 = 0;
              if (a_rename1->bp == 0)
              {
                loop_counter[10]++;
                break;
              }

            }

            if (a_rename1->down_sample && (cnt_rename1 != 0))
            {
              loop_counter[11]++;
              cnt_rename1 = ds48to16(&a_rename1->buffer[a_rename1->bp], a_rename1->buffer48, cnt_rename1, a_rename1->bpmax - a_rename1->bp, a_rename1->ds);
              if (cnt_rename1 < 0)
              {
                loop_counter[12]++;
                jlog("ERROR: adin_cut: error in down sampling\n");
                end_status_rename1 = -1;
                a_rename1->end_of_stream = TRUE;
                cnt_rename1 = 0;
                if (a_rename1->bp == 0)
                {
                  loop_counter[13]++;
                  break;
                }

              }

            }

            if ((cnt_rename1 > 0) && (a_rename1->level_coef != 1.0))
            {
              loop_counter[14]++;
              for (i_rename1 = a_rename1->bp; i_rename1 < (a_rename1->bp + cnt_rename1); i_rename1++)
              {
                loop_counter[15]++;
                a_rename1->buffer[i_rename1] = (SP16) (((float) a_rename1->buffer[i_rename1]) * a_rename1->level_coef);
              }

            }

            if (cnt_rename1 > 0)
            {
              loop_counter[16]++;
              callback_exec_adin(CALLBACK_ADIN_CAPTURED, recog, &a_rename1->buffer[a_rename1->bp], cnt_rename1);
              a_rename1->total_captured_len += cnt_rename1;
            }

            if (cnt_rename1 > 0)
            {
              loop_counter[17]++;
              if (a_rename1->strip_flag)
              {
                loop_counter[18]++;
                len_rename1 = strip_zero(&a_rename1->buffer[a_rename1->bp], cnt_rename1);
                if (len_rename1 != cnt_rename1)
                {
                  loop_counter[19]++;
                  cnt_rename1 = len_rename1;
                }

              }

              if (a_rename1->need_zmean)
              {
                loop_counter[20]++;
                sub_zmean(&a_rename1->buffer[a_rename1->bp], cnt_rename1);
              }

            }

            a_rename1->current_len = a_rename1->bp + cnt_rename1;
          }

          if (callback_check_in_adin != NULL)
          {
            loop_counter[21]++;
            i_rename1 = callback_check_in_adin(recog);
            if (i_rename1 < 0)
            {
              loop_counter[22]++;
              if ((i_rename1 == (-2)) || ((i_rename1 == (-1)) && (a_rename1->is_valid_data == FALSE)))
              {
                loop_counter[23]++;
                end_status_rename1 = -2;
                if (a_rename1->current_len > 0)
                {
                  loop_counter[24]++;
                  callback_exec(CALLBACK_EVENT_SPEECH_STOP, recog);
                }

                a_rename1->need_init = TRUE;
                goto break_input;
              }

            }

          }

          if (a_rename1->current_len == 0)
          {
            loop_counter[25]++;
            continue;
          }

          if (((!a_rename1->adin_cut_on) && (a_rename1->is_valid_data == FALSE)) && (a_rename1->current_len > 0))
          {
            loop_counter[26]++;
            a_rename1->is_valid_data = TRUE;
            callback_exec(CALLBACK_EVENT_SPEECH_START, recog);
          }

          wstep_rename1 = a_rename1->chunk_size;
          imax_rename1 = a_rename1->current_len < wstep_rename1 ? a_rename1->current_len : wstep_rename1;
          if (wstep_rename1 > a_rename1->current_len)
          {
            loop_counter[27]++;
            wstep_rename1 = a_rename1->current_len;
          }

          i_rename1 = 0;
          while ((i_rename1 + wstep_rename1) <= imax_rename1)
          {
            loop_counter[28]++;
            if (a_rename1->adin_cut_on)
            {
              loop_counter[29]++;
              zc_rename1 = count_zc_e(&a_rename1->zc, &a_rename1->buffer[i_rename1], wstep_rename1);
              if (zc_rename1 > a_rename1->noise_zerocross)
              {
                loop_counter[30]++;
                if (a_rename1->is_valid_data == FALSE)
                {
                  loop_counter[31]++;
                  a_rename1->is_valid_data = TRUE;
                  a_rename1->nc = 0;
                  a_rename1->last_trigger_sample = (((a_rename1->total_captured_len - a_rename1->current_len) + i_rename1) + wstep_rename1) - a_rename1->zc.valid_len;
                  callback_exec(CALLBACK_EVENT_SPEECH_START, recog);
                  a_rename1->last_trigger_len = 0;
                  if (a_rename1->zc.valid_len > wstep_rename1)
                  {
                    loop_counter[32]++;
                    a_rename1->last_trigger_len += a_rename1->zc.valid_len - wstep_rename1;
                  }

                  if (RealTimePipeLine != NULL)
                  {
                    loop_counter[33]++;
                    zc_copy_buffer(&a_rename1->zc, a_rename1->cbuf, &len_rename1);
                    if ((len_rename1 - wstep_rename1) > 0)
                    {
                      loop_counter[34]++;
                      callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a_rename1->cbuf, len_rename1 - wstep_rename1);
                      {
                        int return_value;
                        int nowlen_rename2 = len_rename1 - wstep_rename1;
                        int i_rename2;
                        int now_rename2;
                        int ret_rename2;
                        MFCCCalc *mfcc_rename2;
                        RealBeam *r_rename2;
                        r_rename2 = &recog->real;
                        now_rename2 = 0;
                        r_rename2->last_is_segmented = FALSE;
                        while (now_rename2 < nowlen_rename2)
                        {
                          loop_counter[35]++;
                          for (mfcc_rename2 = recog->mfcclist; mfcc_rename2; mfcc_rename2 = mfcc_rename2->next)
                          {
                            loop_counter[36]++;
                            if (mfcc_rename2->f >= r_rename2->maxframelen)
                            {
                              loop_counter[37]++;
                              jlog("Warning: too long input (> %d frames), segment it now\n", r_rename2->maxframelen);
                              {
                                return_value = 1;
                                goto return2;
                              }
                            }

                          }

                          for (i_rename2 = min(r_rename2->windowlen - r_rename2->windownum, nowlen_rename2 - now_rename2); i_rename2 > 0; i_rename2--)
                          {
                            loop_counter[38]++;
                            r_rename2->window[r_rename2->windownum++] = (float) a_rename1->cbuf[now_rename2++];
                          }

                          if (r_rename2->windownum < r_rename2->windowlen)
                          {
                            loop_counter[39]++;
                            break;
                          }

                          for (mfcc_rename2 = recog->mfcclist; mfcc_rename2; mfcc_rename2 = mfcc_rename2->next)
                          {
                            loop_counter[40]++;
                            mfcc_rename2->valid = FALSE;
                            if ((*recog->calc_vector)(mfcc_rename2, r_rename2->window, r_rename2->windowlen))
                            {
                              loop_counter[41]++;
                              mfcc_rename2->valid = TRUE;
                              if (param_alloc(mfcc_rename2->param, mfcc_rename2->f + 1, mfcc_rename2->param->veclen) == FALSE)
                              {
                                loop_counter[42]++;
                                jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
                                {
                                  return_value = -1;
                                  goto return2;
                                }
                              }

                              memcpy(mfcc_rename2->param->parvec[mfcc_rename2->f], mfcc_rename2->tmpmfcc, (sizeof(VECT)) * mfcc_rename2->param->veclen);
                            }

                          }

                          {
                            int return_value;
                            MFCCCalc *mfcc_rename10;
                            RealBeam *r_rename10;
                            int maxf_rename10;
                            PROCESS_AM *am_rename10;
                            int rewind_frame_rename10;
                            boolean reprocess_rename10;
                            boolean ok_p_rename10;
                            r_rename10 = &recog->real;
                            ok_p_rename10 = FALSE;
                            maxf_rename10 = 0;
                            for (mfcc_rename10 = recog->mfcclist; mfcc_rename10; mfcc_rename10 = mfcc_rename10->next)
                            {
                              loop_counter[43]++;
                              if (!mfcc_rename10->valid)
                              {
                                loop_counter[44]++;
                                continue;
                              }

                              if (maxf_rename10 < mfcc_rename10->f)
                              {
                                loop_counter[45]++;
                                maxf_rename10 = mfcc_rename10->f;
                              }

                              if (mfcc_rename10->f == 0)
                              {
                                loop_counter[46]++;
                                ok_p_rename10 = TRUE;
                              }

                            }

                            if (ok_p_rename10 && (maxf_rename10 == 0))
                            {
                              loop_counter[47]++;
                              if (recog->jconf->decodeopt.segment)
                              {
                                loop_counter[48]++;
                                if (!recog->process_segment)
                                {
                                  loop_counter[49]++;
                                  callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
                                }

                                callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
                                callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
                                recog->triggered = TRUE;
                              }
                              else
                              {
                                callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
                                callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
                                recog->triggered = TRUE;
                              }

                            }

                            switch (decode_proceed(recog))
                            {
                              case -1:
                                loop_counter[50]++;
                              {
                                return_value = -1;
                                goto return10;
                              }
                                break;

                              case 0:
                                loop_counter[51]++;
                                break;

                              case 1:
                                loop_counter[52]++;
                                r_rename10->last_is_segmented = TRUE;
                              {
                                return_value = 1;
                                goto return10;
                              }

                            }

                            if (spsegment_need_restart(recog, &rewind_frame_rename10, &reprocess_rename10) == TRUE)
                            {
                              loop_counter[53]++;
                              for (mfcc_rename10 = recog->mfcclist; mfcc_rename10; mfcc_rename10 = mfcc_rename10->next)
                              {
                                loop_counter[54]++;
                                if (!mfcc_rename10->valid)
                                {
                                  loop_counter[55]++;
                                  continue;
                                }

                                mfcc_rename10->param->header.samplenum = mfcc_rename10->f + 1;
                                mfcc_rename10->param->samplenum = mfcc_rename10->f + 1;
                              }

                              spsegment_restart_mfccs(recog, rewind_frame_rename10, reprocess_rename10);
                              recog->adin->rehash = TRUE;
                              for (am_rename10 = recog->amlist; am_rename10; am_rename10 = am_rename10->next)
                              {
                                loop_counter[56]++;
                                outprob_prepare(&am_rename10->hmmwrk, am_rename10->mfcc->param->samplenum);
                              }

                              if (reprocess_rename10)
                              {
                                loop_counter[57]++;
                                while (1)
                                {
                                  loop_counter[58]++;
                                  ok_p_rename10 = TRUE;
                                  for (mfcc_rename10 = recog->mfcclist; mfcc_rename10; mfcc_rename10 = mfcc_rename10->next)
                                  {
                                    loop_counter[59]++;
                                    if (!mfcc_rename10->valid)
                                    {
                                      loop_counter[60]++;
                                      continue;
                                    }

                                    mfcc_rename10->f++;
                                    if (mfcc_rename10->f < mfcc_rename10->param->samplenum)
                                    {
                                      loop_counter[61]++;
                                      mfcc_rename10->valid = TRUE;
                                      ok_p_rename10 = FALSE;
                                    }
                                    else
                                    {
                                      mfcc_rename10->valid = FALSE;
                                    }

                                  }

                                  if (ok_p_rename10)
                                  {
                                    loop_counter[62]++;
                                    for (mfcc_rename10 = recog->mfcclist; mfcc_rename10; mfcc_rename10 = mfcc_rename10->next)
                                    {
                                      loop_counter[63]++;
                                      if (!mfcc_rename10->valid)
                                      {
                                        loop_counter[64]++;
                                        continue;
                                      }

                                      mfcc_rename10->f--;
                                    }

                                    break;
                                  }

                                  switch (decode_proceed(recog))
                                  {
                                    case -1:
                                      loop_counter[65]++;
                                    {
                                      return_value = -1;
                                      goto return10;
                                    }
                                      break;

                                    case 0:
                                      loop_counter[66]++;
                                      break;

                                    case 1:
                                      loop_counter[67]++;
                                      break;

                                  }

                                  callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
                                }

                              }

                            }

                            for (mfcc_rename10 = recog->mfcclist; mfcc_rename10; mfcc_rename10 = mfcc_rename10->next)
                            {
                              loop_counter[68]++;
                              if (mfcc_rename10->valid)
                              {
                                loop_counter[69]++;
                                callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
                                break;
                              }

                            }

                            {
                              return_value = 0;
                              goto return10;
                            }
                            return10:
                            ;

                            ret_rename2 = return_value;
                          }
                          if ((ret_rename2 == 1) && recog->jconf->decodeopt.segment)
                          {
                            loop_counter[70]++;
                            r_rename2->rest_len = nowlen_rename2 - now_rename2;
                            if (r_rename2->rest_len > 0)
                            {
                              loop_counter[71]++;
                              if (r_rename2->rest_Speech == NULL)
                              {
                                loop_counter[72]++;
                                r_rename2->rest_alloc_len = r_rename2->rest_len;
                                r_rename2->rest_Speech = (SP16 *) mymalloc((sizeof(SP16)) * r_rename2->rest_alloc_len);
                              }
                              else
                                if (r_rename2->rest_alloc_len < r_rename2->rest_len)
                              {
                                loop_counter[73]++;
                                r_rename2->rest_alloc_len = r_rename2->rest_len;
                                r_rename2->rest_Speech = (SP16 *) myrealloc(r_rename2->rest_Speech, (sizeof(SP16)) * r_rename2->rest_alloc_len);
                              }


                              memcpy(r_rename2->rest_Speech, &a_rename1->cbuf[now_rename2], (sizeof(SP16)) * r_rename2->rest_len);
                            }

                          }

                          if (ret_rename2 != 0)
                          {
                            loop_counter[74]++;
                            return_value = ret_rename2;
                            goto return2;
                          }

                          for (mfcc_rename2 = recog->mfcclist; mfcc_rename2; mfcc_rename2 = mfcc_rename2->next)
                          {
                            loop_counter[75]++;
                            if (!mfcc_rename2->valid)
                            {
                              loop_counter[76]++;
                              continue;
                            }

                            mfcc_rename2->f++;
                          }

                          memmove(r_rename2->window, &r_rename2->window[recog->jconf->input.frameshift], (sizeof(SP16)) * (r_rename2->windowlen - recog->jconf->input.frameshift));
                          r_rename2->windownum -= recog->jconf->input.frameshift;
                        }

                        {
                          return_value = 0;
                          goto return2;
                        }
                        return2:
                        ;

                        ad_process_ret_rename1 = return_value;
                      }
                      switch (ad_process_ret_rename1)
                      {
                        case 1:
                          loop_counter[77]++;
                          end_status_rename1 = 2;
                        {
                          int from_rename3 = i_rename1;
                          if ((from_rename3 > 0) && ((a_rename1->current_len - from_rename3) > 0))
                          {
                            loop_counter[78]++;
                            memmove(a_rename1->buffer, &a_rename1->buffer[from_rename3], (a_rename1->current_len - from_rename3) * (sizeof(SP16)));
                          }

                          a_rename1->bp = a_rename1->current_len - from_rename3;
                          return3:
                          ;

                        }
                          goto break_input;

                        case -1:
                          loop_counter[79]++;
                          end_status_rename1 = -1;
                          goto break_input;

                      }

                    }

                  }

                }
                else
                {
                  if (a_rename1->nc > 0)
                  {
                    loop_counter[80]++;
                    a_rename1->nc = 0;
                    if (a_rename1->sblen > 0)
                    {
                      loop_counter[81]++;
                      a_rename1->last_trigger_len += a_rename1->sblen;
                    }

                    if (RealTimePipeLine != NULL)
                    {
                      loop_counter[82]++;
                      if (a_rename1->sblen > 0)
                      {
                        loop_counter[83]++;
                        callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a_rename1->swapbuf, a_rename1->sblen);
                        {
                          int return_value;
                          int nowlen_rename4 = a_rename1->sblen;
                          int i_rename4;
                          int now_rename4;
                          int ret_rename4;
                          MFCCCalc *mfcc_rename4;
                          RealBeam *r_rename4;
                          r_rename4 = &recog->real;
                          now_rename4 = 0;
                          r_rename4->last_is_segmented = FALSE;
                          while (now_rename4 < nowlen_rename4)
                          {
                            loop_counter[84]++;
                            for (mfcc_rename4 = recog->mfcclist; mfcc_rename4; mfcc_rename4 = mfcc_rename4->next)
                            {
                              loop_counter[85]++;
                              if (mfcc_rename4->f >= r_rename4->maxframelen)
                              {
                                loop_counter[86]++;
                                jlog("Warning: too long input (> %d frames), segment it now\n", r_rename4->maxframelen);
                                {
                                  return_value = 1;
                                  goto return4;
                                }
                              }

                            }

                            for (i_rename4 = min(r_rename4->windowlen - r_rename4->windownum, nowlen_rename4 - now_rename4); i_rename4 > 0; i_rename4--)
                            {
                              loop_counter[87]++;
                              r_rename4->window[r_rename4->windownum++] = (float) a_rename1->swapbuf[now_rename4++];
                            }

                            if (r_rename4->windownum < r_rename4->windowlen)
                            {
                              loop_counter[88]++;
                              break;
                            }

                            for (mfcc_rename4 = recog->mfcclist; mfcc_rename4; mfcc_rename4 = mfcc_rename4->next)
                            {
                              loop_counter[89]++;
                              mfcc_rename4->valid = FALSE;
                              if ((*recog->calc_vector)(mfcc_rename4, r_rename4->window, r_rename4->windowlen))
                              {
                                loop_counter[90]++;
                                mfcc_rename4->valid = TRUE;
                                if (param_alloc(mfcc_rename4->param, mfcc_rename4->f + 1, mfcc_rename4->param->veclen) == FALSE)
                                {
                                  loop_counter[91]++;
                                  jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
                                  {
                                    return_value = -1;
                                    goto return4;
                                  }
                                }

                                memcpy(mfcc_rename4->param->parvec[mfcc_rename4->f], mfcc_rename4->tmpmfcc, (sizeof(VECT)) * mfcc_rename4->param->veclen);
                              }

                            }

                            {
                              int return_value;
                              MFCCCalc *mfcc_rename11;
                              RealBeam *r_rename11;
                              int maxf_rename11;
                              PROCESS_AM *am_rename11;
                              int rewind_frame_rename11;
                              boolean reprocess_rename11;
                              boolean ok_p_rename11;
                              r_rename11 = &recog->real;
                              ok_p_rename11 = FALSE;
                              maxf_rename11 = 0;
                              for (mfcc_rename11 = recog->mfcclist; mfcc_rename11; mfcc_rename11 = mfcc_rename11->next)
                              {
                                loop_counter[92]++;
                                if (!mfcc_rename11->valid)
                                {
                                  loop_counter[93]++;
                                  continue;
                                }

                                if (maxf_rename11 < mfcc_rename11->f)
                                {
                                  loop_counter[94]++;
                                  maxf_rename11 = mfcc_rename11->f;
                                }

                                if (mfcc_rename11->f == 0)
                                {
                                  loop_counter[95]++;
                                  ok_p_rename11 = TRUE;
                                }

                              }

                              if (ok_p_rename11 && (maxf_rename11 == 0))
                              {
                                loop_counter[96]++;
                                if (recog->jconf->decodeopt.segment)
                                {
                                  loop_counter[97]++;
                                  if (!recog->process_segment)
                                  {
                                    loop_counter[98]++;
                                    callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
                                  }

                                  callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
                                  callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
                                  recog->triggered = TRUE;
                                }
                                else
                                {
                                  callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
                                  callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
                                  recog->triggered = TRUE;
                                }

                              }

                              switch (decode_proceed(recog))
                              {
                                case -1:
                                  loop_counter[99]++;
                                {
                                  return_value = -1;
                                  goto return11;
                                }
                                  break;

                                case 0:
                                  loop_counter[100]++;
                                  break;

                                case 1:
                                  loop_counter[101]++;
                                  r_rename11->last_is_segmented = TRUE;
                                {
                                  return_value = 1;
                                  goto return11;
                                }

                              }

                              if (spsegment_need_restart(recog, &rewind_frame_rename11, &reprocess_rename11) == TRUE)
                              {
                                loop_counter[102]++;
                                for (mfcc_rename11 = recog->mfcclist; mfcc_rename11; mfcc_rename11 = mfcc_rename11->next)
                                {
                                  loop_counter[103]++;
                                  if (!mfcc_rename11->valid)
                                  {
                                    loop_counter[104]++;
                                    continue;
                                  }

                                  mfcc_rename11->param->header.samplenum = mfcc_rename11->f + 1;
                                  mfcc_rename11->param->samplenum = mfcc_rename11->f + 1;
                                }

                                spsegment_restart_mfccs(recog, rewind_frame_rename11, reprocess_rename11);
                                recog->adin->rehash = TRUE;
                                for (am_rename11 = recog->amlist; am_rename11; am_rename11 = am_rename11->next)
                                {
                                  loop_counter[105]++;
                                  outprob_prepare(&am_rename11->hmmwrk, am_rename11->mfcc->param->samplenum);
                                }

                                if (reprocess_rename11)
                                {
                                  loop_counter[106]++;
                                  while (1)
                                  {
                                    loop_counter[107]++;
                                    ok_p_rename11 = TRUE;
                                    for (mfcc_rename11 = recog->mfcclist; mfcc_rename11; mfcc_rename11 = mfcc_rename11->next)
                                    {
                                      loop_counter[108]++;
                                      if (!mfcc_rename11->valid)
                                      {
                                        loop_counter[109]++;
                                        continue;
                                      }

                                      mfcc_rename11->f++;
                                      if (mfcc_rename11->f < mfcc_rename11->param->samplenum)
                                      {
                                        loop_counter[110]++;
                                        mfcc_rename11->valid = TRUE;
                                        ok_p_rename11 = FALSE;
                                      }
                                      else
                                      {
                                        mfcc_rename11->valid = FALSE;
                                      }

                                    }

                                    if (ok_p_rename11)
                                    {
                                      loop_counter[111]++;
                                      for (mfcc_rename11 = recog->mfcclist; mfcc_rename11; mfcc_rename11 = mfcc_rename11->next)
                                      {
                                        loop_counter[112]++;
                                        if (!mfcc_rename11->valid)
                                        {
                                          loop_counter[113]++;
                                          continue;
                                        }

                                        mfcc_rename11->f--;
                                      }

                                      break;
                                    }

                                    switch (decode_proceed(recog))
                                    {
                                      case -1:
                                        loop_counter[114]++;
                                      {
                                        return_value = -1;
                                        goto return11;
                                      }
                                        break;

                                      case 0:
                                        loop_counter[115]++;
                                        break;

                                      case 1:
                                        loop_counter[116]++;
                                        break;

                                    }

                                    callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
                                  }

                                }

                              }

                              for (mfcc_rename11 = recog->mfcclist; mfcc_rename11; mfcc_rename11 = mfcc_rename11->next)
                              {
                                loop_counter[117]++;
                                if (mfcc_rename11->valid)
                                {
                                  loop_counter[118]++;
                                  callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
                                  break;
                                }

                              }

                              {
                                return_value = 0;
                                goto return11;
                              }
                              return11:
                              ;

                              ret_rename4 = return_value;
                            }
                            if ((ret_rename4 == 1) && recog->jconf->decodeopt.segment)
                            {
                              loop_counter[119]++;
                              r_rename4->rest_len = nowlen_rename4 - now_rename4;
                              if (r_rename4->rest_len > 0)
                              {
                                loop_counter[120]++;
                                if (r_rename4->rest_Speech == NULL)
                                {
                                  loop_counter[121]++;
                                  r_rename4->rest_alloc_len = r_rename4->rest_len;
                                  r_rename4->rest_Speech = (SP16 *) mymalloc((sizeof(SP16)) * r_rename4->rest_alloc_len);
                                }
                                else
                                  if (r_rename4->rest_alloc_len < r_rename4->rest_len)
                                {
                                  loop_counter[122]++;
                                  r_rename4->rest_alloc_len = r_rename4->rest_len;
                                  r_rename4->rest_Speech = (SP16 *) myrealloc(r_rename4->rest_Speech, (sizeof(SP16)) * r_rename4->rest_alloc_len);
                                }


                                memcpy(r_rename4->rest_Speech, &a_rename1->swapbuf[now_rename4], (sizeof(SP16)) * r_rename4->rest_len);
                              }

                            }

                            if (ret_rename4 != 0)
                            {
                              loop_counter[123]++;
                              return_value = ret_rename4;
                              goto return4;
                            }

                            for (mfcc_rename4 = recog->mfcclist; mfcc_rename4; mfcc_rename4 = mfcc_rename4->next)
                            {
                              loop_counter[124]++;
                              if (!mfcc_rename4->valid)
                              {
                                loop_counter[125]++;
                                continue;
                              }

                              mfcc_rename4->f++;
                            }

                            memmove(r_rename4->window, &r_rename4->window[recog->jconf->input.frameshift], (sizeof(SP16)) * (r_rename4->windowlen - recog->jconf->input.frameshift));
                            r_rename4->windownum -= recog->jconf->input.frameshift;
                          }

                          {
                            return_value = 0;
                            goto return4;
                          }
                          return4:
                          ;

                          ad_process_ret_rename1 = return_value;
                        }
                        a_rename1->sblen = 0;
                        switch (ad_process_ret_rename1)
                        {
                          case 1:
                            loop_counter[126]++;
                            end_status_rename1 = 2;
                          {
                            int from_rename5 = i_rename1;
                            if ((from_rename5 > 0) && ((a_rename1->current_len - from_rename5) > 0))
                            {
                              loop_counter[127]++;
                              memmove(a_rename1->buffer, &a_rename1->buffer[from_rename5], (a_rename1->current_len - from_rename5) * (sizeof(SP16)));
                            }

                            a_rename1->bp = a_rename1->current_len - from_rename5;
                            return5:
                            ;

                          }
                            goto break_input;

                          case -1:
                            loop_counter[128]++;
                            end_status_rename1 = -1;
                            goto break_input;

                        }

                      }

                    }

                  }

                }

              }
              else
                if (a_rename1->is_valid_data == TRUE)
              {
                loop_counter[129]++;
                if (a_rename1->nc == 0)
                {
                  loop_counter[130]++;
                  a_rename1->rest_tail = a_rename1->sbsize - a_rename1->c_length;
                  a_rename1->sblen = 0;
                }

                a_rename1->nc++;
              }


            }

            if (((a_rename1->adin_cut_on && a_rename1->is_valid_data) && (a_rename1->nc > 0)) && (a_rename1->rest_tail == 0))
            {
              loop_counter[131]++;
              if ((a_rename1->sblen + wstep_rename1) > a_rename1->sbsize)
              {
                loop_counter[132]++;
                jlog("ERROR: adin_cut: swap buffer for re-triggering overflow\n");
              }

              memcpy(&a_rename1->swapbuf[a_rename1->sblen], &a_rename1->buffer[i_rename1], wstep_rename1 * (sizeof(SP16)));
              a_rename1->sblen += wstep_rename1;
            }
            else
            {
              if ((!a_rename1->adin_cut_on) || (a_rename1->is_valid_data == TRUE))
              {
                loop_counter[133]++;
                if (a_rename1->nc > 0)
                {
                  loop_counter[134]++;
                  if (a_rename1->rest_tail < wstep_rename1)
                  {
                    loop_counter[135]++;
                    a_rename1->rest_tail = 0;
                  }
                  else
                    a_rename1->rest_tail -= wstep_rename1;

                }

                a_rename1->last_trigger_len += wstep_rename1;
                if (RealTimePipeLine != NULL)
                {
                  loop_counter[136]++;
                  callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, &a_rename1->buffer[i_rename1], wstep_rename1);
                  {
                    int return_value;
                    int nowlen_rename6 = wstep_rename1;
                    int i_rename6;
                    int now_rename6;
                    int ret_rename6;
                    MFCCCalc *mfcc_rename6;
                    RealBeam *r_rename6;
                    r_rename6 = &recog->real;
                    now_rename6 = 0;
                    r_rename6->last_is_segmented = FALSE;
                    while (now_rename6 < nowlen_rename6)
                    {
                      loop_counter[137]++;
                      for (mfcc_rename6 = recog->mfcclist; mfcc_rename6; mfcc_rename6 = mfcc_rename6->next)
                      {
                        loop_counter[138]++;
                        if (mfcc_rename6->f >= r_rename6->maxframelen)
                        {
                          loop_counter[139]++;
                          jlog("Warning: too long input (> %d frames), segment it now\n", r_rename6->maxframelen);
                          {
                            return_value = 1;
                            goto return6;
                          }
                        }

                      }

                      for (i_rename6 = min(r_rename6->windowlen - r_rename6->windownum, nowlen_rename6 - now_rename6); i_rename6 > 0; i_rename6--)
                      {
                        loop_counter[140]++;
                        r_rename6->window[r_rename6->windownum++] = (float) a_rename1->buffer[now_rename6++];
                      }

                      if (r_rename6->windownum < r_rename6->windowlen)
                      {
                        loop_counter[141]++;
                        break;
                      }

                      for (mfcc_rename6 = recog->mfcclist; mfcc_rename6; mfcc_rename6 = mfcc_rename6->next)
                      {
                        loop_counter[142]++;
                        mfcc_rename6->valid = FALSE;
                        if ((*recog->calc_vector)(mfcc_rename6, r_rename6->window, r_rename6->windowlen))
                        {
                          loop_counter[143]++;
                          mfcc_rename6->valid = TRUE;
                          if (param_alloc(mfcc_rename6->param, mfcc_rename6->f + 1, mfcc_rename6->param->veclen) == FALSE)
                          {
                            loop_counter[144]++;
                            jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
                            {
                              return_value = -1;
                              goto return6;
                            }
                          }

                          memcpy(mfcc_rename6->param->parvec[mfcc_rename6->f], mfcc_rename6->tmpmfcc, (sizeof(VECT)) * mfcc_rename6->param->veclen);
                        }

                      }

                      {
                        int return_value;
                        MFCCCalc *mfcc_rename12;
                        RealBeam *r_rename12;
                        int maxf_rename12;
                        PROCESS_AM *am_rename12;
                        int rewind_frame_rename12;
                        boolean reprocess_rename12;
                        boolean ok_p_rename12;
                        r_rename12 = &recog->real;
                        ok_p_rename12 = FALSE;
                        maxf_rename12 = 0;
                        for (mfcc_rename12 = recog->mfcclist; mfcc_rename12; mfcc_rename12 = mfcc_rename12->next)
                        {
                          loop_counter[145]++;
                          if (!mfcc_rename12->valid)
                          {
                            loop_counter[146]++;
                            continue;
                          }

                          if (maxf_rename12 < mfcc_rename12->f)
                          {
                            loop_counter[147]++;
                            maxf_rename12 = mfcc_rename12->f;
                          }

                          if (mfcc_rename12->f == 0)
                          {
                            loop_counter[148]++;
                            ok_p_rename12 = TRUE;
                          }

                        }

                        if (ok_p_rename12 && (maxf_rename12 == 0))
                        {
                          loop_counter[149]++;
                          if (recog->jconf->decodeopt.segment)
                          {
                            loop_counter[150]++;
                            if (!recog->process_segment)
                            {
                              loop_counter[151]++;
                              callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
                            }

                            callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
                            callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
                            recog->triggered = TRUE;
                          }
                          else
                          {
                            callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
                            callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
                            recog->triggered = TRUE;
                          }

                        }

                        switch (decode_proceed(recog))
                        {
                          case -1:
                            loop_counter[152]++;
                          {
                            return_value = -1;
                            goto return12;
                          }
                            break;

                          case 0:
                            loop_counter[153]++;
                            break;

                          case 1:
                            loop_counter[154]++;
                            r_rename12->last_is_segmented = TRUE;
                          {
                            return_value = 1;
                            goto return12;
                          }

                        }

                        if (spsegment_need_restart(recog, &rewind_frame_rename12, &reprocess_rename12) == TRUE)
                        {
                          loop_counter[155]++;
                          for (mfcc_rename12 = recog->mfcclist; mfcc_rename12; mfcc_rename12 = mfcc_rename12->next)
                          {
                            loop_counter[156]++;
                            if (!mfcc_rename12->valid)
                            {
                              loop_counter[157]++;
                              continue;
                            }

                            mfcc_rename12->param->header.samplenum = mfcc_rename12->f + 1;
                            mfcc_rename12->param->samplenum = mfcc_rename12->f + 1;
                          }

                          spsegment_restart_mfccs(recog, rewind_frame_rename12, reprocess_rename12);
                          recog->adin->rehash = TRUE;
                          for (am_rename12 = recog->amlist; am_rename12; am_rename12 = am_rename12->next)
                          {
                            loop_counter[158]++;
                            outprob_prepare(&am_rename12->hmmwrk, am_rename12->mfcc->param->samplenum);
                          }

                          if (reprocess_rename12)
                          {
                            loop_counter[159]++;
                            while (1)
                            {
                              loop_counter[160]++;
                              ok_p_rename12 = TRUE;
                              for (mfcc_rename12 = recog->mfcclist; mfcc_rename12; mfcc_rename12 = mfcc_rename12->next)
                              {
                                loop_counter[161]++;
                                if (!mfcc_rename12->valid)
                                {
                                  loop_counter[162]++;
                                  continue;
                                }

                                mfcc_rename12->f++;
                                if (mfcc_rename12->f < mfcc_rename12->param->samplenum)
                                {
                                  loop_counter[163]++;
                                  mfcc_rename12->valid = TRUE;
                                  ok_p_rename12 = FALSE;
                                }
                                else
                                {
                                  mfcc_rename12->valid = FALSE;
                                }

                              }

                              if (ok_p_rename12)
                              {
                                loop_counter[164]++;
                                for (mfcc_rename12 = recog->mfcclist; mfcc_rename12; mfcc_rename12 = mfcc_rename12->next)
                                {
                                  loop_counter[165]++;
                                  if (!mfcc_rename12->valid)
                                  {
                                    loop_counter[166]++;
                                    continue;
                                  }

                                  mfcc_rename12->f--;
                                }

                                break;
                              }

                              switch (decode_proceed(recog))
                              {
                                case -1:
                                  loop_counter[167]++;
                                {
                                  return_value = -1;
                                  goto return12;
                                }
                                  break;

                                case 0:
                                  loop_counter[168]++;
                                  break;

                                case 1:
                                  loop_counter[169]++;
                                  break;

                              }

                              callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
                            }

                          }

                        }

                        for (mfcc_rename12 = recog->mfcclist; mfcc_rename12; mfcc_rename12 = mfcc_rename12->next)
                        {
                          loop_counter[170]++;
                          if (mfcc_rename12->valid)
                          {
                            loop_counter[171]++;
                            callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
                            break;
                          }

                        }

                        {
                          return_value = 0;
                          goto return12;
                        }
                        return12:
                        ;

                        ret_rename6 = return_value;
                      }
                      if ((ret_rename6 == 1) && recog->jconf->decodeopt.segment)
                      {
                        loop_counter[172]++;
                        r_rename6->rest_len = nowlen_rename6 - now_rename6;
                        if (r_rename6->rest_len > 0)
                        {
                          loop_counter[173]++;
                          if (r_rename6->rest_Speech == NULL)
                          {
                            loop_counter[174]++;
                            r_rename6->rest_alloc_len = r_rename6->rest_len;
                            r_rename6->rest_Speech = (SP16 *) mymalloc((sizeof(SP16)) * r_rename6->rest_alloc_len);
                          }
                          else
                            if (r_rename6->rest_alloc_len < r_rename6->rest_len)
                          {
                            loop_counter[175]++;
                            r_rename6->rest_alloc_len = r_rename6->rest_len;
                            r_rename6->rest_Speech = (SP16 *) myrealloc(r_rename6->rest_Speech, (sizeof(SP16)) * r_rename6->rest_alloc_len);
                          }


                          memcpy(r_rename6->rest_Speech, &a_rename1->buffer[now_rename6], (sizeof(SP16)) * r_rename6->rest_len);
                        }

                      }

                      if (ret_rename6 != 0)
                      {
                        loop_counter[176]++;
                        return_value = ret_rename6;
                        goto return6;
                      }

                      for (mfcc_rename6 = recog->mfcclist; mfcc_rename6; mfcc_rename6 = mfcc_rename6->next)
                      {
                        loop_counter[177]++;
                        if (!mfcc_rename6->valid)
                        {
                          loop_counter[178]++;
                          continue;
                        }

                        mfcc_rename6->f++;
                      }

                      memmove(r_rename6->window, &r_rename6->window[recog->jconf->input.frameshift], (sizeof(SP16)) * (r_rename6->windowlen - recog->jconf->input.frameshift));
                      r_rename6->windownum -= recog->jconf->input.frameshift;
                    }

                    {
                      return_value = 0;
                      goto return6;
                    }
                    return6:
                    ;

                    ad_process_ret_rename1 = return_value;
                  }
                  switch (ad_process_ret_rename1)
                  {
                    case 1:
                      loop_counter[179]++;
                    {
                      int from_rename7 = i_rename1 + wstep_rename1;
                      if ((from_rename7 > 0) && ((a_rename1->current_len - from_rename7) > 0))
                      {
                        loop_counter[180]++;
                        memmove(a_rename1->buffer, &a_rename1->buffer[from_rename7], (a_rename1->current_len - from_rename7) * (sizeof(SP16)));
                      }

                      a_rename1->bp = a_rename1->current_len - from_rename7;
                      return7:
                      ;

                    }
                      end_status_rename1 = 2;
                      goto break_input;

                    case -1:
                      loop_counter[181]++;
                      end_status_rename1 = -1;
                      goto break_input;

                  }

                }

              }

            }

            if ((a_rename1->adin_cut_on && a_rename1->is_valid_data) && (a_rename1->nc >= a_rename1->nc_max))
            {
              loop_counter[182]++;
              a_rename1->is_valid_data = FALSE;
              a_rename1->sblen = 0;
              callback_exec(CALLBACK_EVENT_SPEECH_STOP, recog);
              {
                int from_rename8 = i_rename1 + wstep_rename1;
                if ((from_rename8 > 0) && ((a_rename1->current_len - from_rename8) > 0))
                {
                  loop_counter[183]++;
                  memmove(a_rename1->buffer, &a_rename1->buffer[from_rename8], (a_rename1->current_len - from_rename8) * (sizeof(SP16)));
                }

                a_rename1->bp = a_rename1->current_len - from_rename8;
                return8:
                ;

              }
              end_status_rename1 = 1;
              goto break_input;
            }

            i_rename1 += wstep_rename1;
          }

          {
            int from_rename9 = i_rename1;
            if ((from_rename9 > 0) && ((a_rename1->current_len - from_rename9) > 0))
            {
              loop_counter[184]++;
              memmove(a_rename1->buffer, &a_rename1->buffer[from_rename9], (a_rename1->current_len - from_rename9) * (sizeof(SP16)));
            }

            a_rename1->bp = a_rename1->current_len - from_rename9;
            return9:
            ;

          }
        }

        break_input:
        if (a_rename1->end_of_stream)
        {
          loop_counter[185]++;
          callback_exec(CALLBACK_EVENT_SPEECH_STOP, recog);
          if (a_rename1->bp == 0)
          {
            loop_counter[186]++;
            a_rename1->need_init = TRUE;
          }

          if (end_status_rename1 >= 0)
          {
            loop_counter[187]++;
            end_status_rename1 = a_rename1->bp ? 1 : 0;
          }

        }


        {
          return_value = end_status_rename1;
          goto return1;
        }
        return1:
        ;

        return_value = return_value;
      }
      goto return0;
    }
    return0:
    ;

    ret = return_value;
  }
  {
    print_loop_counter:
    

    printf("loop counter = (");
    int i;
    for (i = 0; i < 188; i++)
      printf("%d, ", loop_counter[i]);

    printf(")\n");
  }
//---------------------modified by TJSong----------------------//
//float exec_time;
//exec_time = -42349.000000*loop_counter[0] + -325.460000*loop_counter[4] + 55.279200*loop_counter[140] + 115.722000*loop_counter[141] + -8792.340000*loop_counter[147] + 0.000000;
    float exec_time;
    exec_time = 20241.300000*loop_counter[0] + -4915.170000*loop_counter[4] + 27231.900000*loop_counter[27] + -73.834400*loop_counter[140] + 12716.000000*loop_counter[147] + 0.000000;
    printf("predicted time = %f\n", exec_time);

    end_timing();
    print_slice_timing();
#if PREDICT_EN
    start_timing();
    set_freq(exec_time); //TJSong
    end_timing();
    print_set_dvfs_timing();
#endif
//---------------------modified by TJSong----------------------//
 
  return ret;
}


/** 
 * <EN>
 * @brief  Execute recognition.
 *
 * This function repeats recognition sequences until the input stream
 * reached its end.  It detects speech segment (if needed), recognize
 * the detected segment, output result, and go back to the first.
 *
 * This function will be stopped and exited if reached end of stream
 * (mostly in case of file input), some error has been occured, or
 * termination requested from application by calling
 * j_request_pause() and j_request_terminate().
 * 
 * </EN>
 * <JA>
 * @brief  音声認識の実行. 
 *
 * この関数は入力ストリームが終わるまで音声認識を繰り返す. 
 * 必要であれば入力待ちを行って区間を検出し，音声認識を行い，結果を
 * 出力してふたたび入力待ちに戻る. 
 *
 * 入力ストリームを終わりまで認識するか，エラーが生じたときに終了する. 
 *
 * あるいは，認識処理中に，j_request_pause() や j_request_terminate() が
 * アプリから呼ばれた場合，認識処理の切れ目で終了する. 
 * 
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * 
 * @return 1 when stopped by application request, 0 when reached end of stream,
 * or -1 when an error occured.  Note that the input stream can still continues
 * when 1 is returned.
 * 
 */
static int
j_recognize_stream_core(Recog *recog)
{
  Jconf *jconf;
  int ret;
  float seclen, mseclen;
  RecogProcess *r;
  MFCCCalc *mfcc;
  PROCESS_AM *am;
  PROCESS_LM *lm;
  boolean ok_p;
  boolean process_segment_last;
  boolean on_the_fly;
  boolean pass2_p;

  jconf = recog->jconf;

  /* determine whether on-the-fly decoding should be done */
  on_the_fly = FALSE;
  switch(jconf->input.type) {
  case INPUT_VECTOR:
    switch(jconf->input.speech_input) {
    case SP_MFCFILE: 
    case SP_OUTPROBFILE:
      on_the_fly = FALSE;
      break;
    case SP_MFCMODULE:
      on_the_fly = TRUE;
      break;
    }
    break;
  case INPUT_WAVEFORM:
    if (jconf->decodeopt.realtime_flag) {
      on_the_fly = TRUE;
    } else {
      on_the_fly = FALSE;
    }
    break;
  }

  if (jconf->input.type == INPUT_WAVEFORM || jconf->input.speech_input == SP_MFCMODULE) {
    for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
      param_init_content(mfcc->param);
    }
  }

  /* if no process instance exist, start with terminated */
  if (recog->process_list == NULL) {
    jlog("STAT: no recog process, engine inactive\n");
    j_request_pause(recog);
  }

  /* update initial recognition process status */
  for(r=recog->process_list;r;r=r->next) {
    if (r->active > 0) {
      r->live = TRUE;
    } else if (r->active < 0) {
      r->live = FALSE;
    }
    r->active = 0;
  }

  /******************************************************************/
  /* do recognition for each incoming segment from the input stream */
  /******************************************************************/
  while (1) {
    
  start_recog:

    /*************************************/
    /* Update recognition process status */
    /*************************************/
    for(r=recog->process_list;r;r=r->next) {
      if (r->active > 0) {
	r->live = TRUE;
	jlog("STAT: SR%02d %s now active\n", r->config->id, r->config->name);
      } else if (r->active < 0) {
	r->live = FALSE;
	jlog("STAT: SR%02d %s now inactive\n", r->config->id, r->config->name);
      }
      r->active = 0;
    }
    if (debug2_flag) {
      for(r=recog->process_list;r;r=r->next) {
	jlog("DEBUG: %s: SR%02d %s\n", r->live ? "live" : "dead", r->config->id, r->config->name);
      }
    }
    /* check if any process is live */
    if (recog->process_active) {
      ok_p = FALSE;
      for(r=recog->process_list;r;r=r->next) {
	if (r->live) ok_p = TRUE;
      }
      if (!ok_p) {		/* no process is alive */
	/* make whole process as inactive */
	jlog("STAT: all recog process inactive, pause engine now\n");
	j_request_pause(recog);
      }
    }

    /* Check whether process status was changed while in the last run */
    if (recog->process_online != recog->process_active) {
      recog->process_online = recog->process_active;
      if (recog->process_online) callback_exec(CALLBACK_EVENT_PROCESS_ONLINE, recog);
      else callback_exec(CALLBACK_EVENT_PROCESS_OFFLINE, recog);
    }
    /* execute poll callback */
    if (recog->process_active) {
      callback_exec(CALLBACK_POLL, recog);
    }
    /* reset reload flag here */
    j_reset_reload(recog);

    if (!recog->process_active) {
      /* now sleeping, return */
      /* in the next call, we will resume from here */
      return 1;
    }
    /* update process status */
    if (recog->process_online != recog->process_active) {
      recog->process_online = recog->process_active;
      if (recog->process_online) callback_exec(CALLBACK_EVENT_PROCESS_ONLINE, recog);
      else callback_exec(CALLBACK_EVENT_PROCESS_OFFLINE, recog);
    }

    /*********************************************************/
    /* check for grammar to change, and rebuild if necessary */
    /*********************************************************/
    for(lm=recog->lmlist;lm;lm=lm->next) {
      if (lm->lmtype == LM_DFA) {
	multigram_update(lm); /* some modification occured if return TRUE*/
      }
    }
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      if (r->lmtype == LM_DFA && r->lm->global_modified) {
	multigram_build(r);
      }
    }
    for(lm=recog->lmlist;lm;lm=lm->next) {
      if (lm->lmtype == LM_DFA) lm->global_modified = FALSE;
    }

    ok_p = FALSE;
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      if (r->lmtype == LM_DFA) {
	if (r->lm->winfo == NULL ||
	    (r->lmvar == LM_DFA_GRAMMAR && r->lm->dfa == NULL)) {
	  /* make this instance inactive */
	  r->active = -1;
	  ok_p = TRUE;
	}
      }
    }
    if (ok_p) {			/* at least one instance has no grammar */
      goto start_recog;
    }


    /******************/
    /* start 1st pass */
    /******************/
    if (on_the_fly) {

      /********************************************/
      /* REALTIME ON-THE-FLY DECODING OF 1ST-PASS */
      /********************************************/
      /* store, analysis and search in a pipeline  */
      /* main function is RealTimePipeLine() at realtime-1stpass.c, and
	 it will be periodically called for each incoming input segment
	 from the AD-in function adin_go().  RealTimePipeLine() will be
	 called as a callback function from adin_go() */
      /* after this part, directly jump to the beginning of the 2nd pass */
      
      if (recog->process_segment) {
	/*****************************************************************/
	/* short-pause segmentation: process last remaining frames first */
	/*****************************************************************/
	/* last was segmented by short pause */
	/* the margin segment in the last input will be re-processed first,
	   and then the speech input will be processed */
	/* process the last remaining parameters */
	ret = RealTimeResume(recog);
	if (ret < 0) {		/* error end in the margin */
	  jlog("ERROR: failed to process last remaining samples on RealTimeResume\n"); /* exit now! */
	  return -1;
	}
	if (ret != 1) {	/* if segmented again in the margin, not process the rest */
	  /* last parameters has been processed, so continue with the
	     current input as normal */
	  /* process the incoming input */
	  if (jconf->input.type == INPUT_WAVEFORM) {
	    /* get speech and process it on real-time */
	    ret = adin_go(RealTimePipeLine, callback_check_in_adin, recog);
	  } else {
	    /* get feature vector and process it */
	    ret = mfcc_go(recog, callback_check_in_adin);
	  }
	  if (ret < 0) {		/* error end in adin_go */
	    if (ret == -2 || recog->process_want_terminate) {
	      /* terminated by callback */
	      RealTimeTerminate(recog);
	      /* reset param */
	      for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
		param_init_content(mfcc->param);
	      }
	      /* execute callback at end of pass1 */
	      if (recog->triggered) {
		callback_exec(CALLBACK_EVENT_PASS1_END, recog);
		/* output result terminate */
		result_error(recog, J_RESULT_STATUS_TERMINATE);
	      }
	      goto end_recog; /* cancel this recognition */
	    }
	    jlog("ERROR: an error occured at on-the-fly 1st pass decoding\n");          /* exit now! */
	    return(-1);
	  }
	}
	
      } else {

	/***********************************************************/
	/* last was not segmented, process the new incoming input  */
	/***********************************************************/
	/* end of this input will be determined by either end of stream
	   (in case of file input), or silence detection by adin_go(), or
	   'TERMINATE' command from module (if module mode) */
	/* prepare work area for on-the-fly processing */
	if (RealTimePipeLinePrepare(recog) == FALSE) {
	  jlog("ERROR: failed to prepare for on-the-fly 1st pass decoding\n");
	  return (-1);
	}
	/* process the incoming input */
	if (jconf->input.type == INPUT_WAVEFORM) {
	  /* get speech and process it on real-time */
	  //ret = adin_go(RealTimePipeLine, callback_check_in_adin, recog);

    fopen_all();//TJSong 
    printf("============ deadline time : %d us ===========\n", DEADLINE_TIME);//TJSong

      // Run adin_go with loop counts
      call_adin_go(recog);//slice
	} else {
	  /* get feature vector and process it */
	  ret = mfcc_go(recog, callback_check_in_adin);
	}

  start_timing();
	
	if (ret < 0) {		/* error end in adin_go */
	  if (ret == -2 || recog->process_want_terminate) {	
	    /* terminated by callback */
	    RealTimeTerminate(recog);
	    /* reset param */
	    for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
	      param_init_content(mfcc->param);
	    }
	    /* execute callback at end of pass1 */
	    if (recog->triggered) {
	      callback_exec(CALLBACK_EVENT_PASS1_END, recog);
	      /* output result terminate */
	      result_error(recog, J_RESULT_STATUS_TERMINATE);
	    }
	    goto end_recog;
	  }
	  jlog("ERROR: an error occured at on-the-fly 1st pass decoding\n");          /* exit now! */
	  return(-1);
	}
      }
      /******************************************************************/
      /* speech stream has been processed on-the-fly, and 1st pass ends */
      /******************************************************************/
      if (ret == 1 || ret == 2) {		/* segmented */
#ifdef HAVE_PTHREAD
	/* check for audio overflow */
	if (recog->adin->enable_thread && recog->adin->adinthread_buffer_overflowed) {
	  jlog("Warning: input buffer overflow: some input may be dropped, so disgard the input\n");
	  result_error(recog, J_RESULT_STATUS_BUFFER_OVERFLOW);
	  /* skip 2nd pass */
	  goto end_recog;
	}
#endif
      }
      /* last procedure of 1st-pass */
      if (RealTimeParam(recog) == FALSE) {
	jlog("ERROR: fatal error occured, program terminates now\n");
	return -1;
      }
      
#ifdef BACKEND_VAD
      /* if not triggered, skip this segment */
      if (recog->jconf->decodeopt.segment && ! recog->triggered) {
	goto end_recog;
      }
#endif

      /* output segment status */
      if (recog->adin->adin_cut_on && (jconf->input.speech_input == SP_RAWFILE || jconf->input.speech_input == SP_STDIN)) {
	seclen = (float)recog->adin->last_trigger_sample / (float)jconf->input.sfreq;
	jlog("STAT: triggered: [%d..%d] %.2fs from %02d:%02d:%02.2f\n", recog->adin->last_trigger_sample, recog->adin->last_trigger_sample + recog->adin->last_trigger_len, (float)(recog->adin->last_trigger_len) / (float)jconf->input.sfreq, (int)(seclen / 3600), (int)(seclen / 60), seclen - (int)(seclen / 60) * 60);
      }

      /* execute callback for 1st pass result */
      /* result.status <0 must be skipped inside callback */
      callback_exec(CALLBACK_RESULT_PASS1, recog);
#ifdef WORD_GRAPH
      /* result.wg1 == NULL should be skipped inside callback */
      callback_exec(CALLBACK_RESULT_PASS1_GRAPH, recog);
#endif
      /* execute callback at end of pass1 */
      callback_exec(CALLBACK_EVENT_PASS1_END, recog);
      /* output frame length */
      callback_exec(CALLBACK_STATUS_PARAM, recog);
      /* if terminate signal has been received, discard this input */
      if (recog->process_want_terminate) {
	result_error(recog, J_RESULT_STATUS_TERMINATE);
	goto end_recog;
      }

      /* END OF ON-THE-FLY INPUT AND DECODING OF 1ST PASS */

    } else {

      /******************/
      /* buffered input */
      /******************/

      if (jconf->input.type == INPUT_VECTOR) {
	/***********************/
	/* vector input */
	/************************/
	if (jconf->input.speech_input == SP_OUTPROBFILE) {
	  /**********************************/
	  /* state output probability input */
	  /**********************************/
	  /* AM is dummy, so skip parameter type check */
	  ret = 0;
	} else if (jconf->input.speech_input == SP_MFCFILE) {
	  /************************/
	  /* parameter file input */
	  /************************/
	  /* parameter type check --- compare the type to that of HMM,
	     and adjust them if necessary */
	  if (jconf->input.paramtype_check_flag) {
	    for(am=recog->amlist;am;am=am->next) {
	      /* return param itself or new malloced param */
	      if (param_check_and_adjust(am->hmminfo, am->mfcc->param, verbose_flag) == -1) {	/* failed */
		
		for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
		  param_init_content(mfcc->param);
		}
		/* tell failure */
		result_error(recog, J_RESULT_STATUS_FAIL);
		goto end_recog;
	      }
	    }
	  }
	  /* whole input is already read, so set input status to end of stream */
	  /* and jump to the start point of 1st pass */
	  ret = 0;
	}
      } else {
	/*************************/
	/* buffered speech input */
	/*************************/
	if (!recog->process_segment) { /* no segment left */

	  /****************************************/
	  /* store raw speech samples to speech[] */
	  /****************************************/
	  recog->speechlen = 0;
	  for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
	    param_init_content(mfcc->param);
	  }
	  /* tell module to start recording */
	  /* the "adin_cut_callback_store_buffer" simply stores
	     the input speech to a buffer "speech[]" */
	  /* end of this input will be determined by either end of stream
	     (in case of file input), or silence detection by adin_go(), or
	     'TERMINATE' command from module (if module mode) */
	  ret = adin_go(adin_cut_callback_store_buffer, callback_check_in_adin, recog);
	  if (ret < 0) {		/* error end in adin_go */
	    if (ret == -2 || recog->process_want_terminate) {
	      /* terminated by module */
	      /* output fail */
	      result_error(recog, J_RESULT_STATUS_TERMINATE);
	      goto end_recog;
	    }
	    jlog("ERROR: an error occured while recording input\n");
	    return -1;
	  }
	  
	  /* output recorded length */
	  seclen = (float)recog->speechlen / (float)jconf->input.sfreq;
	  jlog("STAT: %d samples (%.2f sec.)\n", recog->speechlen, seclen);
	  
	  /* -rejectshort 指定時, 入力が指定時間以下であれば
	     ここで入力を棄却する */
	  /* when using "-rejectshort", and input was shorter than
	     specified, reject the input here */
	  if (jconf->reject.rejectshortlen > 0) {
	    if (seclen * 1000.0 < jconf->reject.rejectshortlen) {
	      result_error(recog, J_RESULT_STATUS_REJECT_SHORT);
	      goto end_recog;
	    }
	  }
	  /* when using "-rejectlong", and input was longer than specified,
	     terminate the input here */
	  if (recog->jconf->reject.rejectlonglen >= 0) {
	    if (seclen * 1000.0 >= recog->jconf->reject.rejectlonglen) {
	      result_error(recog, J_RESULT_STATUS_REJECT_LONG);
	      goto end_recog;
	    }
	  }
	
	  /**********************************************/
	  /* acoustic analysis and encoding of speech[] */
	  /**********************************************/
	  jlog("STAT: ### speech analysis (waveform -> MFCC)\n");
	  /* CMN will be computed for the whole buffered input */
	  if (wav2mfcc(recog->speech, recog->speechlen, recog) == FALSE) {
	    /* error end, end stream */
	    ret = -1;
	    /* tell failure */
	    result_error(recog, J_RESULT_STATUS_FAIL);
	    goto end_recog;
	  }
	  /* if terminate signal has been received, cancel this input */
	  if (recog->process_want_terminate) {
	    result_error(recog, J_RESULT_STATUS_TERMINATE);
	    goto end_recog;
	  }
	  
	  /* output frame length */
	  callback_exec(CALLBACK_STATUS_PARAM, recog);
	}
      }

#ifdef ENABLE_PLUGIN
      /* call post-process plugin if exist */
      plugin_exec_vector_postprocess_all(recog->mfcclist->param);
#endif

      /******************************************************/
      /* 1st-pass --- backward search to compute heuristics */
      /******************************************************/
      if (!jconf->decodeopt.realtime_flag) {
	/* prepare for outprob cache for each HMM state and time frame */
	/* assume all MFCCCalc has params of the same sample num */
	for(am=recog->amlist;am;am=am->next) {
	  outprob_prepare(&(am->hmmwrk), am->mfcc->param->samplenum);
	}
      }
      
      /* if terminate signal has been received, cancel this input */
      if (recog->process_want_terminate) {
	result_error(recog, J_RESULT_STATUS_TERMINATE);
	goto end_recog;
      }
    
      /* execute computation of left-to-right backtrellis */
      if (get_back_trellis(recog) == FALSE) {
	jlog("ERROR: fatal error occured, program terminates now\n");
	return -1;
      }
#ifdef BACKEND_VAD
      /* if not triggered, skip this segment */
      if (recog->jconf->decodeopt.segment && ! recog->triggered) {
	goto end_recog;
      }
#endif
      
      /* execute callback for 1st pass result */
      /* result.status <0 must be skipped inside callback */
      callback_exec(CALLBACK_RESULT_PASS1, recog);
#ifdef WORD_GRAPH
      /* result.wg1 == NULL should be skipped inside callback */
      callback_exec(CALLBACK_RESULT_PASS1_GRAPH, recog);
#endif
      
      /* execute callback at end of pass1 */
      if (recog->triggered) {
	callback_exec(CALLBACK_EVENT_PASS1_END, recog);
      }

      /* END OF BUFFERED 1ST PASS */

    }

    /**********************************/
    /* end processing of the 1st-pass */
    /**********************************/
    /* on-the-fly 1st pass processing will join here */
    
    /* -rejectshort 指定時, 入力が指定時間以下であれば探索失敗として */
    /* 第２パスを実行せずにここで終了する */
    /* when using "-rejectshort", and input was shorter than the specified
       length, terminate search here and output recognition failure */
    if (jconf->reject.rejectshortlen > 0) {
      mseclen = (float)recog->mfcclist->param->samplenum * (float)jconf->input.period * (float)jconf->input.frameshift / 10000.0;
      if (mseclen < jconf->reject.rejectshortlen) {
	result_error(recog, J_RESULT_STATUS_REJECT_SHORT);
	goto end_recog;
      }
    }
    if (jconf->reject.rejectlonglen >= 0) {
      mseclen = (float)recog->mfcclist->param->samplenum * (float)jconf->input.period * (float)jconf->input.frameshift / 10000.0;
      if (mseclen >= jconf->reject.rejectlonglen) {
	result_error(recog, J_RESULT_STATUS_REJECT_LONG);
	goto end_recog;
      }
    }
#ifdef POWER_REJECT
    if (power_reject(recog)) {
      result_error(recog, J_RESULT_STATUS_REJECT_POWER);
      goto end_recog;
    }
#endif

    if (jconf->outprob_outfile) {
      FILE *fp;
      char *buf;
      /* store the whole state outprob cache as a state outprob vector file */
      if ((fp = fopen(jconf->outprob_outfile, "wb")) != NULL) {
	for(r=recog->process_list;r;r=r->next) {
	  if (!r->live) continue;
	  if (outprob_cache_output(fp, r->wchmm->hmmwrk, recog->mfcclist->param->samplenum) == FALSE) {
	    jlog("ERROR: error in writing state probabilities to %s\n", jconf->outprob_outfile);
	    fclose(fp);
	    goto end_recog;
	  }
	}
	fclose(fp);
	jlog("STAT: state probabilities written to %s\n", jconf->outprob_outfile);
      } else{
	jlog("ERROR: failed to write state probabilities to %s\n", jconf->outprob_outfile);
      }
    }
    
    /* if terminate signal has been received, cancel this input */
    if (recog->process_want_terminate) {
      result_error(recog, J_RESULT_STATUS_TERMINATE);
      goto end_recog;
    }
    
    /* if GMM is specified and result are to be rejected, terminate search here */
    if (jconf->reject.gmm_reject_cmn_string != NULL) {
      if (! gmm_valid_input(recog)) {
	result_error(recog, J_RESULT_STATUS_REJECT_GMM);
	goto end_recog;
      }
    }

    /* for instances with "-1pass", copy 1st pass result as final */
    /* execute stack-decoding search */
    /* they will be skipepd in the next pass */
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      /* skip if 1st pass was failed */
      if (r->result.status < 0) continue;
      /* already stored on word recognition, so skip this */
      if (r->lmvar == LM_DFA_WORD) continue;
      if (r->config->compute_only_1pass) {
	if (verbose_flag) {
	  jlog("%02d %s: \"-1pass\" specified, output 1st pass result as a final result\n", r->config->id, r->config->name);
	}
	/* prepare result storage */
	result_sentence_malloc(r, 1);
	/* finalize result when no hypothesis was obtained */
	pass2_finalize_on_no_result(r, TRUE);
      }
    }

    /***********************************************/
    /* 2nd-pass --- forward search with heuristics */
    /***********************************************/
    pass2_p = FALSE;
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      /* if [-1pass] is specified, skip 2nd pass */
      if (r->config->compute_only_1pass) continue;
      /* if search already failed on 1st pass, skip 2nd pass */
      if (r->result.status < 0) continue;
      pass2_p = TRUE;
    }
    if (pass2_p) callback_exec(CALLBACK_EVENT_PASS2_BEGIN, recog);

#if !defined(PASS2_STRICT_IWCD) || defined(FIX_35_PASS2_STRICT_SCORE)    
    /* adjust trellis score not to contain outprob of the last frames */
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      /* if [-1pass] is specified, skip 2nd pass */
      if (r->config->compute_only_1pass) continue;
      /* if search already failed on 1st pass, skip 2nd pass */
      if (r->result.status < 0) continue;
      if (! r->am->hmminfo->multipath) {
	bt_discount_pescore(r->wchmm, r->backtrellis, r->am->mfcc->param);
      }
#ifdef LM_FIX_DOUBLE_SCORING
      if (r->lmtype == LM_PROB) {
	bt_discount_lm(r->backtrellis);
      }
#endif
    }
#endif
    
    /* execute stack-decoding search */
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      /* if [-1pass] is specified, just copy from 1st pass result */
      if (r->config->compute_only_1pass) continue;
      /* if search already failed on 1st pass, skip 2nd pass */
      if (r->result.status < 0) continue;
      /* prepare result storage */
      if (r->lmtype == LM_DFA && r->config->output.multigramout_flag) {
	result_sentence_malloc(r, r->config->output.output_hypo_maxnum * multigram_get_all_num(r->lm));
      } else {
	result_sentence_malloc(r, r->config->output.output_hypo_maxnum);
      }
      /* do 2nd pass */
      if (r->lmtype == LM_PROB) {
	wchmm_fbs(r->am->mfcc->param, r, 0, 0);
      } else if (r->lmtype == LM_DFA) {
	if (r->config->output.multigramout_flag) {
	  /* execute 2nd pass multiple times for each grammar sequencially */
	  /* to output result for each grammar */
	  MULTIGRAM *m;
	  boolean has_success = FALSE;
	  for(m = r->lm->grammars; m; m = m->next) {
	    if (m->active) {
	      jlog("STAT: execute 2nd pass limiting words for gram #%d\n", m->id);
	      wchmm_fbs(r->am->mfcc->param, r, m->cate_begin, m->dfa->term_num);
	      if (r->result.status == J_RESULT_STATUS_SUCCESS) {
		has_success = TRUE;
	      }
	    }
	  }
	  r->result.status = (has_success == TRUE) ? J_RESULT_STATUS_SUCCESS : J_RESULT_STATUS_FAIL;
	} else {
	  /* only the best among all grammar will be output */
	  wchmm_fbs(r->am->mfcc->param, r, 0, r->lm->dfa->term_num);
	}
      }
    }

    /* do forced alignment if needed */
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      /* if search failed on 2nd pass, skip this */
      if (r->result.status < 0) continue;
      /* do needed alignment */
      do_alignment_all(r, r->am->mfcc->param);
    }

    /* output result */
    callback_exec(CALLBACK_RESULT, recog);

    end_timing();
//---------------------modified by TJSong----------------------//
#if DELAY_EN
    int delay_time;
    static int instance_number = 0;
    if( (delay_time = DEADLINE_TIME - exec_timing()) > 0 ){
        start_timing();  
        usleep(delay_time);
        end_timing();
        printf("delayed by %d us\n", exec_timing());
        printf("time %d = %d us\n", instance_number, DEADLINE_TIME - delay_time + exec_timing());
    }else
        printf("time %d = %d us\n", instance_number, exec_timing());
    instance_number++;
#else
    print_timing();
    write_timing();
#endif
    print_power();//TJSong
    fclose_all();//TJSong
//---------------------modified by TJSong----------------------//

#ifdef ENABLE_PLUGIN
    plugin_exec_process_result(recog);
#endif
    /* output graph */
    /* r->result.wg == NULL should be skipped inside the callback */
    ok_p = FALSE;
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      if (r->config->compute_only_1pass) continue;
      if (r->result.status < 0) continue;
      if (r->config->graph.lattice) ok_p = TRUE;
    }
    if (ok_p) callback_exec(CALLBACK_RESULT_GRAPH, recog);
    /* output confnet */
    /* r->result.confnet == NULL should be skipped inside the callback */
    ok_p = FALSE;
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      if (r->config->compute_only_1pass) continue;
      if (r->result.status < 0) continue;
      if (r->config->graph.confnet) ok_p = TRUE;
    }
    if (ok_p) callback_exec(CALLBACK_RESULT_CONFNET, recog);

    /* clear work area for output */
    for(r=recog->process_list;r;r=r->next) {
      if (!r->live) continue;
      clear_result(r);
    }
    
    /* output end of 2nd pass */
    if (pass2_p) callback_exec(CALLBACK_EVENT_PASS2_END, recog);

#ifdef DEBUG_VTLN_ALPHA_TEST
    if (r->am->mfcc->para->vtln_alpha == 1.0) {
      /* if vtln parameter remains default, search for VTLN parameter */
      vtln_alpha(recog, r);
    }
#endif

  end_recog:
    /**********************/
    /* end of recognition */
    /**********************/

    /* update CMN info for next input (in case of realtime wave input) */
    if (jconf->input.type == INPUT_WAVEFORM && jconf->decodeopt.realtime_flag) {
      for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
	if (mfcc->param->samplenum > 0) {
	  RealTimeCMNUpdate(mfcc, recog);
	}
      }
    }
    
    process_segment_last = recog->process_segment;
    if (jconf->decodeopt.segment) { /* sp-segment mode */
      /* param is now shrinked to hold only the processed input, and */
      /* the rests are holded in (newly allocated) "rest_param" */
      /* if this is the last segment, rest_param is NULL */
      /* assume all segmentation are synchronized */
      recog->process_segment = FALSE;
      for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
	if (mfcc->rest_param != NULL) {
	  /* process the rest parameters in the next loop */
	  recog->process_segment = TRUE;
	  free_param(mfcc->param);
	  mfcc->param = mfcc->rest_param;
	  mfcc->rest_param = NULL;
	}
      }
    }

    /* callback of recognition end */
    if (jconf->decodeopt.segment) {
#ifdef BACKEND_VAD
      if (recog->triggered) callback_exec(CALLBACK_EVENT_SEGMENT_END, recog);
      if (process_segment_last && !recog->process_segment) callback_exec(CALLBACK_EVENT_RECOGNITION_END, recog);
#else
      callback_exec(CALLBACK_EVENT_SEGMENT_END, recog);
      if (!recog->process_segment) callback_exec(CALLBACK_EVENT_RECOGNITION_END, recog);
#endif
    } else {
      callback_exec(CALLBACK_EVENT_RECOGNITION_END, recog);
    }


    if (verbose_flag) jlog("\n");
    jlog_flush();

    if (jconf->decodeopt.segment) { /* sp-segment mode */
      if (recog->process_segment == TRUE) {
	if (verbose_flag) jlog("STAT: <<<restart the rest>>>\n\n");
      } else {
	/* input has reached end of stream, terminate program */
	if (ret <= 0 && ret != -2) break;
      }
    } else {			/* not sp-segment mode */
      /* input has reached end of stream, terminate program */
      if (ret <= 0 && ret != -2) break;
    }

    /* recognition continues for next (silence-aparted) segment */
      
  } /* END OF STREAM LOOP */
    
  /* close the stream */
  if (jconf->input.type == INPUT_WAVEFORM) {
    if (adin_end(recog->adin) == FALSE) return -1;
  }
  if (jconf->input.speech_input == SP_MFCMODULE) {
    if (mfc_module_end(recog->mfcclist) == FALSE) return -1;
  }

  /* return to the opening of input stream */

  return(0);

}

/** 
 * <EN>
 * @brief  Recognize an input stream.
 *
 * This function repeat recognition process for the whole input stream,
 * using segmentation and detection if required.  It ends when the
 * whole input has been processed.
 *
 * When a recognition stop is requested from application, the following
 * callbacks will be called in turn: CALLBACK_EVENT_PAUSE,
 * CALLBACK_PAUSE_FUNCTION, CALLBACK_EVENT_RESUME.  After finishing executing
 * all functions in these callbacks, recognition will restart.
 * If you have something to be processed while recognition stops,
 * you should write the function as callback to CALLBACK_PAUSE_FUNCTION.
 * Note that recognition will restart immediately after all functions
 * registered in CALLBACK_PAUSE_FUNCTION has been finished.
 * 
 * </EN>
 * <JA>
 * @brief  入力ストリームの認識を行う
 *
 * 入力ストリームに対して
 * （必要であれば）区間検出やVADを行いながら認識を繰り返し行っていく. 
 * 入力が終端に達するかあるいはエラーで終了する. 
 *
 * アプリケーションから認識の中断をリクエストされたときは，
 * CALLBACK_EVENT_PAUSE，CALLBACK_PAUSE_FUNCTION,
 * CALLBACK_EVENT_RESUME の順に呼んだあと認識に戻る. このため，
 * 認識を中断させている間に行う処理は，CALLBACK_PAUSE_FUNCTION
 * に登録しておく必要がある. CALLBACK_PAUSE_FUNCTION に
 * 登録されている全ての処理が終了したら認識を自動的に再開するので
 * 注意すること. 
 * 
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * 
 * @return 0 when finished recognizing all the input stream to the end,
 * or -1 on error.
 * 
 * @callgraph
 * @callergraph
 * @ingroup engine
 */
int
j_recognize_stream(Recog *recog)
{
  int ret;

  do {
    
    ret = j_recognize_stream_core(recog);

    switch(ret) {
    case 1:	      /* paused by a callback (stream will continue) */
      /* call pause event callbacks */
      callback_exec(CALLBACK_EVENT_PAUSE, recog);
      /* call pause functions */
      /* block until all pause functions exits */
      if (! callback_exist(recog, CALLBACK_PAUSE_FUNCTION)) {
	jlog("WARNING: pause requested but no pause function specified\n");
	jlog("WARNING: engine will resume now immediately\n");
      }
      callback_exec(CALLBACK_PAUSE_FUNCTION, recog);
      /* after here, recognition will restart for the rest input */
      /* call resume event callbacks */
      callback_exec(CALLBACK_EVENT_RESUME, recog);
      break;
    case 0:			/* end of stream */
      /* go on to the next input */
      break;
    case -1: 		/* error */
      jlog("ERROR: an error occured while recognition, terminate stream\n");
      return -1;
    }
  } while (ret == 1);		/* loop when paused by callback */

  return 0;
}

/* end of file */
