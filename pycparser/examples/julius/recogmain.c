/**
 * @file   realtime-1stpass.c
 * 
 * <JA>
 * @brief  第1パス：フレーム同期ビーム探索（実時間処理版）
 *
 * 第1パスを入力開始と同時にスタートし，入力と平行して認識処理を行うための
 * 関数が定義されている. 
 * 
 * バッチ処理の場合，Julius の音声認識処理は以下の手順で
 * main_recognition_loop() 内で実行される. 
 *
 *  -# 音声入力 adin_go()  → 入力音声が speech[] に格納される
 *  -# 特徴量抽出 new_wav2mfcc() →speechから特徴パラメータを param に格納
 *  -# 第1パス実行 get_back_trellis() →param とモデルから単語トレリスの生成
 *  -# 第2パス実行 wchmm_fbs()
 *  -# 認識結果出力
 *
 * 第1パスを平行処理する場合，上記の 1 〜 3 が平行して行われる. 
 * Julius では，この並行処理を，音声入力の断片が得られるたびに
 * 認識処理をその分だけ漸次的に進めることで実装している. 
 * 
 *  - 特徴量抽出と第1パス実行を，一つにまとめてコールバック関数として定義. 
 *  - 音声入力関数 adin_go() のコールバックとして上記の関数を与える
 *
 * 具体的には，ここで定義されている RealTimePipeLine() がコールバックとして
 * adin_go() に与えられる. adin_go() は音声入力がトリガするとその得られた入力
 * 断片ごとに RealTimePipeLine() を呼び出す. RealTimePipeLine() は得られた
 * 断片分について特徴量抽出と第1パスの計算を進める. 
 *
 * CMN について注意が必要である. CMN は通常発話単位で行われるが，
 * マイク入力やネットワーク入力のように，第1パスと平行に認識を行う
 * 処理時は発話全体のケプストラム平均を得ることができない. バージョン 3.5
 * 以前では直前の発話5秒分(棄却された入力を除く)の CMN がそのまま次発話に
 * 流用されていたが，3.5.1 からは，上記の直前発話 CMN を初期値として
 * 発話内 CMN を MAP-CMN を持ちいて計算するようになった. なお，
 * 最初の発話用の初期CMNを "-cmnload" で与えることもでき，また
 * "-cmnnoupdate" で入力ごとの CMN 更新を行わないようにできる. 
 * "-cmnnoupdate" と "-cmnload" と組み合わせることで, 最初にグローバルな
 * ケプストラム平均を与え，それを常に初期値として MAP-CMN することができる. 
 *
 * 主要な関数は以下の通りである. 
 *
 *  - RealTimeInit() - 起動時の初期化
 *  - RealTimePipeLinePrepare() - 入力ごとの初期化
 *  - RealTimePipeLine() - 第1パス平行処理用コールバック（上述）
 *  - RealTimeResume() - ショートポーズセグメンテーション時の認識復帰
 *  - RealTimeParam() - 入力ごとの第1パス終了処理
 *  - RealTimeCMNUpdate() - CMN の更新
 *  
 * </JA>
 * 
 * <EN>
 * @brief  The first pass: frame-synchronous beam search (on-the-fly version)
 *
 * These are functions to perform on-the-fly decoding of the 1st pass
 * (frame-synchronous beam search).  These function can be used
 * instead of new_wav2mfcc() and get_back_trellis().  These functions enable
 * recognition as soon as an input triggers.  The 1st pass processing
 * will be done concurrently with the input.
 *
 * The basic recognition procedure of Julius in main_recognition_loop()
 * is as follows:
 *
 *  -# speech input: (adin_go())  ... buffer `speech' holds the input
 *  -# feature extraction: (new_wav2mfcc()) ... compute feature vector
 *     from `speech' and store the vector sequence to `param'.
 *  -# recognition 1st pass: (get_back_trellis()) ... frame-wise beam decoding
 *     to generate word trellis index from `param' and models.
 *  -# recognition 2nd pass: (wchmm_fbs())
 *  -# Output result.
 *
 * At on-the-fly decoding, procedures from 1 to 3 above will be performed
 * in parallel.  It is implemented by a simple scheme, processing the captured
 * small speech fragments one by one progressively:
 *
 *  - Define a callback function that can do feature extraction and 1st pass
 *    processing progressively.
 *  - The callback will be given to A/D-in function adin_go().
 *
 * Actual procedure is as follows. The function RealTimePipeLine()
 * will be given to adin_go() as callback.  Then adin_go() will watch
 * the input, and if speech input starts, it calls RealTimePipeLine()
 * for every captured input fragments.  RealTimePipeLine() will
 * compute the feature vector of the given fragment and proceed the
 * 1st pass processing for them, and return to the capture function.
 * The current status will be hold to the next call, to perform
 * inter-frame processing (computing delta coef. etc.).
 *
 * Note about CMN: With acoustic models trained with CMN, Julius performs
 * CMN to the input.  On file input, the whole sentence mean will be computed
 * and subtracted.  At the on-the-fly decoding, the ceptral mean will be
 * performed using the cepstral mean of last 5 second input (excluding
 * rejected ones).  This was a behavier earlier than 3.5, and 3.5.1 now
 * applies MAP-CMN at on-the-fly decoding, using the last 5 second cepstrum
 * as initial mean.  Initial cepstral mean at start can be given by option
 * "-cmnload", and you can also prohibit the updates of initial cepstral
 * mean at each input by "-cmnnoupdate".  The last option is useful to always
 * use static global cepstral mean as initial mean for each input.
 *
 * The primary functions in this file are:
 *  - RealTimeInit() - initialization at application startup
 *  - RealTimePipeLinePrepare() - initialization before each input
 *  - RealTimePipeLine() - callback for on-the-fly 1st pass decoding
 *  - RealTimeResume() - recognition resume procedure for short-pause segmentation.
 *  - RealTimeParam() - finalize the on-the-fly 1st pass when input ends.
 *  - RealTimeCMNUpdate() - update CMN data for next input
 * 
 * </EN>
 * 
 * @author Akinobu Lee
 * @date   Tue Aug 23 11:44:14 2005
 *
 * $Revision: 1.14 $
 * 
 */
/*
 * Copyright (c) 1991-2013 Kawahara Lab., Kyoto University
 * Copyright (c) 2000-2005 Shikano Lab., Nara Institute of Science and Technology
 * Copyright (c) 2005-2013 Julius project team, Nagoya Institute of Technology
 * All rights reserved
 */

//#include <julius/julius.h>

#undef RDEBUG			///< Define if you want local debug message

/** 
 * <JA>
 * MFCC計算インスタンス内に特徴パラメータベクトル格納エリアを準備する.
 * 
 * mfcc->para の情報に基づいてヘッダ情報を格納し，初期格納領域を確保する. 
 * 格納領域は，入力時に必要に応じて自動的に伸長されるので，ここでは
 * その準備だけ行う. すでに格納領域が確保されているときはそれをキープする. 
 * 
 * これは入力/認識1回ごとに繰り返し呼ばれる.
 * 
 * </JA>
 * <EN>
 * 
 * Prepare parameter holder in MFCC calculation instance to store MFCC
 * vectors.
 *
 * This function will store header information based on the parameters
 * in mfcc->para, and allocate initial buffer for the incoming
 * vectors.  The vector buffer will be expanded as needed while
 * recognition, so at this time only the minimal amount is allocated.
 * If the instance already has a certain length of vector buffer, it
 * will be kept.
 * 
 * This function will be called each time a new input begins.
 * 
 * </EN>
 *
 * @param mfcc [i/o] MFCC calculation instance
 * 
 */

int call_adin_go(struct Recog * recog) {
  int ret;
	ret = adin_go(RealTimePipeLine, callback_check_in_adin, recog);
  return ret;
}


static void
init_param(MFCCCalc *mfcc)
{
  Value *para;

  para = mfcc->para;

  /* これから計算されるパラメータの型をヘッダに設定 */
  /* set header types */
  mfcc->param->header.samptype = para->basetype;
  if (para->delta) mfcc->param->header.samptype |= F_DELTA;
  if (para->acc) mfcc->param->header.samptype |= F_ACCL;
  if (para->energy) mfcc->param->header.samptype |= F_ENERGY;
  if (para->c0) mfcc->param->header.samptype |= F_ZEROTH;
  if (para->absesup) mfcc->param->header.samptype |= F_ENERGY_SUP;
  if (para->cmn) mfcc->param->header.samptype |= F_CEPNORM;
  
  mfcc->param->header.wshift = para->smp_period * para->frameshift;
  mfcc->param->header.sampsize = para->veclen * sizeof(VECT); /* not compressed */
  mfcc->param->veclen = para->veclen;
  
  /* 認識処理中/終了後にセットされる変数:
     param->parvec (パラメータベクトル系列)
     param->header.samplenum, param->samplenum (全フレーム数)
  */
  /* variables that will be set while/after computation has been done:
     param->parvec (parameter vector sequence)
     param->header.samplenum, param->samplenum (total number of frames)
  */
  /* MAP-CMN の初期化 */
  /* Prepare for MAP-CMN */
  if (mfcc->para->cmn || mfcc->para->cvn) CMN_realtime_prepare(mfcc->cmn.wrk);
}

/** 
 * <JA>
 * @brief  第1パス平行認識処理の初期化.
 *
 * MFCC計算のワークエリア確保を行う. また必要な場合は，スペクトル減算用の
 * ワークエリア準備，ノイズスペクトルのロード，CMN用の初期ケプストラム
 * 平均データのロードなども行われる. 
 *
 * この関数は，システム起動後1回だけ呼ばれる.
 * </JA>
 * <EN>
 * @brief  Initializations for the on-the-fly 1st pass decoding.
 *
 * Work areas for all MFCC caculation instances are allocated.
 * Additionaly,
 * some initialization will be done such as allocating work area
 * for spectral subtraction, loading noise spectrum from file,
 * loading initial ceptral mean data for CMN from file, etc.
 *
 * This will be called only once, on system startup.
 * </EN>
 *
 * @param recog [i/o] engine instance
 *
 * @callgraph
 * @callergraph
 */
boolean
RealTimeInit(Recog *recog)
{
  Value *para;
  Jconf *jconf;
  RealBeam *r;
  MFCCCalc *mfcc;


  jconf = recog->jconf;
  r = &(recog->real);

  /* 最大フレーム長を最大入力時間数から計算 */
  /* set maximum allowed frame length */
  r->maxframelen = MAXSPEECHLEN / recog->jconf->input.frameshift;

  /* -ssload 指定時, SS用のノイズスペクトルをファイルから読み込む */
  /* if "-ssload", load noise spectrum for spectral subtraction from file */
  for(mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (mfcc->frontend.ssload_filename && mfcc->frontend.ssbuf == NULL) {
      if ((mfcc->frontend.ssbuf = new_SS_load_from_file(mfcc->frontend.ssload_filename, &(mfcc->frontend.sslen))) == NULL) {
	jlog("ERROR: failed to read \"%s\"\n", mfcc->frontend.ssload_filename);
	return FALSE;
      }
      /* check ssbuf length */
      if (mfcc->frontend.sslen != mfcc->wrk->bflen) {
	jlog("ERROR: noise spectrum length not match\n");
	return FALSE;
      }
      mfcc->wrk->ssbuf = mfcc->frontend.ssbuf;
      mfcc->wrk->ssbuflen = mfcc->frontend.sslen;
      mfcc->wrk->ss_alpha = mfcc->frontend.ss_alpha;
      mfcc->wrk->ss_floor = mfcc->frontend.ss_floor;
    }
  }

  for(mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
  
    para = mfcc->para;

    /* 対数エネルギー正規化のための初期値 */
    /* set initial value for log energy normalization */
    if (para->energy && para->enormal) energy_max_init(&(mfcc->ewrk));
    /* デルタ計算のためのサイクルバッファを用意 */
    /* initialize cycle buffers for delta and accel coef. computation */
    if (para->delta) mfcc->db = WMP_deltabuf_new(para->baselen, para->delWin);
    if (para->acc) mfcc->ab = WMP_deltabuf_new(para->baselen * 2, para->accWin);
    /* デルタ計算のためのワークエリアを確保 */
    /* allocate work area for the delta computation */
    mfcc->tmpmfcc = (VECT *)mymalloc(sizeof(VECT) * para->vecbuflen);
    /* MAP-CMN 用の初期ケプストラム平均を読み込んで初期化する */
    /* Initialize the initial cepstral mean data from file for MAP-CMN */
    if (para->cmn || para->cvn) mfcc->cmn.wrk = CMN_realtime_new(para, mfcc->cmn.map_weight);
    /* -cmnload 指定時, CMN用のケプストラム平均の初期値をファイルから読み込む */
    /* if "-cmnload", load initial cepstral mean data from file for CMN */
    if (mfcc->cmn.load_filename) {
      if (para->cmn) {
	if ((mfcc->cmn.loaded = CMN_load_from_file(mfcc->cmn.wrk, mfcc->cmn.load_filename))== FALSE) {
	  jlog("ERROR: failed to read initial cepstral mean from \"%s\", do flat start\n", mfcc->cmn.load_filename);
	  return FALSE;
	}
      } else {
	jlog("WARNING: CMN not required on AM, file \"%s\" ignored\n", mfcc->cmn.load_filename);
      }
    }

  }
  /* 窓長をセット */
  /* set window length */
  r->windowlen = recog->jconf->input.framesize + 1;
  /* 窓かけ用バッファを確保 */
  /* set window buffer */
  r->window = mymalloc(sizeof(SP16) * r->windowlen);

  return TRUE;
}

/** 
 * <EN>
 * Prepare work are a for MFCC calculation.
 * Reset values in work area for starting the next input.
 * Output probability cache for each acoustic model will be also
 * prepared at this function.
 *
 * This function will be called before starting each input (segment).
 * </EN>
 * <JA>
 * MFCC計算を準備する. 
 * いくつかのワークエリアをリセットして認識に備える. 
 * また，音響モデルごとの出力確率計算キャッシュを準備する. 
 *
 * この関数は，ある入力（あるいはセグメント）の認識が
 * 始まる前に必ず呼ばれる. 
 * 
 * </JA>
 * 
 * @param recog [i/o] engine instance
 * 
 * @callgraph
 * @callergraph
 */
void
reset_mfcc(Recog *recog) 
{
  Value *para;
  MFCCCalc *mfcc;
  RealBeam *r;

  r = &(recog->real);

  /* 特徴抽出モジュールを初期化 */
  /* initialize parameter extraction module */
  for(mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {

    para = mfcc->para;

    /* 対数エネルギー正規化のための初期値をセット */
    /* set initial value for log energy normalization */
    if (para->energy && para->enormal) energy_max_prepare(&(mfcc->ewrk), para);
    /* デルタ計算用バッファを準備 */
    /* set the delta cycle buffer */
    if (para->delta) WMP_deltabuf_prepare(mfcc->db);
    if (para->acc) WMP_deltabuf_prepare(mfcc->ab);
  }

}

/** 
 * <JA>
 * @brief  第1パス平行認識処理の準備
 *
 * 計算用変数をリセットし，各種データを準備する. 
 * この関数は，ある入力（あるいはセグメント）の認識が
 * 始まる前に呼ばれる. 
 * 
 * </JA>
 * <EN>
 * @brief  Preparation for the on-the-fly 1st pass decoding.
 *
 * Variables are reset and data are prepared for the next input recognition.
 *
 * This function will be called before starting each input (segment).
 * 
 * </EN>
 *
 * @param recog [i/o] engine instance
 *
 * @return TRUE on success. FALSE on failure.
 *
 * @callgraph
 * @callergraph
 * 
 */
boolean
RealTimePipeLinePrepare(Recog *recog)
{
  RealBeam *r;
  PROCESS_AM *am;
  MFCCCalc *mfcc;
#ifdef SPSEGMENT_NAIST
  RecogProcess *p;
#endif

  r = &(recog->real);

  /* 計算用の変数を初期化 */
  /* initialize variables for computation */
  r->windownum = 0;
  /* parameter check */
  for(mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    /* パラメータ初期化 */
    /* parameter initialization */
    if (recog->jconf->input.speech_input == SP_MFCMODULE) {
      if (mfc_module_set_header(mfcc, recog) == FALSE) return FALSE;
    } else {
      init_param(mfcc);
    }
    /* フレームごとのパラメータベクトル保存の領域を確保 */
    /* あとで必要に応じて伸長される */
    if (param_alloc(mfcc->param, 1, mfcc->param->veclen) == FALSE) {
      j_internal_error("ERROR: segmented: failed to allocate memory for rest param\n");
    }
    /* フレーム数をリセット */
    /* reset frame count */
    mfcc->f = 0;
  }
  /* 準備した param 構造体のデータのパラメータ型を音響モデルとチェックする */
  /* check type coherence between param and hmminfo here */
  if (recog->jconf->input.paramtype_check_flag) {
    for(am=recog->amlist;am;am=am->next) {
      if (!check_param_coherence(am->hmminfo, am->mfcc->param)) {
	jlog("ERROR: input parameter type does not match AM\n");
	return FALSE;
      }
    }
  }

  /* 計算用のワークエリアを準備 */
  /* prepare work area for calculation */
  if (recog->jconf->input.type == INPUT_WAVEFORM) {
    reset_mfcc(recog);
  }
  /* 音響尤度計算用キャッシュを準備 */
  /* prepare cache area for acoustic computation of HMM states and mixtures */
  for(am=recog->amlist;am;am=am->next) {
    outprob_prepare(&(am->hmmwrk), r->maxframelen);
  }

#ifdef BACKEND_VAD
  if (recog->jconf->decodeopt.segment) {
    /* initialize segmentation parameters */
    spsegment_init(recog);
  }
#else
  recog->triggered = FALSE;
#endif

#ifdef DEBUG_VTLN_ALPHA_TEST
  /* store speech */
  recog->speechlen = 0;
#endif

  return TRUE;
}

/** 
 * <JA>
 * @brief  音声波形からパラメータベクトルを計算する.
 * 
 * 窓単位で取り出された音声波形からMFCCベクトルを計算する.
 * 計算結果は mfcc->tmpmfcc に保存される. 
 * 
 * @param mfcc [i/o] MFCC計算インスタンス
 * @param window [in] 窓単位で取り出された音声波形データ
 * @param windowlen [in] @a window の長さ
 * 
 * @return 計算成功時，TRUE を返す. デルタ計算において入力フレームが
 * 少ないなど，まだ得られていない場合は FALSE を返す. 
 * </JA>
 * <EN>
 * @brief  Compute a parameter vector from a speech window.
 *
 * This function calculates an MFCC vector from speech data windowed from
 * input speech.  The obtained MFCC vector will be stored to mfcc->tmpmfcc.
 * 
 * @param mfcc [i/o] MFCC calculation instance
 * @param window [in] speech input (windowed from input stream)
 * @param windowlen [in] length of @a window
 * 
 * @return TRUE on success (an vector obtained).  Returns FALSE if no
 * parameter vector obtained yet (due to delta delay).
 * </EN>
 *
 * @callgraph
 * @callergraph
 * 
 */
boolean
RealTimeMFCC(MFCCCalc *mfcc, SP16 *window, int windowlen)
{
  int i;
  boolean ret;
  VECT *tmpmfcc;
  Value *para;

  tmpmfcc = mfcc->tmpmfcc;
  para = mfcc->para;

  /* 音声波形から base MFCC を計算 (recog->mfccwrk を利用) */
  /* calculate base MFCC from waveform (use recog->mfccwrk) */
  for (i=0; i < windowlen; i++) {
    mfcc->wrk->bf[i+1] = (float) window[i];
  }
  WMP_calc(mfcc->wrk, tmpmfcc, para);

  if (para->energy && para->enormal) {
    /* 対数エネルギー項を正規化する */
    /* normalize log energy */
    /* リアルタイム入力では発話ごとの最大エネルギーが得られないので
       直前の発話のパワーで代用する */
    /* Since the maximum power of the whole input utterance cannot be
       obtained at real-time input, the maximum of last input will be
       used to normalize.
    */
    tmpmfcc[para->baselen-1] = energy_max_normalize(&(mfcc->ewrk), tmpmfcc[para->baselen-1], para);
  }

  if (para->delta) {
    /* デルタを計算する */
    /* calc delta coefficients */
    ret = WMP_deltabuf_proceed(mfcc->db, tmpmfcc);
#ifdef RDEBUG
    printf("DeltaBuf: ret=%d, status=", ret);
    for(i=0;i<mfcc->db->len;i++) {
      printf("%d", mfcc->db->is_on[i]);
    }
    printf(", nextstore=%d\n", mfcc->db->store);
#endif
    /* ret == FALSE のときはまだディレイ中なので認識処理せず次入力へ */
    /* if ret == FALSE, there is no available frame.  So just wait for
       next input */
    if (! ret) {
      return FALSE;
    }

    /* db->vec に現在の元データとデルタ係数が入っているので tmpmfcc にコピー */
    /* now db->vec holds the current base and full delta, so copy them to tmpmfcc */
    memcpy(tmpmfcc, mfcc->db->vec, sizeof(VECT) * para->baselen * 2);
  }

  if (para->acc) {
    /* Accelerationを計算する */
    /* calc acceleration coefficients */
    /* base+delta をそのまま入れる */
    /* send the whole base+delta to the cycle buffer */
    ret = WMP_deltabuf_proceed(mfcc->ab, tmpmfcc);
#ifdef RDEBUG
    printf("AccelBuf: ret=%d, status=", ret);
    for(i=0;i<mfcc->ab->len;i++) {
      printf("%d", mfcc->ab->is_on[i]);
    }
    printf(", nextstore=%d\n", mfcc->ab->store);
#endif
    /* ret == FALSE のときはまだディレイ中なので認識処理せず次入力へ */
    /* if ret == FALSE, there is no available frame.  So just wait for
       next input */
    if (! ret) {
      return FALSE;
    }
    /* ab->vec には，(base+delta) とその差分係数が入っている. 
       [base] [delta] [delta] [acc] の順で入っているので,
       [base] [delta] [acc] を tmpmfcc にコピーする. */
    /* now ab->vec holds the current (base+delta) and their delta coef. 
       it holds a vector in the order of [base] [delta] [delta] [acc], 
       so copy the [base], [delta] and [acc] to tmpmfcc.  */
    memcpy(tmpmfcc, mfcc->ab->vec, sizeof(VECT) * para->baselen * 2);
    memcpy(&(tmpmfcc[para->baselen*2]), &(mfcc->ab->vec[para->baselen*3]), sizeof(VECT) * para->baselen);
  }

#ifdef POWER_REJECT
  if (para->energy || para->c0) {
    mfcc->avg_power += tmpmfcc[para->baselen-1];
  }
#endif

  if (para->delta && (para->energy || para->c0) && para->absesup) {
    /* 絶対値パワーを除去 */
    /* suppress absolute power */
    memmove(&(tmpmfcc[para->baselen-1]), &(tmpmfcc[para->baselen]), sizeof(VECT) * (para->vecbuflen - para->baselen));
  }

  /* この時点で tmpmfcc に現時点での最新の特徴ベクトルが格納されている */
  /* tmpmfcc[] now holds the latest parameter vector */

  /* CMN を計算 */
  /* perform CMN */
  if (para->cmn || para->cvn) CMN_realtime(mfcc->cmn.wrk, tmpmfcc);

  return TRUE;
}

static int
proceed_one_frame(Recog *recog)
{
  MFCCCalc *mfcc;
  RealBeam *r;
  int maxf;
  PROCESS_AM *am;
  int rewind_frame;
  boolean reprocess;
  boolean ok_p;

  r = &(recog->real);

  /* call recognition start callback */
  ok_p = FALSE;
  maxf = 0;
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (!mfcc->valid) continue;
    if (maxf < mfcc->f) maxf = mfcc->f;
    if (mfcc->f == 0) {
      ok_p = TRUE;
    }
  }
  if (ok_p && maxf == 0) {
    /* call callback when at least one of MFCC has initial frame */
    if (recog->jconf->decodeopt.segment) {
#ifdef BACKEND_VAD
      /* not exec pass1 begin callback here */
#else
      if (!recog->process_segment) {
	callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
      }
      callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
      callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
      recog->triggered = TRUE;
#endif
    } else {
      callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
      callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
      recog->triggered = TRUE;
    }
  }
  /* 各インスタンスについて mfcc->f の認識処理を1フレーム進める */
  switch (decode_proceed(recog)) {
  case -1: /* error */
    return -1;
    break;
  case 0:			/* success */
    break;
  case 1:			/* segmented */
    /* 認識処理のセグメント要求で終わったことをフラグにセット */
    /* set flag which indicates that the input has ended with segmentation request */
    r->last_is_segmented = TRUE;
    /* tell the caller to be segmented by this function */
    /* 呼び出し元に，ここで入力を切るよう伝える */
    return 1;
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
    /* set total length to the current frame */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (!mfcc->valid) continue;
      mfcc->param->header.samplenum = mfcc->f + 1;
      mfcc->param->samplenum = mfcc->f + 1;
    }
    /* do rewind for all mfcc here */
    spsegment_restart_mfccs(recog, rewind_frame, reprocess);
    /* also tell adin module to rehash the concurrent audio input */
    recog->adin->rehash = TRUE;
    /* reset outprob cache for all AM */
    for(am=recog->amlist;am;am=am->next) {
      outprob_prepare(&(am->hmmwrk), am->mfcc->param->samplenum);
    }
    if (reprocess) {
      /* process the backstep MFCCs here */
      while(1) {
	ok_p = TRUE;
	for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
	  if (! mfcc->valid) continue;
	  mfcc->f++;
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
	  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
	    if (! mfcc->valid) continue;
	    mfcc->f--;
	  }
	  break;
	}
	/* 各インスタンスについて mfcc->f の認識処理を1フレーム進める */
	switch (decode_proceed(recog)) {
	case -1: /* error */
	  return -1;
	  break;
	case 0:			/* success */
	  break;
	case 1:			/* segmented */
	  /* ignore segmentation while in the backstep segment */
	  break;
	}
	/* call frame-wise callback */
	callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
      }
    }
  }
  /* call frame-wise callback if at least one of MFCC is valid at this frame */
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (mfcc->valid) {
      callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
      break;
    }
  }
  
  return 0;
}


/** 
 * <JA>
 * @brief  第1パス平行音声認識処理のメイン
 *
 * この関数内では，漸次的な特徴量抽出および第1パスの認識が行われる. 
 * 入力データに対して窓掛け・シフトを行いMFCC計算を行いながら，
 * 音声認識を1フレームずつ並列実行する. 
 *
 * 認識処理（decode_proceed()）において，音声区間終了が要求される
 * ことがある. この場合，未処理の音声を保存して第1パスを終了する
 * よう呼出元に要求する. 
 *
 * SPSEGMENT_NAIST あるいは GMM_VAD などのバックエンドVAD定義時は，デコーダベースの
 * VAD （音声区間開始検出）に伴うデコーディング制御が行われる. 
 * トリガ前は，認識処理が呼ばれるが，実際には各関数内で認識処理は
 * 行われていない. 開始を検出した時，この関数はそこまでに得られた
 * MFCC列を一定フレーム長分巻戻し，その巻戻し先から通常の認識処理を
 * 再開する. なお，複数処理インスタンス間がある場合，開始トリガは
 * どれかのインスタンスが検出した時点で全ての開始が同期される. 
 * 
 * この関数は，音声入力ルーチンのコールバックとして呼ばれる.
 * 音声データの数千サンプル録音ごとにこの関数が呼び出される. 
 * 
 * @param Speech [in] 音声データへのバッファへのポインタ
 * @param nowlen [in] 音声データの長さ
 * @param recog [i/o] engine instance
 * 
 * @return エラー時に -1 を，正常時に 0 を返す. また，第1パスを
 * 終了するよう呼出元に要求するときは 1 を返す. 
 * </JA>
 * <EN>
 * @brief  Main function of the on-the-fly 1st pass decoding
 *
 * This function performs sucessive MFCC calculation and 1st pass decoding.
 * The given input data are windowed to a certain length, then converted
 * to MFCC, and decoding for the input frame will be performed in one
 * process cycle.  The loop cycle will continue with window shift, until
 * the whole given input has been processed.
 *
 * In case of input segment request from decoding process (in
 * decode_proceed()), this function keeps the rest un-processed speech
 * to a buffer and tell the caller to stop input and end the 1st pass.
 *
 * When back-end VAD such as SPSEGMENT_NAIST or GMM_VAD is defined,  Decoder-based
 * VAD is enabled and its decoding control will be managed here.
 * In decoder-based VAD mode, the recognition will be processed but
 * no output will be done at the first un-triggering input area.
 * when speech input start is detected, this function will rewind the
 * already obtained MFCC sequence to a certain frames, and re-start
 * normal recognition at that point.  When multiple recognition process
 * instance is running, their segmentation will be synchronized.
 * 
 * This function will be called each time a new speech sample comes as
 * as callback from A/D-in routine.
 * 
 * @param Speech [in] pointer to the speech sample segments
 * @param nowlen [in] length of above
 * @param recog [i/o] engine instance
 * 
 * @return -1 on error (will close stream and terminate recognition),
 * 0 on success (allow caller to call me for the next segment).  It
 * returns 1 when telling the caller to segment now at the middle of
 * input , and 2 when input length overflow is detected.
 * </EN>
 *
 * @callgraph
 * @callergraph
 * 
 */
int
RealTimePipeLine(SP16 *Speech, int nowlen, Recog *recog) /* Speech[0...nowlen] = input */
{
  int i, now, ret;
  MFCCCalc *mfcc;
  RealBeam *r;

  r = &(recog->real);

#ifdef DEBUG_VTLN_ALPHA_TEST
  /* store speech */
  adin_cut_callback_store_buffer(Speech, nowlen, recog);
#endif

  /* window[0..windownum-1] は前回の呼び出しで残った音声データが格納されている */
  /* window[0..windownum-1] are speech data left from previous call */

  /* 処理用ポインタを初期化 */
  /* initialize pointer for local processing */
  now = 0;
  
  /* 認識処理がセグメント要求で終わったのかどうかのフラグをリセット */
  /* reset flag which indicates whether the input has ended with segmentation request */
  r->last_is_segmented = FALSE;

#ifdef RDEBUG
  printf("got %d samples\n", nowlen);
#endif

  while (now < nowlen) {	/* till whole input is processed */
    /* 入力長が maxframelen に達したらここで強制終了 */
    /* if input length reaches maximum buffer size, terminate 1st pass here */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (mfcc->f >= r->maxframelen) {
	jlog("Warning: too long input (> %d frames), segment it now\n", r->maxframelen);
	return(1);
      }
    }
    /* 窓バッファを埋められるだけ埋める */
    /* fill window buffer as many as possible */
    for(i = min(r->windowlen - r->windownum, nowlen - now); i > 0 ; i--)
      r->window[r->windownum++] = (float) Speech[now++];
    /* もし窓バッファが埋まらなければ, このセグメントの処理はここで終わる. 
       処理されなかったサンプル (window[0..windownum-1]) は次回に持ち越し. */
    /* if window buffer was not filled, end processing here, keeping the
       rest samples (window[0..windownum-1]) in the window buffer. */
    if (r->windownum < r->windowlen) break;
#ifdef RDEBUG
    /*    printf("%d used, %d rest\n", now, nowlen - now);

	  printf("[f = %d]\n", f);*/
#endif

    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      mfcc->valid = FALSE;
      /* 窓内の音声波形から特徴量を計算して r->tmpmfcc に格納  */
      /* calculate a parameter vector from current waveform windows
	 and store to r->tmpmfcc */
      boolean calc_vector_result = ((*(recog->calc_vector))(mfcc, r->window, r->windowlen)); 
      //if ((*(recog->calc_vector))(mfcc, r->window, r->windowlen)) {
			if (calc_vector_result) {
#ifdef ENABLE_PLUGIN
	/* call post-process plugin if exist */
	plugin_exec_vector_postprocess(mfcc->tmpmfcc, mfcc->param->veclen, mfcc->f);
#endif
	/* MFCC完成，登録 */
  	mfcc->valid = TRUE;
	/* now get the MFCC vector of current frame, now store it to param */
	if (param_alloc(mfcc->param, mfcc->f + 1, mfcc->param->veclen) == FALSE) {
	  jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
	  return -1;
	}
	memcpy(mfcc->param->parvec[mfcc->f], mfcc->tmpmfcc, sizeof(VECT) * mfcc->param->veclen);
#ifdef RDEBUG
	printf("DeltaBuf: %02d: got frame %d\n", mfcc->id, mfcc->f);
#endif
      }
    }

    /* 処理を1フレーム進める */
    /* proceed one frame */
    ret = proceed_one_frame(recog);

    if (ret == 1 && recog->jconf->decodeopt.segment) {
      /* ショートポーズセグメンテーション: バッファに残っているデータを
	 別に保持して，次回の最初に処理する */
      /* short pause segmentation: there is some data left in buffer, so
	 we should keep them for next processing */
      r->rest_len = nowlen - now;
      if (r->rest_len > 0) {
	/* copy rest samples to rest_Speech */
	if (r->rest_Speech == NULL) {
	  r->rest_alloc_len = r->rest_len;
	  r->rest_Speech = (SP16 *)mymalloc(sizeof(SP16)*r->rest_alloc_len);
	} else if (r->rest_alloc_len < r->rest_len) {
	  r->rest_alloc_len = r->rest_len;
	  r->rest_Speech = (SP16 *)myrealloc(r->rest_Speech, sizeof(SP16)*r->rest_alloc_len);
	}
	memcpy(r->rest_Speech, &(Speech[now]), sizeof(SP16) * r->rest_len);
      }
    }
    if (ret != 0) return ret;

    /* 1フレーム処理が進んだのでポインタを進める */
    /* proceed frame pointer */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (!mfcc->valid) continue;
      mfcc->f++;
    }

    /* 窓バッファを処理が終わった分シフト */
    /* shift window */
    memmove(r->window, &(r->window[recog->jconf->input.frameshift]), sizeof(SP16) * (r->windowlen - recog->jconf->input.frameshift));
    r->windownum -= recog->jconf->input.frameshift;
  }

  /* 与えられた音声セグメントに対する認識処理が全て終了
     呼び出し元に, 入力を続けるよう伝える */
  /* input segment is fully processed
     tell the caller to continue input */
  return(0);			
}

/** 
 * <JA>
 * @brief  セグメントの認識再開処理
 *
 * この関数はデコーダベースVADやショートポーズセグメンテーションによって
 * 入力がセグメントに切られた場合に，その後の認識の再開に関する処理を行う. 
 * 具体的には，入力の認識を開始する前に，前回の入力セグメントにおける
 * 巻戻し分のMFCC列から認識を開始する. さらに，前回のセグメンテーション時に
 * 未処理だった残りの音声サンプルがあればそれも処理する.
 *
 * @param recog [i/o] エンジンインスタンス
 * 
 * @return エラー時 -1，正常時 0 を返す. また，この入力断片の処理中に
 * 文章の区切りが見つかったときは第1パスをここで中断するために 1 を返す. 
 * </JA>
 * </JA>
 * <EN>
 * @brief  Resuming recognition for short pause segmentation.
 *
 * This function process overlapped data and remaining speech prior
 * to the next input when input was segmented at last processing.
 *
 * @param recog [i/o] engine instance
 *
 * @return -1 on error (tell caller to terminate), 0 on success (allow caller
 * to call me for the next segment), or 1 when an end-of-sentence detected
 * at this point (in that case caller will stop input and go to 2nd pass)
 * </EN>
 *
 * @callgraph
 * @callergraph
 * 
 */
int
RealTimeResume(Recog *recog)
{
  MFCCCalc *mfcc;
  RealBeam *r;
  boolean ok_p;
#ifdef SPSEGMENT_NAIST
  RecogProcess *p;
#endif
  PROCESS_AM *am;

  r = &(recog->real);

  /* 計算用のワークエリアを準備 */
  /* prepare work area for calculation */
  if (recog->jconf->input.type == INPUT_WAVEFORM) {
    reset_mfcc(recog);
  }
  /* 音響尤度計算用キャッシュを準備 */
  /* prepare cache area for acoustic computation of HMM states and mixtures */
  for(am=recog->amlist;am;am=am->next) {
    outprob_prepare(&(am->hmmwrk), r->maxframelen);
  }

  /* param にある全パラメータを処理する準備 */
  /* prepare to process all data in param */
  for(mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (mfcc->param->samplenum == 0) mfcc->valid = FALSE;
    else mfcc->valid = TRUE;
#ifdef RDEBUG
    printf("Resume: %02d: f=%d\n", mfcc->id, mfcc->mfcc->param->samplenum-1);
#endif
    /* フレーム数をリセット */
    /* reset frame count */
    mfcc->f = 0;
    /* MAP-CMN の初期化 */
    /* Prepare for MAP-CMN */
    if (mfcc->para->cmn || mfcc->para->cvn) CMN_realtime_prepare(mfcc->cmn.wrk);
  }

#ifdef BACKEND_VAD
  if (recog->jconf->decodeopt.segment) {
    spsegment_init(recog);
  }
  /* not exec pass1 begin callback here */
#else
  recog->triggered = FALSE;
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (!mfcc->valid) continue;
    callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
    callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
    recog->triggered = TRUE;
    break;
  }
#endif

  /* param 内の全フレームについて認識処理を進める */
  /* proceed recognition for all frames in param */

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

    /* 各インスタンスについて mfcc->f の認識処理を1フレーム進める */
    switch (decode_proceed(recog)) {
    case -1: /* error */
      return -1;
      break;
    case 0:			/* success */
      break;
    case 1:			/* segmented */
      /* segmented, end procs ([0..f])*/
      r->last_is_segmented = TRUE;
      return 1;		/* segmented by this function */
    }

#ifdef BACKEND_VAD
    /* check up trigger in case of VAD segmentation */
    if (recog->jconf->decodeopt.segment) {
      if (recog->triggered == FALSE) {
	if (spsegment_trigger_sync(recog)) {
	  callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
	  callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
	  recog->triggered = TRUE;
	}
      }
    }
#endif

    /* call frame-wise callback */
    callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);

    /* 1フレーム処理が進んだのでポインタを進める */
    /* proceed frame pointer */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (!mfcc->valid) continue;
      mfcc->f++;
    }

  }
  /* 前回のセグメント時に入力をシフトしていない分をシフトする */
  /* do the last shift here */
  if (recog->jconf->input.type == INPUT_WAVEFORM) {
    memmove(r->window, &(r->window[recog->jconf->input.frameshift]), sizeof(SP16) * (r->windowlen - recog->jconf->input.frameshift));
    r->windownum -= recog->jconf->input.frameshift;
    /* これで再開の準備が整ったので,まずは前回の処理で残っていた音声データから
       処理する */
    /* now that the search status has been prepared for the next input, we
       first process the rest unprocessed samples at the last session */
    if (r->rest_len > 0) {
      int RealTimePipeLine_result;
      RealTimePipeLine_result = RealTimePipeLine(r->rest_Speech, r->rest_len, recog);
      return RealTimePipeLine_result;
      //return(RealTimePipeLine(r->rest_Speech, r->rest_len, recog));
    }
  }

  /* 新規の入力に対して認識処理は続く… */
  /* the recognition process will continue for the newly incoming samples... */
  return 0;

}

/*
 * RealTimeResume sliced for loop counts.
 */
void RealTimeResume_slice(Recog *recog)
{
  int loop_counter[23] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  MFCCCalc *mfcc;
  RealBeam *r;
  boolean ok_p;
  PROCESS_AM *am;
  r = &recog->real;
  if (recog->jconf->input.type == INPUT_WAVEFORM)
  {
    loop_counter[0]++;
    {
      Value *para_rename0;
      MFCCCalc *mfcc_rename0;
      RealBeam *r_rename0;
      r_rename0 = &recog->real;
      for (mfcc_rename0 = recog->mfcclist; mfcc_rename0; mfcc_rename0 = mfcc_rename0->next)
      {
        loop_counter[1]++;
        para_rename0 = mfcc_rename0->para;
        if (para_rename0->energy && para_rename0->enormal)
        {
          loop_counter[2]++;
{}
        }

        if (para_rename0->delta)
        {
          loop_counter[3]++;
{}
        }

        if (para_rename0->acc)
        {
          loop_counter[4]++;
{}
        }

      }

      return0:
      ;

    }
  }

  for (am = recog->amlist; am; am = am->next)
  {
    loop_counter[5]++;
{}
  }

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
  {
    loop_counter[6]++;
    if (mfcc->param->samplenum == 0)
    {
      loop_counter[7]++;
      mfcc->valid = FALSE;
    }
    else
      mfcc->valid = TRUE;

    mfcc->f = 0;
    if (mfcc->para->cmn || mfcc->para->cvn)
    {
      loop_counter[8]++;
{}
    }

  }

{}
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
  {
    loop_counter[9]++;
    if (!mfcc->valid)
    {
      loop_counter[10]++;
      continue;
    }

{}
{}
{}
    break;
  }

  while (1)
  {
    loop_counter[11]++;
    ok_p = TRUE;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[12]++;
      if (!mfcc->valid)
      {
        loop_counter[13]++;
        continue;
      }

      if (mfcc->f < mfcc->param->samplenum)
      {
        loop_counter[14]++;
        mfcc->valid = TRUE;
        ok_p = FALSE;
      }
      else
      {
        mfcc->valid = FALSE;
      }

    }

    if (ok_p)
    {
      loop_counter[15]++;
      break;
    }

    switch (decode_proceed(recog))
    {
      case -1:
        loop_counter[16]++;
      {
        goto print_loop_counter;
      }
        break;

      case 0:
        loop_counter[17]++;
        break;

      case 1:
        loop_counter[18]++;
{}
      {
        goto print_loop_counter;
      }

    }

{}
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[19]++;
      if (!mfcc->valid)
      {
        loop_counter[20]++;
        continue;
      }

      mfcc->f++;
    }

  }

  if (recog->jconf->input.type == INPUT_WAVEFORM)
  {
    loop_counter[21]++;
{}
{}
    if (r->rest_len > 0)
    {
      loop_counter[22]++;
{}
{}
      {
        goto print_loop_counter;
      }
    }

  }

  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 23; i++)
      printf("%d, ", loop_counter[i]++);

    printf(")\n");
  }
}


/** 
 * <JA>
 * @brief  第1パス平行認識処理の終了処理を行う.
 *
 * この関数は第1パス終了時に呼ばれ，入力長を確定したあと，
 * decode_end() （セグメントで終了したときは decode_end_segmented()）を
 * 呼び出して第1パス終了処理を行う. 
 *
 * もし音声入力ストリームの終了によって認識が終わった場合（ファイル入力で
 * 終端に達した場合など）は，デルタバッファに未処理の入力が残っているので，
 * それをここで処理する. 
 *
 * @param recog [i/o] エンジンインスタンス
 * 
 * @return 処理成功時 TRUE, エラー時 FALSE を返す. 
 * </JA>
 * <EN>
 * @brief  Finalize the 1st pass on-the-fly decoding.
 *
 * This function will be called after the 1st pass processing ends.
 * It fix the input length of parameter vector sequence, call
 * decode_end() (or decode_end_segmented() when last input was ended
 * by segmentation) to finalize the 1st pass.
 *
 * If the last input was ended by end-of-stream (in case input reached
 * EOF in file input etc.), process the rest samples remaining in the
 * delta buffers.
 *
 * @param recog [i/o] engine instance
 * 
 * @return TRUE on success, or FALSE on error.
 * </EN>
 */

/*
 * Slice of RealTimeParam to calculate features.
 */
void RealTimeParam_slice(Recog *recog)
{
  int loop_counter[35] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  boolean ret1;
  boolean ret2;
  RealBeam *r;
  int ret;
  int maxf;
  boolean ok_p;
  MFCCCalc *mfcc;
  Value *para;
  r = &recog->real;
  if (r->last_is_segmented)
  {
    loop_counter[0]++;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[1]++;
{}
{}
    }

{}
    {
      goto print_loop_counter;
    }
  }

  if (recog->jconf->input.type == INPUT_VECTOR)
  {
    loop_counter[2]++;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[3]++;
{}
{}
    }

{}
    {
      goto print_loop_counter;
    }
  }

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
  {
    loop_counter[4]++;
    if (mfcc->para->delta || mfcc->para->acc)
    {
      loop_counter[5]++;
      mfcc->valid = TRUE;
    }
    else
    {
      mfcc->valid = FALSE;
    }

  }

  while (1)
  {
    loop_counter[6]++;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[7]++;
      if (!mfcc->valid)
      {
        loop_counter[8]++;
        continue;
      }

      if (mfcc->f >= r->maxframelen)
      {
        loop_counter[9]++;
        mfcc->valid = FALSE;
      }

    }

    ok_p = FALSE;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[10]++;
      if (mfcc->valid)
      {
        loop_counter[11]++;
        ok_p = TRUE;
        break;
      }

    }

    if (!ok_p)
    {
      loop_counter[12]++;
      break;
    }

    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[13]++;
      para = mfcc->para;
      if (!mfcc->valid)
      {
        loop_counter[14]++;
        continue;
      }

      ret1 = WMP_deltabuf_flush(mfcc->db);
      if (ret1)
      {
        loop_counter[15]++;
        if (para->energy && para->absesup)
        {
          loop_counter[16]++;
{}
{}
        }
        else
        {
{}
        }

        if (para->acc)
        {
          loop_counter[17]++;
          ret2 = WMP_deltabuf_proceed(mfcc->ab, mfcc->tmpmfcc);
          if (ret2)
          {
            loop_counter[18]++;
{}
{}
          }
          else
          {
            continue;
          }

        }

      }
      else
      {
        if (para->acc)
        {
          loop_counter[19]++;
          ret2 = WMP_deltabuf_flush(mfcc->ab);
          if (ret2)
          {
            loop_counter[20]++;
{}
{}
          }
          else
          {
            mfcc->valid = FALSE;
            continue;
          }

        }
        else
        {
          mfcc->valid = FALSE;
          continue;
        }

      }

      if (para->cmn || para->cvn)
      {
        loop_counter[21]++;
{}
      }

      if (param_alloc(mfcc->param, mfcc->f + 1, mfcc->param->veclen) == FALSE)
      {
        loop_counter[22]++;
{}
        {
          goto print_loop_counter;
        }
      }

{}
    }

    ok_p = FALSE;
    maxf = 0;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[23]++;
      if (!mfcc->valid)
      {
        loop_counter[24]++;
        continue;
      }

      if (maxf < mfcc->f)
      {
        loop_counter[25]++;
        maxf = mfcc->f;
      }

      if (mfcc->f == 0)
      {
        loop_counter[26]++;
        ok_p = TRUE;
      }

    }

    if (ok_p && (maxf == 0))
    {
      loop_counter[27]++;
      if (recog->jconf->decodeopt.segment)
      {
        loop_counter[28]++;
        if (!recog->process_segment)
        {
          loop_counter[29]++;
{}
        }

{}
{}
{}
      }
      else
      {
{}
{}
{}
      }

    }

    ret = decode_proceed(recog);
    if (ret == (-1))
    {
      loop_counter[30]++;
      {
        goto print_loop_counter;
      }
    }
    else
      if (ret == 1)
    {
      loop_counter[31]++;
      break;
    }


{}
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[32]++;
      if (!mfcc->valid)
      {
        loop_counter[33]++;
        continue;
      }

      mfcc->f++;
    }

  }

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
  {
    loop_counter[34]++;
{}
{}
  }

{}
  {
    goto print_loop_counter;
  }
  {
    print_loop_counter:
    printf("loop counter = (");

    int i;
    for (i = 0; i < 35; i++)
      printf("%d, ", loop_counter[i]++);

    printf(")\n");
  }
}


/*
 * RealTimeParam with loop count instrumentation.
 */
boolean RealTimeParam_loop_counter(Recog *recog)
{
  boolean return_value = TRUE;
  int loop_counter[35] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  boolean ret1;
  boolean ret2;
  RealBeam * r;
  int ret;
  int maxf;
  boolean ok_p;
  MFCCCalc *mfcc;
  Value * para;
  r = &recog->real;
  if (r->last_is_segmented)
  {
    loop_counter[0]++;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[1]++;
      mfcc->param->header.samplenum = mfcc->f + 1;
      mfcc->param->samplenum = mfcc->f + 1;
    }

    decode_end_segmented(recog);
    return_value = TRUE;
    goto print_loop_counter;
    //return TRUE;
  }

  if (recog->jconf->input.type == INPUT_VECTOR)
  {
    loop_counter[2]++;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[3]++;
      mfcc->param->header.samplenum = mfcc->f;
      mfcc->param->samplenum = mfcc->f;
    }

    decode_end(recog);
    return_value = TRUE;
    goto print_loop_counter;
    //return TRUE;
  }

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
  {
    loop_counter[4]++;
    if (mfcc->para->delta || mfcc->para->acc)
    {
      loop_counter[5]++;
      mfcc->valid = TRUE;
    }
    else
    {
      mfcc->valid = FALSE;
    }

  }

  while (1)
  {
    loop_counter[6]++;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[7]++;
      if (!mfcc->valid)
      {
        loop_counter[8]++;
        continue;
      }

      if (mfcc->f >= r->maxframelen)
      {
        loop_counter[9]++;
        mfcc->valid = FALSE;
      }

    }

    ok_p = FALSE;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[10]++;
      if (mfcc->valid)
      {
        loop_counter[11]++;
        ok_p = TRUE;
        break;
      }

    }

    if (!ok_p)
    {
      loop_counter[12]++;
      break;
    }

    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[13]++;
      para = mfcc->para;
      if (!mfcc->valid)
      {
        loop_counter[14]++;
        continue;
      }

      ret1 = WMP_deltabuf_flush(mfcc->db);
      if (ret1)
      {
        loop_counter[15]++;
        if (para->energy && para->absesup)
        {
          loop_counter[16]++;
          memcpy(mfcc->tmpmfcc, mfcc->db->vec, (sizeof(VECT)) * (para->baselen - 1));
          memcpy(&mfcc->tmpmfcc[para->baselen - 1], &mfcc->db->vec[para->baselen], (sizeof(VECT)) * para->baselen);
        }
        else
        {
          memcpy(mfcc->tmpmfcc, mfcc->db->vec, ((sizeof(VECT)) * para->baselen) * 2);
        }

        if (para->acc)
        {
          loop_counter[17]++;
          ret2 = WMP_deltabuf_proceed(mfcc->ab, mfcc->tmpmfcc);
          if (ret2)
          {
            loop_counter[18]++;
            memcpy(mfcc->tmpmfcc, mfcc->ab->vec, (sizeof(VECT)) * (para->veclen - para->baselen));
            memcpy(&mfcc->tmpmfcc[para->veclen - para->baselen], &mfcc->ab->vec[para->veclen - para->baselen], (sizeof(VECT)) * para->baselen);
          }
          else
          {
            continue;
          }

        }

      }
      else
      {
        if (para->acc)
        {
          loop_counter[19]++;
          ret2 = WMP_deltabuf_flush(mfcc->ab);
          if (ret2)
          {
            loop_counter[20]++;
            memcpy(mfcc->tmpmfcc, mfcc->ab->vec, (sizeof(VECT)) * (para->veclen - para->baselen));
            memcpy(&mfcc->tmpmfcc[para->veclen - para->baselen], &mfcc->ab->vec[para->veclen - para->baselen], (sizeof(VECT)) * para->baselen);
          }
          else
          {
            mfcc->valid = FALSE;
            continue;
          }

        }
        else
        {
          mfcc->valid = FALSE;
          continue;
        }

      }

      if (para->cmn || para->cvn)
      {
        loop_counter[21]++;
        CMN_realtime(mfcc->cmn.wrk, mfcc->tmpmfcc);
      }

      if (param_alloc(mfcc->param, mfcc->f + 1, mfcc->param->veclen) == FALSE)
      {
        loop_counter[22]++;
        jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
        return_value = FALSE;
        goto print_loop_counter;
        //return FALSE;
      }

      memcpy(mfcc->param->parvec[mfcc->f], mfcc->tmpmfcc, (sizeof(VECT)) * mfcc->param->veclen);
    }

    ok_p = FALSE;
    maxf = 0;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[23]++;
      if (!mfcc->valid)
      {
        loop_counter[24]++;
        continue;
      }

      if (maxf < mfcc->f)
      {
        loop_counter[25]++;
        maxf = mfcc->f;
      }

      if (mfcc->f == 0)
      {
        loop_counter[26]++;
        ok_p = TRUE;
      }

    }

    if (ok_p && (maxf == 0))
    {
      loop_counter[27]++;
      if (recog->jconf->decodeopt.segment)
      {
        loop_counter[28]++;
        if (!recog->process_segment)
        {
          loop_counter[29]++;
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

    ret = decode_proceed(recog);
    if (ret == (-1))
    {
      loop_counter[30]++;
      return_value = -1;
      goto print_loop_counter;
      //return -1;
    }
    else
      if (ret == 1)
    {
      loop_counter[31]++;
      break;
    }


    callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
    {
      loop_counter[32]++;
      if (!mfcc->valid)
      {
        loop_counter[33]++;
        continue;
      }

      mfcc->f++;
    }

  }

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next)
  {
    loop_counter[34]++;
    mfcc->param->header.samplenum = mfcc->f;
    mfcc->param->samplenum = mfcc->f;
  }

  decode_end(recog);
  print_loop_counter:
  {
    printf("loop counter = (");
    int i;
    for (i = 0; i < 35; i++)
      printf("%d, ", loop_counter[i]++);

    printf(")\n");
  }
  return return_value;
  //return TRUE;

}

boolean
RealTimeParam(Recog *recog)
{

  boolean ret1, ret2;
  RealBeam *r;
  int ret;
  int maxf;
  boolean ok_p;
  MFCCCalc *mfcc;
  Value *para;
#ifdef RDEBUG
  int i;
#endif

  r = &(recog->real);

  if (r->last_is_segmented) {

    /* RealTimePipeLine で認識処理側の理由により認識が中断した場合,
       現状態のMFCC計算データをそのまま次回へ保持する必要があるので,
       MFCC計算終了処理を行わずに第１パスの結果のみ出力して終わる. */
    /* When input segmented by recognition process in RealTimePipeLine(),
       we have to keep the whole current status of MFCC computation to the
       next call.  So here we only output the 1st pass result. */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      mfcc->param->header.samplenum = mfcc->f + 1;/* len = lastid + 1 */
      mfcc->param->samplenum = mfcc->f + 1;
    }
    decode_end_segmented(recog);

    /* この区間の param データを第２パスのために返す */
    /* return obtained parameter for 2nd pass */
    return(TRUE);
  }

  if (recog->jconf->input.type == INPUT_VECTOR) {
    /* finalize real-time 1st pass */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      mfcc->param->header.samplenum = mfcc->f;
      mfcc->param->samplenum = mfcc->f;
    }
    /* 最終フレーム処理を行い，認識の結果出力と終了処理を行う */
    decode_end(recog);
    return TRUE;
  }

  /* MFCC計算の終了処理を行う: 最後の遅延フレーム分を処理 */
  /* finish MFCC computation for the last delayed frames */
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    if (mfcc->para->delta || mfcc->para->acc) {
      mfcc->valid = TRUE;
    } else {
      mfcc->valid = FALSE;
    }
  }

  /* loop until all data has been flushed */
  while (1) {

    /* check frame overflow */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (! mfcc->valid) continue;
      if (mfcc->f >= r->maxframelen) mfcc->valid = FALSE;
    }

    /* if all mfcc became invalid, exit loop here */
    ok_p = FALSE;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (mfcc->valid) {
	ok_p = TRUE;
	break;
      }
    }
    if (!ok_p) break;

    /* try to get 1 frame for all mfcc instances */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      
      para = mfcc->para;
      
      if (! mfcc->valid) continue;
      
      /* check if there is data in cycle buffer of delta */
      ret1 = WMP_deltabuf_flush(mfcc->db);
#ifdef RDEBUG
      printf("DeltaBufLast: ret=%d, status=", ret1);
      for(i=0;i<mfcc->db->len;i++) {
	printf("%d", mfcc->db->is_on[i]);
      }
      printf(", nextstore=%d\n", mfcc->db->store);
#endif
      if (ret1) {
	/* uncomputed delta has flushed, compute it with tmpmfcc */
	if (para->energy && para->absesup) {
	  memcpy(mfcc->tmpmfcc, mfcc->db->vec, sizeof(VECT) * (para->baselen - 1));
	  memcpy(&(mfcc->tmpmfcc[para->baselen-1]), &(mfcc->db->vec[para->baselen]), sizeof(VECT) * para->baselen);
	} else {
	  memcpy(mfcc->tmpmfcc, mfcc->db->vec, sizeof(VECT) * para->baselen * 2);
	}
	if (para->acc) {
	  /* this new delta should be given to the accel cycle buffer */
	  ret2 = WMP_deltabuf_proceed(mfcc->ab, mfcc->tmpmfcc);
#ifdef RDEBUG
	  printf("AccelBuf: ret=%d, status=", ret2);
	  for(i=0;i<mfcc->ab->len;i++) {
	    printf("%d", mfcc->ab->is_on[i]);
	  }
	  printf(", nextstore=%d\n", mfcc->ab->store);
#endif
	  if (ret2) {
	    /* uncomputed accel was given, compute it with tmpmfcc */
	    memcpy(mfcc->tmpmfcc, mfcc->ab->vec, sizeof(VECT) * (para->veclen - para->baselen));
	    memcpy(&(mfcc->tmpmfcc[para->veclen - para->baselen]), &(mfcc->ab->vec[para->veclen - para->baselen]), sizeof(VECT) * para->baselen);
	  } else {
	    /* still no input is given: */
	    /* in case of very short input: go on to the next input */
	    continue;
	  }
	}
	
      } else {
      
	/* no data left in the delta buffer */
	if (para->acc) {
	  /* no new data, just flush the accel buffer */
	  ret2 = WMP_deltabuf_flush(mfcc->ab);
#ifdef RDEBUG
	  printf("AccelBuf: ret=%d, status=", ret2);
	  for(i=0;i<mfcc->ab->len;i++) {
	    printf("%d", mfcc->ab->is_on[i]);
	  }
	  printf(", nextstore=%d\n", mfcc->ab->store);
#endif
	  if (ret2) {
	    /* uncomputed data has flushed, compute it with tmpmfcc */
	    memcpy(mfcc->tmpmfcc, mfcc->ab->vec, sizeof(VECT) * (para->veclen - para->baselen));
	    memcpy(&(mfcc->tmpmfcc[para->veclen - para->baselen]), &(mfcc->ab->vec[para->veclen - para->baselen]), sizeof(VECT) * para->baselen);
	  } else {
	    /* actually no data exists in both delta and accel */
	    mfcc->valid = FALSE; /* disactivate this instance */
	    continue;		/* end this loop */
	  }
	} else {
	  /* only delta: input fully flushed */
	  mfcc->valid = FALSE; /* disactivate this instance */
	  continue;		/* end this loop */
	}
      }
      /* a new frame has been obtained from delta buffer to tmpmfcc */
      if(para->cmn || para->cvn) CMN_realtime(mfcc->cmn.wrk, mfcc->tmpmfcc);
      if (param_alloc(mfcc->param, mfcc->f + 1, mfcc->param->veclen) == FALSE) {
	jlog("ERROR: failed to allocate memory for incoming MFCC vectors\n");
	return FALSE;
      }
      /* store to mfcc->f */
      memcpy(mfcc->param->parvec[mfcc->f], mfcc->tmpmfcc, sizeof(VECT) * mfcc->param->veclen);
#ifdef ENABLE_PLUGIN
      /* call postprocess plugin if any */
      plugin_exec_vector_postprocess(mfcc->param->parvec[mfcc->f], mfcc->param->veclen, mfcc->f);
#endif
    }

    /* call recognition start callback */
    ok_p = FALSE;
    maxf = 0;
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (!mfcc->valid) continue;
      if (maxf < mfcc->f) maxf = mfcc->f;
      if (mfcc->f == 0) {
	ok_p = TRUE;
      }
    }

    if (ok_p && maxf == 0) {
      /* call callback when at least one of MFCC has initial frame */
      if (recog->jconf->decodeopt.segment) {
#ifdef BACKEND_VAD
	  /* not exec pass1 begin callback here */
#else
	if (!recog->process_segment) {
	  callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
	}
	callback_exec(CALLBACK_EVENT_SEGMENT_BEGIN, recog);
	callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
	recog->triggered = TRUE;
#endif
      } else {
	callback_exec(CALLBACK_EVENT_RECOGNITION_BEGIN, recog);
	callback_exec(CALLBACK_EVENT_PASS1_BEGIN, recog);
	recog->triggered = TRUE;
      }
    }

    /* proceed for the curent frame */
    ret = decode_proceed(recog);
    if (ret == -1) {		/* error */
      return -1;
    } else if (ret == 1) {	/* segmented */
      /* loop out */
      break;
    } /* else no event occured */

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

    /* call frame-wise callback */
    callback_exec(CALLBACK_EVENT_PASS1_FRAME, recog);

    /* move to next */
    for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
      if (! mfcc->valid) continue;
      mfcc->f++;
    }
  }

  /* finalize real-time 1st pass */
  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    mfcc->param->header.samplenum = mfcc->f;
    mfcc->param->samplenum = mfcc->f;
  }
  /* 最終フレーム処理を行い，認識の結果出力と終了処理を行う */
  decode_end(recog);

  return(TRUE);
}

/** 
 * <JA>
 * ケプストラム平均の更新. 
 * 次回の認識に備えて，入力データからCMN用のケプストラム平均を更新する. 
 * 
 * @param mfcc [i/o] 計算対象の MFCC計算インスタンス
 * @param recog [i/o] エンジンインスタンス
 *
 * </JA>
 * <EN>
 * Update cepstral mean.
 *
 * This function updates the initial cepstral mean for CMN of the next input.
 *
 * @param mfcc [i/o] MFCC Calculation instance to update its CMN
 * @param recog [i/o] engine instance
 * </EN>
 */
void
RealTimeCMNUpdate(MFCCCalc *mfcc, Recog *recog)
{
  boolean cmn_update_p;
  Value *para;
  Jconf *jconf;
  RecogProcess *r;

  jconf = recog->jconf;
  para = mfcc->para;
  
  /* update CMN vector for next speech */
  if(para->cmn) {
    if (mfcc->cmn.update) {
      cmn_update_p = TRUE;
      for(r=recog->process_list;r;r=r->next) {
	if (!r->live) continue;
	if (r->am->mfcc != mfcc) continue;
	if (r->result.status < 0) { /* input rejected */
	  cmn_update_p = FALSE;
	  break;
	}
      }
      if (cmn_update_p) {
	/* update last CMN parameter for next spech */
	CMN_realtime_update(mfcc->cmn.wrk, mfcc->param);
      } else {
	/* do not update, because the last input is bogus */
	if (verbose_flag) {
#ifdef BACKEND_VAD
	  if (!recog->jconf->decodeopt.segment || recog->triggered) {
	    jlog("STAT: skip CMN parameter update since last input was invalid\n");
	  }
#else
	  jlog("STAT: skip CMN parameter update since last input was invalid\n");
#endif
	}
      }
    }
    /* if needed, save the updated CMN parameter to a file */
    if (mfcc->cmn.save_filename) {
      if (CMN_save_to_file(mfcc->cmn.wrk, mfcc->cmn.save_filename) == FALSE) {
	jlog("WARNING: failed to save CMN parameter to \"%s\"\n", mfcc->cmn.save_filename);
      }
    }
  }
}

/** 
 * <JA>
 * 第1パス平行認識処理を中断する. 
 *
 * @param recog [i/o] エンジンインスタンス
 * </JA>
 * <EN>
 * Terminate the 1st pass on-the-fly decoding.
 *
 * @param recog [i/o] engine instance
 * </EN>
 */
void
RealTimeTerminate(Recog *recog)
{
  MFCCCalc *mfcc;

  for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
    mfcc->param->header.samplenum = mfcc->f;
    mfcc->param->samplenum = mfcc->f;
  }

  /* 最終フレーム処理を行い，認識の結果出力と終了処理を行う */
  decode_end(recog);
}

/** 
 * <EN>
 * Free the whole work area for 1st pass on-the-fly decoding
 * </EN>
 * <JA>
 * 第1パス並行処理のためのワークエリアを開放する
 * </JA>
 * 
 * @param recog [in] engine instance
 * 
 */
void
realbeam_free(Recog *recog)
{
  RealBeam *r;

  r = &(recog->real);

  if (recog->real.window) {
    free(recog->real.window);
    recog->real.window = NULL;
  }
  if (recog->real.rest_Speech) {
    free(recog->real.rest_Speech);
    recog->real.rest_Speech = NULL;
  }
}



/************************************************************************/
/************************************************************************/
/************************************************************************/
/************************************************************************/

/* MFCC realtime input */
/** 
 * <EN>
 * 
 * </EN>
 * <JA>
 * 
 * </JA>
 * 
 * @param recog 
 * @param ad_check 
 * 
 * @return 2 when input termination requested by recognition process,
 * 1 when segmentation request returned from input module, 0 when end
 * of input returned from input module, -1 on error, -2 when input
 * termination requested by ad_check().
 * 
 */
int
mfcc_go(Recog *recog, int (*ad_check)(Recog *))
{
  RealBeam *r;
  MFCCCalc *mfcc;
  int new_f;
  int ret, ret3;

  r = &(recog->real);

  r->last_is_segmented = FALSE;
  
  while(1/*in_data_vec*/) {

    ret = mfc_module_read(recog->mfcclist, &new_f);

    if (debug2_flag) {
      if (recog->mfcclist->f < new_f) {
	jlog("%d: %d (%d)\n", recog->mfcclist->f, new_f, ret);
      }
    }
 
    /* callback poll */
    if (ad_check != NULL) {
      ret3 = (*(ad_check))(recog);
      if (ret3 < 0) {
      //if ((ret3 = (*(ad_check))(recog)) < 0) {
	if ((ret3 == -1 && recog->mfcclist->f == 0) || ret3 == -2) {
	  return(-2);
	}
      }
    }

    while(recog->mfcclist->f < new_f) {

      recog->mfcclist->valid = TRUE;

#ifdef ENABLE_PLUGIN
      /* call post-process plugin if exist */
      plugin_exec_vector_postprocess(recog->mfcclist->param->parvec[recog->mfcclist->f], recog->mfcclist->param->veclen, recog->mfcclist->f);
#endif

      /* 処理を1フレーム進める */
      /* proceed one frame */
      
      switch(proceed_one_frame(recog)) {
      case -1:			/* error */
	return -1;
      case 0:			/* normal */
	break;
      case 1:			/* segmented by process */
	return 2;
      }

      /* 1フレーム処理が進んだのでポインタを進める */
      /* proceed frame pointer */
      for (mfcc = recog->mfcclist; mfcc; mfcc = mfcc->next) {
	if (!mfcc->valid) continue;
	mfcc->f++;
      }
    }
    
    /* check if input end */
    switch(ret) {
    case -1: 			/* end of input */
      return 0;
    case -2:			/* error */
      return -1;
    case -3:			/* end of segment request */
      return 1;
    }
  }
  /* 与えられた音声セグメントに対する認識処理が全て終了
     呼び出し元に, 入力を続けるよう伝える */
  /* input segment is fully processed
     tell the caller to continue input */
  return(1);
}

/* end of file */



/**
 * @file   adin-cut.c
 *
 * <JA>
 * @brief  音声キャプチャおよび有音区間検出
 *
 * 音声入力デバイスからの音声データの取り込み，および
 * 音の存在する区間の検出を行ないます. 
 *
 * 有音区間の検出は，振幅レベルと零交差数を用いて行ないます. 
 * 入力断片ごとに，レベルしきい値を越える振幅について零交差数をカウントし，
 * それが指定した数以上になれば，音の区間開始検出として
 * 取り込みを開始します. 取り込み中に零交差数が指定数以下になれば，
 * 取り込みを停止します. 実際には頑健に切り出しを行なうため，開始部と
 * 停止部の前後にマージンを持たせて切り出します. 
 * 
 * また，オプション指定 (-zmean)により DC offset の除去をここで行ないます. 
 * offset は最初の @a ZMEANSAMPLES 個のサンプルの平均から計算されます. 
 *
 * 音声データの取り込みと並行して入力音声の処理を行ないます. このため，
 * 取り込んだ音声データはその取り込み単位（live入力では一定時間，音声ファイル
 * ではバッファサイズ）ごとに，それらを引数としてコールバック関数が呼ばれます. 
 * このコールバック関数としてデータの保存や特徴量抽出，
 * （フレーム同期の）認識処理を進める関数を指定します. 
 *
 * マイク入力や NetAudio 入力などの Live 入力では，
 * コールバック内の処理が重く処理が入力の速度に追い付かないと，
 * デバイスのバッファが溢れ，入力断片がロストする場合があります. 
 * このエラーを防ぐため，実行環境で pthread が使用可能である場合，
 * 音声取り込み・区間検出部は本体と独立したスレッドで動作します. 
 * この場合，このスレッドは本スレッドとバッファ @a speech を介して
 * 以下のように協調動作します. 
 * 
 *    - Thread 1: 音声取り込み・音区間検出スレッド
 *        - デバイスから音声データを読み込みながら音区間検出を行なう. 
 *          検出した音区間のサンプルはバッファ @a speech の末尾に逐次
 *          追加される. 
 *        - このスレッドは起動時から本スレッドから独立して動作し，
 *          上記の動作を行ない続ける. 
 *    - Thread 2: 音声処理・認識処理を行なう本スレッド
 *        - バッファ @a speech を一定時間ごとに監視し，新たなサンプルが
 *          Thread 1 によって追加されたらそれらを処理し，処理が終了した
 *          分バッファを詰める. 
 *
 * </JA>
 * <EN>
 * @brief  Capture audio and detect sound trigger
 *
 * This file contains functions to get waveform from an audio device
 * and detect speech/sound input segment
 *
 * Sound detection at this stage is based on level threshold and zero
 * cross count.  The number of zero cross are counted for each
 * incoming sound fragment.  If the number becomes larger than
 * specified threshold, the fragment is treated as a beginning of
 * sound/speech input (trigger on).  If the number goes below the threshold,
 * the fragment will be treated as an end of input (trigger
 * off).  In actual detection, margins are considered on the beginning
 * and ending point, which will be treated as head and tail silence
 * part.  DC offset normalization will be also performed if configured
 * so (-zmean).
 *
 * The triggered input data should be processed concurrently with the
 * detection for real-time recognition.  For this purpose, after the
 * beginning of input has been detected, the following triggered input
 * fragments (samples of a certain period in live input, or buffer size in
 * file input) are passed sequencially in turn to a callback function.
 * The callback function should be specified by the caller, typicaly to
 * store the recoded data, or to process them into a frame-synchronous
 * recognition process.
 *
 * When source is a live input such as microphone, the device buffer will
 * overflow if the processing callback is slow.  In that case, some input
 * fragments may be lost.  To prevent this, the A/D-in part together with
 * sound detection will become an independent thread if @em pthread functions
 * are supported.  The A/D-in and detection thread will cooperate with
 * the original main thread through @a speech buffer, like the followings:
 *
 *    - Thread 1: A/D-in and speech detection thread
 *        - reads audio input from source device and perform sound detection.
 *          The detected fragments are immediately appended
 *          to the @a speech buffer.
 *        - will be detached after created, and run forever till the main
 *          thread dies.
 *    - Thread 2: Main thread
 *        - performs input processing and recognition.
 *        - watches @a speech buffer, and if detect appendings of new samples
 *          by the Thread 1, proceed the processing for the appended samples
 *          and purge the finished samples from @a speech buffer.
 *
 * </EN>
 *
 * @sa adin.c
 *
 * @author Akinobu LEE
 * @date   Sat Feb 12 13:20:53 2005
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
//#ifdef HAVE_PTHREAD
//#include <pthread.h>
//#endif

/// Define this if you want to output a debug message for threading
#undef THREAD_DEBUG
/// Enable some fixes relating adinnet+module
#define TMP_FIX_200602		

/** 
 * <EN>
 * @brief  Set up parameters for A/D-in and input detection.
 *
 * Set variables in work area according to the configuration values.
 * 
 * </EN>
 * <JA>
 * @brief  音声切り出し用各種パラメータをセット
 *
 * 設定を元に切り出し用のパラメータを計算し，ワークエリアにセットします. 
 * 
 * </JA>
 * @param adin [in] AD-in work area
 * @param jconf [in] configuration data
 *
 * @callergraph
 * @callgraph
 */
boolean
adin_setup_param(ADIn *adin, Jconf *jconf)
{
  float samples_in_msec;
  int freq;

  if (jconf->input.sfreq <= 0) {
    jlog("ERROR: adin_setup_param: going to set smpfreq to %d\n", jconf->input.sfreq);
    return FALSE;
  }
  if (jconf->detect.silence_cut < 2) {
    adin->adin_cut_on = (jconf->detect.silence_cut == 1) ? TRUE : FALSE;
  } else {
    adin->adin_cut_on = adin->silence_cut_default;
  }
  adin->strip_flag = jconf->preprocess.strip_zero_sample;
  adin->thres = jconf->detect.level_thres;
#ifdef HAVE_PTHREAD
  if (adin->enable_thread && jconf->decodeopt.segment) {
    adin->ignore_speech_while_recog = FALSE;
  } else {
    adin->ignore_speech_while_recog = TRUE;
  }
#endif
  adin->need_zmean = jconf->preprocess.use_zmean;
  adin->level_coef = jconf->preprocess.level_coef;
  /* calc & set internal parameter from configuration */
  freq = jconf->input.sfreq;
  samples_in_msec = (float) freq / (float)1000.0;
  adin->chunk_size = jconf->detect.chunk_size;
  /* cycle buffer length = head margin length */
  adin->c_length = (int)((float)jconf->detect.head_margin_msec * samples_in_msec);	/* in msec. */
  if (adin->chunk_size > adin->c_length) {
    jlog("ERROR: adin_setup_param: chunk size (%d) > header margin (%d)\n", adin->chunk_size, adin->c_length);
    return FALSE;
  }
  /* compute zerocross trigger count threshold in the cycle buffer */
  adin->noise_zerocross = jconf->detect.zero_cross_num * adin->c_length / freq;
  /* variables that comes from the tail margin length (in wstep) */
  adin->nc_max = (int)((float)(jconf->detect.tail_margin_msec * samples_in_msec / (float)adin->chunk_size)) + 2;
  adin->sbsize = jconf->detect.tail_margin_msec * samples_in_msec + (adin->c_length * jconf->detect.zero_cross_num / 200);
  adin->c_offset = 0;

#ifdef HAVE_PTHREAD
  adin->transfer_online = FALSE;
  adin->speech = NULL;
  if (jconf->reject.rejectlonglen >= 0) {
    adin->freezelen = (jconf->reject.rejectlonglen + 500.0) * jconf->input.sfreq / 1000.0;
  } else {
    adin->freezelen = MAXSPEECHLEN;
  }
#endif

  /**********************/
  /* initialize buffers */
  /**********************/
  adin->buffer = (SP16 *)mymalloc(sizeof(SP16) * MAXSPEECHLEN);
  adin->cbuf = (SP16 *)mymalloc(sizeof(SP16) * adin->c_length);
  adin->swapbuf = (SP16 *)mymalloc(sizeof(SP16) * adin->sbsize);
  if (adin->down_sample) {
    adin->io_rate = 3;		/* 48 / 16 (fixed) */
    adin->buffer48 = (SP16 *)mymalloc(sizeof(SP16) * MAXSPEECHLEN * adin->io_rate);
  }
  if (adin->adin_cut_on) {
    init_count_zc_e(&(adin->zc), adin->c_length);
  }
  
  adin->need_init = TRUE;

  adin->rehash = FALSE;

  return TRUE;

}

/** 
 * <EN>
 * Purge samples already processed in the temporary buffer.
 * </EN>
 * <JA>
 * テンポラリバッファにある処理されたサンプルをパージする.
 * </JA>
 * 
 * @param a [in] AD-in work area
 * @param from [in] Purge samples in range [0..from-1].
 * 
 */
static void
adin_purge(ADIn *a, int from)
{
  if (from > 0 && a->current_len - from > 0) {
    memmove(a->buffer, &(a->buffer[from]), (a->current_len - from) * sizeof(SP16));
  }
  a->bp = a->current_len - from;
}

/** 
 * <EN>
 * @brief  Main A/D-in and sound detection function
 *
 * This function read inputs from device and do sound detection
 * (both up trigger and down trigger) until end of device.
 *
 * In threaded mode, this function will detach and loop forever as
 * ad-in thread, (adin_thread_create()) storing triggered samples in
 * speech[], and telling the status to another process thread via @a
 * transfer_online in work area.  The process thread, called from
 * adin_go(), polls the length of speech[] and transfer_online in work area
 * and process them if new samples has been stored.
 *
 * In non-threaded mode, this function will be called directly from
 * adin_go(), and triggered samples are immediately processed within here.
 *
 * Threaded mode should be used for "live" input such as microphone input
 * where input is infinite and capture delay is severe.  For file input,
 * adinnet input and other "buffered" input, non-threaded mode will be used.
 *
 * Argument "ad_process()" should be a function to process the triggered
 * input samples.  On real-time recognition, a frame-synchronous search
 * function for the first pass will be specified by the caller.  The current
 * input will be segmented if it returns 1, and will be terminated as error
 * if it returns -1.
 *
 * When the argument "ad_check()" specified, it will be called periodically.
 * When it returns less than 0, this function will be terminated.
 * 
 * </EN>
 * <JA>
 * @brief  音声入力と音検出を行うメイン関数
 *
 * ここでは音声入力の取り込み，音区間の開始・終了の検出を行います. 
 *
 * スレッドモード時，この関数は独立したAD-inスレッドとしてデタッチされます. 
 * (adin_thread_create()), 音入力を検知するとこの関数はワークエリア内の
 * speech[] にトリガしたサンプルを記録し，かつ transfer_online を TRUE に
 * セットします. Julius のメイン処理スレッド (adin_go()) は
 * adin_thread_process() に移行し，そこで transfer_online 時に speech[] を
 * 参照しながら認識処理を行います. 
 *
 * 非スレッドモード時は，メイン処理関数 adin_go() は直接この関数を呼び，
 * 認識処理はこの内部で直接行われます. 
 *
 * スレッドモードはマイク入力など，入力が無限で処理の遅延がデータの
 * 取りこぼしを招くような live input で用いられます. 一方，ファイル入力
 * やadinnet 入力のような buffered input では非スレッドモードが用いられます. 
 *
 * 引数の ad_process は，取り込んだサンプルに対して処理を行う関数を
 * 指定します. リアルタイム認識を行う場合は，ここに第1パスの認識処理を
 * 行う関数が指定されます. 返り値が 1 であれば，入力をここで区切ります. 
 * -1 であればエラー終了します. 
 * 
 * 引数の ad_check は一定処理ごとに繰り返し呼ばれる関数を指定します. この
 * 関数の返り値が 0 以下だった場合，入力を即時中断して関数を終了します. 
 * </JA>
 *
 * @param ad_process [in] function to process triggerted input.
 * @param ad_check [in] function to be called periodically.
 * @param recog [in] engine instance
 * 
 * @return 2 when input termination requested by ad_process(), 1 when
 * if detect end of an input segment (down trigger detected after up
 * trigger), 0 when reached end of input device, -1 on error, -2 when
 * input termination requested by ad_check().
 *
 * @callergraph
 * @callgraph
 *
 */
static int
adin_cut(int (*ad_process)(SP16 *, int, Recog *), int (*ad_check)(Recog *), Recog *recog)
{
  ADIn *a;
  int i;
  int ad_process_ret;
  int imax, len, cnt;
  int wstep;
  int end_status = 0;	/* return value */
  boolean transfer_online_local;	/* local repository of transfer_online */
  int zc;		/* count of zero cross */

  a = recog->adin;

  /*
   * there are 3 buffers:
   *   temporary storage queue: buffer[]
   *   cycle buffer for zero-cross counting: (in zc_e)
   *   swap buffer for re-starting after short tail silence
   *
   * Each samples are first read to buffer[], then passed to count_zc_e()
   * to find trigger.  Samples between trigger and end of speech are 
   * passed to (*ad_process) with pointer to the first sample and its length.
   *
   */

  if (a->need_init) {
    a->bpmax = MAXSPEECHLEN;
    a->bp = 0;
    a->is_valid_data = FALSE;
    /* reset zero-cross status */
    if (a->adin_cut_on) {
      reset_count_zc_e(&(a->zc), a->thres, a->c_length, a->c_offset);
    }
    a->end_of_stream = FALSE;
    a->nc = 0;
    a->sblen = 0;
    a->need_init = FALSE;		/* for next call */
  }

  /****************/
  /* resume input */
  /****************/
  //  if (!a->adin_cut_on && a->is_valid_data == TRUE) {
  //    callback_exec(CALLBACK_EVENT_SPEECH_START, recog);
  //  }

  /*************/
  /* main loop */
  /*************/
  for (;;) {

    /* check end of input by end of stream */
    if (a->end_of_stream && a->bp == 0) break;

    /****************************/
    /* read in new speech input */
    /****************************/
    if (a->end_of_stream) {
      /* already reaches end of stream, just process the rest */
      a->current_len = a->bp;
    } else {
      /*****************************************************/
      /* get samples from input device to temporary buffer */
      /*****************************************************/
      /* buffer[0..bp] is the current remaining samples */
      /*
	mic input - samples exist in a device buffer
        tcpip input - samples exist in a socket
        file input - samples in a file
	   
	Return value is the number of read samples.
	If no data exists in the device (in case of mic input), ad_read()
	will return 0.  If reached end of stream (in case end of file or
	receive end ack from tcpip client), it will return -1.
	If error, returns -2. If the device requests segmentation, returns -3.
      */
      if (a->down_sample) {
	/* get 48kHz samples to temporal buffer */
	int ad_read_result = (*(a->ad_read))(a->buffer48, (a->bpmax - a->bp) * a->io_rate);
  cnt = ad_read_result;
	//cnt = (*(a->ad_read))(a->buffer48, (a->bpmax - a->bp) * a->io_rate);
      } else {
	int ad_read_result = (*(a->ad_read))(&(a->buffer[a->bp]), a->bpmax - a->bp);
  cnt = ad_read_result;
	//cnt = (*(a->ad_read))(&(a->buffer[a->bp]), a->bpmax - a->bp);
      }
      if (cnt < 0) {		/* end of stream / segment or error */
	/* set the end status */
	switch(cnt) {
	case -1:		/* end of stream */
	  a->input_side_segment = FALSE;
	  end_status = 0;
	  break;
	case -2:
	  a->input_side_segment = FALSE;
	  end_status = -1;
	  break;
	case -3:
	  a->input_side_segment = TRUE;
	  end_status = 0;
	}
	/* now the input has been ended, 
	   we should not get further speech input in the next loop, 
	   instead just process the samples in the temporary buffer until
	   the entire data is processed. */
	a->end_of_stream = TRUE;		
	cnt = 0;			/* no new input */
	/* in case the first trial of ad_read() fails, exit this loop */
	if (a->bp == 0) break;
      }
      if (a->down_sample && cnt != 0) {
	/* convert to 16kHz  */
	cnt = ds48to16(&(a->buffer[a->bp]), a->buffer48, cnt, a->bpmax - a->bp, a->ds);
	if (cnt < 0) {		/* conversion error */
	  jlog("ERROR: adin_cut: error in down sampling\n");
	  end_status = -1;
	  a->end_of_stream = TRUE;
	  cnt = 0;
	  if (a->bp == 0) break;
	}
      }
      if (cnt > 0 && a->level_coef != 1.0) {
	/* scale the level of incoming input */
	for (i = a->bp; i < a->bp + cnt; i++) {
	  a->buffer[i] = (SP16) ((float)a->buffer[i] * a->level_coef);
	}
      }

      /*************************************************/
      /* execute callback here for incoming raw data stream.*/
      /* the content of buffer[bp...bp+cnt-1] or the   */
      /* length can be modified in the functions.      */
      /*************************************************/
      if (cnt > 0) {
#ifdef ENABLE_PLUGIN
	plugin_exec_adin_captured(&(a->buffer[a->bp]), cnt);
#endif
	callback_exec_adin(CALLBACK_ADIN_CAPTURED, recog, &(a->buffer[a->bp]), cnt);
	/* record total number of captured samples */
	a->total_captured_len += cnt;
      }

      /*************************************************/
      /* some speech processing for the incoming input */
      /*************************************************/
      if (cnt > 0) {
	if (a->strip_flag) {
	  /* strip off successive zero samples */
	  len = strip_zero(&(a->buffer[a->bp]), cnt);
	  if (len != cnt) cnt = len;
	}
	if (a->need_zmean) {
	  /* remove DC offset */
	  sub_zmean(&(a->buffer[a->bp]), cnt);
	}
      }
      
      /* current len = current samples in buffer */
      a->current_len = a->bp + cnt;
    }
#ifdef THREAD_DEBUG
    if (a->end_of_stream) {
      jlog("DEBUG: adin_cut: stream already ended\n");
    }
    if (cnt > 0) {
      jlog("DEBUG: adin_cut: get %d samples [%d-%d]\n", a->current_len - a->bp, a->bp, a->current_len);
    }
#endif

    /**************************************************/
    /* call the periodic callback (non threaded mode) */
    /*************************************************/
    /* this function is mainly for periodic checking of incoming command
       in module mode */
    /* in threaded mode, this will be done in process thread, not here in adin thread */
    if (ad_check != NULL
#ifdef HAVE_PTHREAD
	&& !a->enable_thread
#endif
	) {
      /* if ad_check() returns value < 0, termination of speech input is required */
      //i = (*(ad_check))(recog);
      i = callback_check_in_adin(recog);
      if (i < 0) {
      //if ((i = (*ad_check)(recog)) < 0) { /* -1: soft termination -2: hard termination */
	//	if ((i == -1 && current_len == 0) || i == -2) {
 	if (i == -2 ||
	    (i == -1 && a->is_valid_data == FALSE)) {
	  end_status = -2;	/* recognition terminated by outer function */
	  /* execute callback */
	  if (a->current_len > 0) {
	    callback_exec(CALLBACK_EVENT_SPEECH_STOP, recog);
	  }
	  a->need_init = TRUE; /* bufer status shoule be reset at next call */
	  goto break_input;
	}
      }
    }

    /***********************************************************************/
    /* if no data has got but not end of stream, repeat next input samples */
    /***********************************************************************/
    if (a->current_len == 0) continue;

    /* When not adin_cut mode, all incoming data is valid.
       So is_valid_data should be set to TRUE when some input first comes
       till this input ends.  So, if some data comes, set is_valid_data to
       TRUE here. */ 
    if (!a->adin_cut_on && a->is_valid_data == FALSE && a->current_len > 0) {
      a->is_valid_data = TRUE;
      callback_exec(CALLBACK_EVENT_SPEECH_START, recog);
    }

    /******************************************************/
    /* prepare for processing samples in temporary buffer */
    /******************************************************/
    
    wstep = a->chunk_size;	/* process unit (should be smaller than cycle buffer) */

    /* imax: total length that should be processed at one ad_read() call */
    /* if in real-time mode and not threaded, recognition process 
       will be called and executed as the ad_process() callback within
       this function.  If the recognition speed is over the real time,
       processing all the input samples at the loop below may result in the
       significant delay of getting next input, that may result in the buffer
       overflow of the device (namely a microphone device will suffer from
       this). So, in non-threaded mode, in order to avoid buffer overflow and
       input frame dropping, we will leave here by processing 
       only one segment [0..wstep], and leave the rest in the temporary buffer.
    */
#ifdef HAVE_PTHREAD
    if (a->enable_thread) imax = a->current_len; /* process whole */
    else imax = (a->current_len < wstep) ? a->current_len : wstep; /* one step */
#else
    imax = (a->current_len < wstep) ? a->current_len : wstep;	/* one step */
#endif
    
    /* wstep: unit length for the loop below */
    if (wstep > a->current_len) wstep = a->current_len;

#ifdef THREAD_DEBUG
    jlog("DEBUG: process %d samples by %d step\n", imax, wstep);
#endif

#ifdef HAVE_PTHREAD
    if (a->enable_thread) {
      /* get transfer status to local */
      pthread_mutex_lock(&(a->mutex));
      transfer_online_local = a->transfer_online;
      pthread_mutex_unlock(&(a->mutex));
    }
#endif

    /*********************************************************/
    /* start processing buffer[0..current_len] by wstep step */
    /*********************************************************/
    i = 0;
    while (i + wstep <= imax) {

      if (a->adin_cut_on) {

	/********************/
	/* check triggering */
	/********************/
	/* the cycle buffer in count_zc_e() holds the last
	   samples of (head_margin) miliseconds, and the zerocross
	   over the threshold level are counted within the cycle buffer */
	
	/* store the new data to cycle buffer and update the count */
	/* return zero-cross num in the cycle buffer */
	zc = count_zc_e(&(a->zc), &(a->buffer[i]), wstep);
	
	if (zc > a->noise_zerocross) { /* now triggering */
	  
	  if (a->is_valid_data == FALSE) {
	    /*****************************************************/
	    /* process off, trigger on: detect speech triggering */
	    /*****************************************************/
	    a->is_valid_data = TRUE;   /* start processing */
	    a->nc = 0;
#ifdef THREAD_DEBUG
	    jlog("DEBUG: detect on\n");
#endif
	    /* record time */
	    a->last_trigger_sample = a->total_captured_len - a->current_len + i + wstep - a->zc.valid_len;
	    callback_exec(CALLBACK_EVENT_SPEECH_START, recog);
	    a->last_trigger_len = 0;
	    if (a->zc.valid_len > wstep) {
	      a->last_trigger_len += a->zc.valid_len - wstep;
	    }

	    /****************************************/
	    /* flush samples stored in cycle buffer */
	    /****************************************/
	    /* (last (head_margin) msec samples */
	    /* if threaded mode, processing means storing them to speech[].
	       if ignore_speech_while_recog is on (default), ignore the data
	       if transfer is offline (=while processing second pass).
	       Else, datas are stored even if transfer is offline */
	    if ( ad_process != NULL
#ifdef HAVE_PTHREAD
		 && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
		 ) {
	      /* copy content of cycle buffer to cbuf */
	      zc_copy_buffer(&(a->zc), a->cbuf, &len);
	      /* Note that the last 'wstep' samples are the same as
		 the current samples 'buffer[i..i+wstep]', and
		 they will be processed later.  So, here only the samples
		 cbuf[0...len-wstep] will be processed
	      */
	      if (len - wstep > 0) {
#ifdef THREAD_DEBUG
		jlog("DEBUG: callback for buffered samples (%d bytes)\n", len - wstep);
#endif
#ifdef ENABLE_PLUGIN
		plugin_exec_adin_triggered(a->cbuf, len - wstep);
#endif
		callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a->cbuf, len - wstep);
		ad_process_ret = RealTimePipeLine(a->cbuf, len - wstep, recog);
		switch(ad_process_ret) {
		case 1:		/* segmentation notification from process callback */
#ifdef HAVE_PTHREAD
		  if (a->enable_thread) {
		    /* in threaded mode, just stop transfer */
		    pthread_mutex_lock(&(a->mutex));
		    a->transfer_online = transfer_online_local = FALSE;
		    pthread_mutex_unlock(&(a->mutex));
		  } else {
		    /* in non-threaded mode, set end status and exit loop */
		    end_status = 2;
		    adin_purge(a, i);
		    goto break_input;
		  }
		  break;
#else
		  /* in non-threaded mode, set end status and exit loop */
		  end_status = 2;
		  adin_purge(a, i);
		  goto break_input;
#endif
		case -1:		/* error occured in callback */
		  /* set end status and exit loop */
		  end_status = -1;
		  goto break_input;
		}
	      }
	    }
	    
	  } else {		/* is_valid_data == TRUE */
	    /******************************************************/
	    /* process on, trigger on: we are in a speech segment */
	    /******************************************************/
	    
	    if (a->nc > 0) {
	      
	      /*************************************/
	      /* re-triggering in trailing silence */
	      /*************************************/
	      
#ifdef THREAD_DEBUG
	      jlog("DEBUG: re-triggered\n");
#endif
	      /* reset noise counter */
	      a->nc = 0;

	      if (a->sblen > 0) {
		a->last_trigger_len += a->sblen;
	      }

#ifdef TMP_FIX_200602
	      if (ad_process != NULL
#ifdef HAVE_PTHREAD
		  && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
		  ) {
#endif
	      
	      /*************************************************/
	      /* process swap buffer stored while tail silence */
	      /*************************************************/
	      /* In trailing silence, the samples within the tail margin length
		 will be processed immediately, but samples after the tail
		 margin will not be processed, instead stored in swapbuf[].
		 If re-triggering occurs while in the trailing silence,
		 the swapped samples should be processed now to catch up
		 with current input
	      */
	      if (a->sblen > 0) {
#ifdef THREAD_DEBUG
		jlog("DEBUG: callback for swapped %d samples\n", a->sblen);
#endif
#ifdef ENABLE_PLUGIN
		plugin_exec_adin_triggered(a->swapbuf, a->sblen);
#endif
		callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, a->swapbuf, a->sblen);
		ad_process_ret = RealTimePipeLine(a->swapbuf, a->sblen, recog);
		a->sblen = 0;
		switch(ad_process_ret) {
		case 1:		/* segmentation notification from process callback */
#ifdef HAVE_PTHREAD
		  if (a->enable_thread) {
		    /* in threaded mode, just stop transfer */
		    pthread_mutex_lock(&(a->mutex));
		    a->transfer_online = transfer_online_local = FALSE;
		    pthread_mutex_unlock(&(a->mutex));
		  } else {
		    /* in non-threaded mode, set end status and exit loop */
		    end_status = 2;
		    adin_purge(a, i);
		    goto break_input;
		  }
		  break;
#else
		  /* in non-threaded mode, set end status and exit loop */
		  end_status = 2;
		  adin_purge(a, i);
		  goto break_input;
#endif
		case -1:		/* error occured in callback */
		  /* set end status and exit loop */
		  end_status = -1;
		  goto break_input;
		}
	      }
#ifdef TMP_FIX_200602
	      }
#endif
	    }
	  } 
	} else if (a->is_valid_data == TRUE) {
	  
	  /*******************************************************/
	  /* process on, trigger off: processing tailing silence */
	  /*******************************************************/
	  
#ifdef THREAD_DEBUG
	  jlog("DEBUG: TRAILING SILENCE\n");
#endif
	  if (a->nc == 0) {
	    /* start of tail silence: prepare valiables for start swapbuf[] */
	    a->rest_tail = a->sbsize - a->c_length;
	    a->sblen = 0;
#ifdef THREAD_DEBUG
	    jlog("DEBUG: start tail silence, rest_tail = %d\n", a->rest_tail);
#endif
	  }

	  /* increment noise counter */
	  a->nc++;
	}
      }	/* end of triggering handlers */
      
      
      /********************************************************************/
      /* process the current segment buffer[i...i+wstep] if process == on */
      /********************************************************************/
      
      if (a->adin_cut_on && a->is_valid_data && a->nc > 0 && a->rest_tail == 0) {
	
	/* The current trailing silence is now longer than the user-
	   specified tail margin length, so the current samples
	   should not be processed now.  But if 're-triggering'
	   occurs in the trailing silence later, they should be processed
	   then.  So we just store the overed samples in swapbuf[] and
	   not process them now */
	
#ifdef THREAD_DEBUG
	jlog("DEBUG: tail silence over, store to swap buffer (nc=%d, rest_tail=%d, sblen=%d-%d)\n", a->nc, a->rest_tail, a->sblen, a->sblen+wstep);
#endif
	if (a->sblen + wstep > a->sbsize) {
	  jlog("ERROR: adin_cut: swap buffer for re-triggering overflow\n");
	}
	memcpy(&(a->swapbuf[a->sblen]), &(a->buffer[i]), wstep * sizeof(SP16));
	a->sblen += wstep;
	
      } else {

	/* we are in a normal speech segment (nc == 0), or
	   trailing silence (shorter than tail margin length) (nc>0,rest_tail>0)
	   The current trailing silence is shorter than the user-
	   specified tail margin length, so the current samples
	   should be processed now as same as the normal speech segment */
	
#ifdef TMP_FIX_200602
	if (!a->adin_cut_on || a->is_valid_data == TRUE) {
#else
	if(
	   (!a->adin_cut_on || a->is_valid_data == TRUE)
#ifdef HAVE_PTHREAD
	   && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
	   ) {
#endif
	  if (a->nc > 0) {
	    /* if we are in a trailing silence, decrease the counter to detect
	     start of swapbuf[] above */
	    if (a->rest_tail < wstep) a->rest_tail = 0;
	    else a->rest_tail -= wstep;
#ifdef THREAD_DEBUG
	    jlog("DEBUG: %d processed, rest_tail=%d\n", wstep, a->rest_tail);
#endif
	  }
	  a->last_trigger_len += wstep;

#ifdef TMP_FIX_200602
	  if (ad_process != NULL
#ifdef HAVE_PTHREAD
	      && (!a->enable_thread || !a->ignore_speech_while_recog || transfer_online_local)
#endif
	      ) {

#else
	  if ( ad_process != NULL ) {
#endif
#ifdef THREAD_DEBUG
	    jlog("DEBUG: callback for input sample [%d-%d]\n", i, i+wstep);
#endif
	    /* call external function */
#ifdef ENABLE_PLUGIN
	    plugin_exec_adin_triggered(&(a->buffer[i]), wstep);
#endif
	    callback_exec_adin(CALLBACK_ADIN_TRIGGERED, recog, &(a->buffer[i]), wstep);
    //printf("loop a\n");
	    ad_process_ret = RealTimePipeLine(&(a->buffer[i]), wstep, recog);
    //printf("loop b\n");
	    switch(ad_process_ret) {
	    case 1:		/* segmentation notification from process callback */
#ifdef HAVE_PTHREAD
	      if (a->enable_thread) {
		/* in threaded mode, just stop transfer */
		pthread_mutex_lock(&(a->mutex));
		a->transfer_online = transfer_online_local = FALSE;
		pthread_mutex_unlock(&(a->mutex));
	      } else {
		/* in non-threaded mode, set end status and exit loop */
		adin_purge(a, i+wstep);
		end_status = 2;
		goto break_input;
	      }
	      break;
#else
	      /* in non-threaded mode, set end status and exit loop */
	      adin_purge(a, i+wstep);
	      end_status = 2;
	      goto break_input;
#endif
	    case -1:		/* error occured in callback */
	      /* set end status and exit loop */
	      end_status = -1;
	      goto break_input;
	    }
	  }
	}
      }	/* end of current segment processing */

      
      if (a->adin_cut_on && a->is_valid_data && a->nc >= a->nc_max) {
	/*************************************/
	/* process on, trailing silence over */
	/* = end of input segment            */
	/*************************************/
#ifdef THREAD_DEBUG
	jlog("DEBUG: detect off\n");
#endif
	/* end input by silence */
	a->is_valid_data = FALSE;	/* turn off processing */
	a->sblen = 0;
	callback_exec(CALLBACK_EVENT_SPEECH_STOP, recog);
#ifdef HAVE_PTHREAD
	if (a->enable_thread) { /* just stop transfer */
	  pthread_mutex_lock(&(a->mutex));
	  a->transfer_online = transfer_online_local = FALSE;
	  pthread_mutex_unlock(&(a->mutex));
	} else {
	  adin_purge(a, i+wstep);
	  end_status = 1;
	  goto break_input;
	}
#else
	adin_purge(a, i+wstep);
	end_status = 1;
	goto break_input;
#endif
      }

      /*********************************************************/
      /* end of processing buffer[0..current_len] by wstep step */
      /*********************************************************/
      i += wstep;		/* increment to next wstep samples */
    }
    
    /* purge processed samples and update queue */
    adin_purge(a, i);

  }

break_input:

  /****************/
  /* pause input */
  /****************/
  if (a->end_of_stream) {			/* input already ends */
    /* execute callback */
    callback_exec(CALLBACK_EVENT_SPEECH_STOP, recog);
    if (a->bp == 0) {		/* rest buffer successfully flushed */
      /* reset status */
      a->need_init = TRUE;		/* bufer status shoule be reset at next call */
    }
    if (end_status >= 0) {
      end_status = (a->bp) ? 1 : 0;
    }
  }
  
  return(end_status);
}

#ifdef HAVE_PTHREAD
/***********************/
/* threading functions */
/***********************/

/*************************/
/* adin thread functions */
/*************************/

/**
 * <EN>
 * Callback to store triggered samples within A/D-in thread.
 * </EN>
 * <JA>
 * A/D-in スレッドにてトリガした入力サンプルを保存するコールバック.
 * </JA>
 * 
 * @param now [in] triggered fragment
 * @param len [in] length of above
 * @param recog [in] engine instance
 * 
 * @return always 0, to tell caller to just continue the input
 */
static int
adin_store_buffer(SP16 *now, int len, Recog *recog)
{
  ADIn *a;

  a = recog->adin;
  if (a->speechlen + len > MAXSPEECHLEN) {
    /* just mark as overflowed, and continue this thread */
    pthread_mutex_lock(&(a->mutex));
    a->adinthread_buffer_overflowed = TRUE;
    pthread_mutex_unlock(&(a->mutex));
    return(0);
  }
  pthread_mutex_lock(&(a->mutex));
  memcpy(&(a->speech[a->speechlen]), now, len * sizeof(SP16));
  a->speechlen += len;
  pthread_mutex_unlock(&(a->mutex));
#ifdef THREAD_DEBUG
  jlog("DEBUG: input: stored %d samples, total=%d\n", len, a->speechlen);
#endif

  return(0);			/* continue */
}

/**
 * <EN>
 * A/D-in thread main function.
 * </EN>
 * <JA>
 * A/D-in スレッドのメイン関数.
 * </JA>
 * 
 * @param dummy [in] a dummy data, not used.
 */
static void
adin_thread_input_main(void *dummy)
{
  Recog *recog;
  int ret;

  recog = dummy;

  ret = adin_cut(adin_store_buffer, NULL, recog);

  if (ret == -2) {		/* termination request by ad_check? */
    jlog("Error: adin thread exit with termination request by checker\n");
  } else if (ret == -1) {	/* error */
    jlog("Error: adin thread exit with error\n");
  } else if (ret == 0) {	/* EOF */
    jlog("Stat: adin thread end with EOF\n");
  }
  recog->adin->adinthread_ended = TRUE;

  /* return to end this thread */
}

/**
 * <EN>
 * Start new A/D-in thread, and initialize buffer.
 * </EN>
 * <JA>
 * バッファを初期化して A/D-in スレッドを開始する. 
 * </JA>
 * @param recog [in] engine instance
 *
 * @callergraph
 * @callgraph
 */
boolean
adin_thread_create(Recog *recog)
{
  ADIn *a;

  a = recog->adin;

  /* init storing buffer */
  a->speech = (SP16 *)mymalloc(sizeof(SP16) * MAXSPEECHLEN);
  a->speechlen = 0;

  a->transfer_online = FALSE; /* tell adin-mic thread to wait at initial */
  a->adinthread_buffer_overflowed = FALSE;
  a->adinthread_ended = FALSE;

  if (pthread_mutex_init(&(a->mutex), NULL) != 0) { /* error */
    jlog("ERROR: adin_thread_create: failed to initialize mutex\n");
    return FALSE;
  }
  if (pthread_create(&(recog->adin->adin_thread), NULL, (void *)adin_thread_input_main, recog) != 0) {
    jlog("ERROR: adin_thread_create: failed to create AD-in thread\n");
    return FALSE;
  }
  if (pthread_detach(recog->adin->adin_thread) != 0) { /* not join, run forever */
    jlog("ERROR: adin_thread_create: failed to detach AD-in thread\n");
    return FALSE;
  }
  jlog("STAT: AD-in thread created\n");
  return TRUE;
}

/**
 * <EN>
 * Delete A/D-in thread
 * </EN>
 * <JA>
 * A/D-in スレッドを終了する
 * </JA>
 * @param recog [in] engine instance
 *
 * @callergraph
 * @callgraph
 */
boolean
adin_thread_cancel(Recog *recog)
{
  ADIn *a;
  int ret;

  if (recog->adin->adinthread_ended) return TRUE;

  /* send a cencellation request to the A/D-in thread */
  ret = pthread_cancel(recog->adin->adin_thread);
  if (ret != 0) {
    if (ret == ESRCH) {
      jlog("STAT: adin_thread_cancel: no A/D-in thread\n");
      recog->adin->adinthread_ended = TRUE;
      return TRUE;
    } else {
      jlog("Error: adin_thread_cancel: failed to cancel A/D-in thread\n");
      return FALSE;
    }
  }
  /* wait for the thread to terminate */
  ret = pthread_join(recog->adin->adin_thread, NULL);
  if (ret != 0) {
    if (ret == EINVAL) {
      jlog("InternalError: adin_thread_cancel: AD-in thread is invalid\n");
      recog->adin->adinthread_ended = TRUE;
      return FALSE;
    } else if (ret == ESRCH) {
      jlog("STAT: adin_thread_cancel: no A/D-in thread\n");
      recog->adin->adinthread_ended = TRUE;
      return TRUE;
    } else if (ret == EDEADLK) {
      jlog("InternalError: adin_thread_cancel: dead lock or self thread?\n");
      recog->adin->adinthread_ended = TRUE;
      return FALSE;
    } else {
      jlog("Error: adin_thread_cancel: failed to wait end of A/D-in thread\n");
      return FALSE;
    }
  }

  jlog("STAT: AD-in thread deleted\n");
  recog->adin->adinthread_ended = TRUE;
  return TRUE;
}

/****************************/
/* process thread functions */
/****************************/

/**
 * <EN>
 * @brief  Main processing function for thread mode.
 *
 * It waits for the new samples to be stored in @a speech by A/D-in thread,
 * and if found, process them.  The interface are the same as adin_cut().
 * </EN>
 * <JA>
 * @brief  スレッドモード用メイン関数
 *
 * この関数は A/D-in スレッドによってサンプルが保存されるのを待ち，
 * 保存されたサンプルを順次処理していきます. 引数や返り値は adin_cut() と
 * 同一です. 
 * </JA>
 * 
 * @param ad_process [in] function to process triggerted input.
 * @param ad_check [in] function to be called periodically.
 * @param recog [in] engine instance
 * 
 * @return 2 when input termination requested by ad_process(), 1 when
 * if detect end of an input segment (down trigger detected after up
 * trigger), 0 when reached end of input device, -1 on error, -2 when
 * input termination requested by ad_check().
 */
static int
adin_thread_process(int (*ad_process)(SP16 *, int, Recog *), int (*ad_check)(Recog *), Recog *recog)
{
  int prev_len, nowlen;
  int ad_process_ret;
  int i;
  boolean overflowed_p;
  boolean transfer_online_local;
  boolean ended_p;
  ADIn *a;

  a = recog->adin;

  /* reset storing buffer --- input while recognition will be ignored */
  pthread_mutex_lock(&(a->mutex));
  /*if (speechlen == 0) transfer_online = TRUE;*/ /* tell adin-mic thread to start recording */
  a->transfer_online = TRUE;
#ifdef THREAD_DEBUG
  jlog("DEBUG: process: reset, speechlen = %d, online=%d\n", a->speechlen, a->transfer_online);
#endif
  a->adinthread_buffer_overflowed = FALSE;
  pthread_mutex_unlock(&(a->mutex));

  /* main processing loop */
  prev_len = 0;
  for(;;) {
    /* get current length (locking) */
    pthread_mutex_lock(&(a->mutex));
    nowlen = a->speechlen;
    overflowed_p = a->adinthread_buffer_overflowed;
    transfer_online_local = a->transfer_online;
    ended_p = a->adinthread_ended;
    pthread_mutex_unlock(&(a->mutex));
    /* check if thread is alive */
    if (ended_p) {
      /* adin thread has already exited, so return EOF to stop this input */
      return(0);
    }
    /* check if other input thread has overflowed */
    if (overflowed_p) {
      jlog("WARNING: adin_thread_process: too long input (> %d samples), segmented now\n", MAXSPEECHLEN);
      /* segment input here */
      pthread_mutex_lock(&(a->mutex));
      a->speechlen = 0;
      a->transfer_online = transfer_online_local = FALSE;
      pthread_mutex_unlock(&(a->mutex));
      return(1);		/* return with segmented status */
    }
    /* callback poll */
    if (ad_check != NULL) {
      if ((i = (*(ad_check))(recog)) < 0) {
	if ((i == -1 && nowlen == 0) || i == -2) {
	  pthread_mutex_lock(&(a->mutex));
	  a->transfer_online = transfer_online_local = FALSE;
	  a->speechlen = 0;
	  pthread_mutex_unlock(&(a->mutex));
	  return(-2);
	}
      }
    }
    if (prev_len < nowlen && nowlen <= a->freezelen) {
#ifdef THREAD_DEBUG
      jlog("DEBUG: process: proceed [%d-%d]\n",prev_len, nowlen);
#endif
      /* got new sample, process */
      /* As the speech[] buffer is monotonously increase,
	 content of speech buffer [prev_len..nowlen] would not alter
	 in both threads
	 So locking is not needed while processing.
       */
      /*jlog("DEBUG: main: read %d-%d\n", prev_len, nowlen);*/
      if (ad_process != NULL) {
	ad_process_ret = (*ad_process)(&(a->speech[prev_len]), nowlen - prev_len, recog);
#ifdef THREAD_DEBUG
	jlog("DEBUG: ad_process_ret=%d\n", ad_process_ret);
#endif
	switch(ad_process_ret) {
	case 1:			/* segmented */
	  /* segmented by callback function */
	  /* purge processed samples and keep transfering */
	  pthread_mutex_lock(&(a->mutex));
	  if(a->speechlen > nowlen) {
	    memmove(a->speech, &(a->speech[nowlen]), (a->speechlen - nowlen) * sizeof(SP16));
	    a->speechlen -= nowlen;
	  } else {
	    a->speechlen = 0;
	  }
	  a->transfer_online = transfer_online_local = FALSE;
	  pthread_mutex_unlock(&(a->mutex));
	  /* keep transfering */
	  return(2);		/* return with segmented status */
	case -1:		/* error */
	  pthread_mutex_lock(&(a->mutex));
	  a->transfer_online = transfer_online_local = FALSE;
	  pthread_mutex_unlock(&(a->mutex));
	  return(-1);		/* return with error */
	}
      }
      if (a->rehash) {
	/* rehash */
	pthread_mutex_lock(&(a->mutex));
	if (debug2_flag) jlog("STAT: adin_cut: rehash from %d to %d\n", a->speechlen, a->speechlen - prev_len);
	a->speechlen -= prev_len;
	nowlen -= prev_len;
	memmove(a->speech, &(a->speech[prev_len]), a->speechlen * sizeof(SP16));
	pthread_mutex_unlock(&(a->mutex));
	a->rehash = FALSE;
      }
      prev_len = nowlen;
    } else {
      if (transfer_online_local == FALSE) {
	/* segmented by zero-cross */
	/* reset storing buffer for next input */
	pthread_mutex_lock(&(a->mutex));
	a->speechlen = 0;
	pthread_mutex_unlock(&(a->mutex));
        break;
      }
      usleep(50000);   /* wait = 0.05sec*/            
    }
  }

  /* as threading assumes infinite input */
  /* return value should be 1 (segmented) */
  return(1);
}
#endif /* HAVE_PTHREAD */




/**
 * <EN>
 * @brief  Top function to start input processing
 *
 * If threading mode is enabled, this function simply enters to
 * adin_thread_process() to process triggered samples detected by
 * another running A/D-in thread.
 *
 * If threading mode is not available or disabled by either device requirement
 * or OS capability, this function simply calls adin_cut() to detect speech
 * segment from input device and process them concurrently by one process.
 * </EN>
 * <JA>
 * @brief  入力処理を行うトップ関数
 *
 * スレッドモードでは，この関数は adin_thead_process() を呼び出し，
 * 非スレッドモードでは adin_cut() を直接呼び出す. 引数や返り値は
 * adin_cut() と同一である. 
 * </JA>
 * 
 * @param ad_process [in] function to process triggerted input.
 * @param ad_check [in] function to be called periodically.
 * @param recog [in] engine instance
 * 
 * @return 2 when input termination requested by ad_process(), 1 when
 * if detect end of an input segment (down trigger detected after up
 * trigger), 0 when reached end of input device, -1 on error, -2 when
 * input termination requested by ad_check().
 *
 * @callergraph
 * @callgraph
 * 
 */
int
adin_go(int (*ad_process)(SP16 *, int, Recog *), int (*ad_check)(Recog *), Recog *recog)
{
  /* output listening start message */
  callback_exec(CALLBACK_EVENT_SPEECH_READY, recog);
#ifdef HAVE_PTHREAD
  if (recog->adin->enable_thread) {
    return(adin_thread_process(RealTimePipeLine, callback_check_in_adin, recog));
  }
#endif
  return(adin_cut(RealTimePipeLine, callback_check_in_adin, recog));
}

/** 
 * <EN>
 * Call device-specific initialization.
 * </EN>
 * <JA>
 * デバイス依存の初期化関数を呼び出す. 
 * </JA>
 * 
 * @param a [in] A/D-in work area
 * @param freq [in] sampling frequency
 * @param arg [in] device-dependent argument
 * 
 * @return TRUE if succeeded, FALSE if failed.
 * 
 * @callergraph
 * @callgraph
 * 
 */
boolean
adin_standby(ADIn *a, int freq, void *arg)
{
  if (a->need_zmean) zmean_reset();
  if (a->ad_standby != NULL) return(a->ad_standby(freq, arg));
  return TRUE;
}
/** 
 * <EN>
 * Call device-specific function to begin capturing of the audio stream.
 * </EN>
 * <JA>
 * 音の取り込みを開始するデバイス依存の関数を呼び出す. 
 * </JA>
 * 
 * @param a [in] A/D-in work area
 * @param file_or_dev_name [in] device / file path to open or NULL for default
 * 
 * @return TRUE on success, FALSE on failure.
 * 
 * @callergraph
 * @callgraph
 * 
 */
boolean
adin_begin(ADIn *a, char *file_or_dev_name)
{
  if (debug2_flag && a->input_side_segment) jlog("Stat: adin_begin: skip\n");
  if (a->input_side_segment == FALSE) {
    a->total_captured_len = 0;
    a->last_trigger_len = 0;
    if (a->need_zmean) zmean_reset();
    if (a->ad_begin != NULL) return(a->ad_begin(file_or_dev_name));
  }
  return TRUE;
}
/** 
 * <EN>
 * Call device-specific function to end capturing of the audio stream.
 * </EN>
 * <JA>
 * 音の取り込みを終了するデバイス依存の関数を呼び出す. 
 * </JA>
 * 
 * @param a [in] A/D-in work area
 * 
 * @return TRUE on success, FALSE on failure.
 *
 * @callergraph
 * @callgraph
 */
boolean
adin_end(ADIn *a)
{
  if (debug2_flag && a->input_side_segment) jlog("Stat: adin_end: skip\n");
  if (a->input_side_segment == FALSE) {
    if (a->ad_end != NULL) return(a->ad_end());
  }
  return TRUE;
}

/** 
 * <EN>
 * Free memories of A/D-in work area.
 * </EN>
 * <JA>
 * 音取り込み用ワークエリアのメモリを開放する. 
 * </JA>
 * 
 * @param recog [in] engine instance
 *
 * @callergraph
 * @callgraph
 * 
 */
void
adin_free_param(Recog *recog)
{
  ADIn *a;

  a = recog->adin;

  if (a->ds) {
    ds48to16_free(a->ds);
    a->ds = NULL;
  }
  if (a->adin_cut_on) {
    free_count_zc_e(&(a->zc));
  }
  if (a->down_sample) {
    free(a->buffer48);
  }
  free(a->swapbuf);
  free(a->cbuf);
  free(a->buffer);
#ifdef HAVE_PTHREAD
  if (a->speech) free(a->speech);
#endif
}

/* end of file */
