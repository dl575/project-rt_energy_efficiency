/* Produced by CVXGEN, 2015-10-08 17:09:33 -0400.  */
/* CVXGEN is Copyright (C) 2006-2012 Jacob Mattingley, jem@cvxgen.com. */
/* The code in this file is Copyright (C) 2006-2012 Jacob Mattingley. */
/* CVXGEN, or solvers produced by CVXGEN, cannot be used for commercial */
/* applications without prior written permission from Jacob Mattingley. */

/* Filename: matrix_support.c. */
/* Description: Support functions for matrix multiplication and vector filling. */
#include "solver.h"
void multbymA(double *lhs, double *rhs) {
}
void multbymAT(double *lhs, double *rhs) {
  lhs[0] = 0;
  lhs[1] = 0;
  lhs[2] = 0;
  lhs[3] = 0;
  lhs[4] = 0;
}
void multbymG(double *lhs, double *rhs) {
}
void multbymGT(double *lhs, double *rhs) {
  lhs[0] = 0;
  lhs[1] = 0;
  lhs[2] = 0;
  lhs[3] = 0;
  lhs[4] = 0;
}
void multbyP(double *lhs, double *rhs) {
  /* TODO use the fact that P is symmetric? */
  /* TODO check doubling / half factor etc. */
  lhs[0] = rhs[0]*(2*work.quad_184610709504[0])+rhs[1]*(2*work.quad_184610709504[5])+rhs[2]*(2*work.quad_184610709504[10])+rhs[3]*(2*work.quad_184610709504[15])+rhs[4]*(2*work.quad_184610709504[20]);
  lhs[1] = rhs[0]*(2*work.quad_184610709504[1])+rhs[1]*(2*work.quad_184610709504[6])+rhs[2]*(2*work.quad_184610709504[11])+rhs[3]*(2*work.quad_184610709504[16])+rhs[4]*(2*work.quad_184610709504[21]);
  lhs[2] = rhs[0]*(2*work.quad_184610709504[2])+rhs[1]*(2*work.quad_184610709504[7])+rhs[2]*(2*work.quad_184610709504[12])+rhs[3]*(2*work.quad_184610709504[17])+rhs[4]*(2*work.quad_184610709504[22]);
  lhs[3] = rhs[0]*(2*work.quad_184610709504[3])+rhs[1]*(2*work.quad_184610709504[8])+rhs[2]*(2*work.quad_184610709504[13])+rhs[3]*(2*work.quad_184610709504[18])+rhs[4]*(2*work.quad_184610709504[23]);
  lhs[4] = rhs[0]*(2*work.quad_184610709504[4])+rhs[1]*(2*work.quad_184610709504[9])+rhs[2]*(2*work.quad_184610709504[14])+rhs[3]*(2*work.quad_184610709504[19])+rhs[4]*(2*work.quad_184610709504[24]);
}
void fillq(void) {
  work.q[0] = -2*(params.xx[0]*params.yy[0]+params.xx[1]*params.yy[1]+params.xx[2]*params.yy[2]+params.xx[3]*params.yy[3]+params.xx[4]*params.yy[4]+params.xx[5]*params.yy[5]+params.xx[6]*params.yy[6]+params.xx[7]*params.yy[7]+params.xx[8]*params.yy[8]+params.xx[9]*params.yy[9]);
  work.q[1] = -2*(params.xx[10]*params.yy[0]+params.xx[11]*params.yy[1]+params.xx[12]*params.yy[2]+params.xx[13]*params.yy[3]+params.xx[14]*params.yy[4]+params.xx[15]*params.yy[5]+params.xx[16]*params.yy[6]+params.xx[17]*params.yy[7]+params.xx[18]*params.yy[8]+params.xx[19]*params.yy[9]);
  work.q[2] = -2*(params.xx[20]*params.yy[0]+params.xx[21]*params.yy[1]+params.xx[22]*params.yy[2]+params.xx[23]*params.yy[3]+params.xx[24]*params.yy[4]+params.xx[25]*params.yy[5]+params.xx[26]*params.yy[6]+params.xx[27]*params.yy[7]+params.xx[28]*params.yy[8]+params.xx[29]*params.yy[9]);
  work.q[3] = -2*(params.xx[30]*params.yy[0]+params.xx[31]*params.yy[1]+params.xx[32]*params.yy[2]+params.xx[33]*params.yy[3]+params.xx[34]*params.yy[4]+params.xx[35]*params.yy[5]+params.xx[36]*params.yy[6]+params.xx[37]*params.yy[7]+params.xx[38]*params.yy[8]+params.xx[39]*params.yy[9]);
  work.q[4] = -2*(params.xx[40]*params.yy[0]+params.xx[41]*params.yy[1]+params.xx[42]*params.yy[2]+params.xx[43]*params.yy[3]+params.xx[44]*params.yy[4]+params.xx[45]*params.yy[5]+params.xx[46]*params.yy[6]+params.xx[47]*params.yy[7]+params.xx[48]*params.yy[8]+params.xx[49]*params.yy[9]);
}
void fillh(void) {
}
void fillb(void) {
}
void pre_ops(void) {
  work.quad_184610709504[0] = params.xx[0]*params.xx[0]+params.xx[1]*params.xx[1]+params.xx[2]*params.xx[2]+params.xx[3]*params.xx[3]+params.xx[4]*params.xx[4]+params.xx[5]*params.xx[5]+params.xx[6]*params.xx[6]+params.xx[7]*params.xx[7]+params.xx[8]*params.xx[8]+params.xx[9]*params.xx[9];
  work.quad_184610709504[5] = params.xx[0]*params.xx[10]+params.xx[1]*params.xx[11]+params.xx[2]*params.xx[12]+params.xx[3]*params.xx[13]+params.xx[4]*params.xx[14]+params.xx[5]*params.xx[15]+params.xx[6]*params.xx[16]+params.xx[7]*params.xx[17]+params.xx[8]*params.xx[18]+params.xx[9]*params.xx[19];
  work.quad_184610709504[10] = params.xx[0]*params.xx[20]+params.xx[1]*params.xx[21]+params.xx[2]*params.xx[22]+params.xx[3]*params.xx[23]+params.xx[4]*params.xx[24]+params.xx[5]*params.xx[25]+params.xx[6]*params.xx[26]+params.xx[7]*params.xx[27]+params.xx[8]*params.xx[28]+params.xx[9]*params.xx[29];
  work.quad_184610709504[15] = params.xx[0]*params.xx[30]+params.xx[1]*params.xx[31]+params.xx[2]*params.xx[32]+params.xx[3]*params.xx[33]+params.xx[4]*params.xx[34]+params.xx[5]*params.xx[35]+params.xx[6]*params.xx[36]+params.xx[7]*params.xx[37]+params.xx[8]*params.xx[38]+params.xx[9]*params.xx[39];
  work.quad_184610709504[20] = params.xx[0]*params.xx[40]+params.xx[1]*params.xx[41]+params.xx[2]*params.xx[42]+params.xx[3]*params.xx[43]+params.xx[4]*params.xx[44]+params.xx[5]*params.xx[45]+params.xx[6]*params.xx[46]+params.xx[7]*params.xx[47]+params.xx[8]*params.xx[48]+params.xx[9]*params.xx[49];
  work.quad_184610709504[1] = params.xx[10]*params.xx[0]+params.xx[11]*params.xx[1]+params.xx[12]*params.xx[2]+params.xx[13]*params.xx[3]+params.xx[14]*params.xx[4]+params.xx[15]*params.xx[5]+params.xx[16]*params.xx[6]+params.xx[17]*params.xx[7]+params.xx[18]*params.xx[8]+params.xx[19]*params.xx[9];
  work.quad_184610709504[6] = params.xx[10]*params.xx[10]+params.xx[11]*params.xx[11]+params.xx[12]*params.xx[12]+params.xx[13]*params.xx[13]+params.xx[14]*params.xx[14]+params.xx[15]*params.xx[15]+params.xx[16]*params.xx[16]+params.xx[17]*params.xx[17]+params.xx[18]*params.xx[18]+params.xx[19]*params.xx[19];
  work.quad_184610709504[11] = params.xx[10]*params.xx[20]+params.xx[11]*params.xx[21]+params.xx[12]*params.xx[22]+params.xx[13]*params.xx[23]+params.xx[14]*params.xx[24]+params.xx[15]*params.xx[25]+params.xx[16]*params.xx[26]+params.xx[17]*params.xx[27]+params.xx[18]*params.xx[28]+params.xx[19]*params.xx[29];
  work.quad_184610709504[16] = params.xx[10]*params.xx[30]+params.xx[11]*params.xx[31]+params.xx[12]*params.xx[32]+params.xx[13]*params.xx[33]+params.xx[14]*params.xx[34]+params.xx[15]*params.xx[35]+params.xx[16]*params.xx[36]+params.xx[17]*params.xx[37]+params.xx[18]*params.xx[38]+params.xx[19]*params.xx[39];
  work.quad_184610709504[21] = params.xx[10]*params.xx[40]+params.xx[11]*params.xx[41]+params.xx[12]*params.xx[42]+params.xx[13]*params.xx[43]+params.xx[14]*params.xx[44]+params.xx[15]*params.xx[45]+params.xx[16]*params.xx[46]+params.xx[17]*params.xx[47]+params.xx[18]*params.xx[48]+params.xx[19]*params.xx[49];
  work.quad_184610709504[2] = params.xx[20]*params.xx[0]+params.xx[21]*params.xx[1]+params.xx[22]*params.xx[2]+params.xx[23]*params.xx[3]+params.xx[24]*params.xx[4]+params.xx[25]*params.xx[5]+params.xx[26]*params.xx[6]+params.xx[27]*params.xx[7]+params.xx[28]*params.xx[8]+params.xx[29]*params.xx[9];
  work.quad_184610709504[7] = params.xx[20]*params.xx[10]+params.xx[21]*params.xx[11]+params.xx[22]*params.xx[12]+params.xx[23]*params.xx[13]+params.xx[24]*params.xx[14]+params.xx[25]*params.xx[15]+params.xx[26]*params.xx[16]+params.xx[27]*params.xx[17]+params.xx[28]*params.xx[18]+params.xx[29]*params.xx[19];
  work.quad_184610709504[12] = params.xx[20]*params.xx[20]+params.xx[21]*params.xx[21]+params.xx[22]*params.xx[22]+params.xx[23]*params.xx[23]+params.xx[24]*params.xx[24]+params.xx[25]*params.xx[25]+params.xx[26]*params.xx[26]+params.xx[27]*params.xx[27]+params.xx[28]*params.xx[28]+params.xx[29]*params.xx[29];
  work.quad_184610709504[17] = params.xx[20]*params.xx[30]+params.xx[21]*params.xx[31]+params.xx[22]*params.xx[32]+params.xx[23]*params.xx[33]+params.xx[24]*params.xx[34]+params.xx[25]*params.xx[35]+params.xx[26]*params.xx[36]+params.xx[27]*params.xx[37]+params.xx[28]*params.xx[38]+params.xx[29]*params.xx[39];
  work.quad_184610709504[22] = params.xx[20]*params.xx[40]+params.xx[21]*params.xx[41]+params.xx[22]*params.xx[42]+params.xx[23]*params.xx[43]+params.xx[24]*params.xx[44]+params.xx[25]*params.xx[45]+params.xx[26]*params.xx[46]+params.xx[27]*params.xx[47]+params.xx[28]*params.xx[48]+params.xx[29]*params.xx[49];
  work.quad_184610709504[3] = params.xx[30]*params.xx[0]+params.xx[31]*params.xx[1]+params.xx[32]*params.xx[2]+params.xx[33]*params.xx[3]+params.xx[34]*params.xx[4]+params.xx[35]*params.xx[5]+params.xx[36]*params.xx[6]+params.xx[37]*params.xx[7]+params.xx[38]*params.xx[8]+params.xx[39]*params.xx[9];
  work.quad_184610709504[8] = params.xx[30]*params.xx[10]+params.xx[31]*params.xx[11]+params.xx[32]*params.xx[12]+params.xx[33]*params.xx[13]+params.xx[34]*params.xx[14]+params.xx[35]*params.xx[15]+params.xx[36]*params.xx[16]+params.xx[37]*params.xx[17]+params.xx[38]*params.xx[18]+params.xx[39]*params.xx[19];
  work.quad_184610709504[13] = params.xx[30]*params.xx[20]+params.xx[31]*params.xx[21]+params.xx[32]*params.xx[22]+params.xx[33]*params.xx[23]+params.xx[34]*params.xx[24]+params.xx[35]*params.xx[25]+params.xx[36]*params.xx[26]+params.xx[37]*params.xx[27]+params.xx[38]*params.xx[28]+params.xx[39]*params.xx[29];
  work.quad_184610709504[18] = params.xx[30]*params.xx[30]+params.xx[31]*params.xx[31]+params.xx[32]*params.xx[32]+params.xx[33]*params.xx[33]+params.xx[34]*params.xx[34]+params.xx[35]*params.xx[35]+params.xx[36]*params.xx[36]+params.xx[37]*params.xx[37]+params.xx[38]*params.xx[38]+params.xx[39]*params.xx[39];
  work.quad_184610709504[23] = params.xx[30]*params.xx[40]+params.xx[31]*params.xx[41]+params.xx[32]*params.xx[42]+params.xx[33]*params.xx[43]+params.xx[34]*params.xx[44]+params.xx[35]*params.xx[45]+params.xx[36]*params.xx[46]+params.xx[37]*params.xx[47]+params.xx[38]*params.xx[48]+params.xx[39]*params.xx[49];
  work.quad_184610709504[4] = params.xx[40]*params.xx[0]+params.xx[41]*params.xx[1]+params.xx[42]*params.xx[2]+params.xx[43]*params.xx[3]+params.xx[44]*params.xx[4]+params.xx[45]*params.xx[5]+params.xx[46]*params.xx[6]+params.xx[47]*params.xx[7]+params.xx[48]*params.xx[8]+params.xx[49]*params.xx[9];
  work.quad_184610709504[9] = params.xx[40]*params.xx[10]+params.xx[41]*params.xx[11]+params.xx[42]*params.xx[12]+params.xx[43]*params.xx[13]+params.xx[44]*params.xx[14]+params.xx[45]*params.xx[15]+params.xx[46]*params.xx[16]+params.xx[47]*params.xx[17]+params.xx[48]*params.xx[18]+params.xx[49]*params.xx[19];
  work.quad_184610709504[14] = params.xx[40]*params.xx[20]+params.xx[41]*params.xx[21]+params.xx[42]*params.xx[22]+params.xx[43]*params.xx[23]+params.xx[44]*params.xx[24]+params.xx[45]*params.xx[25]+params.xx[46]*params.xx[26]+params.xx[47]*params.xx[27]+params.xx[48]*params.xx[28]+params.xx[49]*params.xx[29];
  work.quad_184610709504[19] = params.xx[40]*params.xx[30]+params.xx[41]*params.xx[31]+params.xx[42]*params.xx[32]+params.xx[43]*params.xx[33]+params.xx[44]*params.xx[34]+params.xx[45]*params.xx[35]+params.xx[46]*params.xx[36]+params.xx[47]*params.xx[37]+params.xx[48]*params.xx[38]+params.xx[49]*params.xx[39];
  work.quad_184610709504[24] = params.xx[40]*params.xx[40]+params.xx[41]*params.xx[41]+params.xx[42]*params.xx[42]+params.xx[43]*params.xx[43]+params.xx[44]*params.xx[44]+params.xx[45]*params.xx[45]+params.xx[46]*params.xx[46]+params.xx[47]*params.xx[47]+params.xx[48]*params.xx[48]+params.xx[49]*params.xx[49];
  work.quad_828655190016[0] = params.yy[0]*params.yy[0]+params.yy[1]*params.yy[1]+params.yy[2]*params.yy[2]+params.yy[3]*params.yy[3]+params.yy[4]*params.yy[4]+params.yy[5]*params.yy[5]+params.yy[6]*params.yy[6]+params.yy[7]*params.yy[7]+params.yy[8]*params.yy[8]+params.yy[9]*params.yy[9];
}
