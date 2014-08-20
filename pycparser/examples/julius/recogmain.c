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

/*
#define GLOBAL_VARIABLE_DEFINE	///< Actually make global vars in global.h
#include <julius/julius.h>
#include <signal.h>
#if defined(_WIN32) && !defined(__CYGWIN32__)
#include <mbctype.h>
#include <mbstring.h>
#endif
*/

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
typedef int Jconf;
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

  struct timeval start, end;

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
	  ret = adin_go(RealTimePipeLine, callback_check_in_adin, recog);
	} else {
	  /* get feature vector and process it */
	  ret = mfcc_go(recog, callback_check_in_adin);
	}

  // dlo: start timing
  gettimeofday(&start, NULL);

	
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
      boolean get_back_trellis_result;
      get_back_trellis_result = get_back_trellis(recog);
      if (get_back_trellis_result == FALSE) {
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
int multigram_get_all_num_result;
multigram_get_all_num_result= multigram_get_all_num(r->lm);
	result_sentence_malloc(r, r->config->output.output_hypo_maxnum * multigram_get_all_num_result);
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
    // dlo: end timing
    gettimeofday(&end, NULL);
    printf("time 000 = %d us\n", (int)(end.tv_sec - start.tv_sec)*1000000 + (int)(end.tv_usec - start.tv_usec));

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

/**
 * @file   pass1.c
 * 
 * <JA>
 * @brief  第1パス：フレーム同期ビーム探索
 *
 * 静的木構造辞書を用いて，入力特徴量ベクトル列に対して，Juliusの第１パス
 * であるフレーム同期ビーム探索を行います. 
 *
 * 入力データ全体があらかじめ得られている場合は，一括で計算を
 * 行う関数 get_back_trellis() がメインから呼ばれます. オンライン認識
 * の場合は realtime_1stpass.c から，初期化，フレームごとの計算，
 * 終了処理のそれぞれが入力の進行にあわせて個別に呼ばれます. 
 *
 * 実際の個々の認識処理インスタンスごとの処理は beam.c に記述されています. 
 *
 * </JA>
 * 
 * <EN>
 * @brief  The first pass: frame-synchronous beam search
 *
 * These functions perform a frame-synchronous beam search using a static
 * lexicon tree, as the first pass of Julius/Julian.
 *
 * When the whole input is already obtained, get_back_trellis() simply
 * does all the processing of the 1st pass.  When performing online
 * real-time recognition with concurrent speech input, each function
 * will be called separately from realtime_1stpass.c according on the
 * basis of input processing.
 *
 * The core recognition processing functions for each recognition
 * process instances are written in beam.c.
 *
 * </EN>
 * 
 * @author Akinobu Lee
 * @date   Fri Oct 12 23:14:13 2007
 *
 * $Revision: 1.11 $
 * 
 */
/*
 * Copyright (c) 1991-2013 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2013 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

//#include <julius/julius.h>

/********************************************************************/
/* 第１パスを実行するメイン関数                                     */
/* 入力をパイプライン処理する場合は realtime_1stpass.c を参照のこと */
/* main function to execute 1st pass                                */
/* the pipeline processing is not here: see realtime_1stpass.c      */
/********************************************************************/

/** 
 * <EN>
 * @brief  Process one input frame for all recognition process instance.
 *
 * This function proceeds the recognition for one frame.  All
 * recognition process instance will be processed synchronously.
 * The input frame for each instance is stored in mfcc->f, where mfcc
 * is the MFCC calculation instance assigned to each process instance.
 *
 * If an instance's mfcc->invalid is set to TRUE, its processing will
 * be skipped.
 *
 * When using GMM, GMM computation will also be executed here.
 * If GMM_VAD is defined, GMM-based voice detection will be performed
 * inside this function, by using a scheme of short-pause segmentation.
 *
 * This function also handles segmentation of recognition process.  A
 * segmentation will occur when end of speech is detected by level-based
 * sound detection or GMM-based / decoder-based VAD, or by request from
 * application.  When segmented, it stores current frame and return with
 * that status.
 *
 * The frame-wise callbacks will be executed inside this function,
 * when at least one valid recognition process instances exists.
 * 
 * </EN>
 * <JA>
 * @brief  全ての認識処理インスタンス処理を1フレーム分進める.
 *
 * 全ての認識処理インスタンスについて，割り付けられているMFCC計算インスタンス
 * の mfcc->f をカレントフレームとして処理を1フレーム進める. 
 *
 * なお，mfcc->invalid が TRUE となっている処理インスタンスの処理はスキップ
 * される. 
 *
 * GMMの計算もここで呼び出される. GMM_VAD 定義時は，GMM による
 * 発話区間開始・終了の検出がここで行われる. また，GMMの計算結果，
 * あるいは認識処理内のショートポーズセグメンテーション判定やデバイス・外部
 * からの要求によりセグメンテーションが要求されたかどうかの判定も行う. 
 *
 * フレーム単位で呼び出されるコールバックが登録されている場合は，それらの
 * 呼出しも行う. 
 * </JA>
 * 
 * @param recog [in] engine instance
 * 
 * @return 0 on success, -1 on error, or 1 when an input segmentation
 * occured/requested inside this function.
 *
 * @callgraph
 * @callergraph
 * 
 */
int
decode_proceed(Recog *recog)
{
  MFCCCalc *mfcc;
  boolean break_flag;
  boolean break_decode;
  RecogProcess *p;
  boolean ok_p;
#ifdef GMM_VAD
  GMMCalc *gmm;
  boolean break_gmm;
#endif
  
  break_decode = FALSE;

  for(p = recog->process_list; p; p = p->next) {
#ifdef DETERMINE
    p->have_determine = FALSE;
#endif
    p->have_interim = FALSE;
  }
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    mfcc->segmented = FALSE;
  }

#ifdef POWER_REJECT
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (!mfcc->valid) continue;
    if (mfcc->f == 0) {
      mfcc->avg_power = 0.0;
      if (debug2_flag) jlog("STAT: power_reject: reset\n");
    }
  }
#endif


#ifdef GMM_VAD
  if (recog->gmm != NULL) {
    /* reset flags */
    break_gmm = FALSE;
    recog->gc->want_rewind = FALSE;
  }
#endif
  if (recog->gmm != NULL && recog->gmmmfcc->valid) {
    /* GMM 計算を行う */
    if (recog->gmmmfcc->f == 0) {
      /* GMM 計算の初期化 */
      gmm_prepare(recog);
    }
    /* このフレームに対するGMMの尤度を計算 */
    gmm_proceed(recog);
#ifdef GMM_VAD
    /* Check for GMM-based VAD */
    gmm = recog->gc;
    gmm_check_trigger(recog);
    if (gmm->after_trigger) {
      /* after trigger, in speech area */
      if (gmm->down_trigger) {
	/* down trigger, end segment */
#ifdef GMM_VAD_DEBUG
	printf("GMM_VAD: %d: down trigger\n", recog->gmmmfcc->f);
#endif
	recog->gmmmfcc->sparea_start = recog->gmmmfcc->f + 1 - recog->jconf->detect.gmm_margin;
	if (recog->gmmmfcc->sparea_start < 0) recog->gmmmfcc->sparea_start = 0;
	gmm->after_trigger = FALSE;
	recog->gmmmfcc->segmented = TRUE;
	break_gmm = TRUE;
      } else {
	/* keep recognition */
      }
    } else {
      /* before trigger, in noise area */
      if (gmm->up_trigger) {
	/* start recognition */
	/* request caller to rewind to the backstep point and
	   re-start with normal search */
	if (recog->gmmmfcc->f + 1 < recog->jconf->detect.gmm_margin) {
	  gmm->rewind_frame = 0;
	} else {
	  gmm->rewind_frame = recog->gmmmfcc->f + 1 - recog->jconf->detect.gmm_margin;
	}
#ifdef GMM_VAD_DEBUG
	printf("GMM_VAD: %d: up trigger, start recognition with %d frame rewind\n", recog->gmmmfcc->f, recog->gmmmfcc->f - gmm->rewind_frame);
#endif
	gmm->want_rewind = TRUE;
	gmm->want_rewind_reprocess = TRUE;
	gmm->after_trigger = TRUE;
	return 0;
      } else {
	/* before trigger, noise continues */

	/* if noise goes more than a certain frame, shrink the noise area
	   to avoid unlimited memory usage */
	if (recog->gmmmfcc->f + 1 > GMM_VAD_AUTOSHRINK_LIMIT) {
	  gmm->want_rewind = TRUE;
	  gmm->want_rewind_reprocess = FALSE;
	  gmm->rewind_frame = recog->gmmmfcc->f + 1 - recog->jconf->detect.gmm_margin;
	  if (debug2_flag) {
	    jlog("DEBUG: GMM_VAD: pause exceeded %d, rewind\n", GMM_VAD_AUTOSHRINK_LIMIT);
	  }
	}

	/* skip recognition processing */
	return 0;
      }
    }
#endif /* GMM_VAD */
  }

  for(p = recog->process_list; p; p = p->next) {
    if (!p->live) continue;
    mfcc = p->am->mfcc;
    if (!mfcc->valid) {
      /* このフレームの処理をスキップ */
      /* skip processing the frame */
      continue;
    }

    /* mfcc-f のフレームについて認識処理(フレーム同期ビーム探索)を進める */
    /* proceed beam search for mfcc->f */
    if (mfcc->f == 0) {
      /* 最初のフレーム: 探索処理を初期化 */
      /* initial frame: initialize search process */
      boolean get_back_trellis_init_result;
      get_back_trellis_init_result = get_back_trellis_init(mfcc->param, p);
      if (get_back_trellis_init_result == FALSE) {
	jlog("ERROR: %02d %s: failed to initialize the 1st pass\n", p->config->id, p->config->name);
	return -1;
      }
    }
    if (mfcc->f > 0 || p->am->hmminfo->multipath) {
      /* 1フレーム探索を進める */
      /* proceed search for 1 frame */
      boolean get_back_trellis_proceed_result;
      get_back_trellis_proceed_result = get_back_trellis_proceed(mfcc->f, mfcc->param, p, FALSE);
      if (get_back_trellis_proceed_result == FALSE) {
	mfcc->segmented = TRUE;
	break_decode = TRUE;
      }
      if (p->config->successive.enabled) {
	if (detect_end_of_segment(p, mfcc->f - 1)) {
	  /* セグメント終了検知: 第１パスここで中断 */
	  mfcc->segmented = TRUE;
	  break_decode = TRUE;
	}
      }
    }
  }

  /* セグメントすべきかどうか最終的な判定を行う．
     デコーダベースVADあるいは spsegment の場合，複数インスタンス間で OR
     を取る．また，GMMなど複数基準がある場合は基準間で AND を取る．*/
  /* determine whether to segment at here
     If multiple segmenter exists, take their AND */
  break_flag = FALSE;
  if (break_decode
#ifdef GMM_VAD
      || (recog->gmm != NULL && break_gmm)
#endif
      ) {
    break_flag = TRUE;
  }

  if (break_flag) {
    /* 探索処理の終了が発生したのでここで認識を終える. 
       最初のフレームから [f-1] 番目までが認識されたことになる
    */
    /* the recognition process tells us to stop recognition, so
       recognition should be terminated here.
       the recognized data are [0..f-1] */

    /* 最終フレームを last_time にセット */
    /* set the last frame to last_time */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      mfcc->last_time = mfcc->f - 1;
    }

    if (! recog->jconf->decodeopt.segment) {
      /* ショートポーズ以外で切れた場合，残りのサンプルは認識せずに捨てる */
      /* drop rest inputs if segmented by error */
      for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
	mfcc->param->header.samplenum = mfcc->f;
	mfcc->param->samplenum = mfcc->f;
      }
    }

    return 1;
  }

  /* call frame-wise callback for the processing results if any */
#ifdef DETERMINE
  ok_p = FALSE;
  for(p=recog->process_list;p;p=p->next) {
    if (!p->live) continue;
    if (p->have_determine) {
      ok_p = TRUE;
    }
  }
  if (ok_p) callback_exec(CALLBACK_RESULT_PASS1_DETERMINED, recog);
#endif
  ok_p = FALSE;
  for(p=recog->process_list;p;p=p->next) {
    if (!p->live) continue;
    if (p->have_interim) {
      ok_p = TRUE;
    }
  }
  if (ok_p) callback_exec(CALLBACK_RESULT_PASS1_INTERIM, recog);
  
  return 0;
}

#ifdef POWER_REJECT
boolean
power_reject(Recog *recog)
{
  MFCCCalc *mfcc;

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    /* skip if not realtime and raw file processing */
    if (mfcc->avg_power == 0.0) continue;
    if (debug2_flag) jlog("STAT: power_reject: MFCC%02d: avg_power = %f\n", mfcc->id, mfcc->avg_power / mfcc->param->samplenum);
    if (mfcc->avg_power / mfcc->param->samplenum < recog->jconf->reject.powerthres) return TRUE;
  }
  return FALSE;
}
#endif

/** 
 * <EN>
 * @brief  End procedure of the first pass (when segmented)
 *
 * This function do things for ending the first pass and prepare for
 * the next recognition, when the input was segmented at the middle of
 * recognition by some reason.
 *
 * First, the best path at each recognition process instance will be parsed
 * and stored.  In case of recognition error or input rejection, the error
 * status will be set.
 *
 * Then, the last pause segment of the processed input will be cut and saved
 * to be processed at first in the recognition of the next or remaining input.
 * 
 * </EN>
 * <JA>
 * @brief  第1パスの終了処理（セグメント時）
 * 
 * 入力が何らかの事由によって途中でセグメントされた時に，第1パスの認識処理を
 * 終了して次回再開するための処理を行う. 
 *
 * まず，各認識処理インスタンスに対して，最尤単語系列を見付け，第1パスの
 * 認識結果として格納する. また，認識失敗・入力棄却の時はエラーステータスをそ
 * れぞれセットする.
 * 
 * そして，次回の認識で，次のセグメントの認識を，検出された末尾雑音
 * 区間から再開するために，その末尾雑音区間を切り出しておく処理を呼ぶ. 
 * 
 * </JA>
 * 
 * @param recog [in] engine instance
 * 
 * @callgraph
 * @callergraph
 */
void
decode_end_segmented(Recog *recog)
{
  boolean ok_p;
  int mseclen;
  RecogProcess *p;
  int last_status;

  /* rejectshort 指定時, 入力が短ければここで第1パス結果を出力しない */
  /* suppress 1st pass output if -rejectshort and input shorter than specified */
  ok_p = TRUE;
  if (recog->jconf->reject.rejectshortlen > 0) {
    mseclen = (float)recog->mfcclist->last_time * (float)recog->jconf->input.period * (float)recog->jconf->input.frameshift / 10000.0;
    if (mseclen < recog->jconf->reject.rejectshortlen) {
      last_status = J_RESULT_STATUS_REJECT_SHORT;
      ok_p = FALSE;
    }
  }
  if (recog->jconf->reject.rejectlonglen >= 0) {
    mseclen = (float)recog->mfcclist->last_time * (float)recog->jconf->input.period * (float)recog->jconf->input.frameshift / 10000.0;
    if (mseclen >= recog->jconf->reject.rejectlonglen) {
      last_status = J_RESULT_STATUS_REJECT_LONG;
      ok_p = FALSE;
    }
  }

#ifdef POWER_REJECT
  if (ok_p) {
    if (power_reject(recog)) {
      last_status = J_RESULT_STATUS_REJECT_POWER;
      ok_p = FALSE;
    }
  }
#endif

  if (ok_p) {
    for(p=recog->process_list;p;p=p->next) {
      if (!p->live) continue;
      finalize_1st_pass(p, p->am->mfcc->last_time);
    }
  } else {
    for(p=recog->process_list;p;p=p->next) {
      if (!p->live) continue;
      p->result.status = last_status;
    }
  }
  if (recog->jconf->decodeopt.segment) {
    finalize_segment(recog);
  }

  if (recog->gmm != NULL) {
    /* GMM 計算の終了 */
    gmm_end(recog);
  }
}

/** 
 * <EN>
 * @brief  End procedure of the first pass
 *
 * This function finish the first pass, when the input was fully
 * processed to the end.
 *
 * The best path at each recognition process instance will be parsed
 * and stored.  In case of recognition error or input rejection, the
 * error status will be set.
 *
 * </EN>
 * <JA>
 * @brief  第1パスの終了処理
 * 
 * 入力が最後まで処理されて終了したときに，第1パスの認識処理を
 * 終了させる. 
 *
 * 各認識処理インスタンスに対して，その時点での第1パスの最尤単語
 * 系列を格納する. また，認識失敗・入力棄却の時はエラーステータスをそ
 * れぞれセットする.
 * 
 * </JA>
 * 
 * @param recog [in] engine instance
 * 
 * @callgraph
 * @callergraph
 */
void
decode_end(Recog *recog)
{
  MFCCCalc *mfcc;
  int mseclen;
  boolean ok_p;
  RecogProcess *p;
  int last_status;

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    mfcc->segmented = FALSE;
  }

  if (recog->gmm != NULL) {
    /* GMM 計算の終了 */
    gmm_end(recog);
  }

#ifdef GMM_VAD
  /* もしトリガがかからないまま入力終了に達したのなら，そのままエラー終了 */
  if (recog->jconf->decodeopt.segment) {
    if (recog->gmm) {
      if (recog->gc->after_trigger == FALSE) {
	for(p=recog->process_list;p;p=p->next) {
	  p->result.status = J_RESULT_STATUS_ONLY_SILENCE;	/* reject by decoding */
	}
	/* ショートポーズセグメンテーションの場合,
	   入力パラメータ分割などの最終処理も行なう */
	/* When short-pause segmentation enabled */
	finalize_segment(recog);
	return;
      }
    }
  }
#endif

  /* 第１パスの最後のフレームの認識処理を行う */
  /* finalize 1st pass */
  for(p=recog->process_list;p;p=p->next) {
    if (!p->live) continue;
#ifdef SPSEGMENT_NAIST
    if (recog->jconf->decodeopt.segment) {
      if (p->pass1.after_trigger == FALSE) continue;
    }
#endif
    mfcc = p->am->mfcc;
    if (mfcc->f > 0) {
      get_back_trellis_end(mfcc->param, p);
    }
  }

  /* 終了処理 */
  for(p=recog->process_list;p;p=p->next) {
    if (!p->live) continue;

    ok_p = TRUE;

    /* check rejection by no input */
    if (ok_p) {
      mfcc = p->am->mfcc;
      /* 入力長がデルタの計算に十分でない場合，入力無しとする． */
      /* if input is short for compute all the delta coeff., terminate here */
      if (mfcc->f == 0) {
	jlog("STAT: no input frame\n");
	last_status = J_RESULT_STATUS_FAIL;
	ok_p = FALSE;
      }
    }

    /* check rejection by input length */
    if (ok_p) {
      if (recog->jconf->reject.rejectshortlen > 0) {
	mseclen = (float)mfcc->param->samplenum * (float)recog->jconf->input.period * (float)recog->jconf->input.frameshift / 10000.0;
	if (mseclen < recog->jconf->reject.rejectshortlen) {
	  last_status = J_RESULT_STATUS_REJECT_SHORT;
	  ok_p = FALSE;
	}
      }
      if (recog->jconf->reject.rejectlonglen >= 0) {
	mseclen = (float)mfcc->param->samplenum * (float)recog->jconf->input.period * (float)recog->jconf->input.frameshift / 10000.0;
	if (mseclen >= recog->jconf->reject.rejectlonglen) {
	  last_status = J_RESULT_STATUS_REJECT_LONG;
	  ok_p = FALSE;
	}
      }
    }

#ifdef POWER_REJECT
    /* check rejection by average power */
    if (ok_p) {
      if (power_reject(recog)) {
	last_status = J_RESULT_STATUS_REJECT_POWER;
	ok_p = FALSE;
      }
    }
#endif

#ifdef SPSEGMENT_NAIST
    /* check rejection non-triggered input segment */
    if (ok_p) {
      if (recog->jconf->decodeopt.segment) {
	if (p->pass1.after_trigger == FALSE) {
	  last_status = J_RESULT_STATUS_ONLY_SILENCE;	/* reject by decoding */
	  ok_p = FALSE;
	}
      }
    }
#endif

    if (ok_p) {
      /* valid input segment, finalize it */
      finalize_1st_pass(p, mfcc->param->samplenum);
    } else {
      /* invalid input segment */
      p->result.status = last_status;
    }
  }
  if (recog->jconf->decodeopt.segment) {
    /* ショートポーズセグメンテーションの場合,
       入力パラメータ分割などの最終処理も行なう */
    /* When short-pause segmentation enabled */
    finalize_segment(recog);
  }
}


/** 
 * <JA>
 * @brief  フレーム同期ビーム探索メイン関数（バッチ処理用）
 *
 * 与えられた入力ベクトル列に対して第１パス(フレーム同期ビーム探索)を
 * 行い，その結果を出力する. また全フレームに渡る単語終端を，第２パス
 * のために単語トレリス構造体に格納する. 
 * 
 * この関数は入力ベクトル列があらかじめ得られている場合に用いられる. 
 * 第１パスが入力と並列して実行されるオンライン認識の場合，
 * この関数は用いられず，代わりにこのファイルで定義されている各サブ関数が
 * 直接 realtime-1stpass.c 内から呼ばれる. 
 * 
 * @param recog [in] エンジンインスタンス
 * </JA>
 * <EN>
 * @brief  Frame synchronous beam search: the main (for batch mode)
 *
 * This function perform the 1st recognition pass of frame-synchronous beam
 * search and output the result.  It also stores all the word ends in every
 * input frame to word trellis structure.
 *
 * This function will be called if the whole input vector is already given
 * to the end.  When online recognition, where the 1st pass will be
 * processed in parallel with input, this function will not be used.
 * In that case, functions defined in this file will be directly called
 * from functions in realtime-1stpass.c.
 * 
 * @param recog [in] engine instance
 * </EN>
 * @callgraph
 * @callergraph
 */
boolean
get_back_trellis(Recog *recog)
{
  boolean ok_p;
  MFCCCalc *mfcc;
  int rewind_frame;
  PROCESS_AM *am;
  boolean reprocess;

  /* initialize mfcc instances */
  for(mfcc=recog->mfcclist;mfcc;mfcc=mfcc->next) {
    /* mark all as valid, since all frames are fully prepared beforehand */
    if (mfcc->param->samplenum == 0) mfcc->valid = FALSE;
    else mfcc->valid = TRUE;
    /* set frame pointers to 0 */
    mfcc->f = 0;
  }

  /* callback of process start */
#ifdef BACKEND_VAD
  if (recog->jconf->decodeopt.segment) {
    /* at first time, recognition does not start yet */
    /* reset segmentation flags */
    spsegment_init(recog);
  } else {
    /* execute callback for pass1 begin here */
    callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
    callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
    recog->triggered = TRUE;
  }
#else
  if (recog->jconf->decodeopt.segment) {
    if (!recog->process_segment) {
      callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
    }
    callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
  } else {
    callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
  }
  callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
  recog->triggered = TRUE;
#endif

  while(1) {
    ok_p = TRUE;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (! mfcc->valid) continue;
      if (mfcc->f < mfcc->param->samplenum) {
	mfcc->valid = TRUE;
	ok_p = FALSE;
      } else {
	mfcc->valid = FALSE;
      }
    }
    if (ok_p) {
      /* すべての MFCC が終わりに達したのでループ終了 */
      /* all MFCC has been processed, end of loop  */
      break;
    }

    int decode_proceed_result;
    decode_proceed_result = decode_proceed(recog);
    switch (decode_proceed_result) {
    case -1: /* error */
      return FALSE;
      break;
    case 0:			/* success */
      break;
    case 1:			/* segmented */
      /* 探索中断: 処理された入力は 0 から t-2 まで */
      /* search terminated: processed input = [0..t-2] */
      /* この時点で第1パスを終了する */
      /* end the 1st pass at this point */
      decode_end_segmented(recog);
      /* terminate 1st pass here */
      return TRUE;
    }

#ifdef BACKEND_VAD
    /* check up trigger in case of VAD segmentation */
    if (recog->jconf->decodeopt.segment) {
      if (recog->triggered == FALSE) {
	if (spsegment_trigger_sync(recog)) {
	  if (!recog->process_segment) {
	    callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
	  }
	  callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
	  callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
	  recog->triggered = TRUE;
	}
      }
    }
#endif

    if (spsegment_need_restart(recog, &rewind_frame, &reprocess) == TRUE) {
      /* do rewind for all mfcc here */
      spsegment_restart_mfccs(recog, rewind_frame, reprocess);
      /* reset outprob cache for all AM */
      for(am=recog->amlist;am;am=am->next) {
	outprob_prepare(&(am->hmmwrk), am->mfcc->param->samplenum);
      }
    }
    /* call frame-wise callback */
    callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);

    /* 1フレーム処理が進んだのでポインタを進める */
    /* proceed frame pointer */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (!mfcc->valid) continue;
      mfcc->f++;
    }

    if (recog->process_want_terminate) {
      /* termination requested */
      decode_end_segmented(recog);
      return TRUE;
    }
  }

  /* 最終フレーム処理を行い，認識の結果出力と終了処理を行う */
  decode_end(recog);

  return TRUE;
}

/* end of file */

/**
 * @file   beam.c
 * 
 * <JA>
 * @brief  フレーム同期ビーム探索の実行（第1パス）
 *
 * 第1パスのフレーム同期ビーム探索を実際に実行する関数群です. 
 * 認識処理インスタンスごとに実行されます. 
 * 初期化，1フレームの認識処理，終了処理，第1パスの結果決定，セグメント
 * 終了の検知などの処理が含まれています. 
 *
 * アルゴリズムについては，単語履歴近似は 1-best 近似がデフォルトです
 * が，単語対近似も使用可能です. 単語N-gram では単語間の接続制約は 1-gram
 * factoring (2-gram factoring も選択可)を用いて計算されます. 文法の
 * 場合，木構造化辞書は文法のカテゴリ単位で作成され，単語間の接続(単語
 * 対制約)は単語間遷移で適用されます. 単語認識モードでは単語間接続は
 * 考慮されません. 
 * </JA>
 * 
 * <EN>
 * @brief  Frame-synchronous beam search for the 1st pass
 *
 * These are core functions of frame-synchronous beam search using a
 * static lexicon tree, as the first pass of Julius.  These functions
 * will be called from pass1.c, to execute for each recognition
 * process instance in turn.  Functions for initialization, frame-wise
 * recognition processing, end procedure, finding best path, detecting
 * end of segment on short-pause segmentation mode, are defined here.
 *
 * About algorithm: 1-best approximation will be performed for word
 * context approximation, but normal word-pair approximation is also
 * supported. With word/class N-gram, Julius computes the language
 * score using 1-gram factoring (can be changed to 2-gram factoring if
 * you want).  With DFA grammar, Julius can compute the connection
 * constraint of words using the category-pair constraint on the
 * beginning of the words, since Julian makes a per-category tree
 * lexicon.  On isolated word recognition mode, the cross-word transitions
 * are ignored.
 *
 * </EN>
 * 
 * @author Akinobu LEE
 * @date   Tue Feb 22 17:00:45 2005
 *
 * $Revision: 1.22 $
 * 
 */
/*
 * Copyright (c) 1991-2013 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2013 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

//#include <julius/julius.h>

#undef DEBUG


/* ---------------------------------------------------------- */
/*                     第１パスの結果処理                     */
/*              end procedure to get result of 1st pass       */
/* ---------------------------------------------------------- */

#ifdef WORD_GRAPH
/** 
 * <JA>
 * @brief  認識結果の単語トレリスから単語グラフを抽出する
 *
 * (WORD_GRAPH 指定時)
 * この関数は第１パスの結果の単語トレリスを終端からバックトレースし，
 * パス上にあるトレリス単語を単語グラフとして抽出する. 実際には，
 * 単語トレリス上でグラフ上に残るもののみにマークを付け，
 * 第2パスでは，マークのついた単語のみを展開する. 
 *
 * グラフは r->result.wg1 に格納される. 
 * 
 * @param frame [in] 単語トレリス上で単語末端を検索するフレーム
 * @param r [i/o] 認識処理インスタンス
 * </JA>
 * <EN>
 * @brief  Extract word graph from the resulting word trellis
 *
 * If WORD_GRAPH is defined, this function trace back through the
 * word trellis from the end point, to extract the trellis words on
 * the path as a word graph.  Actually, this function only marks
 * which trellis words are included in the word graph.  On the 2nd pass,
 * only the words in the word graph will be expanded.
 *
 * The generated word graph will be stored to r->result.wg1.
 * 
 * @param frame [in] frame to lookup for word ends in the word trellis
 * @param r [i/o] recognition process instance
 * </EN>
 */
static void
generate_lattice(int frame, RecogProcess *r)
{
  BACKTRELLIS *bt;
  WORD_INFO *winfo;
  TRELLIS_ATOM *ta;
  int i, j;
  LOGPROB l;
  WordGraph *new;

  bt = r->backtrellis;
  winfo = r->lm->winfo;

  if (frame >= 0) {
    for (i=0;i<bt->num[frame];i++) {
      ta = bt->rw[frame][i];
      /* words will be saved as a part of graph only if any of its
	 following word has been survived in a beam */
      if (! ta->within_context) continue; /* not a candidate */
      if (ta->within_wordgraph) continue; /* already marked */
      /* mark  */
      ta->within_wordgraph = TRUE;

      new = (WordGraph *)mymalloc(sizeof(WordGraph));
      new->wid = ta->wid;
      new->lefttime = ta->begintime;
      new->righttime = ta->endtime;
      new->fscore_head = ta->backscore;
      new->fscore_tail = 0.0;
      new->gscore_head = 0.0;
      new->gscore_tail = 0.0;
      new->lscore_tmp = ta->lscore;
#ifdef CM_SEARCH
      new->cmscore = 0.0;
#endif
      new->forward_score = new->backward_score = 0.0;
      new->headphone = winfo->wseq[ta->wid][0];
      new->tailphone = winfo->wseq[ta->wid][winfo->wlen[ta->wid]-1];

      new->leftwordmaxnum = FANOUTSTEP;
      new->leftword = (WordGraph **)mymalloc(sizeof(WordGraph *) * new->leftwordmaxnum);
      new->left_lscore = (LOGPROB *)mymalloc(sizeof(LOGPROB) * new->leftwordmaxnum);
      new->leftwordnum = 0;
      new->rightwordmaxnum = FANOUTSTEP;
      new->rightword = (WordGraph **)mymalloc(sizeof(WordGraph *) * new->rightwordmaxnum);
      new->right_lscore = (LOGPROB *)mymalloc(sizeof(LOGPROB) * new->rightwordmaxnum);
      new->rightwordnum = 0;

      l = ta->backscore;
      if (ta->last_tre->wid != WORD_INVALID) {
	l -= ta->last_tre->backscore;
      }
      l -= ta->lscore;
      new->amavg = l / (float)(ta->endtime - ta->begintime + 1);

#ifdef GRAPHOUT_DYNAMIC
      new->purged = FALSE;
#endif
      new->saved = FALSE;
      new->graph_cm = 0.0;
      new->mark = FALSE;

      new->next = r->result.wg1;
      r->result.wg1 = new;

      /* recursive call */
      generate_lattice(ta->last_tre->endtime, r);
    }
  }
}

/** 
 * <EN>
 * Link all words in 1st pass word graph extracted by
 * generate_lattice() by their boundary frame.  All words with the
 * same boundary frame will be connected.
 * 
 * </EN>
 * <JA>
 * generate_lattice() で生成した第1パスグラフ中の単語どうしを境界時間
 * に従って連結する. 同じ境界時間を持つすべての単語が接続される. 
 * 
 * </JA>
 * 
 * @param root [in] pointer to the root of lattice words.
 * 
 */
static void
link_lattice_by_time(WordGraph *root)
{
  WordGraph *wg;
  WordGraph *wtmp;
  int lefttime, righttime;
  
  for(wg=root;wg;wg=wg->next) {
    
    for(wtmp=root;wtmp;wtmp=wtmp->next) {
      if (wg->righttime + 1 == wtmp->lefttime) {
	wordgraph_check_and_add_leftword(wtmp, wg, wtmp->lscore_tmp);
	wordgraph_check_and_add_rightword(wg, wtmp, wtmp->lscore_tmp);
      }
      if (wtmp->righttime + 1 == wg->lefttime) {
	wordgraph_check_and_add_leftword(wg, wtmp, wg->lscore_tmp);
	wordgraph_check_and_add_rightword(wtmp, wg, wg->lscore_tmp);
      }
    }
  }

}

/** 
 * <EN>
 * re-compute 2-gram prob for all link in 1st pass word graph mode.
 * </EN>
 * <JA>
 * 第1パスで単語グラフを生成するモードにおいて，生成後に単語グラフ上の
 * 正確な2-gram言語確立を再計算する. 
 * </JA>
 * 
 * @param root [in] pointer to root node of word graph
 * @param wchmm [in] tree lexicon used for the word graph generation
 * 
 */
static void
re_compute_lattice_lm(WordGraph *root, WCHMM_INFO *wchmm)
{
  WordGraph *wg;
  int i;
  
  for(wg=root;wg;wg=wg->next) {
    for(i=0;i<wg->leftwordnum;i++) {
      wg->left_lscore[i] = (*(wchmm->ngram->bigram_prob))(wchmm->ngram, wchmm->winfo->wton[wg->leftword[i]->wid], wchmm->winfo->wton[wg->wid]);
    }
    for(i=0;i<wg->rightwordnum;i++) {
      wg->right_lscore[i] = (*(wchmm->ngram->bigram_prob))(wchmm->ngram, wchmm->winfo->wton[wg->wid], wchmm->winfo->wton[wg->rightword[i]->wid]);
    }
  }
}

#endif

/** 
 * <JA>
 * あるトレリス単語の情報をテキストで出力 (デバッグ用)
 * 
 * @param atom [in] 出力するトレリス単語
 * @param winfo [in] 単語辞書
 * </JA>
 * <EN>
 * Output a trellis word information in text (for debug)
 * 
 * @param atom [in] trellis word to output
 * @param winfo [in] word dictionary
 * </EN>
 */
static void
put_atom(TRELLIS_ATOM *atom, WORD_INFO *winfo)
{
  int i;
  jlog("DEBUG: %3d,%3d %f %16s (id=%5d)", atom->begintime, atom->endtime,
       atom->backscore, winfo->wname[atom->wid], atom->wid);
  for (i=0;i<winfo->wlen[atom->wid]; i++) {
    jlog(" %s",winfo->wseq[atom->wid][i]->name);
  }
  jlog("\n");
}



/** 
 * <JA>
 * @brief  第１パスの認識処理結果から認識結果を判定し，最尤単語系列を見つける. 
 *
 * 第１パスの計算結果である単語トレリスから，第１パスでの最尤候補を求
 * め，インスタンス内の result.pass1 に保存する. 候補が得られない場合
 * はエラー（探索誤り：コード -1）となる. 
 *
 * ショートポーズセグメンテーション時は，認識結果が無音単語のみからなる場合，
 * エラー（デコーダによる棄却：コード -4）となる. 
 *
 * また，WORD_GRAPH 定義時は，この関数内でさらに generate_lattice() を
 * 呼び出し，単語グラフの抽出を行う. 
 * 
 * @param framelen [in] 第１パスで処理が到達したフレーム数
 * @param r [in] 認識処理インスタンス
 * 
 * </JA>
 * <EN>
 * @brief  Find best path from the first pass result and set result status.
 *
 * This function find the best word sequence from the resulting word
 * trellis of the 1st pass, and store them to result.pass1 in the
 * recognition process instance.  If no candidate was found, it sets
 * error code -1 (recognition failure) and exit.
 *
 * On short-pause segmentation, it sets error code -4 (decoder rejection)
 * if the found best path consists of only silence words.
 * 
 * Also, if WORD_GRAPH is defined, this function also calls
 * generate_lattice() to extract word graph from the word trellis.
 * 
 * @param framelen [in] frame length that has been processed
 * @param r [in] recognition process instance
 * </EN>
 */
static void
find_1pass_result(int framelen, RecogProcess *r)
{
  BACKTRELLIS *backtrellis;
  WORD_INFO *winfo;
  WORD_ID wordseq[MAXSEQNUM];
  int wordlen;
  int i;
  TRELLIS_ATOM *best;
  int last_time;
  LOGPROB total_lscore;
  LOGPROB maxscore;
  TRELLIS_ATOM *tmp;
#ifdef SPSEGMENT_NAIST
  boolean ok_p;
#endif

  backtrellis = r->backtrellis;
  winfo = r->lm->winfo;

  /* look for the last trellis word */

  if (r->lmtype == LM_PROB) {

    for (last_time = framelen - 1; last_time >= 0; last_time--) {

      maxscore = LOG_ZERO;
      for (i=0;i<backtrellis->num[last_time];i++) {
	tmp = backtrellis->rw[last_time][i];
#ifdef WORD_GRAPH
	/* treat only words on a graph path */
	if (!tmp->within_context) continue;
#endif
	if (r->config->successive.enabled) {
	  /* short-pause segmentation mode */
	  /* 最終フレームに残った最大スコアの単語 */
	  /* it should be the best trellis word on the last frame */
	  if (maxscore < tmp->backscore) {
	    maxscore = tmp->backscore;
	    best = tmp;
	  }
	} else {
	  /* not segmentation mode */
	  /* 最終単語は winfo->tail_silwid に固定 */
	  /* it is fixed to the tail silence model (winfo->tail_silwid) */
	  if (tmp->wid == winfo->tail_silwid && maxscore < tmp->backscore) {
	    maxscore = tmp->backscore;
	    best = tmp;
	    break;
	  }
	}
      }
      if (maxscore != LOG_ZERO) break;
    }

    if (last_time < 0) {		/* not found */
      jlog("WARNING: %02d %s: no tail silence word survived on the last frame, search failed\n", r->config->id, r->config->name);
      r->result.status = J_RESULT_STATUS_FAIL;
      //callback_exec(CALLBACK_RESULT, r);
      return;
    }
  
  }

  if (r->lmtype == LM_DFA) {

    for (last_time = framelen - 1; last_time >= 0; last_time--) {

      /* 末尾に残った単語の中で最大スコアの単語(cp_endは使用しない) */
      /* the best trellis word on the last frame (not use cp_end[]) */
      maxscore = LOG_ZERO;
      for (i=0;i<backtrellis->num[last_time];i++) {
	tmp = backtrellis->rw[last_time][i];
#ifdef WORD_GRAPH
	/* treat only words on a graph path */
	if (!tmp->within_context) continue;
#endif
	/*      if (dfa->cp_end[winfo->wton[tmp->wid]] == TRUE) {*/
	if (maxscore < tmp->backscore) {
	  maxscore = tmp->backscore;
	  best = tmp;
	}
	/*      }*/
      }
      if (maxscore != LOG_ZERO) break;
    }

    if (last_time < 0) {		/* not found */
      jlog("WARNING: %02d %s: no sentence-end word survived on last beam\n", r->config->id, r->config->name);
      r->result.status = J_RESULT_STATUS_FAIL;
      //callback_exec(CALLBACK_RESULT, r);
      return;
    }
  
  }

  /* traceback word trellis from the best word */
  total_lscore = trace_backptr(wordseq, &wordlen, best, r->lm->winfo);
#ifdef SPSEGMENT_NAIST
  if (r->config->successive.enabled) {
    /* on segmentation mode, recognition result that only consists of
       short-pause words will be treated as recognition rejection */
    ok_p = FALSE;
    for(i=0;i<wordlen;i++) {
      if (! is_sil(wordseq[i], r)) ok_p = TRUE;
    }
    if (ok_p == FALSE) {
      r->result.status = J_RESULT_STATUS_ONLY_SILENCE;
      return;
    }
  }
#endif

  /* just flush last progress output */
  /*
  if (recog->jconf->output.progout_flag) {
    recog->result.status = 1;
    recog->result.num_frame = last_time;
    recog->result.pass1.word = wordseq;
    recog->result.pass1.word_num = wordlen;
    recog->result.pass1.score = best->backscore;
    recog->result.pass1.score_lm = total_lscore;
    recog->result.pass1.score_am = best->backscore - total_lscore;
    //callback_exec(CALLBACK_RESULT_PASS1_INTERIM, recog);
    }*/

  /* output 1st pass result */    
  if (verbose_flag || ! r->config->output.progout_flag) {
    r->result.status = J_RESULT_STATUS_SUCCESS;
    r->result.num_frame = framelen;
    for(i=0;i<wordlen;i++) r->result.pass1.word[i] = wordseq[i];
    r->result.pass1.word_num = wordlen;
    r->result.pass1.score = best->backscore;
    r->result.pass1.score_lm = total_lscore;
    r->result.pass1.score_am = best->backscore - total_lscore;
    //callback_exec(CALLBACK_RESULT_PASS1, r);
  }

  /* store the result to global val (notice: in reverse order) */
  for(i=0;i<wordlen;i++) r->pass1_wseq[i] = wordseq[i];
  r->pass1_wnum = wordlen;
  r->pass1_score = best->backscore;

#ifdef WORD_GRAPH
  /* 単語トレリスから，ラティスを生成する */
  /* generate word graph from the word trellis */
  r->peseqlen = backtrellis->framelen;
  r->result.wg1 = NULL;
  generate_lattice(last_time, r);
  link_lattice_by_time(r->result.wg1);
  if (r->lmtype == LM_PROB) re_compute_lattice_lm(r->result.wg1, r->wchmm);
  r->result.wg1_num = wordgraph_sort_and_annotate_id(&(r->result.wg1), r);
  /* compute graph CM by forward-backward processing */
  graph_forward_backward(r->result.wg1, r);
  //callback_exec(CALLBACK_RESULT_PASS1_GRAPH, r);
  //wordgraph_clean(&(r->result.wg1));
#endif

}

/** 
 * <JA>
 * トレリス単語をスコアでソートするqsort関数. 
 * 
 * @param x1 [in] 要素1へのポインタ
 * @param x2 [in] 要素2へのポインタ
 * 
 * @return qsort の値
 * </JA>
 * <EN>
 * qsort function to sort trellis words by their score.
 * 
 * @param x1 [in] pointer to element #1
 * @param x2 [in] pointer to element #2
 * 
 * @return value required for qsort.
 * </EN>
 */
static int
compare_backscore(TRELLIS_ATOM **x1, TRELLIS_ATOM **x2)
{
  return((*x2)->backscore - (*x1)->backscore);
}

/** 
 * <JA>
 * find_1pass_result() の単語認識モード版. 単語認識モードでは第1パスで
 * 認識を終了するので，得られた候補は通常の第２パスと同じ場所に格納する. 
 * 
 * @param framelen [in] 第１パスで処理が到達したフレーム数
 * @param r [i/o] 認識処理インスタンス
 * 
 * </JA>
 * <EN>
 * Isolated word recognition version of find_1pass_result().
 * Since Julius executes only the 1st pass on Isolated word recognition
 * mode, the result candidate will be stored as the final result.
 * 
 * @param framelen [in] frame length that has been processed
 * @param r [i/o] recognition process instance
 * 
 * </EN>
 */
static void
find_1pass_result_word(int framelen, RecogProcess *r)
{
  BACKTRELLIS *bt;
  TRELLIS_ATOM *best, *tmp;
  int last_time;
  Sentence *s;
#ifdef CONFIDENCE_MEASURE
  LOGPROB sum;
#endif
  LOGPROB maxscore;
  int i;
  TRELLIS_ATOM **idx;
  int num;

  if (r->lmvar != LM_DFA_WORD) return;

  bt = r->backtrellis;
  for (last_time = framelen - 1; last_time >= 0; last_time--) {
    maxscore = LOG_ZERO;
    for (i=0;i<bt->num[last_time];i++) {
      tmp = bt->rw[last_time][i];
#ifdef WORD_GRAPH
      /* treat only words on a graph path */
      if (!tmp->within_context) continue;
#endif
      if (maxscore < tmp->backscore) {
	maxscore = tmp->backscore;
	best = tmp;
      }
    }
    if (maxscore != LOG_ZERO) break;
  }

  if (last_time < 0) {		/* not found */
    jlog("WARNING: %02d %s: no word survived on the last frame, search failed\n", r->config->id, r->config->name);
    r->result.status = J_RESULT_STATUS_FAIL;
    //callback_exec(CALLBACK_RESULT, r);
    return;
  }

#ifdef CONFIDENCE_MEASURE
  sum = 0.0;
  for (i=0;i<bt->num[last_time];i++) {
    tmp = bt->rw[last_time][i];
#ifdef WORD_GRAPH
    /* treat only words on a graph path */
    if (!tmp->within_context) continue;
#endif
    sum += pow(10, r->config->annotate.cm_alpha * (tmp->backscore - maxscore));
  }
#endif

  /* set recognition result status to normal */
  r->result.status = J_RESULT_STATUS_SUCCESS;

  if (r->config->output.output_hypo_maxnum > 1) {
    /* more than one candidate is requested */

    /* get actual number of candidates to output */
    num = r->config->output.output_hypo_maxnum;
    if (num > bt->num[last_time]) {
      num = bt->num[last_time];
    }

    /* prepare result storage */
    result_sentence_malloc(r, num);
    r->result.sentnum = num;

    /* sort by score */
    idx = (TRELLIS_ATOM **)mymalloc(sizeof(TRELLIS_ATOM *)*bt->num[last_time]);
    for (i=0;i<bt->num[last_time];i++) {
      idx[i] = bt->rw[last_time][i];
    }
    qsort(idx, bt->num[last_time], sizeof(TRELLIS_ATOM *),
	  (int (*)(const void *,const void *))compare_backscore);
    
    /* store to result storage */
    for(i=0;i<r->result.sentnum;i++) {
      s = &(r->result.sent[i]);
      tmp = idx[i];
      s->word_num = 1;
      s->word[0] = tmp->wid;
#ifdef CONFIDENCE_MEASURE
      s->confidence[0] = pow(10, r->config->annotate.cm_alpha * (tmp->backscore - maxscore)) / sum;
#endif
      s->score = tmp->backscore;
      s->score_lm = 0.0;
      s->score_am = tmp->backscore;
      if (multigram_get_all_num(r->lm) > 0) {
	s->gram_id = multigram_get_gram_from_wid(s->word[0], r->lm);
      } else {
	s->gram_id = 0;
      }
    }
    /* free work area for sort */
    free(idx);

  } else {			/* only max is needed */

    /* prepare result storage */
    result_sentence_malloc(r, 1);
    r->result.sentnum = 1;
    s = &(r->result.sent[0]);
    s->word_num = 1;
    s->word[0] = best->wid;
#ifdef CONFIDENCE_MEASURE
    s->confidence[0] = 1.0 / sum;
#endif
    s->score = best->backscore;
    s->score_lm = 0.0;
    s->score_am = best->backscore;
    if (multigram_get_all_num(r->lm) > 0) {
      s->gram_id = multigram_get_gram_from_wid(s->word[0], r->lm);
    } else {
      s->gram_id = 0;
    }
  }

  /* copy as 1st pass result */
  memcpy(&(r->result.pass1), &(r->result.sent[0]), sizeof(Sentence));
  r->result.pass1.align = NULL;

  //callback_exec(CALLBACK_RESULT, r);
  //free(r->result.sent);
}


#ifdef DETERMINE

/** 
 * <JA>
 * 第１パスの途中データから早期確定可能かどうか判定する（実験）. tremax が
 * NULL のときは初期化する. 確定時は r->have_determine を TRUE にする. 
 *
 * @param r [i/o] 音声認識処理インスタンス
 * @param t [in] フレーム
 * @param tremax [in] 現在のフレーム上で最尤のトレリス単語
 * @param thres [in] 確定用のスコア閾値
 * @param countthres [in] 確定用の持続フレーム数の閾値
 *
 * @return 確定時は tremax を返す. 未確定時は NULL を返す. 
 * </JA>
 * <EN>
 * Try to Determine a word hypothesis before end of input on isolated
 * word recognition mode (EXPERIMENT).  Initialize if tremax is NULL.
 * Set r->have_determine to TRUE if determined.
 * 
 * @param r [i/o] recognition process instance
 * @param t [in] current frame
 * @param tremax [in] maximum scored trellis word on the current frame
 * @param thres [in] score threshold for determinization
 * @param countthres [in] frame duration threshold for determinization
 *
 * @return the tremax if determined, or NULL if not determined yet., 
 * </EN>
 */
static TRELLIS_ATOM *
determine_word(RecogProcess *r, int t, TRELLIS_ATOM *tremax, LOGPROB thres, int countthres)
{
  TRELLIS_ATOM *ret;
  WORD_ID w;

  //LOGPROB sum;
  //LOGPROB cm;

  int j;
  FSBeam *d;
  TOKEN2 *tk;
    
  if (tremax == NULL) {
    /* initialize */
    r->determine_count = 0;
    r->determine_maxnodescore = LOG_ZERO;
    r->determined = FALSE;
    r->determine_last_wid = WORD_INVALID;
    r->have_determine = FALSE;
    return NULL;
  }

  ret = NULL;

  /* get confidence score of the maximum word hypothesis */
/* 
 *   sum = 0.0;
 *   tre = recog->backtrellis->list;
 *   while (tre != NULL && tre->endtime == t) {
 *     sum += pow(10, recog->jconf->annotate.cm_alpha * (tre->backscore - tremax->backscore));
 *     tre = tre->next;
 *   }
 *   cm = 1.0 / sum;
 */

  /* determinization decision */
  w = tremax->wid;

  r->have_determine = FALSE;

  /* determine by score threshold from maximum node score to maximum word end node score */
  if (r->determine_last_wid == w && r->determine_maxnodescore - tremax->backscore <= thres) {
    r->determine_count++;
    if (r->determine_count > countthres) {
      if (r->determined == FALSE) {
	ret = tremax;
	r->determined = TRUE;
	r->have_determine = TRUE;
      }
    }
  } else {
    r->determine_count = 0;
  }

  //printf("determine: %d: %s: cm=%f, relscore=%f, count=%d, phase=%d\n", t, recog->model->winfo->woutput[w], cm, determine_maxnodescore - tremax->backscore, count, phase);
  r->determine_last_wid = w;

  /* update maximum node score here for next call, since
     the word path determination is always one frame later */
  d = &(r->pass1);
  r->determine_maxnodescore = LOG_ZERO;
  for (j = d->n_start; j <= d->n_end; j++) {
    tk = &(d->tlist[d->tn][d->tindex[d->tn][j]]);
    if (r->determine_maxnodescore < tk->score) r->determine_maxnodescore = tk->score;
  }

  return(ret);
}

/** 
 * <JA>
 * 単語認識時に，第１パスの処理中に早期確定する（実験）. 確定できた場合，
 * 第1パスの結果格納用エリア (r->result.pass1) に確定結果を格納する. 
 * また確定時は r->have_determine に TRUE が入る. 
 * 
 * @param r [in] 認識処理インスタンス
 * @param t [in] 現在の入力フレーム
 * </JA>
 * <EN>
 * Determine word hypothesis before end of input (EXPERIMENT).  When
 * determined, the determined word will be stored to the result area
 * for the 1st pass (r->result.pass1).  r->have_determine will be
 * set to TRUE when determinized.
 * 
 * @param r [in] recognition process instance
 * @param t [in] current input frame
 * </EN>
 */
static void
check_determine_word(RecogProcess *r, int t)
{
  TRELLIS_ATOM *tre;
  TRELLIS_ATOM *tremax;
  LOGPROB maxscore;

  /* bt->list is ordered by time frame */
  maxscore = LOG_ZERO;
  tremax = NULL;
  tre = r->backtrellis->list;
  while (tre != NULL && tre->endtime == t) {
    if (maxscore < tre->backscore) {
      maxscore = tre->backscore;
      tremax = tre;
    }
    tre = tre->next;
  }

  r->result.status = J_RESULT_STATUS_SUCCESS;
  r->result.num_frame = t;

  if (maxscore != LOG_ZERO) {
    //    if ((tre = determine_word(recog, t, tremax, 0.9, 17)) != NULL) {
    if ((tre = determine_word(r, t, tremax, r->config->pass1.determine_score_thres, r->config->pass1.determine_duration_thres)) != NULL) {
      r->result.pass1.word[0] = tremax->wid;
      r->result.pass1.word_num = 1;
      r->result.pass1.score = tremax->backscore;
      r->result.pass1.score_lm = 0.0;
      r->result.pass1.score_am = tremax->backscore;
      r->result.num_frame = t;
      //callback_exec(CALLBACK_RESULT_PASS1_DETERMINED, r);
    }
  }

  
}

#endif /* DETERMINE */

/** 
 * <JA>
 * 第１パスの処理中に，あるフレームまでのベストパスを表示する. 
 * 
 * @param r [i/o] 認識処理インスタンス
 * @param t [in] 現在の入力フレーム
 * </JA>
 * <EN>
 * Output the current best word sequence ending
 * at a specified time frame in the course of the 1st pass.
 * 
 * @param r [i/o] recognition process instance
 * @param t [in] current input frame
 * </EN>
 */
static void
bt_current_max(RecogProcess *r, int t)
{
  int wordlen;
  TRELLIS_ATOM *tre;
  TRELLIS_ATOM *tremax;
  LOGPROB maxscore;
  LOGPROB lscore;

  /* bt->list is ordered by time frame */
  maxscore = LOG_ZERO;
  tremax = NULL;
  tre = r->backtrellis->list;
  while (tre != NULL && tre->endtime == t) {
    if (maxscore < tre->backscore) {
      maxscore = tre->backscore;
      tremax = tre;
    }
    tre = tre->next;
  }

  r->result.status = J_RESULT_STATUS_SUCCESS;
  r->result.num_frame = t;

  if (maxscore == LOG_ZERO) {
    r->result.pass1.word_num = 0;
  } else {
    if (r->lmvar == LM_DFA_WORD) {
      r->result.pass1.word[0] = tremax->wid;
      r->result.pass1.word_num = 1;
      r->result.pass1.score = tremax->backscore;
      r->result.pass1.score_lm = 0.0;
      r->result.pass1.score_am = tremax->backscore;
    } else {
      lscore = trace_backptr(r->result.pass1.word, &wordlen, tremax, r->lm->winfo);
      r->result.pass1.word_num = wordlen;
      r->result.pass1.score = tremax->backscore;
      r->result.pass1.score_lm = lscore;
      r->result.pass1.score_am = tremax->backscore;
    }
  }
  //callback_exec(CALLBACK_RESULT_PASS1_INTERIM, r);
}

/** 
 * <JA>
 * 第１パスの処理中に，あるフレーム上の最尤単語を表示する(デバッグ用)
 * 
 * @param r [i/o] 認識処理インスタンス
 * @param t [in] 現在の入力フレーム
 * </JA>
 * <EN>
 * Output the current best word on a specified time frame in the course
 * of the 1st pass.
 * 
 * @param r [i/o] recognition process instance
 * @param t [in] current input frame
 * </EN>
 */
static void
bt_current_max_word(RecogProcess *r, int t)
{

  TRELLIS_ATOM *tre;
  TRELLIS_ATOM *tremax;
  LOGPROB maxscore;
  WORD_ID w;

  /* bt->list は時間順に格納されている */
  /* bt->list is order by time */
  maxscore = LOG_ZERO;
  tremax = NULL;
  tre = r->backtrellis->list;
  while (tre != NULL && tre->endtime == t) {
    if (maxscore < tre->backscore) {
      maxscore = tre->backscore;
      tremax = tre;
    }
    tre = tre->next;
  }

  if (maxscore != LOG_ZERO) {
    jlog("DEBUG: %3d: ",t);
    w = tremax->wid;
    jlog("\"%s [%s]\"(id=%d)",
	 r->lm->winfo->wname[w], r->lm->winfo->woutput[w], w);
    jlog(" [%d-%d] %f", tremax->begintime, t, tremax->backscore);
    w = tremax->last_tre->wid;
    if (w != WORD_INVALID) {
      jlog(" <- \"%s [%s]\"(id=%d)\n",
	       r->lm->winfo->wname[w], r->lm->winfo->woutput[w], w);
    } else {
      jlog(" <- bgn\n");
    }
  }
}


/* -------------------------------------------------------------------- */
/*                 ビーム探索中のトークンを扱うサブ関数                 */
/*                functions to handle hypothesis tokens                 */
/* -------------------------------------------------------------------- */

/** 
 * <JA>
 * 第１パスのビーム探索用の初期ワークエリアを確保する. 
 * 足りない場合は探索中に動的に伸長される. 
 *
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * @param n [in] 木構造化辞書のノード数
 * @param ntoken_init [in] 最初に確保するトークンの数
 * </JA>
 * <EN>
 * Allocate initial work area for beam search on the 1st pass.
 * If filled while search, they will be expanded on demand.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param n [in] number of nodes in lexicon tree
 * @param ntoken_init [in] number of token space to be allocated at first
 * </EN>
 */
static void
malloc_nodes(FSBeam *d, int n, int ntoken_init)
{
  d->totalnodenum = n;
  d->token     = (TOKENID *)mymalloc(sizeof(TOKENID) * d->totalnodenum);
  //d->maxtnum = ntoken_init;
  if (d->maxtnum < ntoken_init) d->maxtnum = ntoken_init;
  d->tlist[0]  = (TOKEN2 *)mymalloc(sizeof(TOKEN2) * d->maxtnum);
  d->tlist[1]  = (TOKEN2 *)mymalloc(sizeof(TOKEN2) * d->maxtnum);
  d->tindex[0] = (TOKENID *)mymalloc(sizeof(TOKENID) * d->maxtnum);
  d->tindex[1] = (TOKENID *)mymalloc(sizeof(TOKENID) * d->maxtnum);
  //d->expand_step = ntoken_step;
  d->nodes_malloced = TRUE;
  d->expanded = FALSE;
}

/** 
 * <JA>
 * 第１パスのビーム探索用のワークエリアを伸ばして再確保する. 
 *
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * </JA>
 * <EN>
 * Re-allocate work area for beam search on the 1st pass.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * </EN>
 */
static void
expand_tlist(FSBeam *d)
{
  d->maxtnum += d->expand_step;
  d->tlist[0]  = (TOKEN2 *)myrealloc(d->tlist[0],sizeof(TOKEN2) * d->maxtnum);
  d->tlist[1]  = (TOKEN2 *)myrealloc(d->tlist[1],sizeof(TOKEN2) * d->maxtnum);
  d->tindex[0] = (TOKENID *)myrealloc(d->tindex[0],sizeof(TOKENID) * d->maxtnum);
  d->tindex[1] = (TOKENID *)myrealloc(d->tindex[1],sizeof(TOKENID) * d->maxtnum);
  if (debug2_flag) jlog("STAT: token space expanded to %d\n", d->maxtnum);
  d->expanded = TRUE;
}

/** 
 * <EN>
 * Clear nodes for the next input.  Julius will call this function for
 * each input to re-set the work area for the beam search.  If the size
 * of tree lexicon has been changed since the last input, Julius will
 * free and re-allocate the work area.
 * </EN>
 * <JA>
 * ノード情報を初期化する. Julius は，木構造化辞書のサイズが直前の入力
 * 時と変化がないときは，この関数によってノード情報を初期化するだけで
 * よい. サイズが変更されているときはノードを開放・再確保する. 
 * </JA>
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param ntoken_step [in] required token step
 * 
 */
static void
prepare_nodes(FSBeam *d, int ntoken_step)
{
  d->tnum[0] = d->tnum[1] = 0;
  if (d->expand_step < ntoken_step) d->expand_step = ntoken_step;
}

/** 
 * <JA>
 * 第１パスのビーム探索用のワークエリアを全て解放する. 
 *
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * 
 * </JA>
 * <EN>
 * Free all the work area for beam search on the 1st pass.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * 
 * </EN>
 */
static void
free_nodes(FSBeam *d)
{
  if (d->nodes_malloced) {
    free(d->token);
    free(d->tlist[0]);
    free(d->tlist[1]);
    free(d->tindex[0]);
    free(d->tindex[1]);
    d->nodes_malloced = FALSE;
  }
}

/** 
 * <JA>
 * トークンスペースをリセットする. 
 * 
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * @param tt [in] ワークエリアID (0 または 1)
 * </JA>
 * <EN>
 * Reset the token space.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param tt [in] work area id (0 or 1)
 * </EN>
 */
static void
clear_tlist(FSBeam *d, int tt)
{
  d->tnum[tt] = 0;
}

/** 
 * <JA>
 * アクティブトークンリストをクリアする. 
 * 
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * @param tt [in] 直前のワークエリアID (0 または 1)
 * </JA>
 * <EN>
 * Clear the active token list.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param tt [in] work area id of previous frame (0 or 1)
 * </EN>
 */
static void
clear_tokens(FSBeam *d, int tt)
{
  int j;
  /* initialize active token list: only clear ones used in the last call */
  for (j=0; j<d->tnum[tt]; j++) {
    d->token[d->tlist[tt][j].node] = TOKENID_UNDEFINED;
  }
}

/** 
 * <JA>
 * トークンスペースから新たなトークンを取りだす. 
 * 
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * 
 * @return 新たに取り出されたトークンのID
 * </JA>
 * <EN>
 * Assign a new token from token space.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 
 * @return the id of the newly assigned token.
 * </EN>
 */
static TOKENID
create_token(FSBeam *d)
{
  TOKENID newid;
  int tn;
  tn = d->tn;
  newid = d->tnum[tn];
  d->tnum[tn]++;
  while (d->tnum[tn]>=d->maxtnum) expand_tlist(d);
  d->tindex[tn][newid] = newid;
#ifdef WPAIR
  /* initialize link */
  d->tlist[tn][newid].next = TOKENID_UNDEFINED;
#endif
  return(newid);
}


/** 
 * <JA>
 * @brief  木構造化辞書上のあるノードが，現在なんらかのトークンを
 * 保持しているかをチェックする. 
 *
 * WPAIR が定義されている場合，ノードは直前単語ごとに異なるトークンを複数
 * 保持する. この場合, 指定された単語IDを直前単語とするトークンが
 * そのノードに保持されているかどうかがチェックされる. すなわち，既にトークン
 * が存在しても，そのトークンの表すパスの直前単語が指定した単語と異なって
 * いれば未保持 (TOKENID_UNDEFINED) を返す. 
 * 
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * @param tt [in] 直前のワークエリアID (0 または 1)
 * @param node [in] ノード番号
 * @param wid [in] 直前単語のID (WPAIR定義時のみ有効, 他では無視される)
 *
 * @return そのノードが既に保持するトークン番号，無ければ TOKENID_UNDEFINED. 
 * </JA>
 * <EN>
 * @brief  Check if a node holds any token
 *
 * This function checks if a node on the tree lexicon already holds a token.
 *
 * If WPAIR is defined, a node has multiple tokens according to the previous
 * word context.  In this case, only token with the same previous word will be
 * checked.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param tt [in] work area id (0 or 1)
 * @param node [in] node id of lexicon tree
 * @param wid [in] word id of previous word (ignored if WPAIR is not defined)
 * 
 * @return the token id on the node, or TOKENID_UNDEFINED if no token has
 * been assigned in this frame.
 * </EN>
 */
static TOKENID
node_exist_token(FSBeam *d, int tt, int node, WORD_ID wid)
{
#ifdef WPAIR
  /* In word-pair mode, multiple tokens are assigned to a node as a list.
     so we have to search for tokens with same last word ID */
#ifdef WPAIR_KEEP_NLIMIT
  /* 1ノードごとに保持するtoken数の上限を設定 */
  /* tokenが無いが上限に達しているときは一番スコアの低いtokenを上書きする */
  /* N-best: limit number of assigned tokens to a node */
  int i = 0;
  TOKENID lowest_token = TOKENID_UNDEFINED;
#endif
  TOKENID tmp;
  for(tmp=d->token[node]; tmp != TOKENID_UNDEFINED; tmp=d->tlist[tt][tmp].next) {
    if (d->tlist[tt][tmp].last_tre->wid == wid) {
      return(tmp);
    }
#ifdef WPAIR_KEEP_NLIMIT
    if (lowest_token == TOKENID_UNDEFINED ||
	d->tlist[tt][lowest_token].score > d->tlist[tt][tmp].score)
      lowest_token = tmp;
    if (++i >= d->wpair_keep_nlimit) break;
#endif
  }
#ifdef WPAIR_KEEP_NLIMIT
  if (i >= d->wpair_keep_nlimit) { /* overflow, overwrite lowest score */
    return(lowest_token);
  } else {
    return(TOKENID_UNDEFINED);
  }
#else 
  return(TOKENID_UNDEFINED);
#endif
  
#else  /* not WPAIR */
  /* 1つだけ保持,これを常に上書き */
  /* Only one token is kept in 1-best mode (default), so
     simply return the ID */
  return(d->token[node]);
#endif
}

#ifdef DEBUG
/* tlist と token の対応をチェックする(debug) */
/* for debug: check tlist <-> token correspondence
   where  tlist[tt][tokenID].node = nodeID and
          token[nodeID] = tokenID
 */
static void
node_check_token(FSBeam *d, int tt)
{
  int i;
  for(i=0;i<d->tnum[tt];i++) {
    TOKENID node_exist_token_result;
    node_exist_token_result = node_exist_token(d, tt, d->tlist[tt][i].node, d->tlist[tt][i].last_tre->wid);
    if (node_exist_token_result != i) {
      jlog("ERROR: token %d not found on node %d\n", i, d->tlist[tt][i].node);
    }
  }
}
#endif



/* -------------------------------------------------------------------- */
/*       トークンをソートし 上位 N トークンを判別する (heap sort)       */
/*        Sort generated tokens and get N-best (use heap sort)          */
/* -------------------------------------------------------------------- */
/* ビームの閾値として上位 N 番目のスコアが欲しいだけであり，実際にソート
   される必要はない */
/* we only want to know the N-th score for determining beam threshold, so
   order is not considered here */

#define SD(A) tindex_local[A-1]	///< Index locater for sort_token_*()
#define SCOPY(D,S) D = S	///< Content copier for sort_token_*()
#define SVAL(A) (tlist_local[tindex_local[A-1]].score) ///< Score locater for sort_token_*()
#define STVAL (tlist_local[s].score) ///< Indexed score locater for sort_token_*()

/** 
 * <JA>
 * @brief  トークンスペースをスコアの大きい順にソートする. 
 *
 * heap sort を用いて現在のトークン集合をスコアの大きい順にソートする. 
 * 上位 @a neednum 個のトークンがソートされればそこで処理を終了する. 
 * 
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * @param neednum [in] 上位 @a neednum 個が得られるまでソートする
 * @param totalnum [in] トークンスペース内の有効なトークン数
 * </JA>
 * <EN>
 * @brief  Sort the token space upward by score.
 *
 * This function sort the whole token space in upward direction, according
 * to their accumulated score.
 * This function terminates sort as soon as the top
 * @a neednum tokens has been found.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param neednum [in] sort until top @a neednum tokens has been found
 * @param totalnum [in] total number of assigned tokens in the token space
 * </EN>
 */
static void
sort_token_upward(FSBeam *d, int neednum, int totalnum)
{
  int n,root,child,parent;
  TOKENID s;
  TOKEN2 *tlist_local;
  TOKENID *tindex_local;

  tlist_local = d->tlist[d->tn];
  tindex_local = d->tindex[d->tn];

  for (root = totalnum/2; root >= 1; root--) {
    SCOPY(s, SD(root));
    parent = root;
    while ((child = parent * 2) <= totalnum) {
      if (child < totalnum && SVAL(child) < SVAL(child+1)) {
	child++;
      }
      if (STVAL >= SVAL(child)) {
	break;
      }
      SCOPY(SD(parent), SD(child));
      parent = child;
    }
    SCOPY(SD(parent), s);
  }
  n = totalnum;
  while ( n > totalnum - neednum) {
    SCOPY(s, SD(n));
    SCOPY(SD(n), SD(1));
    n--;
    parent = 1;
    while ((child = parent * 2) <= n) {
      if (child < n && SVAL(child) < SVAL(child+1)) {
	child++;
      }
      if (STVAL >= SVAL(child)) {
	break;
      }
      SCOPY(SD(parent), SD(child));
      parent = child;
    }
    SCOPY(SD(parent), s);
  }
}

/** 
 * <JA>
 * @brief  トークンスペースをスコアの小さい順にソートする. 
 *
 * ビームのしきい値決定のために，heap sort を用いて
 * 現在のトークン集合をスコアの小さい順にソートする. 
 * 下位 @a neednum 個のトークンがソートされればそこで処理を終了する. 
 * 
 * @param d [i/o] 第1パス探索処理用ワークエリア
 * @param neednum [in] 下位 @a neednum 個が得られるまでソートする
 * @param totalnum [in] トークンスペース内の有効なトークン数
 * </JA>
 * <EN>
 * @brief  Sort the token space downward by score.
 *
 * This function sort the whole token space in downward direction, according
 * to their accumulated score for hypothesis pruning.
 *
 * This function terminates sort as soon as the bottom
 * @a neednum tokens has been found.
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param neednum [in] sort until bottom @a neednum tokens has been found
 * @param totalnum [in] total number of assigned tokens in the token space
 * </EN>
 */
static void
sort_token_downward(FSBeam *d, int neednum, int totalnum)
{
  int n,root,child,parent;
  TOKENID s;
  TOKEN2 *tlist_local;
  TOKENID *tindex_local;

  tlist_local = d->tlist[d->tn];
  tindex_local = d->tindex[d->tn];

  for (root = totalnum/2; root >= 1; root--) {
    SCOPY(s, SD(root));
    parent = root;
    while ((child = parent * 2) <= totalnum) {
      if (child < totalnum && SVAL(child) > SVAL(child+1)) {
	child++;
      }
      if (STVAL <= SVAL(child)) {
	break;
      }
      SCOPY(SD(parent), SD(child));
      parent = child;
    }
    SCOPY(SD(parent), s);
  }
  n = totalnum;
  while ( n > totalnum - neednum) {
    SCOPY(s, SD(n));
    SCOPY(SD(n), SD(1));
    n--;
    parent = 1;
    while ((child = parent * 2) <= n) {
      if (child < n && SVAL(child) > SVAL(child+1)) {
	child++;
      }
      if (STVAL <= SVAL(child)) {
	break;
      }
      SCOPY(SD(parent), SD(child));
      parent = child;
    }
    SCOPY(SD(parent), s);
  }
}


/* -------------------------------------------------------------------- */
/*             第１パス(フレーム同期ビームサーチ) メイン                */
/*           main routines of 1st pass (frame-synchronous beam search)  */
/* -------------------------------------------------------------------- */

/** 
 * <JA>
 * @brief  初期仮説の生成
 *
 * 初期仮説は，N-gramでは winfo->head_silwid に固定されている. DFA では
 * 文法上文頭にきうる単語すべてが初期仮説となる. 単語認識モードでは
 * 全ての単語が初期仮説となる. 
 *
 * 音響モデルが非multipathの場合，ここで最初のフレームの出力確率
 * 計算まで行われる. 
 * 
 * @param param [in] 入力ベクトル列情報(最初のフレームのみ必要)
 * @param r [in] 音声認識処理インスタンス
 * </JA>
 * <EN>
 * @brief  Generate initial hypotheses
 *
 * The initial hypothesis is: 1) winfo->head_silwid for N-gram, 2) all words
 * that can be at beginning of sentence for DFA, or 3) all words in dictionary
 * for isolated word recognition mode.
 *
 * If acoustic model is NOT a multi-path model, the output probabilities for
 * the first frame (t=0) will also be computed in this function.
 * 
 * @param param [in] input vectors (only the first frame will be used)
 * @param r [in] recognition process instance
 * </EN>
 */
static boolean
init_nodescore(HTK_Param *param, RecogProcess *r)
{
  WCHMM_INFO *wchmm;
  FSBeam *d;
  TOKENID newid;
  TOKEN2 *new;
  WORD_ID beginword;
  int node;
  int i;

  wchmm = r->wchmm;
  d = &(r->pass1);

  /* 初期仮説用単語履歴 */
  /* setup initial word context */
  if (r->config->successive.enabled) { /* sp segment mode */
    /* initial word context = last non-sp word of previous 2nd pass at last segment*/
    if (r->lmtype == LM_PROB) {
      if (r->sp_break_last_nword == wchmm->winfo->tail_silwid) {
	/* if end with silE, initialize as normal start of sentence */
	d->bos.wid = WORD_INVALID;
      } else {
	d->bos.wid = r->sp_break_last_nword;
      }
    } else {
      d->bos.wid = WORD_INVALID;
    }
  } else {			/* not sp segment mode */
    d->bos.wid = WORD_INVALID;	/* no context */
  }

  d->bos.begintime = d->bos.endtime = -1;

  /* ノード・トークンを初期化 */
  /* clear tree lexicon nodes and tokens */
  for(node = 0; node < d->totalnodenum; node++) {
    d->token[node] = TOKENID_UNDEFINED;
  }
  d->tnum[0] = d->tnum[1]  = 0;
  
#ifdef PASS1_IWCD
  /* 出力確率計算キャッシュを初期化 */
  /* initialize outprob cache */
  outprob_style_cache_init(wchmm);
#endif

  /* 初期仮説の作成: 初期単語の決定と初期トークンの生成 */
  /* initial word hypothesis */

  if (r->lmtype == LM_PROB) {

    if (r->config->successive.enabled) { /* sp segment mode */
      if (r->sp_break_last_word != WORD_INVALID) { /* last segment exist */
	/* 開始単語＝前のセグメント計算時の最後の最尤単語 */
	/* 文終了単語(silE,句点(IPAモデル))なら，silB で開始 */
	/* initial word = best last word hypothesis on the last segment */
	/* if silE or sp, begin with silB */
	/*if (is_sil(recog.sp_break_last_word, wchmm->winfo, wchmm->hmminfo)) {*/
	if (r->sp_break_last_word == wchmm->winfo->tail_silwid) {
	  beginword = wchmm->winfo->head_silwid;
	  d->bos.wid = WORD_INVALID;	/* reset initial word context */
	} else {
	  beginword = r->sp_break_last_word;
	}
      } else {
	/* initial segment: initial word set to silB */
	beginword = wchmm->winfo->head_silwid;
      }
    } else {			/* not sp segment mode */
      /* initial word fixed to silB */
      beginword = wchmm->winfo->head_silwid;
    }

#ifdef SP_BREAK_DEBUG
    jlog("DEBUG: startword=[%s], last_nword=[%s]\n",
	   (beginword == WORD_INVALID) ? "WORD_INVALID" : wchmm->winfo->wname[beginword],
	   (d->bos.wid == WORD_INVALID) ? "WORD_INVALID" : wchmm->winfo->wname[d->bos.wid]);
#endif
    /* create the first token at the first node of initial word */
    newid = create_token(d);
    new = &(d->tlist[d->tn][newid]);

    /* initial node = head node of the beginword */
    if (wchmm->hmminfo->multipath) {
      node = wchmm->wordbegin[beginword];
    } else {
      node = wchmm->offset[beginword][0];
    }

    /* set initial LM score */
    if (wchmm->state[node].scid != 0) {
      /* if initial node is on a factoring branch, use the factored score */
      new->last_lscore = max_successor_prob(wchmm, d->bos.wid, node);
    } else {
      /* else, set to 0.0 */
      new->last_lscore = 0.0;
    }
#ifdef FIX_PENALTY
    new->last_lscore = new->last_lscore * d->lm_weight;
#else
    new->last_lscore = new->last_lscore * d->lm_weight + d->lm_penalty;
#endif
    /* set initial word history */
    new->last_tre = &(d->bos);
    new->last_cword = d->bos.wid;
    if (wchmm->hmminfo->multipath) {
      /* set initial score using the initial LM score */
      new->score = new->last_lscore;
    } else {
      /* set initial score using the initial LM score and AM score of the first state */
      new->score = outprob_style(wchmm, node, d->bos.wid, 0, param) + new->last_lscore;
    }
    /* assign the initial node to token list */
    node_assign_token(d, node, newid);

  }

  if (r->lmtype == LM_DFA && r->lmvar == LM_DFA_GRAMMAR) {
  
    /* 初期仮説: 文法上文頭に接続しうる単語集合 */
    /* initial words: all words that can be begin of sentence grammatically */
    /* アクティブな文法に属する単語のみ許す */
    /* only words in active grammars are allowed to be an initial words */
    MULTIGRAM *m;
    int t,tb,te;
    WORD_ID iw;
    boolean flag;
    DFA_INFO *gdfa;

    gdfa = r->lm->dfa;

    flag = FALSE;
    /* for all active grammar */
    for(m = r->lm->grammars; m; m = m->next) {
      if (m->active) {
	tb = m->cate_begin;
	te = tb + m->dfa->term_num;
	for(t=tb;t<te;t++) {
	  /* for all word categories that belong to the grammar */
	  if (dfa_cp_begin(gdfa, t) == TRUE) {
	    /* if the category can appear at beginning of sentence, */
	    flag = TRUE;
	    for (iw=0;iw<gdfa->term.wnum[t];iw++) {
	      /* create the initial token at the first node of all words that belongs to the category */
	      i = gdfa->term.tw[t][iw];
	      if (wchmm->hmminfo->multipath) {
		node = wchmm->wordbegin[i];
	      } else {
		node = wchmm->offset[i][0];
	      }
	      /* in tree lexicon, words in the same category may share the same root node, so skip it if the node has already existed */
        TOKENID node_exist_token_result;
        node_exist_token_result = node_exist_token(d, d->tn, node, d->bos.wid);
	      if (node_exist_token_result != TOKENID_UNDEFINED) continue;
	      newid = create_token(d);
	      new = &(d->tlist[d->tn][newid]);
	      new->last_tre = &(d->bos);
#ifdef FIX_PENALTY
	      new->last_lscore = 0.0;
#else
	      new->last_lscore = d->penalty1;
#ifdef CLASS_NGRAM
	      /* add per-word penalty */
	      new->last_lscore += wchmm->winfo->cprob[i];
#endif
#endif
	      if (wchmm->hmminfo->multipath) {
		new->score = new->last_lscore;
	      } else {
		new->score = outprob_style(wchmm, node, d->bos.wid, 0, param) + new->last_lscore;
	      }
	      node_assign_token(d, node, newid);
	    }
	  }
	}
      }
    }
    if (!flag) {
      jlog("ERROR: init_nodescore: no initial state found in active DFA grammar\n");
      return FALSE;
    }
  }

  if (r->lmtype == LM_DFA && r->lmvar == LM_DFA_WORD) {
    /* アクティブな文法に属する単語のみ許す */
    /* only words in active grammars are allowed to be an initial words */
    MULTIGRAM *m;

    for(m = r->lm->grammars; m; m = m->next) {
      if (m->active) {
	for(i = m->word_begin; i < m->word_begin + m->winfo->num; i++) {
	  if (wchmm->hmminfo->multipath) {
	    node = wchmm->wordbegin[i];
	  } else {
	    node = wchmm->offset[i][0];
	  }
    TOKENID node_exist_token_result;
    node_exist_token_result = node_exist_token(d, d->tn, node, d->bos.wid);
	  if (node_exist_token_result != TOKENID_UNDEFINED) continue;
	  newid = create_token(d);
	  new = &(d->tlist[d->tn][newid]);
	  new->last_tre = &(d->bos);
	  new->last_lscore = 0.0;
	  if (wchmm->hmminfo->multipath) {
	    new->score = 0.0;
	  } else {
	    new->score = outprob_style(wchmm, node, d->bos.wid, 0, param);
	  }
	  node_assign_token(d, node, newid);
	}
      }
    }
  }

  return TRUE;

}

/******************************************************/
/* フレーム同期ビーム探索の実行 --- 最初のフレーム用  */
/* frame synchronous beam search --- first frame only */
/******************************************************/

/** 
 * <JA>
 * @brief  フレーム同期ビーム探索の初期化
 *
 * ここではビームサーチに用いるワークエリアの確保と初期化を行う. 
 * 初期化説の生成は init_nodescore() で行われる. 
 * 
 * @param param [in] 入力ベクトル列情報 (最初の１フレーム目のみ用いられる)
 * @param r [i/o] 音声認識処理インスタンス
 * </JA>
 * <EN>
 * @brief  Initialization of the frame synchronous beam search
 *
 * This function will initialize work area for the 1st pass.
 * Generation of initial hypotheses will be performed in init_nodescore().
 * 
 * @param param [in] input vectors (only the first frame will be used)
 * @param r [i/o] recognition process instance
 * </EN>
 *
 * @callergraph
 * @callgraph
 * 
 */
boolean
get_back_trellis_init(HTK_Param *param,	RecogProcess *r)
{
  WCHMM_INFO *wchmm;
  BACKTRELLIS *backtrellis;
  FSBeam *d;

  wchmm = r->wchmm;
  backtrellis = r->backtrellis;
  d = &(r->pass1);

  /* Viterbi演算用ワークエリアのスイッチャー tl,tn の初期値設定 */
  /* tn: このフレーム用ID   tl: １フレーム前のID */
  /* initialize switch tl, tn for Viterbi computation */
  /* tn: this frame  tl: last frame */
  d->tn = 0;
  d->tl = 1;

  /* 結果の単語トレリスを格納するバックトレリス構造体を初期化 */
  /* initialize backtrellis structure to store resulting word trellis */
  bt_prepare(backtrellis);

  /* 計算用ワークエリアを初期化 */
  /* initialize some data on work area */

  if (r->lmtype == LM_PROB) {
    d->lm_weight  = r->config->lmp.lm_weight;
    d->lm_penalty = r->config->lmp.lm_penalty;
  }
  if (r->lmtype == LM_DFA) {
    d->penalty1 = r->config->lmp.penalty1;
  }
#if defined(WPAIR) && defined(WPAIR_KEEP_NLIMIT)
  d->wpair_keep_nlimit = r->config->pass1.wpair_keep_nlimit;
#endif

  /* ワークエリアを確保 */
  /* malloc work area */
  /* 使用するトークン量 = viterbi時に遷移先となる状態候補の数
   * 予測: ビーム数 x 2 (自己遷移+次状態) + 木構造化辞書のルートノード数
   */
  /* assumed initial number of needed tokens: beam width x 2 (self + next trans.)
   * + root node on the tree lexicon (for inter-word trans.)
   */
  if (d->totalnodenum != wchmm->n) {
    free_nodes(d);
  }
  if (d->nodes_malloced == FALSE) {
    malloc_nodes(d, wchmm->n, r->trellis_beam_width * 2 + wchmm->startnum);
  }
  prepare_nodes(d, r->trellis_beam_width);
  
  /* 初期スコアを nodescore[tn] にセット */
  /* set initial score to nodescore[tn] */
  boolean init_nodescore_result;
  init_nodescore_result = init_nodescore(param, r);
  if (init_nodescore_result == FALSE) {
    jlog("ERROR: get_back_trellis_init: failed to set initial node scores\n");
    return FALSE;
  }

  sort_token_no_order(d, r->trellis_beam_width, &(d->n_start), &(d->n_end));

  /* 漸次出力を行なう場合のインターバルを計算 */
  /* set interval frame for progout */
  r->config->output.progout_interval_frame = (int)((float)r->config->output.progout_interval / ((float)param->header.wshift / 10000.0));

  if (r->config->successive.enabled) {
    /* ショートポーズセグメンテーション用パラメータの初期化 */
    /* initialize parameter for short pause segmentation */
    d->in_sparea = TRUE;		/* assume beginning is silence */
    r->am->mfcc->sparea_start = d->tmp_sparea_start = 0; /* set start frame to 0 */
#ifdef SP_BREAK_RESUME_WORD_BEGIN
    d->tmp_sp_break_last_word = WORD_INVALID;
#endif
    r->sp_break_last_word = WORD_INVALID;
    /* 最初のセグメント: 次の非ポーズフレームで第2パスへ移行しない */
    /* the first end of pause segment should be always silB, so
       skip the first segment */
    d->first_sparea = TRUE;
    r->sp_break_2_begin_word = WORD_INVALID;
  }

#ifdef DETERMINE
  if (r->lmvar == LM_DFA_WORD) {
    /* initialize */
    determine_word(r, 0, NULL, 0, 0);
  }
#endif

#ifdef SCORE_PRUNING
  d->score_pruning_threshold = LOG_ZERO;
  d->score_pruning_count = 0;
#endif

  return TRUE;
}

/*****************************************************/
/* frame synchronous beam search --- proceed 1 frame */
/* フレーム同期ビーム探索の実行 --- 1フレーム進める  */
/*****************************************************/

/** 
 * <EN>
 * Propagate a token to next node.
 * 
 * </EN>
 * <JA>
 * トークンを次ノードに伝搬する. 
 * 
 * </JA>
 * 
 * @param d [i/o] work area for 1st pass recognition processing
 * @param next_node [in] next node id
 * @param next_score [in] score when transmitted to the next node
 * @param last_tre [in] previous word context for the next node
 * @param last_cword [in] previous context-valid word for the next node
 * @param last_lscore [in] LM score to be propagated
 * 
 */
static void
propagate_token(FSBeam *d, int next_node, LOGPROB next_score, TRELLIS_ATOM *last_tre, WORD_ID last_cword, LOGPROB last_lscore)
{
  TOKEN2 *tknext;
  TOKENID tknextid;

  /* does not propagate invalid token */
  if (next_score <= LOG_ZERO) return;

  TOKENID node_exist_token_result;
  node_exist_token_result = node_exist_token(d, d->tn, next_node, last_tre->wid);
  if ((tknextid = node_exist_token_result) != TOKENID_UNDEFINED) {
    /* 遷移先ノードには既に他ノードから伝搬済み: スコアが高いほうを残す */
    /* the destination node already has a token: compare score */
    tknext = &(d->tlist[d->tn][tknextid]);
    if (tknext->score < next_score) {
      /* その遷移先ノードが持つトークンの内容を上書きする(新規トークンは作らない) */
      /* overwrite the content of existing destination token: not create a new token */
      tknext->last_tre = last_tre; /* propagate last word info */
      tknext->last_cword = last_cword; /* propagate last context word info */
      tknext->last_lscore = last_lscore; /* set new LM score */
      tknext->score = next_score; /* set new score */
    }
  } else {
    /* 遷移先ノードは未伝搬: 新規トークンを作って割り付ける */
    /* token unassigned: create new token and assign */
    tknextid = create_token(d); /* get new token */
    tknext = &(d->tlist[d->tn][tknextid]);
    tknext->last_tre = last_tre; /* propagate last word info */
    tknext->last_cword = last_cword; /* propagate last context word info */
    tknext->last_lscore = last_lscore;
    tknext->score = next_score; /* set new score */
    node_assign_token(d, next_node, tknextid); /* assign this new token to the next node */
  }
}

/** 
 * <JA>
 * 単語内のあるノード間の遷移を行う. 
 * 
 * @param wchmm [in] 木構造化辞書
 * @param d [i/o] 第1パスワークエリア
 * @param tk_ret [i/o] 伝搬元のトークン（内部でポインタ更新時は上書き）
 * @param j [in] @a tk_ret の元のトークンリストのID
 * @param next_node [in] 遷移先のノード番号
 * @param next_a [in] 遷移確率
 * </JA>
 * <EN>
 * Word-internal transition for a set of nodes.
 * 
 * @param wchmm [in] tree lexicon
 * @param d [i/o] work area for the 1st pass
 * @param tk_ret [in] source token (if pointer updated, overwrite this)
 * @param j [in] the token ID of @a tk_ret
 * @param next_node [in] id of next node
 * @param next_a [in] transition probability
 * 
 * </EN>
 */
static void
beam_intra_word_core(WCHMM_INFO *wchmm, FSBeam *d, TOKEN2 **tk_ret, int j, int next_node, LOGPROB next_a)
{
  int node; ///< Temporal work to hold the current node number on the lexicon tree
  LOGPROB tmpsum;
  LOGPROB ngram_score_cache;
  TOKEN2 *tk;

  tk = *tk_ret;

  node = tk->node;

  /* now, 'node' is the source node, 'next_node' is the destication node,
     and ac-> holds transition probability */
  /* tk->score is the accumulated score at the 'node' on previous frame */
  
  /******************************************************************/
  /* 2.1.1 遷移先へのスコア計算(遷移確率＋言語スコア)               */
  /*       compute score of destination node (transition prob + LM) */
  /******************************************************************/
  tmpsum = tk->score + next_a;
  ngram_score_cache = LOG_ZERO;
  /* the next score at 'next_node' will be computed on 'tmpsum', and
     the new LM probability (if updated) will be stored on 'ngram_score_cache' at below */
  
  if (!wchmm->category_tree) {
    /* 言語スコア factoring:
       arcが自己遷移でない単語内の遷移で，かつ遷移先にsuccessorリスト
       があれば，lexicon tree の分岐部分の遷移である */
    /* LM factoring:
       If this is not a self transition and destination node has successor
       list, this is branching transition
    */
    if (next_node != node) {
      if (wchmm->state[next_node].scid != 0
#ifdef UNIGRAM_FACTORING
	  /* 1-gram factoring 使用時は, 複数で共有される枝では
	     wchmm->state[node].scid は負の値となり，その絶対値を
	     添字として wchmm->fscore[] に単語集合の1-gramの最大値が格納
	     されている. 末端の枝(複数単語で共有されない)では，
	     wchmm->state[node].scid は正の値となり，
	     １単語を sc として持つのでそこで正確な2-gramを計算する */
	  /* When uni-gram factoring,
	     wchmm->state[node].scid is below 0 for shared branches.
	     In this case the maximum uni-gram probability for sub-tree
	     is stored in wchmm->fscore[- wchmm->state[node].scid].
	     Leaf branches (with only one successor word): the scid is
	     larger than 0, and has
	     the word ID in wchmm->sclist[wchmm->state[node].scid].
	     So precise 2-gram is computed in this point */
#endif
	  ){
	
	if (wchmm->lmtype == LM_PROB) {
	  /* ここで言語モデル確率を更新する */
	  /* LM value should be update at this transition */
	  /* N-gram確率からfactoring 値を計算 */
	  /* compute new factoring value from N-gram probabilities */
#ifdef FIX_PENALTY
	  /* if at the beginning of sentence, not add lm_penalty */
	  if (tk->last_cword == WORD_INVALID) {
	    ngram_score_cache = max_successor_prob(wchmm, tk->last_cword, next_node) * d->lm_weight;
	  } else {
	    ngram_score_cache = max_successor_prob(wchmm, tk->last_cword, next_node) * d->lm_weight + d->lm_penalty;
	  }
#else
	  ngram_score_cache = max_successor_prob(wchmm, tk->last_cword, next_node) * d->lm_weight + d->lm_penalty;
#endif
	  /* スコアの更新: tk->last_lscore に単語内での最後のfactoring値が
	     入っているので, それをスコアから引いてリセットし, 新たなスコアを
	     セットする */
	  /* update score: since 'tk->last_lscore' holds the last LM factoring
	     value in this word, we first remove the score from the current
	     score, and then add the new LM value computed above. */
	  tmpsum -= tk->last_lscore;
	  tmpsum += ngram_score_cache;
	}
	
	if (wchmm->lmtype == LM_DFA && wchmm->lmvar == LM_DFA_GRAMMAR) {
	  /* 文法を用いる場合, カテゴリ単位の木構造化がなされていれば,
	     接続制約は単語間遷移のみで扱われるので，factoring は必要ない. 
	     カテゴリ単位木構造化が行われない場合, 文法間の接続制約はここ
	     で factoring で行われることになる. */
	  /* With DFA, we use category-pair constraint extracted from the DFA
	     at this 1st pass.  So if we compose a tree lexicon per word's
	     category, the each category tree has the same connection
	     constraint and thus we can apply the constraint on the cross-word
	     transition.  This per-category tree lexicon is enabled by default,
	     and in the case the constraint will be applied at the word end.
	     If you disable per-category tree lexicon by undefining
	     'CATEGORY_TREE', the word-pair contrained will be applied in a
	     factoring style at here.
	  */
	  
	  /* 決定的factoring: 直前単語に対して,sub-tree内にカテゴリ対制約上
	     接続しうる単語が１つもなければ, この遷移は不可 */
	  /* deterministic factoring in grammar mode:
	     transition disabled if there are totally no sub-tree word that can
	     grammatically (in category-pair constraint) connect
	     to the previous word.
	  */
	  if (!can_succeed(wchmm, tk->last_tre->wid, next_node)) {
	    tmpsum = LOG_ZERO;
	  }
	}
	
      }
    }
  }
  /* factoring not needed when DFA mode and uses category-tree */
  
  /****************************************/
  /* 2.1.2 遷移先ノードへトークン伝搬     */
  /*       pass token to destination node */
  /****************************************/
  
  if (ngram_score_cache == LOG_ZERO) ngram_score_cache = tk->last_lscore;
  propagate_token(d, next_node, tmpsum, tk->last_tre, tk->last_cword, ngram_score_cache);
  
  if (d->expanded) {
    /* if work area has been expanded at 'create_token()' above,
       the inside 'realloc()' will destroy the pointers.
       so, reset local pointers from token index */
    tk = &(d->tlist[d->tl][d->tindex[d->tl][j]]);
    d->expanded = FALSE;
  }
  
  *tk_ret = tk;

}

/** 
 * <JA>
 * 単語内遷移を行う. 
 * 
 * @param wchmm [in] 木構造化辞書
 * @param d [i/o] 第1パスワークエリア
 * @param tk_ret [i/o] 伝搬元のトークン（内部でポインタ更新時は上書き）
 * @param j [in] @a tk_ret の元のトークンリストのID
 * </JA>
 * <EN>
 * Word-internal transition.
 * 
 * @param wchmm [in] tree lexicon
 * @param d [i/o] work area for the 1st pass
 * @param tk_ret [in] source token (if pointer updated, overwrite this)
 * @param j [in] the token ID of @a tk_ret
 * 
 * </EN>
 */
static void
beam_intra_word(WCHMM_INFO *wchmm, FSBeam *d, TOKEN2 **tk_ret, int j)
{
  A_CELL2 *ac; ///< Temporal work to hold the next states of a node
  TOKEN2 *tk;
  int node;
  int k;

  tk = *tk_ret;

  node = tk->node;

  if (wchmm->self_a[node] != LOG_ZERO) {
    beam_intra_word_core(wchmm, d, tk_ret, j, node, wchmm->self_a[node]);
  }

  if (wchmm->next_a[node] != LOG_ZERO) {
    beam_intra_word_core(wchmm, d, tk_ret, j, node+1, wchmm->next_a[node]);
  }

  for(ac=wchmm->ac[node];ac;ac=ac->next) {
    for(k=0;k<ac->n;k++) {
      beam_intra_word_core(wchmm, d, tk_ret, j, ac->arc[k], ac->a[k]);
    }
  }
}

/**************************/
/* 2.2. トレリス単語保存  */
/*      save trellis word */
/**************************/
/** 
 * <JA>
 * トークンからトレリス単語を保存する. 
 * 
 * @param bt [i/o] バックトレリス構造体
 * @param wchmm [in] 木構造化辞書
 * @param tk [in] 単語末端に到達しているトークン
 * @param t [in] 現在の時間フレーム
 * @param final_for_multipath [in] 入力最後の１回処理時 TRUE
 * 
 * @return 新たに格納されたトレリス単語へのポインタ
 * </JA>
 * <EN>
 * Store a new trellis word on the token.
 *
 * @param bt [i/o] backtrellis data to save it
 * @param wchmm [in] tree lexicon
 * @param tk [in] source token at word edge
 * @param t [in] current time frame
 * @param final_for_multipath [in] TRUE if this is final frame
 *
 * @return pointer to the newly stored trellis word.
 * </EN>
 */
static TRELLIS_ATOM *
save_trellis(BACKTRELLIS *bt, WCHMM_INFO *wchmm, TOKEN2 *tk, int t, boolean final_for_multipath)
{
  TRELLIS_ATOM *tre;
  int sword;
 
  sword = wchmm->stend[tk->node];

  /* この遷移元の単語終端ノードは「直前フレームで」生き残ったノード. 
     (「このフレーム」でないことに注意！！)
     よってここで, 時間(t-1) を単語終端とするトレリス上の単語仮説
     (TRELLIS_ATOM)として，単語トレリス構造体に保存する. */
  /* This source node (= word end node) has been survived in the
     "last" frame (notice: not "this" frame!!).  So this word end
     is saved here to the word trellis structure (BACKTRELLIS) as a
     trellis word (TRELLIS_ATOM) with end frame (t-1). */
  tre = bt_new(bt);
  tre->wid = sword;		/* word ID */
  tre->backscore = tk->score; /* log score (AM + LM) */
  tre->begintime = tk->last_tre->endtime + 1; /* word beginning frame */
  tre->endtime   = t-1;	/* word end frame */
  tre->last_tre  = tk->last_tre; /* link to previous trellis word */
  tre->lscore    = tk->last_lscore;	/* log LM score  */
  bt_store(bt, tre); /* save to backtrellis */
#ifdef WORD_GRAPH
  if (tre->last_tre != NULL) {
    /* mark to indicate that the following words was survived in beam */
    tre->last_tre->within_context = TRUE;
  }
  if (final_for_multipath) {
    /* last node */
    if (tre->wid == wchmm->winfo->tail_silwid) {
      tre->within_context = TRUE;
    }
  }
#endif /* WORD_GRAPH */

  return tre;
}
      

/** 
 * <JA>
 * 単語末トークンからの単語間遷移. 
 * 
 * @param wchmm [in] 木構造化辞書
 * @param d [i/o] 第1パスワークエリア
 * @param tk_ret [in] 伝搬元の単語末トークン
 * @param tre [in] @a tk_ret から生成されたトレリス単語
 * @param j [in] @a tk_ret の元のトークンリストのID
 * </JA>
 * <EN>
 * Cross-word transition processing from word-end token.
 * 
 * @param wchmm [in] tree lexicon
 * @param d [i/o] work area for the 1st pass
 * @param tk_ret [in] source token where the propagation is from
 * @param tre [in] the trellis word generated from @a tk_ret
 * @param j [in] the token ID of @a tk_ret
 * </EN>
 */
static void
beam_inter_word(WCHMM_INFO *wchmm, FSBeam *d, TOKEN2 **tk_ret, TRELLIS_ATOM *tre, int j)
{
  A_CELL2 *ac;
  TOKEN2 *tk;
  int sword;
  int node, next_node;
  LOGPROB *iwparray; ///< Temporal pointer to hold inter-word cache array
  int stid;
#ifdef UNIGRAM_FACTORING
  int isoid; ///< Temporal work to hold isolated node
#endif
  LOGPROB tmpprob, tmpsum, ngram_score_cache;
  int k;
  WORD_ID last_word;

  tk = *tk_ret;
 
  node = tk->node;
  sword = wchmm->stend[node];
  last_word = wchmm->winfo->is_transparent[sword] ? tk->last_cword : sword;

  if (wchmm->lmtype == LM_PROB) {

    /* 遷移元単語が末尾単語の終端なら，どこへも遷移させない */
    /* do not allow transition if the source word is end-of-sentence word */
    if (sword == wchmm->winfo->tail_silwid) return;

#ifdef UNIGRAM_FACTORING
#ifndef WPAIR
    /* あとで共有単語先頭ノードに対して単語間遷移をまとめて計算するため，*/
    /* このループ内では最大尤度を持つ単語終端ノードを記録しておく */
    /* here we will record the best wordend node of maximum likelihood
       at this frame, to compute later the cross-word transitions toward
       shared factoring word-head node */
    tmpprob = tk->score;
    if (!wchmm->hmminfo->multipath) tmpprob += wchmm->wordend_a[sword];
    if (d->wordend_best_score < tmpprob) {
      d->wordend_best_score = tmpprob;
      d->wordend_best_node = node;
      d->wordend_best_tre = tre;
      d->wordend_best_last_cword = tk->last_cword;
    }
#endif
#endif
    
    /* N-gramにおいては常に全単語の接続を考慮する必要があるため，
       ここで単語間の言語確率値をすべて計算しておく. 
       キャッシュは max_successor_prob_iw() 内で考慮. */
    /* As all words are possible to connect in N-gram, we first compute
       all the inter-word LM probability here.
       Cache is onsidered in max_successor_prob_iw(). */
    if (wchmm->winfo->is_transparent[sword]) {
      iwparray = max_successor_prob_iw(wchmm, tk->last_cword);
    } else {
      iwparray = max_successor_prob_iw(wchmm, sword);
    }
  }

  /* すべての単語始端ノードに対して以下を実行 */
  /* for all beginning-of-word nodes, */
  /* wchmm->startnode[0..stid-1] ... 単語始端ノードリスト */
  /* wchmm->startnode[0..stid-1] ... list of word start node (shared) */
  for (stid = wchmm->startnum - 1; stid >= 0; stid--) {
    next_node = wchmm->startnode[stid];
    if (wchmm->hmminfo->multipath) {
      if (wchmm->lmtype == LM_PROB) {
	/* connection to the head silence word is not allowed */
	if (wchmm->wordbegin[wchmm->winfo->head_silwid] == next_node) continue;
      }
    }
    
    /*****************************************/
    /* 2.3.1. 単語間言語制約を適用           */
    /*        apply cross-word LM constraint */
    /*****************************************/
	
    if (wchmm->lmtype == LM_PROB) {
      /* N-gram確率を計算 */
      /* compute N-gram probability */
#ifdef UNIGRAM_FACTORING
      /* wchmm,start2isolate[0..stid-1] ... ノードを共有しない単語は
	 その通しID, 共有する(キャッシュの必要のない)単語は -1 */
      /* wchmm->start2isolate[0..stid-1] ... isolate ID for
	 beginning-of-word state.  value: -1 for states that has
	 1-gram factoring value (share nodes with some other words),
	 and ID for unshared words
      */
      isoid = wchmm->start2isolate[stid];
#ifdef WPAIR
      /* Efficient cross-word LM handling should be disabled for
	 word-pair approximation */
      if (isoid == -1) {
	tmpprob = wchmm->fscore[- wchmm->state[next_node].scid];
      } else {
	tmpprob = iwparray[isoid];
      }
#else  /* ~WPAIR */
      /* 1-gram factoring における単語間言語確率キャッシュの効率化:
	 1-gram factoring は単語履歴に依存しないので，
	 ここで参照する factoring 値の多くは
	 wchmm->fscore[] に既に格納され, 探索中も不変である. 
	 よって計算が必要な単語(どの単語ともノードを共有しない単語)
	 についてのみ iwparray[] で計算・キャッシュする.  */
      /* Efficient cross-word LM cache:
	 As 1-gram factoring values are independent of word context,
	 they remain unchanged while search.  So, in cross-word LM
	 computation, beginning-of-word states which share nodes with
	 others and has factoring value in wchmm does not need cache.
	 So only the unshared beginning-of-word states are computed and
	 cached here in iwparray[].
      */
      /* 計算が必要でない単語先頭ノードはパスをまとめて後に計算するので
	 ここではスキップ */
      /* the shared nodes will be computed afterward, so just skip them
	 here */
      if (isoid == -1) continue;
      tmpprob = iwparray[isoid];
#endif /* ~WPAIR */
#else  /* ~UNIGRAM_FACTORING */
      tmpprob = iwparray[stid];
#endif
    }

    /* 遷移先の単語が先頭単語なら遷移させない. 
       これは wchmm.c で該当単語に stid を割り振らないことで対応
       しているので，ここでは何もしなくてよい */
    /* do not allow transition if the destination word is
       beginning-of-sentence word.  This limitation is realized by
       not assigning 'stid' for the word in wchmm.c, so we have nothing
       to do here.
    */
    
    if (wchmm->category_tree) {
      /* 文法の場合, 制約は決定的: カテゴリ対制約上許されない場合は遷移させない */
      /* With DFA and per-category tree lexicon,
	 LM constraint is deterministic:
	 do not allow transition if the category connection is not allowed
	 (with category tree, constraint can be determined on top node) */
      if (dfa_cp(wchmm->dfa, wchmm->winfo->wton[sword], wchmm->winfo->wton[wchmm->start2wid[stid]]) == FALSE) continue;
    }

    /*******************************************************************/
    /* 2.3.2. 遷移先の単語先頭へのスコア計算(遷移確率＋言語スコア)     */
    /*        compute score of destination node (transition prob + LM) */
    /*******************************************************************/
    tmpsum = tk->score;
    if (!wchmm->hmminfo->multipath) tmpsum += wchmm->wordend_a[sword];

    /* 'tmpsum' now holds outgoing score from the wordend node */
    if (wchmm->lmtype == LM_PROB) {
      /* 言語スコアを追加 */
      /* add LM score */
      ngram_score_cache = tmpprob * d->lm_weight + d->lm_penalty;
      tmpsum += ngram_score_cache;
      if (wchmm->winfo->is_transparent[sword] && wchmm->winfo->is_transparent[tk->last_cword]) {
	    
	tmpsum += d->lm_penalty_trans;
      }
    }
    if (wchmm->lmtype == LM_DFA) {
      /* grammar: 単語挿入ペナルティを追加 */
      /* grammar: add insertion penalty */
      ngram_score_cache = d->penalty1;
#ifdef CLASS_NGRAM
      /* add per-word penalty of last word as delayed penalty */
      ngram_score_cache += wchmm->winfo->cprob[last_word];
#endif
      tmpsum += ngram_score_cache;

      /* grammar: deterministic factoring (in case category-tree not enabled) */
      if (!wchmm->category_tree) {
	if (!can_succeed(wchmm, sword, next_node)) {
	  tmpsum = LOG_ZERO;
	}
      }
    }
    
    /*********************************************************************/
    /* 2.3.3. 遷移先ノードへトークン伝搬(単語履歴情報は更新)             */
    /*        pass token to destination node (updating word-context info */
    /*********************************************************************/

    if (wchmm->hmminfo->multipath) {
      /* since top node has no ouput, we should go one more step further */
      if (wchmm->self_a[next_node] != LOG_ZERO) {
	propagate_token(d, next_node, tmpsum + wchmm->self_a[next_node], tre, last_word, ngram_score_cache);
	if (d->expanded) {
	  /* if work area has been expanded at 'create_token()' above,
	     the inside 'realloc()' will destroy the pointers.
	     so, reset local pointers from token index */
	  tk = &(d->tlist[d->tn][d->tindex[d->tn][j]]);
	  d->expanded = FALSE;
	}
      }
      if (wchmm->next_a[next_node] != LOG_ZERO) {
	propagate_token(d, next_node+1, tmpsum + wchmm->next_a[next_node], tre, last_word, ngram_score_cache);
	if (d->expanded) {
	  /* if work area has been expanded at 'create_token()' above,
	     the inside 'realloc()' will destroy the pointers.
	     so, reset local pointers from token index */
	  tk = &(d->tlist[d->tn][d->tindex[d->tn][j]]);
	  d->expanded = FALSE;
	}
      }
      for(ac=wchmm->ac[next_node];ac;ac=ac->next) {
	for(k=0;k<ac->n;k++) {
	  propagate_token(d, ac->arc[k], tmpsum + ac->a[k], tre, last_word, ngram_score_cache);
	  if (d->expanded) {
	    /* if work area has been expanded at 'create_token()' above,
	       the inside 'realloc()' will destroy the pointers.
	       so, reset local pointers from token index */
	    tk = &(d->tlist[d->tn][d->tindex[d->tn][j]]);
	    d->expanded = FALSE;
	  }
	}
      }
    } else {
      propagate_token(d, next_node, tmpsum, tre, last_word, ngram_score_cache);
      if (d->expanded) {
	/* if work area has been expanded at 'create_token()' above,
	   the inside 'realloc()' will destroy the pointers.
	   so, reset local pointers from token index */
	tk = &(d->tlist[d->tl][d->tindex[d->tl][j]]);
	d->expanded = FALSE;
      }
    }
	
  }	/* end of next word heads */

  *tk_ret = tk;


} /* end of cross-word processing */


#ifdef UNIGRAM_FACTORING

/** 
 * <JA>
 * @brief  1-gram factoring 用単語間遷移の追加処理
 * 
 * 1-gram factoring 使用時は、複数の単語間で共有されている
 * 単語先頭のノード (= factoring されている単語先頭ノード) については、
 * すべて、最も尤度の高い単語終端からの遷移が選択される。この性質を
 * 用いて、この関数ではあらかじめ求められた最も尤度の高い単語終端
 * から、ファクタリングされた単語先頭ノードへの遷移計算を一度に行う。
 * 
 * @param wchmm [in] 木構造化辞書
 * @param d [in] 第1パス用ワークエリア
 * </JA>
 * <EN>
 * @brief  Additional cross-word transition processing for 1-gram factoring.
 *
 * When using 1-gram factoring, The word end of maximum likelihood will be
 * chosen at cross-word viterbi for factored word-head node, since the
 * LM factoring value is independent of the context.  This function performs
 * viterbi processing to the factored word-head nodes from the maximum
 * likelihood word end previously stored.
 * 
 * @param wchmm [in] tree lexicon
 * @param d [in] work area for the 1st pass
 * </EN>
 */
static void
beam_inter_word_factoring(WCHMM_INFO *wchmm, FSBeam *d)
{
  int sword;
  int node, next_node;
  int stid;
  LOGPROB tmpprob, tmpsum, ngram_score_cache;
  A_CELL2 *ac;
  int j;
  WORD_ID last_word;

  node = d->wordend_best_node;
  sword = wchmm->stend[node];
  last_word = wchmm->winfo->is_transparent[sword] ? d->wordend_best_last_cword : sword;

  for (stid = wchmm->startnum - 1; stid >= 0; stid--) {
    next_node = wchmm->startnode[stid];
    /* compute transition from 'node' at end of word 'sword' to 'next_node' */
    /* skip isolated words already calculated in the above main loop */
    if (wchmm->start2isolate[stid] != -1) continue;
    /* rest should have 1-gram factoring score at wchmm->fscore */
    if (wchmm->state[next_node].scid >= 0) {
      j_internal_error("get_back_trellis_proceed: scid mismatch at 1-gram factoring of shared states\n");
    }
    tmpprob = wchmm->fscore[- wchmm->state[next_node].scid];
    ngram_score_cache = tmpprob * d->lm_weight + d->lm_penalty;
    tmpsum = d->wordend_best_score;
    tmpsum += ngram_score_cache;
    if (wchmm->winfo->is_transparent[sword] && wchmm->winfo->is_transparent[d->wordend_best_last_cword]) {
      tmpsum += d->lm_penalty_trans;
    }
#ifdef SCORE_PRUNING
    if (tmpsum < d->score_pruning_threshold) {
      d->score_pruning_count++;
      continue;
    }
#endif
    if (wchmm->hmminfo->multipath) {
      /* since top node has no ouput, we should go one more step further */
      if (wchmm->self_a[next_node] != LOG_ZERO) {
	propagate_token(d, next_node, tmpsum + wchmm->self_a[next_node], d->wordend_best_tre, last_word, ngram_score_cache);
	if (d->expanded) {
	  d->expanded = FALSE;
	}
      }
      if (wchmm->next_a[next_node] != LOG_ZERO) {
	propagate_token(d, next_node+1, tmpsum + wchmm->next_a[next_node], d->wordend_best_tre, last_word, ngram_score_cache);
	if (d->expanded) {
	  d->expanded = FALSE;
	}
      }
      for(ac=wchmm->ac[next_node];ac;ac=ac->next) {
	for(j=0;j<ac->n;j++) {
	  propagate_token(d, ac->arc[j], tmpsum + ac->a[j], d->wordend_best_tre, last_word, ngram_score_cache);
	  if (d->expanded) {
	    d->expanded = FALSE;
	  }
	}
      }
      
    } else {
      propagate_token(d, next_node, tmpsum, d->wordend_best_tre, last_word, ngram_score_cache);
      if (d->expanded) {
	d->expanded = FALSE;
      }
    }

  }
}

#endif /* UNIGRAM_FACTORING */


/** 
 * <JA>
 * @brief  フレーム同期ビーム探索を進行する. 
 *
 * 与えられた１フレーム分，探索処理を進める. また，フレーム内に残った
 * 単語を単語トレリス構造体に保存する. ショートポーズセグメンテーション時
 * はセグメント終了の判断もこの中から呼び出される. 
 * 
 * @param t [in] 現在のフレーム (このフレームについて計算が進められる)
 * @param param [in] 入力ベクトル列構造体 (@a t 番目のフレームのみ用いられる)
 * @param r [in] 認識処理インスタンス
 * @param final_for_multipath [i/o] 入力最後のフレームを処理するときに TRUE
 * 
 * @return TRUE (通常どおり終了) あるいは FALSE (ここで探索を中断する
 * 場合: 逐次デコーディング時にショートポーズ区間を検出したか，ビーム内の
 * アクティブノード数が0になったとき)
 * </JA>
 * <EN>
 * @brief  Frame synchronous beam search: proceed for 2nd frame and later.
 *
 * This is the main function of beam search on the 1st pass.  Given a
 * input vector of a frame, it proceeds the computation for the one frame,
 * and store the words survived in the beam width to the word trellis
 * structure.  get_back_trellis_init() should be used for the first frame.
 * For detailed procedure, please see the comments in this
 * function.
 * 
 * @param t [in] current frame to be computed in @a param
 * @param param [in] input vector structure (only the vector at @a t will be used)
 * @param r [in] recognition process instance
 * @param final_for_multipath [i/o] TRUE if this is last frame of an input
 * 
 * @return TRUE if processing ended normally, or FALSE if the search was
 * terminated (in case of short pause segmentation in successive decoding
 * mode, or active nodes becomes zero).
 * </EN>
 *
 * @callergraph
 * @callgraph
 * 
 */
boolean
get_back_trellis_proceed(int t, HTK_Param *param, RecogProcess *r, boolean final_for_multipath)
{
  /* local static work area for get_back_trellis_proceed() */
  /* these are local work area and need not to be kept for another call */
  TRELLIS_ATOM *tre; ///< Local workarea to hold the generated trellis word
  int node; ///< Temporal work to hold the current node number on the lexicon tree
  int lmtype, lmvar;

  WCHMM_INFO *wchmm;
  FSBeam *d;
  int j;
  TOKEN2  *tk;
  LOGPROB minscore;

  /* local copied variables */
  int tn, tl;

  /* store pointer to local for rapid access */
  wchmm = r->wchmm;
  d = &(r->pass1);
  

  lmtype = r->lmtype;
  lmvar  = r->lmvar;

  /*********************/
  /* 1. 初期化         */
  /*    initialization */
  /*********************/

  /* tl と tn を入れ替えて作業領域を切り替え */
  /* tl (= 直前の tn) は直前フレームの結果を持つ */
  /* swap tl and tn to switch work buffer */
  /* tl (= last tn) holds result of the previous frame */
  d->tl = d->tn;
  if (d->tn == 0) d->tn = 1; else d->tn = 0;
  
  /* store locally for rapid access */
  tl = d->tl;
  tn = d->tn;

#ifdef UNIGRAM_FACTORING
#ifndef WPAIR
  /* 1-gram factoring では単語先頭での言語確率が一定で直前単語に依存しない
     ため，単語間 Viterbi において選ばれる直前単語は,次単語によらず共通である. 
     よって単語終端からfactoring値のある単語先頭への遷移は１つにまとめられる. 
     ただし，木から独立した単語については, 単語先頭で履歴に依存した2-gramが
     与えられるため, 最尤の単語間 Viterbi パスは次単語ごとに異なる. 
     よってそれらについてはまとめずに別に計算する */
  /* In 1-gram factoring, the language score on the word-head node is constant
     and independent of the previous word.  So, the same word hypothesis will
     be selected as the best previous word at the inter-word Viterbi
     processing.  So, in inter-word processing, we can (1) select only
     the best word-end hypothesis, and then (2) process viterbi from the node
     to each word-head node.  On the other hand, the isolated words,
     i.e. words not sharing any node with other word, has unique word-head
     node and the true 2-gram language score is determined at the top node.
     In such case the best word hypothesis prior to each node will differ
     according to the language scores.  So we have to deal such words separately. */
  /* initialize max value to delect best word-end hypothesis */
  if (lmtype == LM_PROB) {
    d->wordend_best_score = LOG_ZERO;
  }
#endif
#endif

#ifdef DEBUG
  /* debug */
  /* node_check_token(d, tl); */
#endif

  /* トークンバッファを初期化: 直前フレームで使われた部分だけクリアすればよい */
  /* initialize token buffer: for speedup, only ones used in the last call will be cleared */
  clear_tokens(d, tl);

  /**************************/
  /* 2. Viterbi計算         */
  /*    Viterbi computation */
  /**************************/
  /* 直前フレームからこのフレームへの Viterbi 計算を行なう */
  /* tindex[tl][n_start..n_end] に直前フレーム上位ノードのIDが格納されている */
  /* do one viterbi computation from last frame to this frame */
  /* tindex[tl][n_start..n_end] holds IDs of survived nodes in last frame */

  if (wchmm->hmminfo->multipath) {
    /*********************************/
    /* MULTIPATH MODE */
    /*********************************/

    for (j = d->n_start; j <= d->n_end; j++) {
      /* tk: 対象トークン  node: そのトークンを持つ木構造化辞書ノードID */
      /* tk: token data  node: lexicon tree node ID that holds the 'tk' */
      tk = &(d->tlist[tl][d->tindex[tl][j]]);
      if (tk->score <= LOG_ZERO) continue; /* invalid node */
#ifdef SCORE_PRUNING
      if (tk->score < d->score_pruning_threshold) {
	d->score_pruning_count++;
	continue;
      }
#endif
      node = tk->node;
      /*********************************/
      /* 2.1. 単語内遷移               */
      /*      word-internal transition */
      /*********************************/
      beam_intra_word(wchmm, d, &tk, j);
    }
    /*******************************************************/
    /* 2.2. スコアでトークンをソートしビーム幅分の上位を決定 */
    /*    sort tokens by score up to beam width            */
    /*******************************************************/
    sort_token_no_order(d, r->trellis_beam_width, &(d->n_start), &(d->n_end));
  
    /*************************/
    /* 2.3. 単語間Viterbi計算  */
    /*    cross-word viterbi */
    /*************************/
    for(j = d->n_start; j <= d->n_end; j++) {
      tk = &(d->tlist[tn][d->tindex[tn][j]]);
      node = tk->node;
#ifdef SCORE_PRUNING
      if (tk->score < d->score_pruning_threshold) {
	d->score_pruning_count++;
	continue;
      }
#endif
      /* 遷移元ノードが単語終端ならば */
      /* if source node is end state of a word, */
      if (wchmm->stend[node] != WORD_INVALID) {

	/**************************/
	/* 2.4. トレリス単語保存  */
	/*      save trellis word */
	/**************************/
#ifdef SPSEGMENT_NAIST
 	if (r->config->successive.enabled && !d->after_trigger) {
 	  tre = tk->last_tre;	/* dummy */
 	} else {
 	  tre = save_trellis(r->backtrellis, wchmm, tk, t, final_for_multipath);
	}
#else
	tre = save_trellis(r->backtrellis, wchmm, tk, t, final_for_multipath);
#endif
	/* 最終フレームであればここまで：遷移はさせない */
	/* If this is a final frame, does not do cross-word transition */
	if (final_for_multipath) continue;
	/* 単語認識モードでは単語間遷移は必要ない */
	if (lmvar == LM_DFA_WORD) continue;

	/******************************/
	/* 2.5. 単語間遷移            */
	/*      cross-word transition */
	/******************************/

#ifdef UNIGRAM_FACTORING
	/* ここで処理されるのは isolated words のみ，
	   shared nodes はまとめてこのループの外で計算する */
	/* Only the isolated words will be processed here.
	   The shared nodes with constant factoring values will be computed
	   after this loop */
#endif
	beam_inter_word(wchmm, d, &tk, tre, j);

      } /* end of cross-word processing */
    
    } /* end of main viterbi loop */



  } else {

    /*********************************/
    /* NORMAL MODE */
    /*********************************/

    for (j = d->n_start; j <= d->n_end; j++) {
      /* tk: 対象トークン  node: そのトークンを持つ木構造化辞書ノードID */
      /* tk: token data  node: lexicon tree node ID that holds the 'tk' */
      tk = &(d->tlist[tl][d->tindex[tl][j]]);
      if (tk->score <= LOG_ZERO) continue; /* invalid node */
#ifdef SCORE_PRUNING
      if (tk->score < d->score_pruning_threshold) {
	d->score_pruning_count++;
	continue;
      }
#endif
      node = tk->node;
      
      /*********************************/
      /* 2.1. 単語内遷移               */
      /*      word-internal transition */
      /*********************************/
      beam_intra_word(wchmm, d, &tk, j);

      /* 遷移元ノードが単語終端ならば */
      /* if source node is end state of a word, */
      if (wchmm->stend[node] != WORD_INVALID) {
	
	/**************************/
	/* 2.2. トレリス単語保存  */
	/*      save trellis word */
	/**************************/
#ifdef SPSEGMENT_NAIST
 	if (r->config->successive.enabled && !d->after_trigger) {
 	  tre = tk->last_tre;	/* dummy */
 	} else {
 	  tre = save_trellis(r->backtrellis, wchmm, tk, t, final_for_multipath);
	}
#else
	tre = save_trellis(r->backtrellis, wchmm, tk, t, final_for_multipath);
#endif
	/* 単語認識モードでは単語間遷移は必要ない */
	if (lmvar == LM_DFA_WORD) continue;

	/******************************/
	/* 2.3. 単語間遷移            */
	/*      cross-word transition */
	/******************************/
	
#ifdef UNIGRAM_FACTORING
	/* ここで処理されるのは isolated words のみ，
	   shared nodes はまとめてこのループの外で計算する */
	/* Only the isolated words will be processed here.
	   The shared nodes with constant factoring values will be computed
	   after this loop */
#endif

	beam_inter_word(wchmm, d, &tk, tre, j);

      } /* end of cross-word processing */
      
    } /* end of main viterbi loop */


  }

#ifdef UNIGRAM_FACTORING
#ifndef WPAIR

  if (lmtype == LM_PROB) {

    /***********************************************************/
    /* 2.x 単語終端からfactoring付き単語先頭への遷移 ***********/
    /*    transition from wordend to shared (factorized) nodes */
    /***********************************************************/
    /* d->wordend_best_* holds the best word ends at this frame. */
    if (d->wordend_best_score > LOG_ZERO) {
      beam_inter_word_factoring(wchmm, d);
    }
  }
#endif
#endif /* UNIGRAM_FACTORING */

  /***************************************/
  /* 3. 状態の出力確率計算               */
  /*    compute state output probability */
  /***************************************/

  /* 次段の有効ノードについて出力確率を計算してスコアに加える */
  /* compute outprob for new valid (token assigned) nodes and add to score */
  /* 今扱っているのが入力の最終フレームの場合出力確率は計算しない */
  /* don't calculate the last frame (transition only) */

#ifdef SCORE_PRUNING
  d->score_pruning_max = LOG_ZERO;
  minscore = 0.0;
#endif
  if (wchmm->hmminfo->multipath) {
    if (! final_for_multipath) {
      for (j = 0; j < d->tnum[tn]; j++) {
	tk = &(d->tlist[tn][d->tindex[tn][j]]);
	/* skip non-output state */
	if (wchmm->state[tk->node].out.state == NULL) continue;
	tk->score += outprob_style(wchmm, tk->node, tk->last_tre->wid, t, param);
#ifdef SCORE_PRUNING
	if (d->score_pruning_max < tk->score) d->score_pruning_max = tk->score;
	if (minscore > tk->score) minscore = tk->score;
#endif
      }
    }
  } else {
    for (j = 0; j < d->tnum[tn]; j++) {
      tk = &(d->tlist[tn][d->tindex[tn][j]]);
      tk->score += outprob_style(wchmm, tk->node, tk->last_tre->wid, t, param);
#ifdef SCORE_PRUNING
      if (d->score_pruning_max < tk->score) d->score_pruning_max = tk->score;
      if (minscore > tk->score) minscore = tk->score;
#endif
    }
  }
#ifdef SCORE_PRUNING
  if (r->config->pass1.score_pruning_width >= 0.0) {
    d->score_pruning_threshold = d->score_pruning_max - r->config->pass1.score_pruning_width;
    //printf("width=%f, tnum=%d\n", d->score_pruning_max - minscore, d->tnum[tn]);
  } else {
    // disable score pruning
    d->score_pruning_threshold = LOG_ZERO;
  }
#endif
  /*******************************************************/
  /* 4. スコアでトークンをソートしビーム幅分の上位を決定 */
  /*    sort tokens by score up to beam width            */
  /*******************************************************/

  /* tlist[tl]を次段のためにリセット */
  clear_tlist(d, tl);

  /* ヒープソートを用いてこの段のノード集合から上位(bwidth)個を得ておく */
  /* (上位内の順列は必要ない) */
  sort_token_no_order(d, r->trellis_beam_width, &(d->n_start), &(d->n_end));
  /***************/
  /* 5. 終了処理 */
  /*    finalize */
  /***************/

#ifdef SPSEGMENT_NAIST
  if (!r->config->successive.enabled || d->after_trigger) {
#endif

    /* call frame-wise callback */
    r->have_interim = FALSE;
    if (t > 0) {
      if (r->config->output.progout_flag) {
	/* 漸次出力: 現フレームのベストパスを一定時間おきに上書き出力 */
	/* progressive result output: output current best path in certain time interval */
	if (((t-1) % r->config->output.progout_interval_frame) == 0) {
	  r->have_interim = TRUE;
	  bt_current_max(r, t-1);
	}
      }
    }
    
    /* jlog("DEBUG: %d: %d\n",t,tnum[tn]); */
    /* for debug: output current max word */
    if (debug2_flag) {
      bt_current_max_word(r, t-1);
    }

#ifdef DETERMINE
    if (lmvar == LM_DFA_WORD) {
      check_determine_word(r, t-1);
    }
#endif

#ifdef SPSEGMENT_NAIST
  }
#endif
    
  /* ビーム内ノード数が 0 になってしまったら，強制終了 */
  if (d->tnum[tn] == 0) {
    jlog("ERROR: get_back_trellis_proceed: %02d %s: frame %d: no nodes left in beam, now terminates search\n", r->config->id, r->config->name, t);
    return(FALSE);
  }

  return(TRUE);
    
}

/*************************************************/
/* frame synchronous beam search --- last frame  */
/* フレーム同期ビーム探索の実行 --- 最終フレーム */
/*************************************************/

/** 
 * <JA>
 * @brief  フレーム同期ビーム探索：最終フレーム
 *
 * 第１パスのフレーム同期ビーム探索を終了するために，
 * (param->samplenum -1) の最終フレームに対する終了処理を行う. 
 * 
 * 
 * @param param [in] 入力ベクトル列 (param->samplenum の値のみ用いられる)
 * @param r [in] 音声認識処理インスタンス
 * </JA>
 * <EN>
 * @brief  Frame synchronous beam search: last frame
 *
 * This function should be called at the end of the 1st pass.
 * The last procedure will be done for the (param->samplenum - 1) frame.
 * 
 * @param param [in] input vectors (only param->samplenum is referred)
 * @param r [in] recognition process instance
 * </EN>
 *
 * @callergraph
 * @callgraph
 * 
 */
void
get_back_trellis_end(HTK_Param *param, RecogProcess *r)
{
  WCHMM_INFO *wchmm;
  FSBeam *d;
  int j;
  TOKEN2 *tk;

  wchmm = r->wchmm;
  d = &(r->pass1);

  /* 最後にビーム内に残った単語終端トークンを処理する */
  /* process the last wordend tokens */


  if (r->am->hmminfo->multipath) {
    /* MULTI-PATH VERSION */

    /* 単語末ノードへの遷移のみ計算 */
    /* only arcs to word-end node is calculated */
    get_back_trellis_proceed(param->samplenum, param, r, TRUE);

  } else {
    /* NORMAL VERSION */

    /* 最後の遷移のあとの単語終端処理を行う */
    /* process the word-ends at the last frame */
    d->tl = d->tn;
    if (d->tn == 0) d->tn = 1; else d->tn = 0;
    for (j = d->n_start; j <= d->n_end; j++) {
      tk = &(d->tlist[d->tl][d->tindex[d->tl][j]]);
      if (wchmm->stend[tk->node] != WORD_INVALID) {
	save_trellis(r->backtrellis, wchmm, tk, param->samplenum, TRUE);
      }
    }

  }
#ifdef SCORE_PRUNING
  if (debug2_flag) jlog("STAT: %d tokens pruned by score beam\n", d->score_pruning_count);
#endif
    
}

/*************************/
/* 探索終了 --- 終了処理 */
/* end of search         */
/*************************/
/** 
 * <JA>
 * @brief  第１パスの終了処理を行う. 
 *
 * この関数は get_back_trellis_end() の直後に呼ばれ，第１パスの終了処理を
 * 行う. 生成した単語トレリス構造体の最終的な後処理を行い第２パスで
 * アクセス可能な形に内部を変換する. また，
 * 仮説のバックトレースを行い第１パスのベスト仮説を出力する. 
 * 
 * @param r [in] 認識処理インスタンス
 * @param len [in] 第１パスで処理された最終的なフレーム長
 * 
 * @return 第１パスの最尤仮説の累積尤度，あるいは仮説が見つからない場合
 * は LOG_ZERO. 
 * </JA>
 * <EN>
 * @brief  Finalize the 1st pass.
 *
 * This function will be called just after get_back_trellis_end() to
 * finalize the 1st pass.  It processes the resulting word trellis structure
 * to be accessible from the 2nd pass, and output the best sentence hypothesis
 * by backtracing the word trellis.
 * 
 * @param r [in] recoginirion process instance
 * @param len [in] total number of processed frames
 * 
 * @return the maximum score of the best hypothesis, or LOG_ZERO if search
 * failed.
 * </EN>
 *
 * @callergraph
 * @callgraph
 * 
 */
void
finalize_1st_pass(RecogProcess *r, int len)
{
  BACKTRELLIS *backtrellis;

  backtrellis = r->backtrellis;
 
  backtrellis->framelen = len;

  /* 単語トレリス(backtrellis) を整理: トレリス単語の再配置とソート */
  /* re-arrange backtrellis: index them by frame, and sort by word ID */

  bt_relocate_rw(backtrellis);
  bt_sort_rw(backtrellis);
  if (backtrellis->num == NULL) {
    if (backtrellis->framelen > 0) {
      jlog("WARNING: %02d %s: input processed, but no survived word found\n", r->config->id, r->config->name);
    }
    /* reognition failed */
    r->result.status = J_RESULT_STATUS_FAIL;
    return;
  }

  /* 第1パスのベストパスを結果に格納する */
  /* store 1st pass result (best hypothesis) to result */
  if (r->lmvar == LM_DFA_WORD) {
    find_1pass_result_word(len, r);
  } else {
    find_1pass_result(len, r);
  }
}

/** 
 * <EN>
 * Free work area for the first pass
 * </EN>
 * <JA>
 * 第1パスのためのワークエリア領域を開放する
 * </JA>
 * 
 * @param d [in] work are for 1st pass input handling
 * 
 * @callergraph
 * @callgraph
 * 
 */
void
fsbeam_free(FSBeam *d)
{
  free_nodes(d);
  if (d->pausemodelnames != NULL) {
    free(d->pausemodelnames);
    free(d->pausemodel);
  }
}

/* end of file */
