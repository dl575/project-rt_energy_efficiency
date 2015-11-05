/* Produced by CVXGEN, 2015-10-08 17:09:33 -0400.  */
/* CVXGEN is Copyright (C) 2006-2012 Jacob Mattingley, jem@cvxgen.com. */
/* The code in this file is Copyright (C) 2006-2012 Jacob Mattingley. */
/* CVXGEN, or solvers produced by CVXGEN, cannot be used for commercial */
/* applications without prior written permission from Jacob Mattingley. */

/* Filename: testsolver.c. */
/* Description: Basic test harness for solver.c. */
#include "solver.h"
Vars vars;
Params params;
Workspace work;
Settings settings;
#define NUMTESTS 1
int main(int argc, char **argv) {
  int num_iters;
#if (NUMTESTS > 0)
  int i;
  double time;
  double time_per;
#endif
  set_defaults();
  setup_indexing();
  load_default_data();
  /* Solve problem instance for the record. */
  settings.verbose = 1;
  num_iters = solve();
  pm(vars.bb, 1, 5); 
  printf("%d\n", sizeof(params.xx)/sizeof(params.xx[0]));
  printf("%d\n", sizeof(vars.bb)/sizeof(vars.bb[0]));
#ifndef ZERO_LIBRARY_MODE
#if (NUMTESTS > 0)
  /* Now solve multiple problem instances for timing purposes. */
  settings.verbose = 0;
  tic();
  for (i = 0; i < NUMTESTS; i++) {
    solve();
  }
  time = tocq();
  printf("Timed %d solves over %.3f seconds.\n", NUMTESTS, time);
  time_per = time / NUMTESTS;
  if (time_per > 1) {
    printf("Actual time taken per solve: %.3g s.\n", time_per);
  } else if (time_per > 1e-3) {
    printf("Actual time taken per solve: %.3g ms.\n", 1e3*time_per);
  } else {
    printf("Actual time taken per solve: %.3g us.\n", 1e6*time_per);
  }
#endif
#endif
  return 0;
}
void load_default_data(void) {
  params.xx[0] = 0.20319161029830202;
  params.xx[1] = 0.8325912904724193;
  params.xx[2] = -0.8363810443482227;
  params.xx[3] = 0.04331042079065206;
  params.xx[4] = 1.5717878173906188;
  params.xx[5] = 1.5851723557337523;
  params.xx[6] = -1.497658758144655;
  params.xx[7] = -1.171028487447253;
  params.xx[8] = -1.7941311867966805;
  params.xx[9] = -0.23676062539745413;
  params.xx[10] = -1.8804951564857322;
  params.xx[11] = -0.17266710242115568;
  params.xx[12] = 0.596576190459043;
  params.xx[13] = -0.8860508694080989;
  params.xx[14] = 0.7050196079205251;
  params.xx[15] = 0.3634512696654033;
  params.xx[16] = -1.9040724704913385;
  params.xx[17] = 0.23541635196352795;
  params.xx[18] = -0.9629902123701384;
  params.xx[19] = -0.3395952119597214;
  params.xx[20] = -0.865899672914725;
  params.xx[21] = 0.7725516732519853;
  params.xx[22] = -0.23818512931704205;
  params.xx[23] = -1.372529046100147;
  params.xx[24] = 0.17859607212737894;
  params.xx[25] = 1.1212590580454682;
  params.xx[26] = -0.774545870495281;
  params.xx[27] = -1.1121684642712744;
  params.xx[28] = -0.44811496977740495;
  params.xx[29] = 1.7455345994417217;
  params.xx[30] = 1.9039816898917352;
  params.xx[31] = 0.6895347036512547;
  params.xx[32] = 1.6113364341535923;
  params.xx[33] = 1.383003485172717;
  params.xx[34] = -0.48802383468444344;
  params.xx[35] = -1.631131964513103;
  params.xx[36] = 0.6136436100941447;
  params.xx[37] = 0.2313630495538037;
  params.xx[38] = -0.5537409477496875;
  params.xx[39] = -1.0997819806406723;
  params.xx[40] = -0.3739203344950055;
  params.xx[41] = -0.12423900520332376;
  params.xx[42] = -0.923057686995755;
  params.xx[43] = -0.8328289030982696;
  params.xx[44] = -0.16925440270808823;
  params.xx[45] = 1.442135651787706;
  params.xx[46] = 0.34501161787128565;
  params.xx[47] = -0.8660485502711608;
  params.xx[48] = -0.8880899735055947;
  params.xx[49] = -0.1815116979122129;
  params.yy[0] = -1.17835862158005;
  params.yy[1] = -1.1944851558277074;
  params.yy[2] = 0.05614023926976763;
  params.yy[3] = -1.6510825248767813;
  params.yy[4] = -0.06565787059365391;
  params.yy[5] = -0.5512951504486665;
  params.yy[6] = 0.8307464872626844;
  params.yy[7] = 0.9869848924080182;
  params.yy[8] = 0.7643716874230573;
  params.yy[9] = 0.7567216550196565;
}
