/*!
 ***********************************************************************
 * \file macroblock.c
 *
 * \brief
 *     Decode a Macroblock
 *
 * \author
 *    Main contributors (see contributors.h for copyright, address and affiliation details)
 *    - Inge Lille-Langøy               <inge.lille-langoy@telenor.com>
 *    - Rickard Sjoberg                 <rickard.sjoberg@era.ericsson.se>
 *    - Jani Lainema                    <jani.lainema@nokia.com>
 *    - Sebastian Purreiter             <sebastian.purreiter@mch.siemens.de>
 *    - Thomas Wedi                     <wedi@tnt.uni-hannover.de>
 *    - Detlev Marpe                    <marpe@hhi.de>
 *    - Gabi Blaettermann               <blaetter@hhi.de>
 *    - Ye-Kui Wang                     <wyk@ieee.org>
 *    - Lowell Winger                   <lwinger@lsil.com>
 ***********************************************************************
*/

#include "contributors.h"

#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "global.h"
#include "mbuffer.h"
#include "elements.h"
#include "errorconcealment.h"
#include "macroblock.h"
#include "fmo.h"
#include "cabac.h"
#include "vlc.h"
#include "image.h"
#include "mb_access.h"
#include "biaridecod.h"

#include "transform8x8.h"

#if TRACE
#define TRACE_STRING(s) strncpy(currSE.tracestring, s, TRACESTRING_SIZE)
#else
#define TRACE_STRING(s) // do nothing
#endif

extern int last_dquant;
extern ColocatedParams *Co_located;
static void SetMotionVectorPredictor(struct img_par *img, short *pmv_x, short *pmv_y, char ref_frame, byte list, char ***refPic, short ****tmp_mv, int block_x, int block_y, int blockshape_x, int blockshape_y);

int loop_counter[652] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void start_macroblock(struct img_par *img, struct inp_par *inp, int CurrentMBInScanOrder)
{
  int i;
  int j;
  int k;
  int l;
  Macroblock *currMB;
  assert(img->current_mb_nr < img->PicSizeInMbs);
  currMB = &img->mb_data[img->current_mb_nr];
  if (img->MbaffFrameFlag)
  {
    loop_counter[0]++;
    img->mb_x = img->current_mb_nr % ((2 * img->width) / 16);
    img->mb_y = 2 * (img->current_mb_nr / ((2 * img->width) / 16));
    if (img->mb_x % 2)
    {
      loop_counter[1]++;
      img->mb_y++;
    }

    img->mb_x /= 2;
  }
  else
  {
    img->mb_x = img->current_mb_nr % (img->width / 16);
    img->mb_y = img->current_mb_nr / (img->width / 16);
  }

  img->block_y = img->mb_y * 4;
  img->pix_y = img->mb_y * 16;
  img->pix_c_y = img->mb_y * img->mb_cr_size_y;
  img->block_x = img->mb_x * 4;
  img->pix_x = img->mb_x * 16;
  img->pix_c_x = img->mb_x * img->mb_cr_size_x;
  currMB->slice_nr = img->current_slice_nr;
  if (img->current_slice_nr >= 50)
  {
    loop_counter[2]++;
    error("maximum number of supported slices exceeded, please recompile with increased value for MAX_NUM_SLICES", 200);
  }

  dec_picture->slice_id[img->mb_y][img->mb_x] = img->current_slice_nr;
  if (img->current_slice_nr > dec_picture->max_slice_id)
  {
    loop_counter[3]++;
    dec_picture->max_slice_id = img->current_slice_nr;
  }

  CheckAvailabilityOfNeighbors(img);
  currMB->qp = img->qp;
  currMB->mb_type = 0;
  currMB->delta_quant = 0;
  currMB->cbp = 0;
  currMB->cbp_blk = 0;
  currMB->c_ipred_mode = 0;
  for (l = 0; l < 2; l++)
  {
    loop_counter[4]++;
    for (j = 0; j < (16 / 4); j++)
    {
      loop_counter[5]++;
      for (i = 0; i < (16 / 4); i++)
      {
        loop_counter[6]++;
        for (k = 0; k < 2; k++)
        {
          loop_counter[7]++;
          currMB->mvd[l][j][i][k] = 0;
        }

      }

    }

  }

  currMB->cbp_bits = 0;
  for (j = 0; j < 16; j++)
  {
    loop_counter[8]++;
    for (i = 0; i < 16; i++)
    {
      loop_counter[9]++;
      img->m7[i][j] = 0;
    }

  }

  currMB->LFDisableIdc = img->currentSlice->LFDisableIdc;
  currMB->LFAlphaC0Offset = img->currentSlice->LFAlphaC0Offset;
  currMB->LFBetaOffset = img->currentSlice->LFBetaOffset;
}

int exit_macroblock(struct img_par *img, struct inp_par *inp, int eos_bit)
{
  img->num_dec_mb++;
  if (img->num_dec_mb == img->PicSizeInMbs)
  {
    loop_counter[10]++;
    assert(nal_startcode_follows(img, inp, eos_bit) == TRUE);
    return TRUE;
  }
  else
  {
    img->current_mb_nr = FmoGetNextMBNr(img->current_mb_nr);
    if (img->current_mb_nr == (-1))
    {
      loop_counter[11]++;
      assert(nal_startcode_follows(img, inp, eos_bit) == TRUE);
      return TRUE;
    }

    if (nal_startcode_follows(img, inp, eos_bit) == FALSE)
    {
      loop_counter[12]++;
      return FALSE;
    }

    if (((img->type == I_SLICE) || (img->type == SI_SLICE)) || (active_pps->entropy_coding_mode_flag == CABAC))
    {
      loop_counter[13]++;
      return TRUE;
    }

    if (img->cod_counter <= 0)
    {
      loop_counter[14]++;
      return TRUE;
    }

    return FALSE;
  }

}

void interpret_mb_mode_P(struct img_par *img)
{
  int i;
  const int ICBPTAB[6] = {0, 16, 32, 15, 31, 47};
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int mbmode = currMB->mb_type;
  if (mbmode < 4)
  {
    loop_counter[15]++;
    currMB->mb_type = mbmode;
    for (i = 0; i < 4; i++)
    {
      loop_counter[16]++;
      currMB->b8mode[i] = mbmode;
      currMB->b8pdir[i] = 0;
    }

  }
  else
    if ((mbmode == 4) || (mbmode == 5))
  {
    loop_counter[17]++;
    currMB->mb_type = 8;
    img->allrefzero = mbmode == 5;
  }
  else
    if (mbmode == 6)
  {
    loop_counter[18]++;
    currMB->mb_type = 9;
    for (i = 0; i < 4; i++)
    {
      loop_counter[19]++;
      currMB->b8mode[i] = 11;
      currMB->b8pdir[i] = -1;
    }

  }
  else
    if (mbmode == 31)
  {
    loop_counter[20]++;
    currMB->mb_type = 14;
    for (i = 0; i < 4; i++)
    {
      loop_counter[21]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = -1;
    currMB->i16mode = 0;
  }
  else
  {
    currMB->mb_type = 10;
    for (i = 0; i < 4; i++)
    {
      loop_counter[22]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = ICBPTAB[(mbmode - 7) >> 2];
    currMB->i16mode = (mbmode - 7) & 0x03;
  }




}

void interpret_mb_mode_I(struct img_par *img)
{
  int i;
  const int ICBPTAB[6] = {0, 16, 32, 15, 31, 47};
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int mbmode = currMB->mb_type;
  if (mbmode == 0)
  {
    loop_counter[23]++;
    currMB->mb_type = 9;
    for (i = 0; i < 4; i++)
    {
      loop_counter[24]++;
      currMB->b8mode[i] = 11;
      currMB->b8pdir[i] = -1;
    }

  }
  else
    if (mbmode == 25)
  {
    loop_counter[25]++;
    currMB->mb_type = 14;
    for (i = 0; i < 4; i++)
    {
      loop_counter[26]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = -1;
    currMB->i16mode = 0;
  }
  else
  {
    currMB->mb_type = 10;
    for (i = 0; i < 4; i++)
    {
      loop_counter[27]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = ICBPTAB[(mbmode - 1) >> 2];
    currMB->i16mode = (mbmode - 1) & 0x03;
  }


}

void interpret_mb_mode_B(struct img_par *img)
{
  static const int offset2pdir16x16[12] = {0, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0};
  static const int offset2pdir16x8[22][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1}, {0, 0}, {1, 0}, {0, 0}, {0, 2}, {0, 0}, {1, 2}, {0, 0}, {2, 0}, {0, 0}, {2, 1}, {0, 0}, {2, 2}, {0, 0}};
  static const int offset2pdir8x16[22][2] = {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {0, 0}, {1, 1}, {0, 0}, {0, 1}, {0, 0}, {1, 0}, {0, 0}, {0, 2}, {0, 0}, {1, 2}, {0, 0}, {2, 0}, {0, 0}, {2, 1}, {0, 0}, {2, 2}};
  const int ICBPTAB[6] = {0, 16, 32, 15, 31, 47};
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int i;
  int mbmode;
  int mbtype = currMB->mb_type;
  int *b8mode = currMB->b8mode;
  int *b8pdir = currMB->b8pdir;
  if (mbtype == 0)
  {
    loop_counter[28]++;
    mbmode = 0;
    for (i = 0; i < 4; i++)
    {
      loop_counter[29]++;
      b8mode[i] = 0;
      b8pdir[i] = 2;
    }

  }
  else
    if (mbtype == 23)
  {
    loop_counter[30]++;
    mbmode = 9;
    for (i = 0; i < 4; i++)
    {
      loop_counter[31]++;
      b8mode[i] = 11;
      b8pdir[i] = -1;
    }

  }
  else
    if ((mbtype > 23) && (mbtype < 48))
  {
    loop_counter[32]++;
    mbmode = 10;
    for (i = 0; i < 4; i++)
    {
      loop_counter[33]++;
      b8mode[i] = 0;
      b8pdir[i] = -1;
    }

    currMB->cbp = ICBPTAB[(mbtype - 24) >> 2];
    currMB->i16mode = (mbtype - 24) & 0x03;
  }
  else
    if (mbtype == 22)
  {
    loop_counter[34]++;
    mbmode = 8;
  }
  else
    if (mbtype < 4)
  {
    loop_counter[35]++;
    mbmode = 1;
    for (i = 0; i < 4; i++)
    {
      loop_counter[36]++;
      b8mode[i] = 1;
      b8pdir[i] = offset2pdir16x16[mbtype];
    }

  }
  else
    if (mbtype == 48)
  {
    loop_counter[37]++;
    mbmode = 14;
    for (i = 0; i < 4; i++)
    {
      loop_counter[38]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = -1;
    currMB->i16mode = 0;
  }
  else
    if ((mbtype % 2) == 0)
  {
    loop_counter[39]++;
    mbmode = 2;
    for (i = 0; i < 4; i++)
    {
      loop_counter[40]++;
      b8mode[i] = 2;
      b8pdir[i] = offset2pdir16x8[mbtype][i / 2];
    }

  }
  else
  {
    mbmode = 3;
    for (i = 0; i < 4; i++)
    {
      loop_counter[41]++;
      b8mode[i] = 3;
      b8pdir[i] = offset2pdir8x16[mbtype][i % 2];
    }

  }







  currMB->mb_type = mbmode;
}

void interpret_mb_mode_SI(struct img_par *img)
{
  int i;
  const int ICBPTAB[6] = {0, 16, 32, 15, 31, 47};
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  int mbmode = currMB->mb_type;
  if (mbmode == 0)
  {
    loop_counter[42]++;
    currMB->mb_type = 12;
    for (i = 0; i < 4; i++)
    {
      loop_counter[43]++;
      currMB->b8mode[i] = 11;
      currMB->b8pdir[i] = -1;
    }

    img->siblock[img->mb_x][img->mb_y] = 1;
  }
  else
    if (mbmode == 1)
  {
    loop_counter[44]++;
    currMB->mb_type = 9;
    for (i = 0; i < 4; i++)
    {
      loop_counter[45]++;
      currMB->b8mode[i] = 11;
      currMB->b8pdir[i] = -1;
    }

  }
  else
    if (mbmode == 26)
  {
    loop_counter[46]++;
    currMB->mb_type = 14;
    for (i = 0; i < 4; i++)
    {
      loop_counter[47]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = -1;
    currMB->i16mode = 0;
  }
  else
  {
    currMB->mb_type = 10;
    for (i = 0; i < 4; i++)
    {
      loop_counter[48]++;
      currMB->b8mode[i] = 0;
      currMB->b8pdir[i] = -1;
    }

    currMB->cbp = ICBPTAB[(mbmode - 1) >> 2];
    currMB->i16mode = (mbmode - 2) & 0x03;
  }



}

void init_macroblock(struct img_par *img)
{
  int i;
  int j;
  for (i = 0; i < 4; i++)
  {
    loop_counter[49]++;
    for (j = 0; j < 4; j++)
    {
      loop_counter[50]++;
      dec_picture->mv[LIST_0][img->block_y + j][img->block_x + i][0] = 0;
      dec_picture->mv[LIST_0][img->block_y + j][img->block_x + i][1] = 0;
      dec_picture->mv[LIST_1][img->block_y + j][img->block_x + i][0] = 0;
      dec_picture->mv[LIST_1][img->block_y + j][img->block_x + i][1] = 0;
      img->ipredmode[img->block_x + i][img->block_y + j] = 2;
    }

  }

  for (j = 0; j < 4; j++)
  {
    loop_counter[51]++;
    for (i = 0; i < 4; i++)
    {
      loop_counter[52]++;
      dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + i] = -1;
      dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + i] = -1;
      dec_picture->ref_pic_id[LIST_0][img->block_y + j][img->block_x + i] = (-9223372036854775807LL) - 1LL;
      dec_picture->ref_pic_id[LIST_1][img->block_y + j][img->block_x + i] = (-9223372036854775807LL) - 1LL;
    }

  }

}

void SetB8Mode(struct img_par *img, Macroblock *currMB, int value, int i)
{
  static const int p_v2b8[5] = {4, 5, 6, 7, 11};
  static const int p_v2pd[5] = {0, 0, 0, 0, -1};
  static const int b_v2b8[14] = {0, 4, 4, 4, 5, 6, 5, 6, 5, 6, 7, 7, 7, 11};
  static const int b_v2pd[14] = {2, 0, 1, 2, 0, 0, 1, 1, 2, 2, 0, 1, 2, -1};
  if (img->type == B_SLICE)
  {
    loop_counter[53]++;
    currMB->b8mode[i] = b_v2b8[value];
    currMB->b8pdir[i] = b_v2pd[value];
  }
  else
  {
    currMB->b8mode[i] = p_v2b8[value];
    currMB->b8pdir[i] = p_v2pd[value];
  }

}

void reset_coeffs()
{
  int i;
  int j;
  int iii;
  int jjj;
  for (i = 0; i < 4; i++)
  {
    loop_counter[54]++;
    for (j = 0; j < 4; j++)
    {
      loop_counter[55]++;
      for (iii = 0; iii < 4; iii++)
      {
        loop_counter[56]++;
        for (jjj = 0; jjj < 4; jjj++)
        {
          loop_counter[57]++;
          img->cof[i][j][iii][jjj] = 0;
        }

      }

    }

  }

  for (j = 4; j < (4 + img->num_blk8x8_uv); j++)
  {
    loop_counter[58]++;
    for (i = 0; i < 4; i++)
    {
      loop_counter[59]++;
      for (iii = 0; iii < 4; iii++)
      {
        loop_counter[60]++;
        for (jjj = 0; jjj < 4; jjj++)
        {
          loop_counter[61]++;
          img->cof[i][j][iii][jjj] = 0;
        }

      }

    }

  }

  for (i = 0; i < 4; i++)
  {
    loop_counter[62]++;
    for (j = 0; j < (4 + img->num_blk8x8_uv); j++)
    {
      loop_counter[63]++;
      img->nz_coeff[img->current_mb_nr][i][j] = 0;
    }

  }

}

void field_flag_inference()
{
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  if (currMB->mbAvailA)
  {
    loop_counter[64]++;
    currMB->mb_field = img->mb_data[currMB->mbAddrA].mb_field;
  }
  else
  {
    if (currMB->mbAvailB)
    {
      loop_counter[65]++;
      currMB->mb_field = img->mb_data[currMB->mbAddrB].mb_field;
    }
    else
      currMB->mb_field = 0;

  }

}

int read_one_macroblock(struct img_par *img, struct inp_par *inp)
{
  int i;
  SyntaxElement currSE;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  Slice *currSlice = img->currentSlice;
  DataPartition *dP;
  int *partMap = assignSE2partition[currSlice->dp_mode];
  Macroblock *topMB = 0;
  int prevMbSkipped = 0;
  int img_block_y;
  int check_bottom;
  int read_bottom;
  int read_top;
  if (img->MbaffFrameFlag)
  {
    loop_counter[66]++;
    if (img->current_mb_nr % 2)
    {
      loop_counter[67]++;
      topMB = &img->mb_data[img->current_mb_nr - 1];
      if (!(img->type == B_SLICE))
      {
        loop_counter[68]++;
        prevMbSkipped = topMB->mb_type == 0;
      }
      else
        prevMbSkipped = topMB->skip_flag;

    }
    else
      prevMbSkipped = 0;

  }

  if ((img->current_mb_nr % 2) == 0)
  {
    loop_counter[69]++;
    currMB->mb_field = 0;
  }
  else
    currMB->mb_field = img->mb_data[img->current_mb_nr - 1].mb_field;

  currMB->qp = img->qp;
  currSE.type = 2;
  dP = &currSlice->partArr[partMap[currSE.type]];
  if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
  {
    loop_counter[70]++;
    currSE.mapping = linfo_ue;
  }

  if ((img->type == I_SLICE) || (img->type == SI_SLICE))
  {
    loop_counter[71]++;
    if (img->MbaffFrameFlag && ((img->current_mb_nr % 2) == 0))
    {
      loop_counter[72]++;
      ;
      if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
      {
        loop_counter[73]++;
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
      }
      else
      {
        currSE.reading = readFieldModeInfo_CABAC;
        dP->readSyntaxElement(&currSE, img, inp, dP);
      }

      currMB->mb_field = currSE.value1;
    }

    if (active_pps->entropy_coding_mode_flag == CABAC)
    {
      loop_counter[74]++;
      CheckAvailabilityOfNeighborsCABAC();
    }

    ;
    currSE.reading = readMB_typeInfo_CABAC;
    dP->readSyntaxElement(&currSE, img, inp, dP);
    currMB->mb_type = currSE.value1;
    if (!dP->bitstream->ei_flag)
    {
      loop_counter[75]++;
      currMB->ei_flag = 0;
    }

  }
  else
    if (active_pps->entropy_coding_mode_flag == CABAC)
  {
    loop_counter[76]++;
    if (img->MbaffFrameFlag && (((img->current_mb_nr % 2) == 0) || prevMbSkipped))
    {
      loop_counter[77]++;
      field_flag_inference();
    }

    CheckAvailabilityOfNeighborsCABAC();
    ;
    currSE.reading = readMB_skip_flagInfo_CABAC;
    dP->readSyntaxElement(&currSE, img, inp, dP);
    currMB->mb_type = currSE.value1;
    currMB->skip_flag = !currSE.value1;
    if (img->type == B_SLICE)
    {
      loop_counter[78]++;
      currMB->cbp = currSE.value2;
    }

    if (!dP->bitstream->ei_flag)
    {
      loop_counter[79]++;
      currMB->ei_flag = 0;
    }

    if (((img->type == B_SLICE) && (currSE.value1 == 0)) && (currSE.value2 == 0))
    {
      loop_counter[80]++;
      img->cod_counter = 0;
    }

    if (img->MbaffFrameFlag)
    {
      loop_counter[81]++;
      check_bottom = (read_bottom = (read_top = 0));
      if ((img->current_mb_nr % 2) == 0)
      {
        loop_counter[82]++;
        check_bottom = currMB->skip_flag;
        read_top = !check_bottom;
      }
      else
      {
        read_bottom = topMB->skip_flag && (!currMB->skip_flag);
      }

      if (read_bottom || read_top)
      {
        loop_counter[83]++;
        ;
        currSE.reading = readFieldModeInfo_CABAC;
        dP->readSyntaxElement(&currSE, img, inp, dP);
        currMB->mb_field = currSE.value1;
      }

      if (check_bottom)
      {
        loop_counter[84]++;
        check_next_mb_and_get_field_mode_CABAC(&currSE, img, inp, dP);
      }

    }

    CheckAvailabilityOfNeighborsCABAC();
    if (currMB->mb_type != 0)
    {
      loop_counter[85]++;
      currSE.reading = readMB_typeInfo_CABAC;
      ;
      dP->readSyntaxElement(&currSE, img, inp, dP);
      currMB->mb_type = currSE.value1;
      if (!dP->bitstream->ei_flag)
      {
        loop_counter[86]++;
        currMB->ei_flag = 0;
      }

    }

  }
  else
  {
    if (img->cod_counter == (-1))
    {
      loop_counter[87]++;
      ;
      dP->readSyntaxElement(&currSE, img, inp, dP);
      img->cod_counter = currSE.value1;
    }

    if (img->cod_counter == 0)
    {
      loop_counter[88]++;
      if (img->MbaffFrameFlag && (((img->current_mb_nr % 2) == 0) || ((img->current_mb_nr % 2) && prevMbSkipped)))
      {
        loop_counter[89]++;
        ;
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        currMB->mb_field = currSE.value1;
      }

      ;
      dP->readSyntaxElement(&currSE, img, inp, dP);
      if ((img->type == P_SLICE) || (img->type == SP_SLICE))
      {
        loop_counter[90]++;
        currSE.value1++;
      }

      currMB->mb_type = currSE.value1;
      if (!dP->bitstream->ei_flag)
      {
        loop_counter[91]++;
        currMB->ei_flag = 0;
      }

      img->cod_counter--;
      currMB->skip_flag = 0;
    }
    else
    {
      img->cod_counter--;
      currMB->mb_type = 0;
      currMB->ei_flag = 0;
      currMB->skip_flag = 1;
      if (img->MbaffFrameFlag)
      {
        loop_counter[92]++;
        if ((img->cod_counter == 0) && ((img->current_mb_nr % 2) == 0))
        {
          loop_counter[93]++;
          ;
          currSE.len = 1;
          readSyntaxElement_FLC(&currSE, dP->bitstream);
          dP->bitstream->frame_bitoffset--;
          currMB->mb_field = currSE.value1;
        }
        else
          if ((img->cod_counter > 0) && ((img->current_mb_nr % 2) == 0))
        {
          loop_counter[94]++;
          if (mb_is_available(img->current_mb_nr - 2, img->current_mb_nr) && ((img->current_mb_nr % (img->PicWidthInMbs * 2)) != 0))
          {
            loop_counter[95]++;
            currMB->mb_field = img->mb_data[img->current_mb_nr - 2].mb_field;
          }
          else
          {
            if (mb_is_available(img->current_mb_nr - (2 * img->PicWidthInMbs), img->current_mb_nr))
            {
              loop_counter[96]++;
              currMB->mb_field = img->mb_data[img->current_mb_nr - (2 * img->PicWidthInMbs)].mb_field;
            }
            else
              currMB->mb_field = 0;

          }

        }


      }

    }

  }


  dec_picture->mb_field[img->current_mb_nr] = currMB->mb_field;
  img->siblock[img->mb_x][img->mb_y] = 0;
  if (img->type == P_SLICE)
  {
    loop_counter[97]++;
    interpret_mb_mode_P(img);
  }
  else
    if (img->type == I_SLICE)
  {
    loop_counter[98]++;
    interpret_mb_mode_I(img);
  }
  else
    if (img->type == B_SLICE)
  {
    loop_counter[99]++;
    interpret_mb_mode_B(img);
  }
  else
    if (img->type == SP_SLICE)
  {
    loop_counter[100]++;
    interpret_mb_mode_P(img);
  }
  else
    if (img->type == SI_SLICE)
  {
    loop_counter[101]++;
    interpret_mb_mode_SI(img);
  }





  if (img->MbaffFrameFlag)
  {
    loop_counter[102]++;
    if (currMB->mb_field)
    {
      loop_counter[103]++;
      img->num_ref_idx_l0_active <<= 1;
      img->num_ref_idx_l1_active <<= 1;
    }

  }

  currMB->NoMbPartLessThan8x8Flag = ((currMB->mb_type == 0) && (img->type == B_SLICE)) && (!active_sps->direct_8x8_inference_flag) ? 0 : 1;
  if (currMB->mb_type == 8)
  {
    loop_counter[104]++;
    currSE.type = 2;
    dP = &currSlice->partArr[partMap[2]];
    for (i = 0; i < 4; i++)
    {
      loop_counter[105]++;
      if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
      {
        loop_counter[106]++;
        currSE.mapping = linfo_ue;
      }
      else
        currSE.reading = readB8_typeInfo_CABAC;

      ;
      dP->readSyntaxElement(&currSE, img, inp, dP);
      SetB8Mode(img, currMB, currSE.value1, i);
      currMB->NoMbPartLessThan8x8Flag &= ((currMB->b8mode[i] == 0) && active_sps->direct_8x8_inference_flag) || (currMB->b8mode[i] == 4);
    }

    init_macroblock(img);
    readMotionInfoFromNAL(img, inp);
  }

  if ((currMB->mb_type == 9) && img->Transform8x8Mode)
  {
    loop_counter[107]++;
    currSE.type = SE_TRANSFORM_SIZE_FLAG;
    dP = &currSlice->partArr[partMap[SE_TRANSFORM_SIZE_FLAG]];
    currSE.reading = readMB_transform_size_flag_CABAC;
    ;
    if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
    {
      loop_counter[108]++;
      currSE.len = 1;
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }
    else
    {
      dP->readSyntaxElement(&currSE, img, inp, dP);
    }

    currMB->luma_transform_size_8x8_flag = currSE.value1;
    if (currMB->luma_transform_size_8x8_flag)
    {
      loop_counter[109]++;
      currMB->mb_type = 13;
      for (i = 0; i < 4; i++)
      {
        loop_counter[110]++;
        currMB->b8mode[i] = 13;
        currMB->b8pdir[i] = -1;
      }

    }

  }
  else
  {
    currMB->luma_transform_size_8x8_flag = 0;
  }

  if (active_pps->constrained_intra_pred_flag && ((img->type == P_SLICE) || (img->type == B_SLICE)))
  {
    loop_counter[111]++;
    if (!(((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12)))
    {
      loop_counter[112]++;
      img->intra_block[img->current_mb_nr] = 0;
    }

  }

  dP = &currSlice->partArr[partMap[6]];
  if (((((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12)) && dP->bitstream->ei_flag) && img->number)
  {
    loop_counter[113]++;
    currMB->mb_type = 0;
    currMB->ei_flag = 1;
    for (i = 0; i < 4; i++)
    {
      loop_counter[114]++;
      currMB->b8mode[i] = (currMB->b8pdir[i] = 0);
    }

  }

  dP = &currSlice->partArr[partMap[currSE.type]];
  if (!(currMB->mb_type == 8))
  {
    loop_counter[115]++;
    init_macroblock(img);
  }

  if (((currMB->mb_type == 0) && (img->type == B_SLICE)) && (img->cod_counter >= 0))
  {
    loop_counter[116]++;
    currMB->cbp = 0;
    reset_coeffs();
    if (active_pps->entropy_coding_mode_flag == CABAC)
    {
      loop_counter[117]++;
      img->cod_counter = -1;
    }

    return 1;
  }

  if ((currMB->mb_type == 0) && ((img->type == P_SLICE) || (img->type == SP_SLICE)))
  {
    loop_counter[118]++;
    int i;
    int j;
    int k;
    short pmv[2];
    int zeroMotionAbove;
    int zeroMotionLeft;
    PixelPos mb_a;
    PixelPos mb_b;
    int a_mv_y = 0;
    int a_ref_idx = 0;
    int b_mv_y = 0;
    int b_ref_idx = 0;
    int list_offset = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? 4 : 2 : 0;
    getLuma4x4Neighbour(img->current_mb_nr, 0, 0, -1, 0, &mb_a);
    getLuma4x4Neighbour(img->current_mb_nr, 0, 0, 0, -1, &mb_b);
    if (mb_a.available)
    {
      loop_counter[119]++;
      a_mv_y = dec_picture->mv[LIST_0][mb_a.pos_y][mb_a.pos_x][1];
      a_ref_idx = dec_picture->ref_idx[LIST_0][mb_a.pos_y][mb_a.pos_x];
      if (currMB->mb_field && (!img->mb_data[mb_a.mb_addr].mb_field))
      {
        loop_counter[120]++;
        a_mv_y /= 2;
        a_ref_idx *= 2;
      }

      if ((!currMB->mb_field) && img->mb_data[mb_a.mb_addr].mb_field)
      {
        loop_counter[121]++;
        a_mv_y *= 2;
        a_ref_idx >>= 1;
      }

    }

    if (mb_b.available)
    {
      loop_counter[122]++;
      b_mv_y = dec_picture->mv[LIST_0][mb_b.pos_y][mb_b.pos_x][1];
      b_ref_idx = dec_picture->ref_idx[LIST_0][mb_b.pos_y][mb_b.pos_x];
      if (currMB->mb_field && (!img->mb_data[mb_b.mb_addr].mb_field))
      {
        loop_counter[123]++;
        b_mv_y /= 2;
        b_ref_idx *= 2;
      }

      if ((!currMB->mb_field) && img->mb_data[mb_b.mb_addr].mb_field)
      {
        loop_counter[124]++;
        b_mv_y *= 2;
        b_ref_idx >>= 1;
      }

    }

    zeroMotionLeft = !mb_a.available ? 1 : ((a_ref_idx == 0) && (dec_picture->mv[LIST_0][mb_a.pos_y][mb_a.pos_x][0] == 0)) && (a_mv_y == 0) ? 1 : 0;
    zeroMotionAbove = !mb_b.available ? 1 : ((b_ref_idx == 0) && (dec_picture->mv[LIST_0][mb_b.pos_y][mb_b.pos_x][0] == 0)) && (b_mv_y == 0) ? 1 : 0;
    currMB->cbp = 0;
    reset_coeffs();
    img_block_y = img->block_y;
    if (zeroMotionAbove || zeroMotionLeft)
    {
      loop_counter[125]++;
      for (i = 0; i < 4; i++)
      {
        loop_counter[126]++;
        for (j = 0; j < 4; j++)
        {
          loop_counter[127]++;
          for (k = 0; k < 2; k++)
          {
            loop_counter[128]++;
            dec_picture->mv[LIST_0][img->block_y + j][img->block_x + i][k] = 0;
          }

        }

      }

    }
    else
    {
      SetMotionVectorPredictor(img, pmv, pmv + 1, 0, LIST_0, dec_picture->ref_idx, dec_picture->mv, 0, 0, 16, 16);
      for (i = 0; i < 4; i++)
      {
        loop_counter[129]++;
        for (j = 0; j < 4; j++)
        {
          loop_counter[130]++;
          for (k = 0; k < 2; k++)
          {
            loop_counter[131]++;
            dec_picture->mv[LIST_0][img_block_y + j][img->block_x + i][k] = pmv[k];
          }

        }

      }

    }

    for (i = 0; i < 4; i++)
    {
      loop_counter[132]++;
      for (j = 0; j < 4; j++)
      {
        loop_counter[133]++;
        dec_picture->ref_idx[LIST_0][img_block_y + j][img->block_x + i] = 0;
        dec_picture->ref_pic_id[LIST_0][img_block_y + j][img->block_x + i] = dec_picture->ref_pic_num[img->current_slice_nr][LIST_0 + list_offset][(short) dec_picture->ref_idx[LIST_0][img_block_y + j][img->block_x + i]];
      }

    }

    return 1;
  }

  if (currMB->mb_type != 14)
  {
    loop_counter[134]++;
    read_ipred_modes(img, inp);
    if ((((((currMB->mb_type != 9) && (currMB->mb_type != 10)) && (currMB->mb_type != 13)) && (currMB->mb_type != 0)) && (currMB->mb_type != 14)) && (!(currMB->mb_type == 8)))
    {
      loop_counter[135]++;
      readMotionInfoFromNAL(img, inp);
    }

    readCBPandCoeffsFromNAL(img, inp);
  }
  else
  {
    dP = &currSlice->partArr[partMap[2]];
    readIPCMcoeffsFromNAL(img, inp, dP);
  }

  return 1;
}

void init_decoding_engine_IPCM(struct img_par *img)
{
  Slice *currSlice = img->currentSlice;
  Bitstream *currStream;
  int ByteStartPosition;
  int PartitionNumber;
  int i;
  if (currSlice->dp_mode == PAR_DP_1)
  {
    loop_counter[136]++;
    PartitionNumber = 1;
  }
  else
    if (currSlice->dp_mode == PAR_DP_3)
  {
    loop_counter[137]++;
    PartitionNumber = 3;
  }
  else
  {
    printf("Partition Mode is not supported\n");
    exit(1);
  }


  for (i = 0; i < PartitionNumber; i++)
  {
    loop_counter[138]++;
    currStream = currSlice->partArr[i].bitstream;
    ByteStartPosition = currStream->read_len;
    arideco_start_decoding(&currSlice->partArr[i].de_cabac, currStream->streamBuffer, ByteStartPosition, &currStream->read_len, img->type);
  }

}

void readIPCMcoeffsFromNAL(struct img_par *img, struct inp_par *inp, struct datapartition *dP)
{
  SyntaxElement currSE;
  int i;
  int j;
  if (active_pps->entropy_coding_mode_flag == CABAC)
  {
    loop_counter[139]++;
    currSE.len = 8;
    for (i = 0; i < 16; i++)
    {
      loop_counter[140]++;
      for (j = 0; j < 16; j++)
      {
        loop_counter[141]++;
        readIPCMBytes_CABAC(&currSE, dP->bitstream);
        img->cof[i / 4][j / 4][i % 4][j % 4] = currSE.value1;
      }

    }

    for (i = 0; i < 8; i++)
    {
      loop_counter[142]++;
      for (j = 0; j < 8; j++)
      {
        loop_counter[143]++;
        readIPCMBytes_CABAC(&currSE, dP->bitstream);
        img->cof[i / 4][(j / 4) + 4][i % 4][j % 4] = currSE.value1;
      }

    }

    for (i = 0; i < 8; i++)
    {
      loop_counter[144]++;
      for (j = 0; j < 8; j++)
      {
        loop_counter[145]++;
        readIPCMBytes_CABAC(&currSE, dP->bitstream);
        img->cof[(i / 4) + 2][(j / 4) + 4][i % 4][j % 4] = currSE.value1;
      }

    }

    init_decoding_engine_IPCM(img);
  }
  else
  {
    if ((dP->bitstream->frame_bitoffset % 8) != 0)
    {
      loop_counter[146]++;
      ;
      currSE.len = 8 - (dP->bitstream->frame_bitoffset % 8);
      readSyntaxElement_FLC(&currSE, dP->bitstream);
    }

    currSE.len = 8;
    ;
    for (i = 0; i < 16; i++)
    {
      loop_counter[147]++;
      for (j = 0; j < 16; j++)
      {
        loop_counter[148]++;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        img->cof[i / 4][j / 4][i % 4][j % 4] = currSE.value1;
      }

    }

    for (i = 0; i < 8; i++)
    {
      loop_counter[149]++;
      for (j = 0; j < 8; j++)
      {
        loop_counter[150]++;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        img->cof[i / 4][(j / 4) + 4][i % 4][j % 4] = currSE.value1;
      }

    }

    for (i = 0; i < 8; i++)
    {
      loop_counter[151]++;
      for (j = 0; j < 8; j++)
      {
        loop_counter[152]++;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
        img->cof[(i / 4) + 2][(j / 4) + 4][i % 4][j % 4] = currSE.value1;
      }

    }

  }

}

void read_ipred_modes(struct img_par *img, struct inp_par *inp)
{
  int b8;
  int i;
  int j;
  int bi;
  int bj;
  int bx;
  int by;
  int dec;
  SyntaxElement currSE;
  Slice *currSlice;
  DataPartition *dP;
  int *partMap;
  Macroblock *currMB;
  int ts;
  int ls;
  int mostProbableIntraPredMode;
  int upIntraPredMode;
  int leftIntraPredMode;
  int IntraChromaPredModeFlag;
  int bs_x;
  int bs_y;
  int ii;
  int jj;
  PixelPos left_block;
  PixelPos top_block;
  currMB = &img->mb_data[img->current_mb_nr];
  IntraChromaPredModeFlag = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
  currSlice = img->currentSlice;
  partMap = assignSE2partition[currSlice->dp_mode];
  currSE.type = 4;
  ;
  dP = &currSlice->partArr[partMap[currSE.type]];
  if (!((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag))
  {
    loop_counter[153]++;
    currSE.reading = readIntraPredMode_CABAC;
  }

  for (b8 = 0; b8 < 4; b8++)
  {
    loop_counter[154]++;
    if ((currMB->b8mode[b8] == 11) || (currMB->b8mode[b8] == 13))
    {
      loop_counter[155]++;
      bs_x = (bs_y = currMB->b8mode[b8] == 13 ? 8 : 4);
      IntraChromaPredModeFlag = 1;
      ii = bs_x >> 2;
      jj = bs_y >> 2;
      for (j = 0; j < 2; j += jj)
      {
        loop_counter[156]++;
        for (i = 0; i < 2; i += ii)
        {
          loop_counter[157]++;
          if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
          {
            loop_counter[158]++;
            readSyntaxElement_Intra4x4PredictionMode(&currSE, img, inp, dP);
          }
          else
          {
            currSE.context = ((b8 << 2) + (j << 1)) + i;
            dP->readSyntaxElement(&currSE, img, inp, dP);
          }

          bx = ((b8 & 1) << 1) + i;
          by = (b8 & 2) + j;
          getLuma4x4Neighbour(img->current_mb_nr, bx, by, -1, 0, &left_block);
          getLuma4x4Neighbour(img->current_mb_nr, bx, by, 0, -1, &top_block);
          bi = img->block_x + bx;
          bj = img->block_y + by;
          if (active_pps->constrained_intra_pred_flag)
          {
            loop_counter[159]++;
            left_block.available = left_block.available ? img->intra_block[left_block.mb_addr] : 0;
            top_block.available = top_block.available ? img->intra_block[top_block.mb_addr] : 0;
          }

          ts = (ls = 0);
          if ((currMB->mb_type == 9) && (img->type == SI_SLICE))
          {
            loop_counter[160]++;
            if (left_block.available)
            {
              loop_counter[161]++;
              if (img->siblock[left_block.pos_x][left_block.pos_y])
              {
                loop_counter[162]++;
                ls = 1;
              }

            }

            if (top_block.available)
            {
              loop_counter[163]++;
              if (img->siblock[top_block.pos_x][top_block.pos_y])
              {
                loop_counter[164]++;
                ts = 1;
              }

            }

          }

          upIntraPredMode = top_block.available && (ts == 0) ? img->ipredmode[top_block.pos_x][top_block.pos_y] : -1;
          leftIntraPredMode = left_block.available && (ls == 0) ? img->ipredmode[left_block.pos_x][left_block.pos_y] : -1;
          mostProbableIntraPredMode = (upIntraPredMode < 0) || (leftIntraPredMode < 0) ? 2 : upIntraPredMode < leftIntraPredMode ? upIntraPredMode : leftIntraPredMode;
          dec = currSE.value1 == (-1) ? mostProbableIntraPredMode : currSE.value1 + (currSE.value1 >= mostProbableIntraPredMode);
          for (jj = 0; jj < (bs_y >> 2); jj++)
          {
            loop_counter[165]++;
            for (ii = 0; ii < (bs_x >> 2); ii++)
            {
              loop_counter[166]++;
              img->ipredmode[bi + ii][bj + jj] = dec;
            }

          }

        }

      }

    }

  }

  if (IntraChromaPredModeFlag && (dec_picture->chroma_format_idc != 0))
  {
    loop_counter[167]++;
    currSE.type = 4;
    ;
    dP = &currSlice->partArr[partMap[currSE.type]];
    if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
    {
      loop_counter[168]++;
      currSE.mapping = linfo_ue;
    }
    else
      currSE.reading = readCIPredMode_CABAC;

    dP->readSyntaxElement(&currSE, img, inp, dP);
    currMB->c_ipred_mode = currSE.value1;
    if ((currMB->c_ipred_mode < 0) || (currMB->c_ipred_mode > 3))
    {
      loop_counter[169]++;
      error("illegal chroma intra pred mode!\n", 600);
    }

  }

}

static void SetMotionVectorPredictor(struct img_par *img, short *pmv_x, short *pmv_y, char ref_frame, byte list, char ***refPic, short ****tmp_mv, int block_x, int block_y, int blockshape_x, int blockshape_y)
{
  int mb_x = 4 * block_x;
  int mb_y = 4 * block_y;
  int mb_nr = img->current_mb_nr;
  int mv_a;
  int mv_b;
  int mv_c;
  int pred_vec = 0;
  int mvPredType;
  int rFrameL;
  int rFrameU;
  int rFrameUR;
  int hv;
  PixelPos block_a;
  PixelPos block_b;
  PixelPos block_c;
  PixelPos block_d;
  getLuma4x4Neighbour(mb_nr, block_x, block_y, -1, 0, &block_a);
  getLuma4x4Neighbour(mb_nr, block_x, block_y, 0, -1, &block_b);
  getLuma4x4Neighbour(mb_nr, block_x, block_y, blockshape_x, -1, &block_c);
  getLuma4x4Neighbour(mb_nr, block_x, block_y, -1, -1, &block_d);
  if (mb_y > 0)
  {
    loop_counter[170]++;
    if (mb_x < 8)
    {
      loop_counter[171]++;
      if (mb_y == 8)
      {
        loop_counter[172]++;
        if (blockshape_x == 16)
        {
          loop_counter[173]++;
          block_c.available = 0;
        }

      }
      else
      {
        if ((mb_x + blockshape_x) == 8)
        {
          loop_counter[174]++;
          block_c.available = 0;
        }

      }

    }
    else
    {
      if ((mb_x + blockshape_x) == 16)
      {
        loop_counter[175]++;
        block_c.available = 0;
      }

    }

  }

  if (!block_c.available)
  {
    loop_counter[176]++;
    block_c = block_d;
  }

  mvPredType = 0;
  if (!img->MbaffFrameFlag)
  {
    loop_counter[177]++;
    rFrameL = block_a.available ? refPic[list][block_a.pos_y][block_a.pos_x] : -1;
    rFrameU = block_b.available ? refPic[list][block_b.pos_y][block_b.pos_x] : -1;
    rFrameUR = block_c.available ? refPic[list][block_c.pos_y][block_c.pos_x] : -1;
  }
  else
  {
    if (img->mb_data[img->current_mb_nr].mb_field)
    {
      loop_counter[178]++;
      rFrameL = block_a.available ? img->mb_data[block_a.mb_addr].mb_field ? refPic[list][block_a.pos_y][block_a.pos_x] : refPic[list][block_a.pos_y][block_a.pos_x] * 2 : -1;
      rFrameU = block_b.available ? img->mb_data[block_b.mb_addr].mb_field ? refPic[list][block_b.pos_y][block_b.pos_x] : refPic[list][block_b.pos_y][block_b.pos_x] * 2 : -1;
      rFrameUR = block_c.available ? img->mb_data[block_c.mb_addr].mb_field ? refPic[list][block_c.pos_y][block_c.pos_x] : refPic[list][block_c.pos_y][block_c.pos_x] * 2 : -1;
    }
    else
    {
      rFrameL = block_a.available ? img->mb_data[block_a.mb_addr].mb_field ? refPic[list][block_a.pos_y][block_a.pos_x] >> 1 : refPic[list][block_a.pos_y][block_a.pos_x] : -1;
      rFrameU = block_b.available ? img->mb_data[block_b.mb_addr].mb_field ? refPic[list][block_b.pos_y][block_b.pos_x] >> 1 : refPic[list][block_b.pos_y][block_b.pos_x] : -1;
      rFrameUR = block_c.available ? img->mb_data[block_c.mb_addr].mb_field ? refPic[list][block_c.pos_y][block_c.pos_x] >> 1 : refPic[list][block_c.pos_y][block_c.pos_x] : -1;
    }

  }

  if (((rFrameL == ref_frame) && (rFrameU != ref_frame)) && (rFrameUR != ref_frame))
  {
    loop_counter[179]++;
    mvPredType = 1;
  }
  else
    if (((rFrameL != ref_frame) && (rFrameU == ref_frame)) && (rFrameUR != ref_frame))
  {
    loop_counter[180]++;
    mvPredType = 2;
  }
  else
    if (((rFrameL != ref_frame) && (rFrameU != ref_frame)) && (rFrameUR == ref_frame))
  {
    loop_counter[181]++;
    mvPredType = 3;
  }



  if ((blockshape_x == 8) && (blockshape_y == 16))
  {
    loop_counter[182]++;
    if (mb_x == 0)
    {
      loop_counter[183]++;
      if (rFrameL == ref_frame)
      {
        loop_counter[184]++;
        mvPredType = 1;
      }

    }
    else
    {
      if (rFrameUR == ref_frame)
      {
        loop_counter[185]++;
        mvPredType = 3;
      }

    }

  }
  else
    if ((blockshape_x == 16) && (blockshape_y == 8))
  {
    loop_counter[186]++;
    if (mb_y == 0)
    {
      loop_counter[187]++;
      if (rFrameU == ref_frame)
      {
        loop_counter[188]++;
        mvPredType = 2;
      }

    }
    else
    {
      if (rFrameL == ref_frame)
      {
        loop_counter[189]++;
        mvPredType = 1;
      }

    }

  }


  for (hv = 0; hv < 2; hv++)
  {
    loop_counter[190]++;
    if ((!img->MbaffFrameFlag) || (hv == 0))
    {
      loop_counter[191]++;
      mv_a = block_a.available ? tmp_mv[list][block_a.pos_y][block_a.pos_x][hv] : 0;
      mv_b = block_b.available ? tmp_mv[list][block_b.pos_y][block_b.pos_x][hv] : 0;
      mv_c = block_c.available ? tmp_mv[list][block_c.pos_y][block_c.pos_x][hv] : 0;
    }
    else
    {
      if (img->mb_data[img->current_mb_nr].mb_field)
      {
        loop_counter[192]++;
        mv_a = block_a.available ? img->mb_data[block_a.mb_addr].mb_field ? tmp_mv[list][block_a.pos_y][block_a.pos_x][hv] : tmp_mv[list][block_a.pos_y][block_a.pos_x][hv] / 2 : 0;
        mv_b = block_b.available ? img->mb_data[block_b.mb_addr].mb_field ? tmp_mv[list][block_b.pos_y][block_b.pos_x][hv] : tmp_mv[list][block_b.pos_y][block_b.pos_x][hv] / 2 : 0;
        mv_c = block_c.available ? img->mb_data[block_c.mb_addr].mb_field ? tmp_mv[list][block_c.pos_y][block_c.pos_x][hv] : tmp_mv[list][block_c.pos_y][block_c.pos_x][hv] / 2 : 0;
      }
      else
      {
        mv_a = block_a.available ? img->mb_data[block_a.mb_addr].mb_field ? tmp_mv[list][block_a.pos_y][block_a.pos_x][hv] * 2 : tmp_mv[list][block_a.pos_y][block_a.pos_x][hv] : 0;
        mv_b = block_b.available ? img->mb_data[block_b.mb_addr].mb_field ? tmp_mv[list][block_b.pos_y][block_b.pos_x][hv] * 2 : tmp_mv[list][block_b.pos_y][block_b.pos_x][hv] : 0;
        mv_c = block_c.available ? img->mb_data[block_c.mb_addr].mb_field ? tmp_mv[list][block_c.pos_y][block_c.pos_x][hv] * 2 : tmp_mv[list][block_c.pos_y][block_c.pos_x][hv] : 0;
      }

    }

    switch (mvPredType)
    {
      case 0:
        loop_counter[193]++;
        if (!(block_b.available || block_c.available))
      {
        loop_counter[194]++;
        pred_vec = mv_a;
      }
      else
        pred_vec = (((mv_a + mv_b) + mv_c) - (mv_a < (mv_b < mv_c ? mv_b : mv_c) ? mv_a : mv_b < mv_c ? mv_b : mv_c)) - (mv_a > (mv_b > mv_c ? mv_b : mv_c) ? mv_a : mv_b > mv_c ? mv_b : mv_c);

        break;

      case 1:
        loop_counter[195]++;
        pred_vec = mv_a;
        break;

      case 2:
        loop_counter[196]++;
        pred_vec = mv_b;
        break;

      case 3:
        loop_counter[197]++;
        pred_vec = mv_c;
        break;

      default:
        break;

    }

    if (hv == 0)
    {
      loop_counter[198]++;
      *pmv_x = pred_vec;
    }
    else
      *pmv_y = pred_vec;

  }

}

int BType2CtxRef(int btype)
{
  if (btype < 4)
  {
    loop_counter[199]++;
    return 0;
  }
  else
    return 1;

}

void readMotionInfoFromNAL(struct img_par *img, struct inp_par *inp)
{
  int i;
  int j;
  int k;
  int step_h;
  int step_v;
  int curr_mvd;
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  SyntaxElement currSE;
  Slice *currSlice = img->currentSlice;
  DataPartition *dP;
  int *partMap = assignSE2partition[currSlice->dp_mode];
  int bframe = img->type == B_SLICE;
  int partmode = currMB->mb_type == 8 ? 4 : currMB->mb_type;
  int step_h0 = BLOCK_STEP[partmode][0];
  int step_v0 = BLOCK_STEP[partmode][1];
  int mv_mode;
  int i0;
  int j0;
  char refframe;
  short pmv[2];
  int j4;
  int i4;
  int ii;
  int jj;
  int vec;
  int mv_scale = 0;
  int flag_mode;
  int list_offset = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? 4 : 2 : 0;
  byte **moving_block;
  short ****co_located_mv;
  char ***co_located_ref_idx;
  int64 ***co_located_ref_id;
  if (img->MbaffFrameFlag && currMB->mb_field)
  {
    loop_counter[200]++;
    if (img->current_mb_nr % 2)
    {
      loop_counter[201]++;
      moving_block = Co_located->bottom_moving_block;
      co_located_mv = Co_located->bottom_mv;
      co_located_ref_idx = Co_located->bottom_ref_idx;
      co_located_ref_id = Co_located->bottom_ref_pic_id;
    }
    else
    {
      moving_block = Co_located->top_moving_block;
      co_located_mv = Co_located->top_mv;
      co_located_ref_idx = Co_located->top_ref_idx;
      co_located_ref_id = Co_located->top_ref_pic_id;
    }

  }
  else
  {
    moving_block = Co_located->moving_block;
    co_located_mv = Co_located->mv;
    co_located_ref_idx = Co_located->ref_idx;
    co_located_ref_id = Co_located->ref_pic_id;
  }

  if (bframe && (currMB->mb_type == 8))
  {
    loop_counter[202]++;
    if (img->direct_spatial_mv_pred_flag)
    {
      loop_counter[203]++;
      int imgblock_y = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? (img->block_y - 4) / 2 : img->block_y / 2 : img->block_y;
      int fw_rFrameL;
      int fw_rFrameU;
      int fw_rFrameUL;
      int fw_rFrameUR;
      int bw_rFrameL;
      int bw_rFrameU;
      int bw_rFrameUL;
      int bw_rFrameUR;
      PixelPos mb_left;
      PixelPos mb_up;
      PixelPos mb_upleft;
      PixelPos mb_upright;
      char fw_rFrame;
      char bw_rFrame;
      short pmvfw[2] = {0, 0};
      short pmvbw[2] = {0, 0};
      getLuma4x4Neighbour(img->current_mb_nr, 0, 0, -1, 0, &mb_left);
      getLuma4x4Neighbour(img->current_mb_nr, 0, 0, 0, -1, &mb_up);
      getLuma4x4Neighbour(img->current_mb_nr, 0, 0, 16, -1, &mb_upright);
      getLuma4x4Neighbour(img->current_mb_nr, 0, 0, -1, -1, &mb_upleft);
      if (!img->MbaffFrameFlag)
      {
        loop_counter[204]++;
        fw_rFrameL = mb_left.available ? dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] : -1;
        fw_rFrameU = mb_up.available ? dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : -1;
        fw_rFrameUL = mb_upleft.available ? dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
        fw_rFrameUR = mb_upright.available ? dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : fw_rFrameUL;
        bw_rFrameL = mb_left.available ? dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : -1;
        bw_rFrameU = mb_up.available ? dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : -1;
        bw_rFrameUL = mb_upleft.available ? dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
        bw_rFrameUR = mb_upright.available ? dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : bw_rFrameUL;
      }
      else
      {
        if (img->mb_data[img->current_mb_nr].mb_field)
        {
          loop_counter[205]++;
          fw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] : dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] * 2 : -1;
          fw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] * 2 : -1;
          fw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] * 2 : -1;
          fw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] * 2 : fw_rFrameUL;
          bw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] * 2 : -1;
          bw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] * 2 : -1;
          bw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] * 2 : -1;
          bw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] * 2 : bw_rFrameUL;
        }
        else
        {
          fw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] : -1;
          fw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : -1;
          fw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
          fw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : fw_rFrameUL;
          bw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : -1;
          bw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : -1;
          bw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
          bw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : bw_rFrameUL;
        }

      }

      fw_rFrame = (fw_rFrameL >= 0) && (fw_rFrameU >= 0) ? fw_rFrameL < fw_rFrameU ? fw_rFrameL : fw_rFrameU : fw_rFrameL > fw_rFrameU ? fw_rFrameL : fw_rFrameU;
      fw_rFrame = (fw_rFrame >= 0) && (fw_rFrameUR >= 0) ? fw_rFrame < fw_rFrameUR ? fw_rFrame : fw_rFrameUR : fw_rFrame > fw_rFrameUR ? fw_rFrame : fw_rFrameUR;
      bw_rFrame = (bw_rFrameL >= 0) && (bw_rFrameU >= 0) ? bw_rFrameL < bw_rFrameU ? bw_rFrameL : bw_rFrameU : bw_rFrameL > bw_rFrameU ? bw_rFrameL : bw_rFrameU;
      bw_rFrame = (bw_rFrame >= 0) && (bw_rFrameUR >= 0) ? bw_rFrame < bw_rFrameUR ? bw_rFrame : bw_rFrameUR : bw_rFrame > bw_rFrameUR ? bw_rFrame : bw_rFrameUR;
      if (fw_rFrame >= 0)
      {
        loop_counter[206]++;
        SetMotionVectorPredictor(img, pmvfw, pmvfw + 1, fw_rFrame, LIST_0, dec_picture->ref_idx, dec_picture->mv, 0, 0, 16, 16);
      }

      if (bw_rFrame >= 0)
      {
        loop_counter[207]++;
        SetMotionVectorPredictor(img, pmvbw, pmvbw + 1, bw_rFrame, LIST_1, dec_picture->ref_idx, dec_picture->mv, 0, 0, 16, 16);
      }

      for (i = 0; i < 4; i++)
      {
        loop_counter[208]++;
        if (currMB->b8mode[i] == 0)
        {
          loop_counter[209]++;
          for (j = 2 * (i / 2); j < ((2 * (i / 2)) + 2); j++)
          {
            loop_counter[210]++;
            for (k = 2 * (i % 2); k < ((2 * (i % 2)) + 2); k++)
            {
              loop_counter[211]++;
              int j6 = imgblock_y + j;
              j4 = img->block_y + j;
              i4 = img->block_x + k;
              if (fw_rFrame >= 0)
              {
                loop_counter[212]++;
                if ((!fw_rFrame) && ((!moving_block[j6][i4]) && (!listX[1 + list_offset][0]->is_long_term)))
                {
                  loop_counter[213]++;
                  dec_picture->mv[LIST_0][j4][i4][0] = 0;
                  dec_picture->mv[LIST_0][j4][i4][1] = 0;
                  dec_picture->ref_idx[LIST_0][j4][i4] = 0;
                }
                else
                {
                  dec_picture->mv[LIST_0][j4][i4][0] = pmvfw[0];
                  dec_picture->mv[LIST_0][j4][i4][1] = pmvfw[1];
                  dec_picture->ref_idx[LIST_0][j4][i4] = fw_rFrame;
                }

              }
              else
              {
                dec_picture->mv[LIST_0][j4][i4][0] = 0;
                dec_picture->mv[LIST_0][j4][i4][1] = 0;
                dec_picture->ref_idx[LIST_0][j4][i4] = -1;
              }

              if (bw_rFrame >= 0)
              {
                loop_counter[214]++;
                if ((bw_rFrame == 0) && ((!moving_block[j6][i4]) && (!listX[1 + list_offset][0]->is_long_term)))
                {
                  loop_counter[215]++;
                  dec_picture->mv[LIST_1][j4][i4][0] = 0;
                  dec_picture->mv[LIST_1][j4][i4][1] = 0;
                  dec_picture->ref_idx[LIST_1][j4][i4] = 0;
                }
                else
                {
                  dec_picture->mv[LIST_1][j4][i4][0] = pmvbw[0];
                  dec_picture->mv[LIST_1][j4][i4][1] = pmvbw[1];
                  dec_picture->ref_idx[LIST_1][j4][i4] = bw_rFrame;
                }

              }
              else
              {
                dec_picture->mv[LIST_1][j4][i4][0] = 0;
                dec_picture->mv[LIST_1][j4][i4][1] = 0;
                dec_picture->ref_idx[LIST_1][j4][i4] = -1;
              }

              if ((fw_rFrame < 0) && (bw_rFrame < 0))
              {
                loop_counter[216]++;
                dec_picture->ref_idx[LIST_0][j4][i4] = 0;
                dec_picture->ref_idx[LIST_1][j4][i4] = 0;
              }

            }

          }

        }

      }

    }
    else
    {
      for (i = 0; i < 4; i++)
      {
        loop_counter[217]++;
        if (currMB->b8mode[i] == 0)
        {
          loop_counter[218]++;
          for (j = 2 * (i / 2); j < ((2 * (i / 2)) + 2); j++)
          {
            loop_counter[219]++;
            for (k = 2 * (i % 2); k < ((2 * (i % 2)) + 2); k++)
            {
              loop_counter[220]++;
              int list_offset = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? 4 : 2 : 0;
              int imgblock_y = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? (img->block_y - 4) / 2 : img->block_y / 2 : img->block_y;
              int refList = co_located_ref_idx[LIST_0][imgblock_y + j][img->block_x + k] == (-1) ? LIST_1 : LIST_0;
              int ref_idx = co_located_ref_idx[refList][imgblock_y + j][img->block_x + k];
              int mapped_idx = -1;
              int iref;
              if (ref_idx == (-1))
              {
                loop_counter[221]++;
                dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + k] = 0;
                dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + k] = 0;
              }
              else
              {
                for (iref = 0; iref < (img->num_ref_idx_l0_active < listXsize[LIST_0 + list_offset] ? img->num_ref_idx_l0_active : listXsize[LIST_0 + list_offset]); iref++)
                {
                  loop_counter[222]++;
                  if (dec_picture->ref_pic_num[img->current_slice_nr][LIST_0 + list_offset][iref] == co_located_ref_id[refList][imgblock_y + j][img->block_x + k])
                  {
                    loop_counter[223]++;
                    mapped_idx = iref;
                    break;
                  }
                  else
                    mapped_idx = -135792468;

                }

                if ((-135792468) == mapped_idx)
                {
                  loop_counter[224]++;
                  error("temporal direct error\ncolocated block has ref that is unavailable", -1111);
                }

                dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + k] = mapped_idx;
                dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + k] = 0;
              }

            }

          }

        }

      }

    }

  }

  if (img->num_ref_idx_l0_active > 1)
  {
    loop_counter[225]++;
    flag_mode = img->num_ref_idx_l0_active == 2 ? 1 : 0;
    currSE.type = 3;
    dP = &currSlice->partArr[partMap[3]];
    if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
    {
      loop_counter[226]++;
      currSE.mapping = linfo_ue;
    }
    else
      currSE.reading = readRefFrame_CABAC;

    for (j0 = 0; j0 < 4; j0 += step_v0)
    {
      loop_counter[227]++;
      for (i0 = 0; i0 < 4; i0 += step_h0)
      {
        loop_counter[228]++;
        k = (2 * (j0 / 2)) + (i0 / 2);
        if (((currMB->b8pdir[k] == 0) || (currMB->b8pdir[k] == 2)) && (currMB->b8mode[k] != 0))
        {
          loop_counter[229]++;
          ;
          img->subblock_x = i0;
          img->subblock_y = j0;
          if (((!(currMB->mb_type == 8)) || bframe) || ((!bframe) && (!img->allrefzero)))
          {
            loop_counter[230]++;
            currSE.context = BType2CtxRef(currMB->b8mode[k]);
            if (((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag) && flag_mode)
            {
              loop_counter[231]++;
              currSE.len = 1;
              readSyntaxElement_FLC(&currSE, dP->bitstream);
              currSE.value1 = 1 - currSE.value1;
            }
            else
            {
              currSE.value2 = LIST_0;
              dP->readSyntaxElement(&currSE, img, inp, dP);
            }

            refframe = currSE.value1;
          }
          else
          {
            refframe = 0;
          }

          for (j = j0; j < (j0 + step_v0); j++)
          {
            loop_counter[232]++;
            for (i = i0; i < (i0 + step_h0); i++)
            {
              loop_counter[233]++;
              dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + i] = refframe;
            }

          }

        }

      }

    }

  }
  else
  {
    for (j0 = 0; j0 < 4; j0 += step_v0)
    {
      loop_counter[234]++;
      for (i0 = 0; i0 < 4; i0 += step_h0)
      {
        loop_counter[235]++;
        k = (2 * (j0 / 2)) + (i0 / 2);
        if (((currMB->b8pdir[k] == 0) || (currMB->b8pdir[k] == 2)) && (currMB->b8mode[k] != 0))
        {
          loop_counter[236]++;
          for (j = j0; j < (j0 + step_v0); j++)
          {
            loop_counter[237]++;
            for (i = i0; i < (i0 + step_h0); i++)
            {
              loop_counter[238]++;
              dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + i] = 0;
            }

          }

        }

      }

    }

  }

  if (img->num_ref_idx_l1_active > 1)
  {
    loop_counter[239]++;
    flag_mode = img->num_ref_idx_l1_active == 2 ? 1 : 0;
    currSE.type = 3;
    dP = &currSlice->partArr[partMap[3]];
    if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
    {
      loop_counter[240]++;
      currSE.mapping = linfo_ue;
    }
    else
      currSE.reading = readRefFrame_CABAC;

    for (j0 = 0; j0 < 4; j0 += step_v0)
    {
      loop_counter[241]++;
      for (i0 = 0; i0 < 4; i0 += step_h0)
      {
        loop_counter[242]++;
        k = (2 * (j0 / 2)) + (i0 / 2);
        if (((currMB->b8pdir[k] == 1) || (currMB->b8pdir[k] == 2)) && (currMB->b8mode[k] != 0))
        {
          loop_counter[243]++;
          ;
          img->subblock_x = i0;
          img->subblock_y = j0;
          currSE.context = BType2CtxRef(currMB->b8mode[k]);
          if (((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag) && flag_mode)
          {
            loop_counter[244]++;
            currSE.len = 1;
            readSyntaxElement_FLC(&currSE, dP->bitstream);
            currSE.value1 = 1 - currSE.value1;
          }
          else
          {
            currSE.value2 = LIST_1;
            dP->readSyntaxElement(&currSE, img, inp, dP);
          }

          refframe = currSE.value1;
          for (j = j0; j < (j0 + step_v0); j++)
          {
            loop_counter[245]++;
            for (i = i0; i < (i0 + step_h0); i++)
            {
              loop_counter[246]++;
              dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + i] = refframe;
            }

          }

        }

      }

    }

  }
  else
  {
    for (j0 = 0; j0 < 4; j0 += step_v0)
    {
      loop_counter[247]++;
      for (i0 = 0; i0 < 4; i0 += step_h0)
      {
        loop_counter[248]++;
        k = (2 * (j0 / 2)) + (i0 / 2);
        if (((currMB->b8pdir[k] == 1) || (currMB->b8pdir[k] == 2)) && (currMB->b8mode[k] != 0))
        {
          loop_counter[249]++;
          for (j = j0; j < (j0 + step_v0); j++)
          {
            loop_counter[250]++;
            for (i = i0; i < (i0 + step_h0); i++)
            {
              loop_counter[251]++;
              dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + i] = 0;
            }

          }

        }

      }

    }

  }

  currSE.type = 5;
  dP = &currSlice->partArr[partMap[5]];
  if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
  {
    loop_counter[252]++;
    currSE.mapping = linfo_se;
  }
  else
    currSE.reading = readMVD_CABAC;

  for (j0 = 0; j0 < 4; j0 += step_v0)
  {
    loop_counter[253]++;
    for (i0 = 0; i0 < 4; i0 += step_h0)
    {
      loop_counter[254]++;
      k = (2 * (j0 / 2)) + (i0 / 2);
      if (((currMB->b8pdir[k] == 0) || (currMB->b8pdir[k] == 2)) && (currMB->b8mode[k] != 0))
      {
        loop_counter[255]++;
        mv_mode = currMB->b8mode[k];
        step_h = BLOCK_STEP[mv_mode][0];
        step_v = BLOCK_STEP[mv_mode][1];
        refframe = dec_picture->ref_idx[LIST_0][img->block_y + j0][img->block_x + i0];
        for (j = j0; j < (j0 + step_v0); j += step_v)
        {
          loop_counter[256]++;
          for (i = i0; i < (i0 + step_h0); i += step_h)
          {
            loop_counter[257]++;
            j4 = img->block_y + j;
            i4 = img->block_x + i;
            SetMotionVectorPredictor(img, pmv, pmv + 1, refframe, LIST_0, dec_picture->ref_idx, dec_picture->mv, i, j, 4 * step_h, 4 * step_v);
            for (k = 0; k < 2; k++)
            {
              loop_counter[258]++;
              ;
              img->subblock_x = i;
              img->subblock_y = j;
              currSE.value2 = k << 1;
              dP->readSyntaxElement(&currSE, img, inp, dP);
              curr_mvd = currSE.value1;
              vec = curr_mvd + pmv[k];
              for (ii = 0; ii < step_h; ii++)
              {
                loop_counter[259]++;
                for (jj = 0; jj < step_v; jj++)
                {
                  loop_counter[260]++;
                  dec_picture->mv[LIST_0][j4 + jj][i4 + ii][k] = vec;
                  currMB->mvd[LIST_0][j + jj][i + ii][k] = curr_mvd;
                }

              }

            }

          }

        }

      }
      else
        if (currMB->b8mode[k = (2 * (j0 / 2)) + (i0 / 2)] == 0)
      {
        loop_counter[261]++;
        if (!img->direct_spatial_mv_pred_flag)
        {
          loop_counter[262]++;
          int list_offset = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? 4 : 2 : 0;
          int imgblock_y = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? (img->block_y - 4) / 2 : img->block_y / 2 : img->block_y;
          int refList = co_located_ref_idx[LIST_0][imgblock_y + j0][img->block_x + i0] == (-1) ? LIST_1 : LIST_0;
          int ref_idx = co_located_ref_idx[refList][imgblock_y + j0][img->block_x + i0];
          if (ref_idx == (-1))
          {
            loop_counter[263]++;
            for (j = j0; j < (j0 + step_v0); j++)
            {
              loop_counter[264]++;
              for (i = i0; i < (i0 + step_h0); i++)
              {
                loop_counter[265]++;
                dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + i] = 0;
                dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + i] = 0;
                j4 = img->block_y + j;
                i4 = img->block_x + i;
                for (ii = 0; ii < 2; ii++)
                {
                  loop_counter[266]++;
                  dec_picture->mv[LIST_0][j4][i4][ii] = 0;
                  dec_picture->mv[LIST_1][j4][i4][ii] = 0;
                }

              }

            }

          }
          else
          {
            int mapped_idx = -1;
            int iref;
            int j6;
            for (iref = 0; iref < (img->num_ref_idx_l0_active < listXsize[LIST_0 + list_offset] ? img->num_ref_idx_l0_active : listXsize[LIST_0 + list_offset]); iref++)
            {
              loop_counter[267]++;
              if (dec_picture->ref_pic_num[img->current_slice_nr][LIST_0 + list_offset][iref] == co_located_ref_id[refList][imgblock_y + j0][img->block_x + i0])
              {
                loop_counter[268]++;
                mapped_idx = iref;
                break;
              }
              else
                mapped_idx = -135792468;

            }

            if ((-135792468) == mapped_idx)
            {
              loop_counter[269]++;
              error("temporal direct error\ncolocated block has ref that is unavailable", -1111);
            }

            for (j = j0; j < (j0 + step_v0); j++)
            {
              loop_counter[270]++;
              for (i = i0; i < (i0 + step_h0); i++)
              {
                loop_counter[271]++;
                {
                  mv_scale = img->mvscale[LIST_0 + list_offset][mapped_idx];
                  dec_picture->ref_idx[LIST_0][img->block_y + j][img->block_x + i] = mapped_idx;
                  dec_picture->ref_idx[LIST_1][img->block_y + j][img->block_x + i] = 0;
                  j4 = img->block_y + j;
                  j6 = imgblock_y + j;
                  i4 = img->block_x + i;
                  for (ii = 0; ii < 2; ii++)
                  {
                    loop_counter[272]++;
                    if ((mv_scale == 9999) || listX[LIST_0 + list_offset][mapped_idx]->is_long_term)
                    {
                      loop_counter[273]++;
                      dec_picture->mv[LIST_0][j4][i4][ii] = co_located_mv[refList][j6][i4][ii];
                      dec_picture->mv[LIST_1][j4][i4][ii] = 0;
                    }
                    else
                    {
                      dec_picture->mv[LIST_0][j4][i4][ii] = ((mv_scale * co_located_mv[refList][j6][i4][ii]) + 128) >> 8;
                      dec_picture->mv[LIST_1][j4][i4][ii] = dec_picture->mv[LIST_0][j4][i4][ii] - co_located_mv[refList][j6][i4][ii];
                    }

                  }

                }
              }

            }

          }

        }

      }


    }

  }

  currSE.type = 5;
  dP = &currSlice->partArr[partMap[5]];
  if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
  {
    loop_counter[274]++;
    currSE.mapping = linfo_se;
  }
  else
    currSE.reading = readMVD_CABAC;

  for (j0 = 0; j0 < 4; j0 += step_v0)
  {
    loop_counter[275]++;
    for (i0 = 0; i0 < 4; i0 += step_h0)
    {
      loop_counter[276]++;
      k = (2 * (j0 / 2)) + (i0 / 2);
      if (((currMB->b8pdir[k] == 1) || (currMB->b8pdir[k] == 2)) && (currMB->b8mode[k] != 0))
      {
        loop_counter[277]++;
        mv_mode = currMB->b8mode[k];
        step_h = BLOCK_STEP[mv_mode][0];
        step_v = BLOCK_STEP[mv_mode][1];
        refframe = dec_picture->ref_idx[LIST_1][img->block_y + j0][img->block_x + i0];
        for (j = j0; j < (j0 + step_v0); j += step_v)
        {
          loop_counter[278]++;
          for (i = i0; i < (i0 + step_h0); i += step_h)
          {
            loop_counter[279]++;
            j4 = img->block_y + j;
            i4 = img->block_x + i;
            SetMotionVectorPredictor(img, pmv, pmv + 1, refframe, LIST_1, dec_picture->ref_idx, dec_picture->mv, i, j, 4 * step_h, 4 * step_v);
            for (k = 0; k < 2; k++)
            {
              loop_counter[280]++;
              ;
              img->subblock_x = i;
              img->subblock_y = j;
              currSE.value2 = (k << 1) + 1;
              dP->readSyntaxElement(&currSE, img, inp, dP);
              curr_mvd = currSE.value1;
              vec = curr_mvd + pmv[k];
              for (ii = 0; ii < step_h; ii++)
              {
                loop_counter[281]++;
                for (jj = 0; jj < step_v; jj++)
                {
                  loop_counter[282]++;
                  dec_picture->mv[LIST_1][j4 + jj][i4 + ii][k] = vec;
                  currMB->mvd[LIST_1][j + jj][i + ii][k] = curr_mvd;
                }

              }

            }

          }

        }

      }

    }

  }

  for (i4 = img->block_x; i4 < (img->block_x + 4); i4++)
  {
    loop_counter[283]++;
    for (j4 = img->block_y; j4 < (img->block_y + 4); j4++)
    {
      loop_counter[284]++;
      if (dec_picture->ref_idx[LIST_0][j4][i4] >= 0)
      {
        loop_counter[285]++;
        dec_picture->ref_pic_id[LIST_0][j4][i4] = dec_picture->ref_pic_num[img->current_slice_nr][LIST_0 + list_offset][(short) dec_picture->ref_idx[LIST_0][j4][i4]];
      }
      else
        dec_picture->ref_pic_id[LIST_0][j4][i4] = (-9223372036854775807LL) - 1LL;

      if (dec_picture->ref_idx[LIST_1][j4][i4] >= 0)
      {
        loop_counter[286]++;
        dec_picture->ref_pic_id[LIST_1][j4][i4] = dec_picture->ref_pic_num[img->current_slice_nr][LIST_1 + list_offset][(short) dec_picture->ref_idx[LIST_1][j4][i4]];
      }
      else
        dec_picture->ref_pic_id[LIST_1][j4][i4] = (-9223372036854775807LL) - 1LL;

    }

  }

}

int predict_nnz(struct img_par *img, int i, int j)
{
  PixelPos pix;
  int pred_nnz = 0;
  int cnt = 0;
  int mb_nr = img->current_mb_nr;
  getLuma4x4Neighbour(mb_nr, i, j, -1, 0, &pix);
  if ((pix.available && active_pps->constrained_intra_pred_flag) && (img->currentSlice->dp_mode == PAR_DP_3))
  {
    loop_counter[287]++;
    pix.available &= img->intra_block[pix.mb_addr];
  }

  if (pix.available)
  {
    loop_counter[288]++;
    pred_nnz = img->nz_coeff[pix.mb_addr][pix.x][pix.y];
    cnt++;
  }

  getLuma4x4Neighbour(mb_nr, i, j, 0, -1, &pix);
  if ((pix.available && active_pps->constrained_intra_pred_flag) && (img->currentSlice->dp_mode == PAR_DP_3))
  {
    loop_counter[289]++;
    pix.available &= img->intra_block[pix.mb_addr];
  }

  if (pix.available)
  {
    loop_counter[290]++;
    pred_nnz += img->nz_coeff[pix.mb_addr][pix.x][pix.y];
    cnt++;
  }

  if (cnt == 2)
  {
    loop_counter[291]++;
    pred_nnz++;
    pred_nnz /= cnt;
  }

  return pred_nnz;
}

int predict_nnz_chroma(struct img_par *img, int i, int j)
{
  PixelPos pix;
  int pred_nnz = 0;
  int cnt = 0;
  int mb_nr = img->current_mb_nr;
  int j_off_tab[12] = {0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8};
  int j_off = j_off_tab[j];
  if (dec_picture->chroma_format_idc != 3)
  {
    loop_counter[292]++;
    getChroma4x4Neighbour(mb_nr, i % 2, j - 4, -1, 0, &pix);
    if ((pix.available && active_pps->constrained_intra_pred_flag) && (img->currentSlice->dp_mode == PAR_DP_3))
    {
      loop_counter[293]++;
      pix.available &= img->intra_block[pix.mb_addr];
    }

    if (pix.available)
    {
      loop_counter[294]++;
      pred_nnz = img->nz_coeff[pix.mb_addr][(2 * (i / 2)) + pix.x][4 + pix.y];
      cnt++;
    }

    getChroma4x4Neighbour(mb_nr, i % 2, j - 4, 0, -1, &pix);
    if ((pix.available && active_pps->constrained_intra_pred_flag) && (img->currentSlice->dp_mode == PAR_DP_3))
    {
      loop_counter[295]++;
      pix.available &= img->intra_block[pix.mb_addr];
    }

    if (pix.available)
    {
      loop_counter[296]++;
      pred_nnz += img->nz_coeff[pix.mb_addr][(2 * (i / 2)) + pix.x][4 + pix.y];
      cnt++;
    }

  }
  else
  {
    getChroma4x4Neighbour(mb_nr, i, j - j_off, -1, 0, &pix);
    if ((pix.available && active_pps->constrained_intra_pred_flag) && (img->currentSlice->dp_mode == PAR_DP_3))
    {
      loop_counter[297]++;
      pix.available &= img->intra_block[pix.mb_addr];
    }

    if (pix.available)
    {
      loop_counter[298]++;
      pred_nnz = img->nz_coeff[pix.mb_addr][pix.x][j_off + pix.y];
      cnt++;
    }

    getChroma4x4Neighbour(mb_nr, i, j - j_off, 0, -1, &pix);
    if ((pix.available && active_pps->constrained_intra_pred_flag) && (img->currentSlice->dp_mode == PAR_DP_3))
    {
      loop_counter[299]++;
      pix.available &= img->intra_block[pix.mb_addr];
    }

    if (pix.available)
    {
      loop_counter[300]++;
      pred_nnz += img->nz_coeff[pix.mb_addr][pix.x][j_off + pix.y];
      cnt++;
    }

  }

  if (cnt == 2)
  {
    loop_counter[301]++;
    pred_nnz++;
    pred_nnz /= cnt;
  }

  return pred_nnz;
}

void readCoeff4x4_CAVLC(struct img_par *img, struct inp_par *inp, int block_type, int i, int j, int levarr[16], int runarr[16], int *number_coefficients)
{
  int mb_nr = img->current_mb_nr;
  Macroblock *currMB = &img->mb_data[mb_nr];
  SyntaxElement currSE;
  Slice *currSlice = img->currentSlice;
  DataPartition *dP;
  int *partMap = assignSE2partition[currSlice->dp_mode];
  int k;
  int code;
  int vlcnum;
  int numcoeff;
  int numtrailingones;
  int numcoeff_vlc;
  int level_two_or_higher;
  int numones;
  int totzeros;
  int level;
  int cdc = 0;
  int cac = 0;
  int zerosleft;
  int ntr;
  int dptype = 0;
  int max_coeff_num = 0;
  int nnz;
  char type[15];
  int incVlc[] = {0, 3, 6, 12, 24, 48, 32768};
  numcoeff = 0;
  switch (block_type)
  {
    case 0:
      loop_counter[302]++;
      max_coeff_num = 16;
      sprintf(type, "%s", "Luma");
      if (((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12))
    {
      loop_counter[303]++;
      dptype = 9;
    }
    else
    {
      dptype = 14;
    }

      break;

    case 1:
      loop_counter[304]++;
      max_coeff_num = 16;
      sprintf(type, "%s", "Lum16DC");
      dptype = 7;
      break;

    case 2:
      loop_counter[305]++;
      max_coeff_num = 15;
      sprintf(type, "%s", "Lum16AC");
      dptype = 9;
      break;

    case 6:
      loop_counter[306]++;
      max_coeff_num = img->num_cdc_coeff;
      cdc = 1;
      sprintf(type, "%s", "ChrDC");
      if (((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12))
    {
      loop_counter[307]++;
      dptype = 8;
    }
    else
    {
      dptype = 13;
    }

      break;

    case 7:
      loop_counter[308]++;
      max_coeff_num = 15;
      cac = 1;
      sprintf(type, "%s", "ChrAC");
      if (((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12))
    {
      loop_counter[309]++;
      dptype = 10;
    }
    else
    {
      dptype = 15;
    }

      break;

    default:
      error("readCoeff4x4_CAVLC: invalid block type", 600);
      break;

  }

  currSE.type = dptype;
  dP = &currSlice->partArr[partMap[dptype]];
  img->nz_coeff[img->current_mb_nr][i][j] = 0;
  if (!cdc)
  {
    loop_counter[310]++;
    if (!cac)
    {
      loop_counter[311]++;
      nnz = predict_nnz(img, i, j);
    }
    else
    {
      nnz = predict_nnz_chroma(img, i, j);
    }

    if (nnz < 2)
    {
      loop_counter[312]++;
      numcoeff_vlc = 0;
    }
    else
      if (nnz < 4)
    {
      loop_counter[313]++;
      numcoeff_vlc = 1;
    }
    else
      if (nnz < 8)
    {
      loop_counter[314]++;
      numcoeff_vlc = 2;
    }
    else
    {
      numcoeff_vlc = 3;
    }



    currSE.value1 = numcoeff_vlc;
    readSyntaxElement_NumCoeffTrailingOnes(&currSE, dP, type);
    numcoeff = currSE.value1;
    numtrailingones = currSE.value2;
    img->nz_coeff[img->current_mb_nr][i][j] = numcoeff;
  }
  else
  {
    readSyntaxElement_NumCoeffTrailingOnesChromaDC(&currSE, dP);
    numcoeff = currSE.value1;
    numtrailingones = currSE.value2;
  }

  for (k = 0; k < max_coeff_num; k++)
  {
    loop_counter[315]++;
    levarr[k] = 0;
    runarr[k] = 0;
  }

  numones = numtrailingones;
  *number_coefficients = numcoeff;
  if (numcoeff)
  {
    loop_counter[316]++;
    if (numtrailingones)
    {
      loop_counter[317]++;
      currSE.len = numtrailingones;
      readSyntaxElement_FLC(&currSE, dP->bitstream);
      code = currSE.inf;
      ntr = numtrailingones;
      for (k = numcoeff - 1; k > ((numcoeff - 1) - numtrailingones); k--)
      {
        loop_counter[318]++;
        ntr--;
        if ((code >> ntr) & 1)
        {
          loop_counter[319]++;
          levarr[k] = -1;
        }
        else
          levarr[k] = 1;

      }

    }

    level_two_or_higher = 1;
    if ((numcoeff > 3) && (numtrailingones == 3))
    {
      loop_counter[320]++;
      level_two_or_higher = 0;
    }

    if ((numcoeff > 10) && (numtrailingones < 3))
    {
      loop_counter[321]++;
      vlcnum = 1;
    }
    else
      vlcnum = 0;

    for (k = (numcoeff - 1) - numtrailingones; k >= 0; k--)
    {
      loop_counter[322]++;
      if (vlcnum == 0)
      {
        loop_counter[323]++;
        readSyntaxElement_Level_VLC0(&currSE, dP);
      }
      else
        readSyntaxElement_Level_VLCN(&currSE, vlcnum, dP);

      if (level_two_or_higher)
      {
        loop_counter[324]++;
        if (currSE.inf > 0)
        {
          loop_counter[325]++;
          currSE.inf++;
        }
        else
          currSE.inf--;

        level_two_or_higher = 0;
      }

      level = (levarr[k] = currSE.inf);
      if (abs(level) == 1)
      {
        loop_counter[326]++;
        numones++;
      }

      if (abs(level) > incVlc[vlcnum])
      {
        loop_counter[327]++;
        vlcnum++;
      }

      if ((k == ((numcoeff - 1) - numtrailingones)) && (abs(level) > 3))
      {
        loop_counter[328]++;
        vlcnum = 2;
      }

    }

    if (numcoeff < max_coeff_num)
    {
      loop_counter[329]++;
      vlcnum = numcoeff - 1;
      currSE.value1 = vlcnum;
      if (cdc)
      {
        loop_counter[330]++;
        readSyntaxElement_TotalZerosChromaDC(&currSE, dP);
      }
      else
        readSyntaxElement_TotalZeros(&currSE, dP);

      totzeros = currSE.value1;
    }
    else
    {
      totzeros = 0;
    }

    zerosleft = totzeros;
    i = numcoeff - 1;
    if ((zerosleft > 0) && (i > 0))
    {
      loop_counter[331]++;
      do
      {
        vlcnum = zerosleft - 1;
        if (vlcnum > (7 - 1))
        {
          loop_counter[332]++;
          vlcnum = 7 - 1;
        }

        currSE.value1 = vlcnum;
        readSyntaxElement_Run(&currSE, dP);
        runarr[i] = currSE.value1;
        zerosleft -= runarr[i];
        i--;
      }
      while ((zerosleft != 0) && (i != 0));
    }

    runarr[i] = zerosleft;
  }

}

void CalculateQuant8Param()
{
  int i;
  int j;
  int k;
  int temp;
  for (k = 0; k < 6; k++)
  {
    loop_counter[333]++;
    for (j = 0; j < 8; j++)
    {
      loop_counter[334]++;
      for (i = 0; i < 8; i++)
      {
        loop_counter[335]++;
        temp = (i << 3) + j;
        InvLevelScale8x8Luma_Intra[k][j][i] = dequant_coef8[k][j][i] * qmatrix[6][temp];
        InvLevelScale8x8Luma_Inter[k][j][i] = dequant_coef8[k][j][i] * qmatrix[7][temp];
      }

    }

  }

}

void readLumaCoeff8x8_CABAC(struct img_par *img, struct inp_par *inp, int b8)
{
  int i;
  int j;
  int k;
  int level;
  int mb_nr = img->current_mb_nr;
  Macroblock *currMB = &img->mb_data[mb_nr];
  int cbp = currMB->cbp;
  SyntaxElement currSE;
  Slice *currSlice = img->currentSlice;
  DataPartition *dP;
  int *partMap = assignSE2partition[currSlice->dp_mode];
  int coef_ctr;
  int start_scan;
  int boff_x;
  int boff_y;
  int any_coeff;
  int dq_lshift = 0;
  int dq_rshift = 0;
  int dq_round = 0;
  int run;
  int len;
  int qp_per = ((img->qp + img->bitdepth_luma_qp_scale) - 0) / 6;
  int qp_rem = ((img->qp + img->bitdepth_luma_qp_scale) - 0) % 6;
  Boolean lossless_qpprime = ((img->qp + img->bitdepth_luma_qp_scale) == 0) && (img->lossless_qpprime_flag == 1);
  if (qp_per < 6)
  {
    loop_counter[336]++;
    dq_rshift = 6 - qp_per;
    dq_round = 1 << (5 - qp_per);
  }
  else
    dq_lshift = qp_per - 6;

  img->is_intra_block = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
  if (cbp & (1 << b8))
  {
    loop_counter[337]++;
    boff_x = (b8 % 2) << 3;
    boff_y = (b8 / 2) << 3;
    img->subblock_x = boff_x >> 2;
    img->subblock_y = boff_y >> 2;
    start_scan = 0;
    coef_ctr = start_scan - 1;
    level = 1;
    for (k = start_scan; (k < 65) && (level != 0); k++)
    {
      loop_counter[338]++;
      currSE.context = 2;
      currSE.type = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12) ? k == 0 ? 7 : 9 : k == 0 ? 12 : 14;
      dP = &currSlice->partArr[partMap[currSE.type]];
      currSE.reading = readRunLevel_CABAC;
      dP->readSyntaxElement(&currSE, img, inp, dP);
      level = currSE.value1;
      run = currSE.value2;
      len = currSE.len;
      if (level != 0)
      {
        loop_counter[339]++;
        any_coeff = 1;
        coef_ctr += run + 1;
        if ((img->structure == FRAME) && (!currMB->mb_field))
        {
          loop_counter[340]++;
          i = SNGL_SCAN8x8[coef_ctr][0];
          j = SNGL_SCAN8x8[coef_ctr][1];
        }
        else
        {
          i = FIELD_SCAN8x8[coef_ctr][0];
          j = FIELD_SCAN8x8[coef_ctr][1];
        }

        currMB->cbp_blk |= 51 << ((4 * b8) - (2 * (b8 % 2)));
        if (lossless_qpprime)
        {
          loop_counter[341]++;
          img->m7[boff_x + i][boff_y + j] = level;
        }
        else
          if (qp_per >= 6)
        {
          loop_counter[342]++;
          if (img->is_intra_block == 1)
          {
            loop_counter[343]++;
            img->m7[boff_x + i][boff_y + j] = (level * InvLevelScale8x8Luma_Intra[qp_rem][i][j]) << dq_lshift;
          }
          else
            img->m7[boff_x + i][boff_y + j] = (level * InvLevelScale8x8Luma_Inter[qp_rem][i][j]) << dq_lshift;

        }
        else
        {
          if (img->is_intra_block == 1)
          {
            loop_counter[344]++;
            img->m7[boff_x + i][boff_y + j] = ((level * InvLevelScale8x8Luma_Intra[qp_rem][i][j]) + dq_round) >> dq_rshift;
          }
          else
            img->m7[boff_x + i][boff_y + j] = ((level * InvLevelScale8x8Luma_Inter[qp_rem][i][j]) + dq_round) >> dq_rshift;

        }


      }

    }

  }

}

void readCBPandCoeffsFromNAL(struct img_par *img, struct inp_par *inp)
{
  int i;
  int j;
  int k;
  int level;
  int mb_nr = img->current_mb_nr;
  int ii;
  int jj;
  int m2;
  int jg2;
  Macroblock *currMB = &img->mb_data[mb_nr];
  int cbp;
  SyntaxElement currSE;
  Slice *currSlice = img->currentSlice;
  DataPartition *dP;
  int *partMap = assignSE2partition[currSlice->dp_mode];
  int iii;
  int jjj;
  int coef_ctr;
  int i0;
  int j0;
  int b8;
  int ll;
  int block_x;
  int block_y;
  int start_scan;
  int run;
  int len;
  int levarr[16];
  int runarr[16];
  int numcoeff;
  int qp_const;
  int qp_per = ((img->qp + img->bitdepth_luma_qp_scale) - 0) / 6;
  int qp_rem = ((img->qp + img->bitdepth_luma_qp_scale) - 0) % 6;
  int smb = ((img->type == SP_SLICE) && ((((currMB->mb_type != 9) && (currMB->mb_type != 10)) && (currMB->mb_type != 13)) && (currMB->mb_type != 14))) || ((img->type == SI_SLICE) && (currMB->mb_type == 12));
  int uv;
  int qp_uv[2];
  int qp_const_uv[2];
  int qp_per_uv[2];
  int qp_rem_uv[2];
  int qp_c[2];
  int intra = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
  int temp[4];
  int b4;
  int yuv = dec_picture->chroma_format_idc - 1;
  int m5[4];
  int m6[4];
  int need_transform_size_flag;
  Boolean lossless_qpprime = ((img->qp + img->bitdepth_luma_qp_scale) == 0) && (img->lossless_qpprime_flag == 1);
  Boolean residual_transform_dc = (img->residue_transform_flag == 1) && ((currMB->mb_type == 9) || (currMB->mb_type == 13));
  if (dec_picture->chroma_format_idc != 0)
  {
    loop_counter[345]++;
    for (i = 0; i < 2; i++)
    {
      loop_counter[346]++;
      qp_uv[i] = img->qp + dec_picture->chroma_qp_offset[i];
      qp_uv[i] = qp_uv[i] < (-img->bitdepth_chroma_qp_scale) ? -img->bitdepth_chroma_qp_scale : qp_uv[i] > 51 ? 51 : qp_uv[i];
      qp_c[i] = qp_uv[i] < 0 ? qp_uv[i] : QP_SCALE_CR[qp_uv[i] - 0];
      qp_per_uv[i] = (qp_c[i] + img->bitdepth_chroma_qp_scale) / 6;
      qp_rem_uv[i] = (qp_c[i] + img->bitdepth_chroma_qp_scale) % 6;
    }

  }

  if (!((currMB->mb_type == 10) || (currMB->mb_type == 14)))
  {
    loop_counter[347]++;
    if (((currMB->mb_type == 9) || (currMB->mb_type == 12)) || (currMB->mb_type == 13))
    {
      loop_counter[348]++;
      currSE.type = 6;
    }
    else
      currSE.type = 11;

    dP = &currSlice->partArr[partMap[currSE.type]];
    if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
    {
      loop_counter[349]++;
      if (((currMB->mb_type == 9) || (currMB->mb_type == 12)) || (currMB->mb_type == 13))
      {
        loop_counter[350]++;
        currSE.mapping = linfo_cbp_intra;
      }
      else
        currSE.mapping = linfo_cbp_inter;

    }
    else
    {
      currSE.reading = readCBP_CABAC;
    }

    ;
    dP->readSyntaxElement(&currSE, img, inp, dP);
    currMB->cbp = (cbp = currSE.value1);
    need_transform_size_flag = (((((((currMB->mb_type >= 1) && (currMB->mb_type <= 3)) || (((currMB->mb_type == 0) && (img->type == B_SLICE)) && active_sps->direct_8x8_inference_flag)) || currMB->NoMbPartLessThan8x8Flag) && (currMB->mb_type != 13)) && (currMB->mb_type != 9)) && (currMB->cbp & 15)) && img->Transform8x8Mode;
    if (need_transform_size_flag)
    {
      loop_counter[351]++;
      currSE.type = SE_TRANSFORM_SIZE_FLAG;
      dP = &currSlice->partArr[partMap[SE_TRANSFORM_SIZE_FLAG]];
      currSE.reading = readMB_transform_size_flag_CABAC;
      ;
      if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
      {
        loop_counter[352]++;
        currSE.len = 1;
        readSyntaxElement_FLC(&currSE, dP->bitstream);
      }
      else
      {
        dP->readSyntaxElement(&currSE, img, inp, dP);
      }

      currMB->luma_transform_size_8x8_flag = currSE.value1;
    }

    if (cbp != 0)
    {
      loop_counter[353]++;
      if ((((currMB->mb_type != 9) && (currMB->mb_type != 10)) && (currMB->mb_type != 13)) && (currMB->mb_type != 14))
      {
        loop_counter[354]++;
        currSE.type = 16;
      }
      else
        currSE.type = 17;

      dP = &currSlice->partArr[partMap[currSE.type]];
      if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
      {
        loop_counter[355]++;
        currSE.mapping = linfo_se;
      }
      else
        currSE.reading = readDquant_CABAC;

      ;
      dP->readSyntaxElement(&currSE, img, inp, dP);
      currMB->delta_quant = currSE.value1;
      img->qp = ((((img->qp + currMB->delta_quant) + 52) + (2 * img->bitdepth_luma_qp_scale)) % (52 + img->bitdepth_luma_qp_scale)) - img->bitdepth_luma_qp_scale;
    }

  }
  else
  {
    cbp = currMB->cbp;
  }

  for (i = 0; i < 4; i++)
  {
    loop_counter[356]++;
    for (j = 0; j < 4; j++)
    {
      loop_counter[357]++;
      for (iii = 0; iii < 4; iii++)
      {
        loop_counter[358]++;
        for (jjj = 0; jjj < 4; jjj++)
        {
          loop_counter[359]++;
          img->cof[i][j][iii][jjj] = 0;
        }

      }

    }

  }

  if ((currMB->mb_type == 10) || (currMB->mb_type == 14))
  {
    loop_counter[360]++;
    currSE.type = 17;
    dP = &currSlice->partArr[partMap[currSE.type]];
    if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
    {
      loop_counter[361]++;
      currSE.mapping = linfo_se;
    }
    else
    {
      currSE.reading = readDquant_CABAC;
    }

    dP->readSyntaxElement(&currSE, img, inp, dP);
    currMB->delta_quant = currSE.value1;
    img->qp = ((((img->qp + currMB->delta_quant) + 52) + (2 * img->bitdepth_luma_qp_scale)) % (52 + img->bitdepth_luma_qp_scale)) - img->bitdepth_luma_qp_scale;
    for (i = 0; i < 4; i++)
    {
      loop_counter[362]++;
      for (j = 0; j < 4; j++)
      {
        loop_counter[363]++;
        img->ipredmode[img->block_x + i][img->block_y + j] = 2;
      }

    }

    if (active_pps->entropy_coding_mode_flag == UVLC)
    {
      loop_counter[364]++;
      readCoeff4x4_CAVLC(img, inp, 1, 0, 0, levarr, runarr, &numcoeff);
      coef_ctr = -1;
      level = 1;
      for (k = 0; k < numcoeff; k++)
      {
        loop_counter[365]++;
        if (levarr[k] != 0)
        {
          loop_counter[366]++;
          coef_ctr = (coef_ctr + runarr[k]) + 1;
          if ((img->structure == FRAME) && (!currMB->mb_field))
          {
            loop_counter[367]++;
            i0 = SNGL_SCAN[coef_ctr][0];
            j0 = SNGL_SCAN[coef_ctr][1];
          }
          else
          {
            i0 = FIELD_SCAN[coef_ctr][0];
            j0 = FIELD_SCAN[coef_ctr][1];
          }

          img->cof[i0][j0][0][0] = levarr[k];
        }

      }

    }
    else
    {
      currSE.type = 7;
      dP = &currSlice->partArr[partMap[currSE.type]];
      currSE.context = 0;
      currSE.type = 7;
      img->is_intra_block = 1;
      if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
      {
        loop_counter[368]++;
        currSE.mapping = linfo_levrun_inter;
      }
      else
      {
        currSE.reading = readRunLevel_CABAC;
      }

      coef_ctr = -1;
      level = 1;
      for (k = 0; (k < 17) && (level != 0); k++)
      {
        loop_counter[369]++;
        dP->readSyntaxElement(&currSE, img, inp, dP);
        level = currSE.value1;
        run = currSE.value2;
        len = currSE.len;
        if (level != 0)
        {
          loop_counter[370]++;
          coef_ctr = (coef_ctr + run) + 1;
          if ((img->structure == FRAME) && (!currMB->mb_field))
          {
            loop_counter[371]++;
            i0 = SNGL_SCAN[coef_ctr][0];
            j0 = SNGL_SCAN[coef_ctr][1];
          }
          else
          {
            i0 = FIELD_SCAN[coef_ctr][0];
            j0 = FIELD_SCAN[coef_ctr][1];
          }

          img->cof[i0][j0][0][0] = level;
        }

      }

    }

    if (!lossless_qpprime)
    {
      loop_counter[372]++;
      itrans_2(img);
    }

  }

  qp_per = ((img->qp + img->bitdepth_luma_qp_scale) - 0) / 6;
  qp_rem = ((img->qp + img->bitdepth_luma_qp_scale) - 0) % 6;
  qp_const = 1 << (3 - qp_per);
  if (dec_picture->chroma_format_idc != 0)
  {
    loop_counter[373]++;
    for (i = 0; i < 2; i++)
    {
      loop_counter[374]++;
      qp_uv[i] = img->qp + dec_picture->chroma_qp_offset[i];
      qp_uv[i] = qp_uv[i] < (-img->bitdepth_chroma_qp_scale) ? -img->bitdepth_chroma_qp_scale : qp_uv[i] > 51 ? 51 : qp_uv[i];
      qp_c[i] = qp_uv[i] < 0 ? qp_uv[i] : QP_SCALE_CR[qp_uv[i] - 0];
      qp_per_uv[i] = (qp_c[i] + img->bitdepth_chroma_qp_scale) / 6;
      qp_rem_uv[i] = (qp_c[i] + img->bitdepth_chroma_qp_scale) % 6;
    }

  }

  currMB->qp = img->qp;
  for (block_y = 0; block_y < 4; block_y += 2)
  {
    loop_counter[375]++;
    for (block_x = 0; block_x < 4; block_x += 2)
    {
      loop_counter[376]++;
      b8 = (2 * (block_y / 2)) + (block_x / 2);
      if (active_pps->entropy_coding_mode_flag == UVLC)
      {
        loop_counter[377]++;
        for (j = block_y; j < (block_y + 2); j++)
        {
          loop_counter[378]++;
          for (i = block_x; i < (block_x + 2); i++)
          {
            loop_counter[379]++;
            ii = block_x / 2;
            jj = block_y / 2;
            b8 = (2 * jj) + ii;
            if (cbp & (1 << b8))
            {
              loop_counter[380]++;
              if ((currMB->mb_type == 10) || (currMB->mb_type == 14))
              {
                loop_counter[381]++;
                readCoeff4x4_CAVLC(img, inp, 2, i, j, levarr, runarr, &numcoeff);
                start_scan = 1;
              }
              else
              {
                readCoeff4x4_CAVLC(img, inp, 0, i, j, levarr, runarr, &numcoeff);
                start_scan = 0;
              }

              coef_ctr = start_scan - 1;
              for (k = 0; k < numcoeff; k++)
              {
                loop_counter[382]++;
                if (levarr[k] != 0)
                {
                  loop_counter[383]++;
                  coef_ctr += runarr[k] + 1;
                  if ((img->structure == FRAME) && (!currMB->mb_field))
                  {
                    loop_counter[384]++;
                    i0 = SNGL_SCAN[coef_ctr][0];
                    j0 = SNGL_SCAN[coef_ctr][1];
                  }
                  else
                  {
                    i0 = FIELD_SCAN[coef_ctr][0];
                    j0 = FIELD_SCAN[coef_ctr][1];
                  }

                  if (!currMB->luma_transform_size_8x8_flag)
                  {
                    loop_counter[385]++;
                    currMB->cbp_blk |= 1 << ((j << 2) + i);
                    if (lossless_qpprime)
                    {
                      loop_counter[386]++;
                      img->cof[i][j][i0][j0] = levarr[k];
                    }
                    else
                      if (qp_per < 4)
                    {
                      loop_counter[387]++;
                      if (intra == 1)
                      {
                        loop_counter[388]++;
                        img->cof[i][j][i0][j0] = ((levarr[k] * InvLevelScale4x4Luma_Intra[qp_rem][i0][j0]) + qp_const) >> (4 - qp_per);
                      }
                      else
                        img->cof[i][j][i0][j0] = ((levarr[k] * InvLevelScale4x4Luma_Inter[qp_rem][i0][j0]) + qp_const) >> (4 - qp_per);

                    }
                    else
                    {
                      if (intra == 1)
                      {
                        loop_counter[389]++;
                        img->cof[i][j][i0][j0] = (levarr[k] * InvLevelScale4x4Luma_Intra[qp_rem][i0][j0]) << (qp_per - 4);
                      }
                      else
                        img->cof[i][j][i0][j0] = (levarr[k] * InvLevelScale4x4Luma_Inter[qp_rem][i0][j0]) << (qp_per - 4);

                    }


                  }
                  else
                  {
                    int b4;
                    int iz;
                    int jz;
                    int dq_rshift = 0;
                    int dq_round = 0;
                    int dq_lshift = 0;
                    currMB->cbp_blk |= 51 << ((block_y << 2) + block_x);
                    b4 = (2 * (j - block_y)) + (i - block_x);
                    if ((img->structure == FRAME) && (!currMB->mb_field))
                    {
                      loop_counter[390]++;
                      iz = SNGL_SCAN8x8[(coef_ctr * 4) + b4][0];
                      jz = SNGL_SCAN8x8[(coef_ctr * 4) + b4][1];
                    }
                    else
                    {
                      iz = FIELD_SCAN8x8[(coef_ctr * 4) + b4][0];
                      jz = FIELD_SCAN8x8[(coef_ctr * 4) + b4][1];
                    }

                    if (qp_per < 6)
                    {
                      loop_counter[391]++;
                      dq_rshift = 6 - qp_per;
                      dq_round = 1 << (5 - qp_per);
                    }
                    else
                      dq_lshift = qp_per - 6;

                    if (lossless_qpprime)
                    {
                      loop_counter[392]++;
                      img->m7[(block_x * 4) + iz][(block_y * 4) + jz] = levarr[k];
                    }
                    else
                      if (qp_per >= 6)
                    {
                      loop_counter[393]++;
                      if (intra == 1)
                      {
                        loop_counter[394]++;
                        img->m7[(block_x * 4) + iz][(block_y * 4) + jz] = (levarr[k] * InvLevelScale8x8Luma_Intra[qp_rem][iz][jz]) << dq_lshift;
                      }
                      else
                        img->m7[(block_x * 4) + iz][(block_y * 4) + jz] = (levarr[k] * InvLevelScale8x8Luma_Inter[qp_rem][iz][jz]) << dq_lshift;

                    }
                    else
                    {
                      if (intra)
                      {
                        loop_counter[395]++;
                        img->m7[(block_x * 4) + iz][(block_y * 4) + jz] = ((levarr[k] * InvLevelScale8x8Luma_Intra[qp_rem][iz][jz]) + dq_round) >> dq_rshift;
                      }
                      else
                        img->m7[(block_x * 4) + iz][(block_y * 4) + jz] = ((levarr[k] * InvLevelScale8x8Luma_Inter[qp_rem][iz][jz]) + dq_round) >> dq_rshift;

                    }


                  }

                }

              }

            }
            else
            {
              img->nz_coeff[img->current_mb_nr][i][j] = 0;
            }

          }

        }

      }
      else
      {
        if (currMB->luma_transform_size_8x8_flag)
        {
          loop_counter[396]++;
          readLumaCoeff8x8_CABAC(img, inp, b8);
        }
        else
        {
          for (j = block_y; j < (block_y + 2); j++)
          {
            loop_counter[397]++;
            for (i = block_x; i < (block_x + 2); i++)
            {
              loop_counter[398]++;
              if ((currMB->mb_type == 10) || (currMB->mb_type == 14))
              {
                loop_counter[399]++;
                start_scan = 1;
              }
              else
                start_scan = 0;

              img->subblock_x = i;
              img->subblock_y = j;
              if (cbp & (1 << b8))
              {
                loop_counter[400]++;
                coef_ctr = start_scan - 1;
                level = 1;
                for (k = start_scan; (k < 17) && (level != 0); k++)
                {
                  loop_counter[401]++;
                  currSE.context = (currMB->mb_type == 10) || (currMB->mb_type == 14) ? 1 : 5;
                  currSE.type = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12) ? k == 0 ? 7 : 9 : k == 0 ? 12 : 14;
                  img->is_intra_block = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
                  dP = &currSlice->partArr[partMap[currSE.type]];
                  if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
                  {
                    loop_counter[402]++;
                    currSE.mapping = linfo_levrun_inter;
                  }
                  else
                    currSE.reading = readRunLevel_CABAC;

                  dP->readSyntaxElement(&currSE, img, inp, dP);
                  level = currSE.value1;
                  run = currSE.value2;
                  len = currSE.len;
                  if (level != 0)
                  {
                    loop_counter[403]++;
                    coef_ctr += run + 1;
                    if ((img->structure == FRAME) && (!currMB->mb_field))
                    {
                      loop_counter[404]++;
                      i0 = SNGL_SCAN[coef_ctr][0];
                      j0 = SNGL_SCAN[coef_ctr][1];
                    }
                    else
                    {
                      i0 = FIELD_SCAN[coef_ctr][0];
                      j0 = FIELD_SCAN[coef_ctr][1];
                    }

                    currMB->cbp_blk |= 1 << ((j << 2) + i);
                    if (lossless_qpprime)
                    {
                      loop_counter[405]++;
                      img->cof[i][j][i0][j0] = level;
                    }
                    else
                      if (qp_per < 4)
                    {
                      loop_counter[406]++;
                      if (intra == 1)
                      {
                        loop_counter[407]++;
                        img->cof[i][j][i0][j0] = ((level * InvLevelScale4x4Luma_Intra[qp_rem][i0][j0]) + qp_const) >> (4 - qp_per);
                      }
                      else
                        img->cof[i][j][i0][j0] = ((level * InvLevelScale4x4Luma_Inter[qp_rem][i0][j0]) + qp_const) >> (4 - qp_per);

                    }
                    else
                    {
                      if (intra == 1)
                      {
                        loop_counter[408]++;
                        img->cof[i][j][i0][j0] = (level * InvLevelScale4x4Luma_Intra[qp_rem][i0][j0]) << (qp_per - 4);
                      }
                      else
                        img->cof[i][j][i0][j0] = (level * InvLevelScale4x4Luma_Inter[qp_rem][i0][j0]) << (qp_per - 4);

                    }


                  }

                }

              }

            }

          }

        }

      }

    }

  }

  if (dec_picture->chroma_format_idc != 0)
  {
    loop_counter[409]++;
    for (j = 4; j < (4 + img->num_blk8x8_uv); j++)
    {
      loop_counter[410]++;
      for (i = 0; i < 4; i++)
      {
        loop_counter[411]++;
        for (iii = 0; iii < 4; iii++)
        {
          loop_counter[412]++;
          for (jjj = 0; jjj < 4; jjj++)
          {
            loop_counter[413]++;
            img->cof[i][j][iii][jjj] = 0;
          }

        }

      }

    }

    m2 = img->mb_x * 2;
    jg2 = img->mb_y * 2;
    qp_const_uv[0] = 1 << (3 - qp_per_uv[0]);
    qp_const_uv[1] = 1 << (3 - qp_per_uv[1]);
    if (cbp > 15)
    {
      loop_counter[414]++;
      for (ll = 0; ll < 3; ll += 2)
      {
        loop_counter[415]++;
        uv = ll >> 1;
        if (dec_picture->chroma_format_idc == 1)
        {
          loop_counter[416]++;
          for (i = 0; i < 4; i++)
          {
            loop_counter[417]++;
            img->cofu[i] = 0;
          }

          if (active_pps->entropy_coding_mode_flag == UVLC)
          {
            loop_counter[418]++;
            readCoeff4x4_CAVLC(img, inp, 6, 0, 0, levarr, runarr, &numcoeff);
            coef_ctr = -1;
            level = 1;
            for (k = 0; k < numcoeff; k++)
            {
              loop_counter[419]++;
              if (levarr[k] != 0)
              {
                loop_counter[420]++;
                currMB->cbp_blk |= 0xf0000 << (ll << 1);
                coef_ctr = (coef_ctr + runarr[k]) + 1;
                img->cofu[coef_ctr] = levarr[k];
              }

            }

          }
          else
          {
            coef_ctr = -1;
            level = 1;
            for (k = 0; (k < (img->num_cdc_coeff + 1)) && (level != 0); k++)
            {
              loop_counter[421]++;
              currSE.context = 6;
              currSE.type = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12) ? 8 : 13;
              img->is_intra_block = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
              img->is_v_block = ll;
              dP = &currSlice->partArr[partMap[currSE.type]];
              if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
              {
                loop_counter[422]++;
                currSE.mapping = linfo_levrun_c2x2;
              }
              else
                currSE.reading = readRunLevel_CABAC;

              dP->readSyntaxElement(&currSE, img, inp, dP);
              level = currSE.value1;
              run = currSE.value2;
              len = currSE.len;
              if (level != 0)
              {
                loop_counter[423]++;
                currMB->cbp_blk |= 0xf0000 << (ll << 1);
                coef_ctr = (coef_ctr + run) + 1;
                assert(coef_ctr < img->num_cdc_coeff);
                img->cofu[coef_ctr] = level;
              }

            }

          }

          if (smb || lossless_qpprime)
          {
            loop_counter[424]++;
            img->cof[0 + ll][4][0][0] = img->cofu[0];
            img->cof[1 + ll][4][0][0] = img->cofu[1];
            img->cof[0 + ll][5][0][0] = img->cofu[2];
            img->cof[1 + ll][5][0][0] = img->cofu[3];
          }
          else
          {
            temp[0] = ((img->cofu[0] + img->cofu[1]) + img->cofu[2]) + img->cofu[3];
            temp[1] = ((img->cofu[0] - img->cofu[1]) + img->cofu[2]) - img->cofu[3];
            temp[2] = ((img->cofu[0] + img->cofu[1]) - img->cofu[2]) - img->cofu[3];
            temp[3] = ((img->cofu[0] - img->cofu[1]) - img->cofu[2]) + img->cofu[3];
            for (i = 0; i < img->num_cdc_coeff; i++)
            {
              loop_counter[425]++;
              if (qp_per_uv[uv] < 5)
              {
                loop_counter[426]++;
                if (intra == 1)
                {
                  loop_counter[427]++;
                  temp[i] = (temp[i] * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) >> (5 - qp_per_uv[uv]);
                }
                else
                  temp[i] = (temp[i] * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) >> (5 - qp_per_uv[uv]);

              }
              else
              {
                if (intra == 1)
                {
                  loop_counter[428]++;
                  temp[i] = (temp[i] * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 5);
                }
                else
                  temp[i] = (temp[i] * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 5);

              }

            }

            img->cof[0 + ll][4][0][0] = temp[0];
            img->cof[1 + ll][4][0][0] = temp[1];
            img->cof[0 + ll][5][0][0] = temp[2];
            img->cof[1 + ll][5][0][0] = temp[3];
          }

        }
        else
          if (dec_picture->chroma_format_idc == 2)
        {
          loop_counter[429]++;
          int i;
          int j;
          int j1;
          int uv_idx = ll;
          int m3[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
          int m4[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
          int qp_per_uv_dc = ((qp_c[uv] + 3) + img->bitdepth_chroma_qp_scale) / 6;
          int qp_rem_uv_dc = ((qp_c[uv] + 3) + img->bitdepth_chroma_qp_scale) % 6;
          if (active_pps->entropy_coding_mode_flag == UVLC)
          {
            loop_counter[430]++;
            readCoeff4x4_CAVLC(img, inp, 6, 0, 0, levarr, runarr, &numcoeff);
            coef_ctr = -1;
            level = 1;
            for (k = 0; k < numcoeff; k++)
            {
              loop_counter[431]++;
              if (levarr[k] != 0)
              {
                loop_counter[432]++;
                currMB->cbp_blk |= ((int64) 0xff0000) << (ll << 2);
                coef_ctr = (coef_ctr + runarr[k]) + 1;
                i0 = SCAN_YUV422[coef_ctr][0];
                j0 = SCAN_YUV422[coef_ctr][1];
                m3[i0][j0] = levarr[k];
              }

            }

          }
          else
          {
            coef_ctr = -1;
            level = 1;
            for (k = 0; (k < 9) && (level != 0); k++)
            {
              loop_counter[433]++;
              currSE.context = 8;
              currSE.type = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12) ? 8 : 13;
              img->is_intra_block = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
              img->is_v_block = ll;
              dP = &currSlice->partArr[partMap[currSE.type]];
              if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
              {
                loop_counter[434]++;
                currSE.mapping = linfo_levrun_c2x2;
              }
              else
                currSE.reading = readRunLevel_CABAC;

              dP->readSyntaxElement(&currSE, img, inp, dP);
              level = currSE.value1;
              run = currSE.value2;
              len = currSE.len;
              if (level != 0)
              {
                loop_counter[435]++;
                currMB->cbp_blk |= ((int64) 0xff0000) << (ll << 2);
                coef_ctr = (coef_ctr + run) + 1;
                assert(coef_ctr < img->num_cdc_coeff);
                i0 = SCAN_YUV422[coef_ctr][0];
                j0 = SCAN_YUV422[coef_ctr][1];
                m3[i0][j0] = level;
              }

            }

          }

          if (!lossless_qpprime)
          {
            loop_counter[436]++;
            m4[0][0] = m3[0][0] + m3[1][0];
            m4[0][1] = m3[0][1] + m3[1][1];
            m4[0][2] = m3[0][2] + m3[1][2];
            m4[0][3] = m3[0][3] + m3[1][3];
            m4[1][0] = m3[0][0] - m3[1][0];
            m4[1][1] = m3[0][1] - m3[1][1];
            m4[1][2] = m3[0][2] - m3[1][2];
            m4[1][3] = m3[0][3] - m3[1][3];
          }
          else
          {
            for (i = 0; i < 2; i++)
            {
              loop_counter[437]++;
              for (j = 0; j < 4; j++)
              {
                loop_counter[438]++;
                img->cof[i + uv_idx][j + 4][0][0] = m3[i][j];
              }

            }

          }

          for (i = 0; (i < 2) && (!lossless_qpprime); i++)
          {
            loop_counter[439]++;
            for (j = 0; j < 4; j++)
            {
              loop_counter[440]++;
              m5[j] = m4[i][j];
            }

            m6[0] = m5[0] + m5[2];
            m6[1] = m5[0] - m5[2];
            m6[2] = m5[1] - m5[3];
            m6[3] = m5[1] + m5[3];
            for (j = 0; j < 2; j++)
            {
              loop_counter[441]++;
              j1 = 3 - j;
              if (qp_per_uv_dc < 4)
              {
                loop_counter[442]++;
                if (intra == 1)
                {
                  loop_counter[443]++;
                  img->cof[i + uv_idx][j + 4][0][0] = (((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv_dc][0][0]) + (1 << (3 - qp_per_uv_dc))) >> (4 - qp_per_uv_dc)) + 2) >> 2;
                  img->cof[i + uv_idx][j1 + 4][0][0] = (((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv_dc][0][0]) + (1 << (3 - qp_per_uv_dc))) >> (4 - qp_per_uv_dc)) + 2) >> 2;
                }
                else
                {
                  img->cof[i + uv_idx][j + 4][0][0] = (((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv_dc][0][0]) + (1 << (3 - qp_per_uv_dc))) >> (4 - qp_per_uv_dc)) + 2) >> 2;
                  img->cof[i + uv_idx][j1 + 4][0][0] = (((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv_dc][0][0]) + (1 << (3 - qp_per_uv_dc))) >> (4 - qp_per_uv_dc)) + 2) >> 2;
                }

              }
              else
              {
                if (intra == 1)
                {
                  loop_counter[444]++;
                  img->cof[i + uv_idx][j + 4][0][0] = ((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv_dc][0][0]) << (qp_per_uv_dc - 4)) + 2) >> 2;
                  img->cof[i + uv_idx][j1 + 4][0][0] = ((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv_dc][0][0]) << (qp_per_uv_dc - 4)) + 2) >> 2;
                }
                else
                {
                  img->cof[i + uv_idx][j + 4][0][0] = ((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv_dc][0][0]) << (qp_per_uv_dc - 4)) + 2) >> 2;
                  img->cof[i + uv_idx][j1 + 4][0][0] = ((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv_dc][0][0]) << (qp_per_uv_dc - 4)) + 2) >> 2;
                }

              }

            }

          }

        }
        else
        {
          int i;
          int j;
          int i1;
          int j1;
          int uv_idx = 4 + (ll << 1);
          if (active_pps->entropy_coding_mode_flag == UVLC)
          {
            loop_counter[445]++;
            readCoeff4x4_CAVLC(img, inp, 6, 0, 0, levarr, runarr, &numcoeff);
            coef_ctr = -1;
            level = 1;
            for (k = 0; k < numcoeff; k++)
            {
              loop_counter[446]++;
              if (levarr[k] != 0)
              {
                loop_counter[447]++;
                currMB->cbp_blk |= ((int64) 0xffff0000) << (ll << 3);
                coef_ctr = (coef_ctr + runarr[k]) + 1;
                i0 = SNGL_SCAN[coef_ctr][0];
                j0 = SNGL_SCAN[coef_ctr][1];
                img->cof[i0][j0 + uv_idx][0][0] = levarr[k];
              }

            }

          }
          else
          {
            coef_ctr = -1;
            level = 1;
            for (k = 0; (k < 17) && (level != 0); k++)
            {
              loop_counter[448]++;
              currSE.context = 9;
              currSE.type = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12) ? 8 : 13;
              img->is_intra_block = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
              img->is_v_block = ll;
              dP = &currSlice->partArr[partMap[currSE.type]];
              if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
              {
                loop_counter[449]++;
                currSE.mapping = linfo_levrun_c2x2;
              }
              else
                currSE.reading = readRunLevel_CABAC;

              dP->readSyntaxElement(&currSE, img, inp, dP);
              level = currSE.value1;
              run = currSE.value2;
              len = currSE.len;
              if (level != 0)
              {
                loop_counter[450]++;
                currMB->cbp_blk |= ((int64) 0xffff0000) << (ll << 3);
                coef_ctr = (coef_ctr + run) + 1;
                assert(coef_ctr < img->num_cdc_coeff);
                i0 = SNGL_SCAN[coef_ctr][0];
                j0 = SNGL_SCAN[coef_ctr][1];
                img->cof[i0][j0 + uv_idx][0][0] = level;
              }

            }

          }

          if (!residual_transform_dc)
          {
            loop_counter[451]++;
            for (j = uv_idx; (j < (4 + uv_idx)) && (!lossless_qpprime); j++)
            {
              loop_counter[452]++;
              for (i = 0; i < 4; i++)
              {
                loop_counter[453]++;
                m5[i] = img->cof[i][j][0][0];
              }

              m6[0] = m5[0] + m5[2];
              m6[1] = m5[0] - m5[2];
              m6[2] = m5[1] - m5[3];
              m6[3] = m5[1] + m5[3];
              for (i = 0; i < 2; i++)
              {
                loop_counter[454]++;
                i1 = 3 - i;
                img->cof[i][j][0][0] = m6[i] + m6[i1];
                img->cof[i1][j][0][0] = m6[i] - m6[i1];
              }

            }

            for (i = 0; (i < 4) && (!lossless_qpprime); i++)
            {
              loop_counter[455]++;
              for (j = 0; j < 4; j++)
              {
                loop_counter[456]++;
                m5[j] = img->cof[i][j + uv_idx][0][0];
              }

              m6[0] = m5[0] + m5[2];
              m6[1] = m5[0] - m5[2];
              m6[2] = m5[1] - m5[3];
              m6[3] = m5[1] + m5[3];
              for (j = 0; j < 2; j++)
              {
                loop_counter[457]++;
                j1 = 3 - j;
                if (qp_per_uv[uv] < 4)
                {
                  loop_counter[458]++;
                  if (intra == 1)
                  {
                    loop_counter[459]++;
                    img->cof[i][j + uv_idx][0][0] = (((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) + (1 << (3 - qp_per_uv[uv]))) >> (4 - qp_per_uv[uv])) + 2) >> 2;
                    img->cof[i][j1 + uv_idx][0][0] = (((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) + (1 << (3 - qp_per_uv[uv]))) >> (4 - qp_per_uv[uv])) + 2) >> 2;
                  }
                  else
                  {
                    img->cof[i][j + uv_idx][0][0] = (((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) + (1 << (3 - qp_per_uv[uv]))) >> (4 - qp_per_uv[uv])) + 2) >> 2;
                    img->cof[i][j1 + uv_idx][0][0] = (((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) + (1 << (3 - qp_per_uv[uv]))) >> (4 - qp_per_uv[uv])) + 2) >> 2;
                  }

                }
                else
                {
                  if (intra == 1)
                  {
                    loop_counter[460]++;
                    img->cof[i][j + uv_idx][0][0] = ((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 4)) + 2) >> 2;
                    img->cof[i][j1 + uv_idx][0][0] = ((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 4)) + 2) >> 2;
                  }
                  else
                  {
                    img->cof[i][j + uv_idx][0][0] = ((((m6[j] + m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 4)) + 2) >> 2;
                    img->cof[i][j1 + uv_idx][0][0] = ((((m6[j] - m6[j1]) * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 4)) + 2) >> 2;
                  }

                }

              }

            }

          }
          else
          {
            for (i = 0; (i < 4) && (!lossless_qpprime); i++)
            {
              loop_counter[461]++;
              for (j = 0; j < 4; j++)
              {
                loop_counter[462]++;
                if (qp_per_uv[uv] < 4)
                {
                  loop_counter[463]++;
                  if (intra == 1)
                  {
                    loop_counter[464]++;
                    img->cof[i][j + uv_idx][0][0] = ((img->cof[i][j + uv_idx][0][0] * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) + (1 << (3 - qp_per_uv[uv]))) >> (4 - qp_per_uv[uv]);
                  }
                  else
                  {
                    img->cof[i][j + uv_idx][0][0] = ((img->cof[i][j + uv_idx][0][0] * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) + (1 << (3 - qp_per_uv[uv]))) >> (4 - qp_per_uv[uv]);
                  }

                }
                else
                {
                  if (intra == 1)
                  {
                    loop_counter[465]++;
                    img->cof[i][j + uv_idx][0][0] = (img->cof[i][j + uv_idx][0][0] * InvLevelScale4x4Chroma_Intra[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 4);
                  }
                  else
                  {
                    img->cof[i][j + uv_idx][0][0] = (img->cof[i][j + uv_idx][0][0] * InvLevelScale4x4Chroma_Inter[uv][qp_rem_uv[uv]][0][0]) << (qp_per_uv[uv] - 4);
                  }

                }

              }

            }

          }

        }


      }

    }

    if (cbp <= 31)
    {
      loop_counter[466]++;
      for (j = 4; j < (4 + img->num_blk8x8_uv); j++)
      {
        loop_counter[467]++;
        for (i = 0; i < 4; i++)
        {
          loop_counter[468]++;
          img->nz_coeff[img->current_mb_nr][i][j] = 0;
        }

      }

    }

    if (cbp > 31)
    {
      loop_counter[469]++;
      for (b8 = 0; b8 < img->num_blk8x8_uv; b8++)
      {
        loop_counter[470]++;
        for (b4 = 0; b4 < 4; b4++)
        {
          loop_counter[471]++;
          i = cofuv_blk_x[yuv][b8][b4];
          j = cofuv_blk_y[yuv][b8][b4];
          img->is_v_block = (uv = b8 > ((img->num_blk8x8_uv >> 1) - 1));
          if (active_pps->entropy_coding_mode_flag == UVLC)
          {
            loop_counter[472]++;
            readCoeff4x4_CAVLC(img, inp, 7, i, j, levarr, runarr, &numcoeff);
            coef_ctr = 0;
            level = 1;
            for (k = 0; k < numcoeff; k++)
            {
              loop_counter[473]++;
              if (levarr[k] != 0)
              {
                loop_counter[474]++;
                currMB->cbp_blk |= ((int64) 1) << cbp_blk_chroma[b8][b4];
                coef_ctr = (coef_ctr + runarr[k]) + 1;
                if ((img->structure == FRAME) && (!currMB->mb_field))
                {
                  loop_counter[475]++;
                  i0 = SNGL_SCAN[coef_ctr][0];
                  j0 = SNGL_SCAN[coef_ctr][1];
                }
                else
                {
                  i0 = FIELD_SCAN[coef_ctr][0];
                  j0 = FIELD_SCAN[coef_ctr][1];
                }

                if (lossless_qpprime)
                {
                  loop_counter[476]++;
                  img->cof[i][j][i0][j0] = levarr[k];
                }
                else
                  if (qp_per_uv[uv] < 4)
                {
                  loop_counter[477]++;
                  if (intra == 1)
                  {
                    loop_counter[478]++;
                    img->cof[i][j][i0][j0] = ((levarr[k] * InvLevelScale4x4Chroma_Intra[img->is_v_block][qp_rem_uv[uv]][i0][j0]) + qp_const_uv[uv]) >> (4 - qp_per_uv[uv]);
                  }
                  else
                    img->cof[i][j][i0][j0] = ((levarr[k] * InvLevelScale4x4Chroma_Inter[img->is_v_block][qp_rem_uv[uv]][i0][j0]) + qp_const_uv[uv]) >> (4 - qp_per_uv[uv]);

                }
                else
                {
                  if (intra == 1)
                  {
                    loop_counter[479]++;
                    img->cof[i][j][i0][j0] = (levarr[k] * InvLevelScale4x4Chroma_Intra[img->is_v_block][qp_rem_uv[uv]][i0][j0]) << (qp_per_uv[uv] - 4);
                  }
                  else
                    img->cof[i][j][i0][j0] = (levarr[k] * InvLevelScale4x4Chroma_Inter[img->is_v_block][qp_rem_uv[uv]][i0][j0]) << (qp_per_uv[uv] - 4);

                }


              }

            }

          }
          else
          {
            coef_ctr = 0;
            level = 1;
            img->subblock_y = subblk_offset_y[yuv][b8][b4] >> 2;
            img->subblock_x = subblk_offset_x[yuv][b8][b4] >> 2;
            currSE.context = 7;
            currSE.type = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12) ? 10 : 15;
            img->is_intra_block = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
            for (k = 0; (k < 16) && (level != 0); k++)
            {
              loop_counter[480]++;
              dP = &currSlice->partArr[partMap[currSE.type]];
              if ((active_pps->entropy_coding_mode_flag == UVLC) || dP->bitstream->ei_flag)
              {
                loop_counter[481]++;
                currSE.mapping = linfo_levrun_inter;
              }
              else
                currSE.reading = readRunLevel_CABAC;

              dP->readSyntaxElement(&currSE, img, inp, dP);
              level = currSE.value1;
              run = currSE.value2;
              len = currSE.len;
              if (level != 0)
              {
                loop_counter[482]++;
                currMB->cbp_blk |= ((int64) 1) << cbp_blk_chroma[b8][b4];
                coef_ctr = (coef_ctr + run) + 1;
                if ((img->structure == FRAME) && (!currMB->mb_field))
                {
                  loop_counter[483]++;
                  i0 = SNGL_SCAN[coef_ctr][0];
                  j0 = SNGL_SCAN[coef_ctr][1];
                }
                else
                {
                  i0 = FIELD_SCAN[coef_ctr][0];
                  j0 = FIELD_SCAN[coef_ctr][1];
                }

                if (lossless_qpprime)
                {
                  loop_counter[484]++;
                  img->cof[i][j][i0][j0] = level;
                }
                else
                  if (qp_per_uv[uv] < 4)
                {
                  loop_counter[485]++;
                  if (intra == 1)
                  {
                    loop_counter[486]++;
                    img->cof[i][j][i0][j0] = ((level * InvLevelScale4x4Chroma_Intra[img->is_v_block][qp_rem_uv[uv]][i0][j0]) + qp_const_uv[uv]) >> (4 - qp_per_uv[uv]);
                  }
                  else
                    img->cof[i][j][i0][j0] = ((level * InvLevelScale4x4Chroma_Inter[img->is_v_block][qp_rem_uv[uv]][i0][j0]) + qp_const_uv[uv]) >> (4 - qp_per_uv[uv]);

                }
                else
                {
                  if (intra == 1)
                  {
                    loop_counter[487]++;
                    img->cof[i][j][i0][j0] = (level * InvLevelScale4x4Chroma_Intra[img->is_v_block][qp_rem_uv[uv]][i0][j0]) << (qp_per_uv[uv] - 4);
                  }
                  else
                    img->cof[i][j][i0][j0] = (level * InvLevelScale4x4Chroma_Inter[img->is_v_block][qp_rem_uv[uv]][i0][j0]) << (qp_per_uv[uv] - 4);

                }


              }

            }

          }

        }

      }

    }

  }

}

void decode_ipcm_mb(struct img_par *img)
{
  int i;
  int j;
  Macroblock *currMb = &img->mb_data[img->current_mb_nr];
  for (i = 0; i < 16; i++)
  {
    loop_counter[488]++;
    for (j = 0; j < 16; j++)
    {
      loop_counter[489]++;
      dec_picture->imgY[img->pix_y + i][img->pix_x + j] = img->cof[i / 4][j / 4][i % 4][j % 4];
    }

  }

  if (dec_picture->chroma_format_idc != 0)
  {
    loop_counter[490]++;
    for (i = 0; i < img->mb_cr_size_y; i++)
    {
      loop_counter[491]++;
      for (j = 0; j < img->mb_cr_size_x; j++)
      {
        loop_counter[492]++;
        dec_picture->imgUV[0][img->pix_c_y + i][img->pix_c_x + j] = img->cof[i / 4][(j / 4) + 4][i % 4][j % 4];
      }

    }

    for (i = 0; i < img->mb_cr_size_y; i++)
    {
      loop_counter[493]++;
      for (j = 0; j < img->mb_cr_size_x; j++)
      {
        loop_counter[494]++;
        dec_picture->imgUV[1][img->pix_c_y + i][img->pix_c_x + j] = img->cof[(i / 4) + 2][(j / 4) + 4][i % 4][j % 4];
      }

    }

  }

  currMb->qp = 0;
  for (i = 0; i < 4; i++)
  {
    loop_counter[495]++;
    for (j = 0; j < (4 + img->num_blk8x8_uv); j++)
    {
      loop_counter[496]++;
      img->nz_coeff[img->current_mb_nr][i][j] = 16;
    }

  }

  currMb->skip_flag = 0;
  currMb->cbp_blk = 0xFFFF;
  last_dquant = 0;
}

int decode_one_macroblock(struct img_par *img, struct inp_par *inp)
{
  int tmp_block[4][4];
  int tmp_blockbw[4][4];
  int i = 0;
  int j = 0;
  int k;
  int l;
  int ii = 0;
  int jj = 0;
  int i1 = 0;
  int j1 = 0;
  int j4 = 0;
  int i4 = 0;
  int uv;
  int hv;
  int vec1_x = 0;
  int vec1_y = 0;
  int vec2_x = 0;
  int vec2_y = 0;
  int ioff;
  int joff;
  int block8x8;
  int bw_pred = 0;
  int fw_pred = 0;
  int pred;
  int ifx;
  int ii0;
  int jj0;
  int ii1;
  int jj1;
  int if1;
  int jf1;
  int if0;
  int jf0;
  int mv_mul;
  int f1_x;
  int f1_y;
  int f2_x;
  int f2_y;
  int f3;
  int f4;
  const byte decode_block_scan[16] = {0, 1, 4, 5, 2, 3, 6, 7, 8, 9, 12, 13, 10, 11, 14, 15};
  Macroblock *currMB = &img->mb_data[img->current_mb_nr];
  short ref_idx;
  short fw_refframe = -1;
  short bw_refframe = -1;
  int mv_mode;
  int pred_dir;
  int intra_prediction;
  short fw_ref_idx = -1;
  short bw_ref_idx = -1;
  short ***mv_array;
  short ***fw_mv_array;
  short ***bw_mv_array;
  int mv_scale;
  int mb_nr = img->current_mb_nr;
  int smb = ((img->type == SP_SLICE) && ((((currMB->mb_type != 9) && (currMB->mb_type != 10)) && (currMB->mb_type != 13)) && (currMB->mb_type != 14))) || ((img->type == SI_SLICE) && (currMB->mb_type == 12));
  int list_offset;
  int max_y_cr;
  StorablePicture **list;
  int jf;
  char fw_rFrame = -1;
  char bw_rFrame = -1;
  short pmvfw[2] = {0, 0};
  short pmvbw[2] = {0, 0};
  int direct_pdir = -1;
  int curr_mb_field = img->MbaffFrameFlag && currMB->mb_field;
  byte **moving_block;
  short ****co_located_mv;
  char ***co_located_ref_idx;
  int64 ***co_located_ref_id;
  int need_4x4_transform = !currMB->luma_transform_size_8x8_flag;
  int b8;
  int b4;
  int uv_shift;
  int yuv = dec_picture->chroma_format_idc - 1;
  int residue_transform_flag = img->residue_transform_flag;
  int residue_R;
  int residue_G;
  int residue_B;
  int temp;
  if (currMB->mb_type == 14)
  {
    loop_counter[497]++;
    decode_ipcm_mb(img);
    return 0;
  }

  if (curr_mb_field)
  {
    loop_counter[498]++;
    if (mb_nr % 2)
    {
      loop_counter[499]++;
      list_offset = 4;
      moving_block = Co_located->bottom_moving_block;
      co_located_mv = Co_located->bottom_mv;
      co_located_ref_idx = Co_located->bottom_ref_idx;
      co_located_ref_id = Co_located->bottom_ref_pic_id;
    }
    else
    {
      list_offset = 2;
      moving_block = Co_located->top_moving_block;
      co_located_mv = Co_located->top_mv;
      co_located_ref_idx = Co_located->top_ref_idx;
      co_located_ref_id = Co_located->top_ref_pic_id;
    }

    max_y_cr = (dec_picture->size_y_cr / 2) - 1;
  }
  else
  {
    list_offset = 0;
    moving_block = Co_located->moving_block;
    co_located_mv = Co_located->mv;
    co_located_ref_idx = Co_located->ref_idx;
    co_located_ref_id = Co_located->ref_pic_id;
    max_y_cr = dec_picture->size_y_cr - 1;
  }

  if (!img->MbaffFrameFlag)
  {
    loop_counter[500]++;
    for (l = 0 + list_offset; l < (2 + list_offset); l++)
    {
      loop_counter[501]++;
      for (k = 0; k < listXsize[l]; k++)
      {
        loop_counter[502]++;
        listX[l][k]->chroma_vector_adjustment = 0;
        if ((img->structure == TOP_FIELD) && (img->structure != listX[l][k]->structure))
        {
          loop_counter[503]++;
          listX[l][k]->chroma_vector_adjustment = -2;
        }

        if ((img->structure == BOTTOM_FIELD) && (img->structure != listX[l][k]->structure))
        {
          loop_counter[504]++;
          listX[l][k]->chroma_vector_adjustment = 2;
        }

      }

    }

  }
  else
  {
    if (curr_mb_field)
    {
      loop_counter[505]++;
      for (l = 0 + list_offset; l < (2 + list_offset); l++)
      {
        loop_counter[506]++;
        for (k = 0; k < listXsize[l]; k++)
        {
          loop_counter[507]++;
          listX[l][k]->chroma_vector_adjustment = 0;
          if (((img->current_mb_nr % 2) == 0) && (listX[l][k]->structure == BOTTOM_FIELD))
          {
            loop_counter[508]++;
            listX[l][k]->chroma_vector_adjustment = -2;
          }

          if (((img->current_mb_nr % 2) == 1) && (listX[l][k]->structure == TOP_FIELD))
          {
            loop_counter[509]++;
            listX[l][k]->chroma_vector_adjustment = 2;
          }

        }

      }

    }
    else
    {
      for (l = 0 + list_offset; l < (2 + list_offset); l++)
      {
        loop_counter[510]++;
        for (k = 0; k < listXsize[l]; k++)
        {
          loop_counter[511]++;
          listX[l][k]->chroma_vector_adjustment = 0;
        }

      }

    }

  }

  mv_mul = 4;
  if ((currMB->mb_type == 10) || (currMB->mb_type == 14))
  {
    loop_counter[512]++;
    intrapred_luma_16x16(img, currMB->i16mode);
  }

  if (((img->type == B_SLICE) && img->direct_spatial_mv_pred_flag) && (((currMB->mb_type == 0) && (img->type == B_SLICE)) || ((currMB->mb_type == 8) && (!(((currMB->b8mode[0] && currMB->b8mode[1]) && currMB->b8mode[2]) && currMB->b8mode[3])))))
  {
    loop_counter[513]++;
    char fw_rFrameL;
    char fw_rFrameU;
    char fw_rFrameUL;
    char fw_rFrameUR;
    char bw_rFrameL;
    char bw_rFrameU;
    char bw_rFrameUL;
    char bw_rFrameUR;
    PixelPos mb_left;
    PixelPos mb_up;
    PixelPos mb_upleft;
    PixelPos mb_upright;
    getLuma4x4Neighbour(img->current_mb_nr, 0, 0, -1, 0, &mb_left);
    getLuma4x4Neighbour(img->current_mb_nr, 0, 0, 0, -1, &mb_up);
    getLuma4x4Neighbour(img->current_mb_nr, 0, 0, 16, -1, &mb_upright);
    getLuma4x4Neighbour(img->current_mb_nr, 0, 0, -1, -1, &mb_upleft);
    if (!img->MbaffFrameFlag)
    {
      loop_counter[514]++;
      fw_rFrameL = mb_left.available ? dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] : -1;
      fw_rFrameU = mb_up.available ? dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : -1;
      fw_rFrameUL = mb_upleft.available ? dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
      fw_rFrameUR = mb_upright.available ? dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : fw_rFrameUL;
      bw_rFrameL = mb_left.available ? dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : -1;
      bw_rFrameU = mb_up.available ? dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : -1;
      bw_rFrameUL = mb_upleft.available ? dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
      bw_rFrameUR = mb_upright.available ? dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : bw_rFrameUL;
    }
    else
    {
      if (img->mb_data[img->current_mb_nr].mb_field)
      {
        loop_counter[515]++;
        fw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] : dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] * 2 : -1;
        fw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] * 2 : -1;
        fw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] * 2 : -1;
        fw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] * 2 : fw_rFrameUL;
        bw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] * 2 : -1;
        bw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] * 2 : -1;
        bw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] * 2 : -1;
        bw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] * 2 : bw_rFrameUL;
      }
      else
      {
        fw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_left.pos_y][mb_left.pos_x] : -1;
        fw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_up.pos_y][mb_up.pos_x] : -1;
        fw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
        fw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] >> 1 : dec_picture->ref_idx[LIST_0][mb_upright.pos_y][mb_upright.pos_x] : fw_rFrameUL;
        bw_rFrameL = mb_left.available ? img->mb_data[mb_left.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_left.pos_y][mb_left.pos_x] : -1;
        bw_rFrameU = mb_up.available ? img->mb_data[mb_up.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_up.pos_y][mb_up.pos_x] : -1;
        bw_rFrameUL = mb_upleft.available ? img->mb_data[mb_upleft.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_upleft.pos_y][mb_upleft.pos_x] : -1;
        bw_rFrameUR = mb_upright.available ? img->mb_data[mb_upright.mb_addr].mb_field || (dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] < 0) ? dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] >> 1 : dec_picture->ref_idx[LIST_1][mb_upright.pos_y][mb_upright.pos_x] : bw_rFrameUL;
      }

    }

    fw_rFrame = (fw_rFrameL >= 0) && (fw_rFrameU >= 0) ? fw_rFrameL < fw_rFrameU ? fw_rFrameL : fw_rFrameU : fw_rFrameL > fw_rFrameU ? fw_rFrameL : fw_rFrameU;
    fw_rFrame = (fw_rFrame >= 0) && (fw_rFrameUR >= 0) ? fw_rFrame < fw_rFrameUR ? fw_rFrame : fw_rFrameUR : fw_rFrame > fw_rFrameUR ? fw_rFrame : fw_rFrameUR;
    bw_rFrame = (bw_rFrameL >= 0) && (bw_rFrameU >= 0) ? bw_rFrameL < bw_rFrameU ? bw_rFrameL : bw_rFrameU : bw_rFrameL > bw_rFrameU ? bw_rFrameL : bw_rFrameU;
    bw_rFrame = (bw_rFrame >= 0) && (bw_rFrameUR >= 0) ? bw_rFrame < bw_rFrameUR ? bw_rFrame : bw_rFrameUR : bw_rFrame > bw_rFrameUR ? bw_rFrame : bw_rFrameUR;
    if (fw_rFrame >= 0)
    {
      loop_counter[516]++;
      SetMotionVectorPredictor(img, pmvfw, pmvfw + 1, fw_rFrame, LIST_0, dec_picture->ref_idx, dec_picture->mv, 0, 0, 16, 16);
    }

    if (bw_rFrame >= 0)
    {
      loop_counter[517]++;
      SetMotionVectorPredictor(img, pmvbw, pmvbw + 1, bw_rFrame, LIST_1, dec_picture->ref_idx, dec_picture->mv, 0, 0, 16, 16);
    }

  }

  for (block8x8 = 0; block8x8 < 4; block8x8++)
  {
    loop_counter[518]++;
    if (currMB->b8mode[block8x8] == 13)
    {
      loop_counter[519]++;
      ioff = 8 * (block8x8 % 2);
      joff = 8 * (block8x8 / 2);
      if (!residue_transform_flag)
      {
        loop_counter[520]++;
        intrapred8x8(img, block8x8);
      }

      itrans8x8(img, ioff, joff);
      for (ii = 0; ii < 8; ii++)
      {
        loop_counter[521]++;
        for (jj = 0; jj < 8; jj++)
        {
          loop_counter[522]++;
          if (!residue_transform_flag)
          {
            loop_counter[523]++;
            dec_picture->imgY[(img->pix_y + joff) + jj][(img->pix_x + ioff) + ii] = img->m7[ii + ioff][jj + joff];
          }
          else
          {
            rec_res[0][ii + ioff][jj + joff] = img->m7[ii + ioff][jj + joff];
          }

        }

      }

      continue;
    }

    for (k = block8x8 * 4; k < ((block8x8 * 4) + 4); k++)
    {
      loop_counter[524]++;
      i = decode_block_scan[k] & 3;
      j = (decode_block_scan[k] >> 2) & 3;
      ioff = i * 4;
      i4 = img->block_x + i;
      joff = j * 4;
      j4 = img->block_y + j;
      mv_mode = currMB->b8mode[(2 * (j / 2)) + (i / 2)];
      pred_dir = currMB->b8pdir[(2 * (j / 2)) + (i / 2)];
      assert(pred_dir <= 2);
      if (mv_mode == 11)
      {
        loop_counter[525]++;
        if (!residue_transform_flag)
        {
          loop_counter[526]++;
          if (intrapred(img, ioff, joff, i4, j4) == 1)
          {
            loop_counter[527]++;
            return 1;
          }

        }

      }
      else
        if (!((currMB->mb_type == 10) || (currMB->mb_type == 14)))
      {
        loop_counter[528]++;
        if (pred_dir != 2)
        {
          loop_counter[529]++;
          fw_refframe = (ref_idx = dec_picture->ref_idx[LIST_0 + pred_dir][j4][i4]);
          mv_array = dec_picture->mv[LIST_0 + pred_dir];
          list = listX[(0 + list_offset) + pred_dir];
          vec1_x = ((i4 * 4) * mv_mul) + mv_array[j4][i4][0];
          if (!curr_mb_field)
          {
            loop_counter[530]++;
            vec1_y = ((j4 * 4) * mv_mul) + mv_array[j4][i4][1];
          }
          else
          {
            if ((mb_nr % 2) == 0)
            {
              loop_counter[531]++;
              vec1_y = (((img->block_y * 2) + joff) * mv_mul) + mv_array[j4][i4][1];
            }
            else
              vec1_y = ((((img->block_y - 4) * 2) + joff) * mv_mul) + mv_array[j4][i4][1];

          }

          get_block(ref_idx, list, vec1_x, vec1_y, img, tmp_block);
          if (img->apply_weights)
          {
            loop_counter[532]++;
            if (((active_pps->weighted_pred_flag && ((img->type == P_SLICE) || (img->type == SP_SLICE))) || ((active_pps->weighted_bipred_idc == 1) && (img->type == B_SLICE))) && curr_mb_field)
            {
              loop_counter[533]++;
              ref_idx >>= 1;
            }

            for (ii = 0; ii < 4; ii++)
            {
              loop_counter[534]++;
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[535]++;
                img->mpr[ii + ioff][jj + joff] = ((((img->wp_weight[pred_dir][ref_idx][0] * tmp_block[ii][jj]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[pred_dir][fw_refframe >> curr_mb_field][0]) > img->max_imgpel_value ? img->max_imgpel_value : ((((img->wp_weight[pred_dir][ref_idx][0] * tmp_block[ii][jj]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[pred_dir][fw_refframe >> curr_mb_field][0]) < 0 ? 0 : (((img->wp_weight[pred_dir][ref_idx][0] * tmp_block[ii][jj]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[pred_dir][fw_refframe >> curr_mb_field][0];
              }

            }

          }
          else
          {
            for (ii = 0; ii < 4; ii++)
            {
              loop_counter[536]++;
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[537]++;
                img->mpr[ii + ioff][jj + joff] = tmp_block[ii][jj];
              }

            }

          }

        }
        else
        {
          if (mv_mode != 0)
          {
            loop_counter[538]++;
            fw_mv_array = dec_picture->mv[LIST_0];
            bw_mv_array = dec_picture->mv[LIST_1];
            fw_refframe = dec_picture->ref_idx[LIST_0][j4][i4];
            bw_refframe = dec_picture->ref_idx[LIST_1][j4][i4];
            fw_ref_idx = fw_refframe;
            bw_ref_idx = bw_refframe;
          }
          else
          {
            fw_mv_array = dec_picture->mv[LIST_0];
            bw_mv_array = dec_picture->mv[LIST_1];
            bw_refframe = 0;
            if (img->direct_spatial_mv_pred_flag)
            {
              loop_counter[539]++;
              int imgblock_y = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? (img->block_y - 4) / 2 : img->block_y / 2 : img->block_y;
              int j6 = imgblock_y + j;
              if (fw_rFrame >= 0)
              {
                loop_counter[540]++;
                if ((!fw_rFrame) && ((!moving_block[j6][i4]) && (!listX[1 + list_offset][0]->is_long_term)))
                {
                  loop_counter[541]++;
                  dec_picture->mv[LIST_0][j4][i4][0] = 0;
                  dec_picture->mv[LIST_0][j4][i4][1] = 0;
                  dec_picture->ref_idx[LIST_0][j4][i4] = 0;
                }
                else
                {
                  dec_picture->mv[LIST_0][j4][i4][0] = pmvfw[0];
                  dec_picture->mv[LIST_0][j4][i4][1] = pmvfw[1];
                  dec_picture->ref_idx[LIST_0][j4][i4] = fw_rFrame;
                }

              }
              else
              {
                dec_picture->ref_idx[LIST_0][j4][i4] = -1;
                dec_picture->mv[LIST_0][j4][i4][0] = 0;
                dec_picture->mv[LIST_0][j4][i4][1] = 0;
              }

              if (bw_rFrame >= 0)
              {
                loop_counter[542]++;
                if ((bw_rFrame == 0) && ((!moving_block[j6][i4]) && (!listX[1 + list_offset][0]->is_long_term)))
                {
                  loop_counter[543]++;
                  dec_picture->mv[LIST_1][j4][i4][0] = 0;
                  dec_picture->mv[LIST_1][j4][i4][1] = 0;
                  dec_picture->ref_idx[LIST_1][j4][i4] = bw_rFrame;
                }
                else
                {
                  dec_picture->mv[LIST_1][j4][i4][0] = pmvbw[0];
                  dec_picture->mv[LIST_1][j4][i4][1] = pmvbw[1];
                  dec_picture->ref_idx[LIST_1][j4][i4] = bw_rFrame;
                }

              }
              else
              {
                dec_picture->mv[LIST_1][j4][i4][0] = 0;
                dec_picture->mv[LIST_1][j4][i4][1] = 0;
                dec_picture->ref_idx[LIST_1][j4][i4] = -1;
              }

              if ((fw_rFrame < 0) && (bw_rFrame < 0))
              {
                loop_counter[544]++;
                dec_picture->ref_idx[LIST_0][j4][i4] = 0;
                dec_picture->ref_idx[LIST_1][j4][i4] = 0;
              }

              fw_refframe = dec_picture->ref_idx[LIST_0][j4][i4] != (-1) ? dec_picture->ref_idx[LIST_0][j4][i4] : 0;
              bw_refframe = dec_picture->ref_idx[LIST_1][j4][i4] != (-1) ? dec_picture->ref_idx[LIST_1][j4][i4] : 0;
              fw_ref_idx = fw_refframe;
              bw_ref_idx = bw_refframe;
              if (dec_picture->ref_idx[LIST_1][j4][i4] == (-1))
              {
                loop_counter[545]++;
                direct_pdir = 0;
              }
              else
                if (dec_picture->ref_idx[LIST_0][j4][i4] == (-1))
              {
                loop_counter[546]++;
                direct_pdir = 1;
              }
              else
                direct_pdir = 2;


            }
            else
            {
              int imgblock_y = img->MbaffFrameFlag && currMB->mb_field ? img->current_mb_nr % 2 ? (img->block_y - 4) / 2 : img->block_y / 2 : img->block_y;
              int j6 = imgblock_y + j;
              int refList = co_located_ref_idx[LIST_0][j6][i4] == (-1) ? LIST_1 : LIST_0;
              int ref_idx = co_located_ref_idx[refList][j6][i4];
              if (ref_idx == (-1))
              {
                loop_counter[547]++;
                for (hv = 0; hv < 2; hv++)
                {
                  loop_counter[548]++;
                  dec_picture->mv[LIST_0][j4][i4][hv] = 0;
                  dec_picture->mv[LIST_1][j4][i4][hv] = 0;
                }

                dec_picture->ref_idx[LIST_0][j4][i4] = 0;
                dec_picture->ref_idx[LIST_1][j4][i4] = 0;
                fw_refframe = 0;
                fw_ref_idx = 0;
              }
              else
              {
                int mapped_idx = 0;
                int iref;
                {
                  for (iref = 0; iref < (img->num_ref_idx_l0_active < listXsize[LIST_0 + list_offset] ? img->num_ref_idx_l0_active : listXsize[LIST_0 + list_offset]); iref++)
                  {
                    loop_counter[549]++;
                    if (dec_picture->ref_pic_num[img->current_slice_nr][LIST_0 + list_offset][iref] == co_located_ref_id[refList][j6][i4])
                    {
                      loop_counter[550]++;
                      mapped_idx = iref;
                      break;
                    }
                    else
                    {
                      mapped_idx = -135792468;
                    }

                  }

                  if ((-135792468) == mapped_idx)
                  {
                    loop_counter[551]++;
                    error("temporal direct error\ncolocated block has ref that is unavailable", -1111);
                  }

                }
                fw_ref_idx = mapped_idx;
                mv_scale = img->mvscale[LIST_0 + list_offset][mapped_idx];
                if ((mv_scale == 9999) || listX[LIST_0 + list_offset][mapped_idx]->is_long_term)
                {
                  loop_counter[552]++;
                  dec_picture->mv[LIST_0][j4][i4][0] = co_located_mv[refList][j6][i4][0];
                  dec_picture->mv[LIST_0][j4][i4][1] = co_located_mv[refList][j6][i4][1];
                  dec_picture->mv[LIST_1][j4][i4][0] = 0;
                  dec_picture->mv[LIST_1][j4][i4][1] = 0;
                }
                else
                {
                  dec_picture->mv[LIST_0][j4][i4][0] = ((mv_scale * co_located_mv[refList][j6][i4][0]) + 128) >> 8;
                  dec_picture->mv[LIST_0][j4][i4][1] = ((mv_scale * co_located_mv[refList][j6][i4][1]) + 128) >> 8;
                  dec_picture->mv[LIST_1][j4][i4][0] = dec_picture->mv[LIST_0][j4][i4][0] - co_located_mv[refList][j6][i4][0];
                  dec_picture->mv[LIST_1][j4][i4][1] = dec_picture->mv[LIST_0][j4][i4][1] - co_located_mv[refList][j6][i4][1];
                }

                fw_refframe = (dec_picture->ref_idx[LIST_0][j4][i4] = mapped_idx);
                bw_refframe = (dec_picture->ref_idx[LIST_1][j4][i4] = 0);
                fw_ref_idx = fw_refframe;
                bw_ref_idx = bw_refframe;
              }

            }

            dec_picture->ref_pic_id[LIST_0][j4][i4] = dec_picture->ref_pic_num[img->current_slice_nr][LIST_0 + list_offset][(short) dec_picture->ref_idx[LIST_0][j4][i4]];
            dec_picture->ref_pic_id[LIST_1][j4][i4] = dec_picture->ref_pic_num[img->current_slice_nr][LIST_1 + list_offset][(short) dec_picture->ref_idx[LIST_1][j4][i4]];
          }

          if ((mv_mode == 0) && img->direct_spatial_mv_pred_flag)
          {
            loop_counter[553]++;
            if (dec_picture->ref_idx[LIST_0][j4][i4] >= 0)
            {
              loop_counter[554]++;
              vec1_x = ((i4 * 4) * mv_mul) + fw_mv_array[j4][i4][0];
              if (!curr_mb_field)
              {
                loop_counter[555]++;
                vec1_y = ((j4 * 4) * mv_mul) + fw_mv_array[j4][i4][1];
              }
              else
              {
                if ((mb_nr % 2) == 0)
                {
                  loop_counter[556]++;
                  vec1_y = (((img->block_y * 2) + joff) * mv_mul) + fw_mv_array[j4][i4][1];
                }
                else
                {
                  vec1_y = ((((img->block_y - 4) * 2) + joff) * mv_mul) + fw_mv_array[j4][i4][1];
                }

              }

              get_block(fw_refframe, listX[0 + list_offset], vec1_x, vec1_y, img, tmp_block);
            }

            if (dec_picture->ref_idx[LIST_1][j4][i4] >= 0)
            {
              loop_counter[557]++;
              vec2_x = ((i4 * 4) * mv_mul) + bw_mv_array[j4][i4][0];
              if (!curr_mb_field)
              {
                loop_counter[558]++;
                vec2_y = ((j4 * 4) * mv_mul) + bw_mv_array[j4][i4][1];
              }
              else
              {
                if ((mb_nr % 2) == 0)
                {
                  loop_counter[559]++;
                  vec2_y = (((img->block_y * 2) + joff) * mv_mul) + bw_mv_array[j4][i4][1];
                }
                else
                {
                  vec2_y = ((((img->block_y - 4) * 2) + joff) * mv_mul) + bw_mv_array[j4][i4][1];
                }

              }

              get_block(bw_refframe, listX[1 + list_offset], vec2_x, vec2_y, img, tmp_blockbw);
            }

          }
          else
          {
            vec1_x = ((i4 * 4) * mv_mul) + fw_mv_array[j4][i4][0];
            vec2_x = ((i4 * 4) * mv_mul) + bw_mv_array[j4][i4][0];
            if (!curr_mb_field)
            {
              loop_counter[560]++;
              vec1_y = ((j4 * 4) * mv_mul) + fw_mv_array[j4][i4][1];
              vec2_y = ((j4 * 4) * mv_mul) + bw_mv_array[j4][i4][1];
            }
            else
            {
              if ((mb_nr % 2) == 0)
              {
                loop_counter[561]++;
                vec1_y = (((img->block_y * 2) + joff) * mv_mul) + fw_mv_array[j4][i4][1];
                vec2_y = (((img->block_y * 2) + joff) * mv_mul) + bw_mv_array[j4][i4][1];
              }
              else
              {
                vec1_y = ((((img->block_y - 4) * 2) + joff) * mv_mul) + fw_mv_array[j4][i4][1];
                vec2_y = ((((img->block_y - 4) * 2) + joff) * mv_mul) + bw_mv_array[j4][i4][1];
              }

            }

            get_block(fw_refframe, listX[0 + list_offset], vec1_x, vec1_y, img, tmp_block);
            get_block(bw_refframe, listX[1 + list_offset], vec2_x, vec2_y, img, tmp_blockbw);
          }

          if (((mv_mode == 0) && img->direct_spatial_mv_pred_flag) && (direct_pdir == 0))
          {
            loop_counter[562]++;
            if (img->apply_weights)
            {
              loop_counter[563]++;
              if (((active_pps->weighted_pred_flag && ((img->type == P_SLICE) || (img->type == SP_SLICE))) || ((active_pps->weighted_bipred_idc == 1) && (img->type == B_SLICE))) && curr_mb_field)
              {
                loop_counter[564]++;
                fw_ref_idx >>= 1;
              }

              for (ii = 0; ii < 4; ii++)
              {
                loop_counter[565]++;
                for (jj = 0; jj < 4; jj++)
                {
                  loop_counter[566]++;
                  img->mpr[ii + ioff][jj + joff] = ((((tmp_block[ii][jj] * img->wp_weight[0][fw_ref_idx][0]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[0][fw_refframe >> curr_mb_field][0]) > img->max_imgpel_value ? img->max_imgpel_value : ((((tmp_block[ii][jj] * img->wp_weight[0][fw_ref_idx][0]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[0][fw_refframe >> curr_mb_field][0]) < 0 ? 0 : (((tmp_block[ii][jj] * img->wp_weight[0][fw_ref_idx][0]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[0][fw_refframe >> curr_mb_field][0];
                }

              }

            }
            else
            {
              for (ii = 0; ii < 4; ii++)
              {
                loop_counter[567]++;
                for (jj = 0; jj < 4; jj++)
                {
                  loop_counter[568]++;
                  img->mpr[ii + ioff][jj + joff] = tmp_block[ii][jj];
                }

              }

            }

          }
          else
            if (((mv_mode == 0) && img->direct_spatial_mv_pred_flag) && (direct_pdir == 1))
          {
            loop_counter[569]++;
            if (img->apply_weights)
            {
              loop_counter[570]++;
              if (((active_pps->weighted_pred_flag && ((img->type == P_SLICE) || (img->type == SP_SLICE))) || ((active_pps->weighted_bipred_idc == 1) && (img->type == B_SLICE))) && curr_mb_field)
              {
                loop_counter[571]++;
                fw_ref_idx >>= 1;
                bw_ref_idx >>= 1;
              }

              for (ii = 0; ii < 4; ii++)
              {
                loop_counter[572]++;
                for (jj = 0; jj < 4; jj++)
                {
                  loop_counter[573]++;
                  img->mpr[ii + ioff][jj + joff] = ((((tmp_blockbw[ii][jj] * img->wp_weight[1][bw_ref_idx][0]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[1][bw_refframe >> curr_mb_field][0]) > img->max_imgpel_value ? img->max_imgpel_value : ((((tmp_blockbw[ii][jj] * img->wp_weight[1][bw_ref_idx][0]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[1][bw_refframe >> curr_mb_field][0]) < 0 ? 0 : (((tmp_blockbw[ii][jj] * img->wp_weight[1][bw_ref_idx][0]) + img->wp_round_luma) >> img->luma_log2_weight_denom) + img->wp_offset[1][bw_refframe >> curr_mb_field][0];
                }

              }

            }
            else
            {
              for (ii = 0; ii < 4; ii++)
              {
                loop_counter[574]++;
                for (jj = 0; jj < 4; jj++)
                {
                  loop_counter[575]++;
                  img->mpr[ii + ioff][jj + joff] = tmp_blockbw[ii][jj];
                }

              }

            }

          }
          else
            if (img->apply_weights)
          {
            loop_counter[576]++;
            int alpha_fw;
            int alpha_bw;
            int wt_list_offset = active_pps->weighted_bipred_idc == 2 ? list_offset : 0;
            if ((mv_mode == 0) && (img->direct_spatial_mv_pred_flag == 0))
            {
              loop_counter[577]++;
              bw_ref_idx = 0;
            }

            if (((active_pps->weighted_pred_flag && ((img->type == P_SLICE) || (img->type == SP_SLICE))) || ((active_pps->weighted_bipred_idc == 1) && (img->type == B_SLICE))) && curr_mb_field)
            {
              loop_counter[578]++;
              fw_ref_idx >>= 1;
              bw_ref_idx >>= 1;
            }

            alpha_fw = img->wbp_weight[0 + wt_list_offset][fw_ref_idx][bw_ref_idx][0];
            alpha_bw = img->wbp_weight[1 + wt_list_offset][fw_ref_idx][bw_ref_idx][0];
            for (ii = 0; ii < 4; ii++)
            {
              loop_counter[579]++;
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[580]++;
                img->mpr[ii + ioff][jj + joff] = (int) ((((((alpha_fw * tmp_block[ii][jj]) + (alpha_bw * tmp_blockbw[ii][jj])) + (1 << img->luma_log2_weight_denom)) >> (img->luma_log2_weight_denom + 1)) + (((img->wp_offset[wt_list_offset + 0][fw_ref_idx][0] + img->wp_offset[wt_list_offset + 1][bw_ref_idx][0]) + 1) >> 1)) > img->max_imgpel_value ? img->max_imgpel_value : (((((alpha_fw * tmp_block[ii][jj]) + (alpha_bw * tmp_blockbw[ii][jj])) + (1 << img->luma_log2_weight_denom)) >> (img->luma_log2_weight_denom + 1)) + (((img->wp_offset[wt_list_offset + 0][fw_ref_idx][0] + img->wp_offset[wt_list_offset + 1][bw_ref_idx][0]) + 1) >> 1)) < 0 ? 0 : ((((alpha_fw * tmp_block[ii][jj]) + (alpha_bw * tmp_blockbw[ii][jj])) + (1 << img->luma_log2_weight_denom)) >> (img->luma_log2_weight_denom + 1)) + (((img->wp_offset[wt_list_offset + 0][fw_ref_idx][0] + img->wp_offset[wt_list_offset + 1][bw_ref_idx][0]) + 1) >> 1));
              }

            }

          }
          else
          {
            for (ii = 0; ii < 4; ii++)
            {
              loop_counter[581]++;
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[582]++;
                img->mpr[ii + ioff][jj + joff] = ((tmp_block[ii][jj] + tmp_blockbw[ii][jj]) + 1) / 2;
              }

            }

          }



        }

      }


      if (smb && (mv_mode != 11))
      {
        loop_counter[583]++;
        itrans_sp(img, ioff, joff, i, j);
      }
      else
      {
        if (need_4x4_transform)
        {
          loop_counter[584]++;
          itrans(img, ioff, joff, i, j, 0);
        }

      }

      if (need_4x4_transform)
      {
        loop_counter[585]++;
        for (ii = 0; ii < 4; ii++)
        {
          loop_counter[586]++;
          for (jj = 0; jj < 4; jj++)
          {
            loop_counter[587]++;
            if (!residue_transform_flag)
            {
              loop_counter[588]++;
              dec_picture->imgY[(j4 * 4) + jj][(i4 * 4) + ii] = img->m7[ii][jj];
            }
            else
            {
              mprRGB[0][ii + ioff][jj + joff] = img->mpr[ii + ioff][jj + joff];
              rec_res[0][ii + ioff][jj + joff] = img->m7[ii][jj];
            }

          }

        }

      }

    }

    if (!need_4x4_transform)
    {
      loop_counter[589]++;
      ioff = 8 * (block8x8 % 2);
      joff = 8 * (block8x8 / 2);
      itrans8x8(img, ioff, joff);
      for (ii = 0; ii < 8; ii++)
      {
        loop_counter[590]++;
        for (jj = 0; jj < 8; jj++)
        {
          loop_counter[591]++;
          if (!residue_transform_flag)
          {
            loop_counter[592]++;
            dec_picture->imgY[(img->pix_y + joff) + jj][(img->pix_x + ioff) + ii] = img->m7[ii + ioff][jj + joff];
          }
          else
          {
            mprRGB[0][ii + ioff][jj + joff] = img->mpr[ii + ioff][jj + joff];
            rec_res[0][ii + ioff][jj + joff] = img->m7[ii + ioff][jj + joff];
          }

        }

      }

    }

  }

  if (dec_picture->chroma_format_idc != 0)
  {
    loop_counter[593]++;
    f1_x = 64 / img->mb_cr_size_x;
    f2_x = f1_x - 1;
    f1_y = 64 / img->mb_cr_size_y;
    f2_y = f1_y - 1;
    f3 = f1_x * f1_y;
    f4 = f3 >> 1;
    for (uv = 0; uv < 2; uv++)
    {
      loop_counter[594]++;
      uv_shift = uv * (img->num_blk8x8_uv / 2);
      intra_prediction = ((((currMB->mb_type == 9) || (currMB->mb_type == 10)) || (currMB->mb_type == 14)) || (currMB->mb_type == 13)) || (currMB->mb_type == 12);
      if (intra_prediction)
      {
        loop_counter[595]++;
        intrapred_chroma(img, uv);
      }

      for (b8 = 0; b8 < (img->num_blk8x8_uv / 2); b8++)
      {
        loop_counter[596]++;
        for (b4 = 0; b4 < 4; b4++)
        {
          loop_counter[597]++;
          joff = subblk_offset_y[yuv][b8][b4];
          j4 = img->pix_c_y + joff;
          ioff = subblk_offset_x[yuv][b8][b4];
          i4 = img->pix_c_x + ioff;
          mv_mode = currMB->b8mode[block8x8_idx[yuv][b8][b4]];
          pred_dir = currMB->b8pdir[block8x8_idx[yuv][b8][b4]];
          assert(pred_dir <= 2);
          if (!intra_prediction)
          {
            loop_counter[598]++;
            if (pred_dir != 2)
            {
              loop_counter[599]++;
              mv_array = dec_picture->mv[LIST_0 + pred_dir];
              list = listX[(0 + list_offset) + pred_dir];
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[600]++;
                jf = (j4 + jj) / (img->mb_cr_size_y / 4);
                for (ii = 0; ii < 4; ii++)
                {
                  loop_counter[601]++;
                  ifx = (i4 + ii) / (img->mb_cr_size_x / 4);
                  fw_refframe = (ref_idx = dec_picture->ref_idx[LIST_0 + pred_dir][jf][ifx]);
                  i1 = ((i4 + ii) * f1_x) + mv_array[jf][ifx][0];
                  if (!curr_mb_field)
                  {
                    loop_counter[602]++;
                    j1 = ((j4 + jj) * f1_y) + mv_array[jf][ifx][1];
                  }
                  else
                  {
                    if ((mb_nr % 2) == 0)
                    {
                      loop_counter[603]++;
                      j1 = ((((img->pix_c_y / 2) + jj) + joff) * f1_y) + mv_array[jf][ifx][1];
                    }
                    else
                      j1 = (((((img->pix_c_y - img->mb_cr_size_y) / 2) + jj) + joff) * f1_y) + mv_array[jf][ifx][1];

                  }

                  j1 += list[ref_idx]->chroma_vector_adjustment;
                  ii0 = 0 > ((i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1) ? 0 : (i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1;
                  jj0 = 0 > ((j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr) ? 0 : (j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr;
                  ii1 = 0 > (((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1) ? 0 : ((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1;
                  jj1 = 0 > (((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr) ? 0 : ((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr;
                  if1 = i1 & f2_x;
                  jf1 = j1 & f2_y;
                  if0 = f1_x - if1;
                  jf0 = f1_y - jf1;
                  if (img->apply_weights)
                  {
                    loop_counter[604]++;
                    pred = ((((((if0 * jf0) * list[ref_idx]->imgUV[uv][jj0][ii0]) + ((if1 * jf0) * list[ref_idx]->imgUV[uv][jj0][ii1])) + ((if0 * jf1) * list[ref_idx]->imgUV[uv][jj1][ii0])) + ((if1 * jf1) * list[ref_idx]->imgUV[uv][jj1][ii1])) + f4) / f3;
                    if (((active_pps->weighted_pred_flag && ((img->type == P_SLICE) || (img->type == SP_SLICE))) || ((active_pps->weighted_bipred_idc == 1) && (img->type == B_SLICE))) && curr_mb_field)
                    {
                      loop_counter[605]++;
                      ref_idx >>= 1;
                    }

                    img->mpr[ii + ioff][jj + joff] = ((((img->wp_weight[pred_dir][ref_idx][uv + 1] * pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[pred_dir][ref_idx][uv + 1]) > img->max_imgpel_value_uv ? img->max_imgpel_value_uv : ((((img->wp_weight[pred_dir][ref_idx][uv + 1] * pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[pred_dir][ref_idx][uv + 1]) < 0 ? 0 : (((img->wp_weight[pred_dir][ref_idx][uv + 1] * pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[pred_dir][ref_idx][uv + 1];
                  }
                  else
                  {
                    img->mpr[ii + ioff][jj + joff] = ((((((if0 * jf0) * list[ref_idx]->imgUV[uv][jj0][ii0]) + ((if1 * jf0) * list[ref_idx]->imgUV[uv][jj0][ii1])) + ((if0 * jf1) * list[ref_idx]->imgUV[uv][jj1][ii0])) + ((if1 * jf1) * list[ref_idx]->imgUV[uv][jj1][ii1])) + f4) / f3;
                  }

                }

              }

            }
            else
            {
              fw_mv_array = dec_picture->mv[LIST_0];
              bw_mv_array = dec_picture->mv[LIST_1];
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[606]++;
                jf = (j4 + jj) / (img->mb_cr_size_y / 4);
                for (ii = 0; ii < 4; ii++)
                {
                  loop_counter[607]++;
                  ifx = (i4 + ii) / (img->mb_cr_size_x / 4);
                  direct_pdir = 2;
                  if ((mv_mode == 0) && img->direct_spatial_mv_pred_flag)
                  {
                    loop_counter[608]++;
                    if (dec_picture->ref_idx[LIST_0][2 * (jf / 2)][(ifx / 2) * 2] != (-1))
                    {
                      loop_counter[609]++;
                      fw_refframe = dec_picture->ref_idx[LIST_0][2 * (jf / 2)][(ifx / 2) * 2];
                      fw_ref_idx = fw_refframe;
                    }

                    if (dec_picture->ref_idx[LIST_1][2 * (jf / 2)][(ifx / 2) * 2] != (-1))
                    {
                      loop_counter[610]++;
                      bw_refframe = dec_picture->ref_idx[LIST_1][2 * (jf / 2)][(ifx / 2) * 2];
                      bw_ref_idx = bw_refframe;
                    }

                    if (dec_picture->ref_idx[LIST_1][2 * (jf / 2)][(ifx / 2) * 2] == (-1))
                    {
                      loop_counter[611]++;
                      direct_pdir = 0;
                    }
                    else
                      if (dec_picture->ref_idx[LIST_0][2 * (jf / 2)][(ifx / 2) * 2] == (-1))
                    {
                      loop_counter[612]++;
                      direct_pdir = 1;
                    }


                    if ((direct_pdir == 0) || (direct_pdir == 2))
                    {
                      loop_counter[613]++;
                      i1 = (((img->pix_c_x + ii) + ioff) * f1_x) + fw_mv_array[jf][ifx][0];
                      if (!curr_mb_field)
                      {
                        loop_counter[614]++;
                        j1 = (((img->pix_c_y + jj) + joff) * f1_y) + fw_mv_array[jf][ifx][1];
                      }
                      else
                      {
                        if ((mb_nr % 2) == 0)
                        {
                          loop_counter[615]++;
                          j1 = ((((img->pix_c_y / 2) + jj) + joff) * f1_y) + fw_mv_array[jf][ifx][1];
                        }
                        else
                          j1 = (((((img->pix_c_y - img->mb_cr_size_y) / 2) + jj) + joff) * f1_y) + fw_mv_array[jf][ifx][1];

                      }

                      j1 += listX[0 + list_offset][fw_refframe]->chroma_vector_adjustment;
                      ii0 = 0 > ((i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1) ? 0 : (i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1;
                      jj0 = 0 > ((j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr) ? 0 : (j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr;
                      ii1 = 0 > (((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1) ? 0 : ((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1;
                      jj1 = 0 > (((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr) ? 0 : ((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr;
                      if1 = i1 & f2_x;
                      jf1 = j1 & f2_y;
                      if0 = f1_x - if1;
                      jf0 = f1_y - jf1;
                      fw_pred = ((((((if0 * jf0) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj0][ii0]) + ((if1 * jf0) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj0][ii1])) + ((if0 * jf1) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj1][ii0])) + ((if1 * jf1) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj1][ii1])) + f4) / f3;
                    }

                    if ((direct_pdir == 1) || (direct_pdir == 2))
                    {
                      loop_counter[616]++;
                      i1 = (((img->pix_c_x + ii) + ioff) * f1_x) + bw_mv_array[jf][ifx][0];
                      if (!curr_mb_field)
                      {
                        loop_counter[617]++;
                        j1 = (((img->pix_c_y + jj) + joff) * f1_y) + bw_mv_array[jf][ifx][1];
                      }
                      else
                      {
                        if ((mb_nr % 2) == 0)
                        {
                          loop_counter[618]++;
                          j1 = ((((img->pix_c_y / 2) + jj) + joff) * f1_y) + bw_mv_array[jf][ifx][1];
                        }
                        else
                          j1 = (((((img->pix_c_y - img->mb_cr_size_y) / 2) + jj) + joff) * f1_y) + bw_mv_array[jf][ifx][1];

                      }

                      j1 += listX[1 + list_offset][bw_refframe]->chroma_vector_adjustment;
                      ii0 = 0 > ((i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1) ? 0 : (i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1;
                      jj0 = 0 > ((j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr) ? 0 : (j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr;
                      ii1 = 0 > (((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1) ? 0 : ((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1;
                      jj1 = 0 > (((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr) ? 0 : ((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr;
                      if1 = i1 & f2_x;
                      jf1 = j1 & f2_y;
                      if0 = f1_x - if1;
                      jf0 = f1_y - jf1;
                      bw_pred = ((((((if0 * jf0) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj0][ii0]) + ((if1 * jf0) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj0][ii1])) + ((if0 * jf1) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj1][ii0])) + ((if1 * jf1) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj1][ii1])) + f4) / f3;
                    }

                  }
                  else
                  {
                    fw_refframe = dec_picture->ref_idx[LIST_0][jf][ifx];
                    bw_refframe = dec_picture->ref_idx[LIST_1][jf][ifx];
                    fw_ref_idx = fw_refframe;
                    bw_ref_idx = bw_refframe;
                    i1 = (((img->pix_c_x + ii) + ioff) * f1_x) + fw_mv_array[jf][ifx][0];
                    if (!curr_mb_field)
                    {
                      loop_counter[619]++;
                      j1 = (((img->pix_c_y + jj) + joff) * f1_y) + fw_mv_array[jf][ifx][1];
                    }
                    else
                    {
                      if ((mb_nr % 2) == 0)
                      {
                        loop_counter[620]++;
                        j1 = ((((img->pix_c_y / 2) + jj) + joff) * f1_y) + fw_mv_array[jf][ifx][1];
                      }
                      else
                        j1 = (((((img->pix_c_y - img->mb_cr_size_y) / 2) + jj) + joff) * f1_y) + fw_mv_array[jf][ifx][1];

                    }

                    j1 += listX[0 + list_offset][fw_refframe]->chroma_vector_adjustment;
                    ii0 = 0 > ((i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1) ? 0 : (i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1;
                    jj0 = 0 > ((j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr) ? 0 : (j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr;
                    ii1 = 0 > (((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1) ? 0 : ((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1;
                    jj1 = 0 > (((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr) ? 0 : ((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr;
                    if1 = i1 & f2_x;
                    jf1 = j1 & f2_y;
                    if0 = f1_x - if1;
                    jf0 = f1_y - jf1;
                    fw_pred = ((((((if0 * jf0) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj0][ii0]) + ((if1 * jf0) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj0][ii1])) + ((if0 * jf1) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj1][ii0])) + ((if1 * jf1) * listX[0 + list_offset][fw_refframe]->imgUV[uv][jj1][ii1])) + f4) / f3;
                    i1 = (((img->pix_c_x + ii) + ioff) * f1_x) + bw_mv_array[jf][ifx][0];
                    if (!curr_mb_field)
                    {
                      loop_counter[621]++;
                      j1 = (((img->pix_c_y + jj) + joff) * f1_y) + bw_mv_array[jf][ifx][1];
                    }
                    else
                    {
                      if ((mb_nr % 2) == 0)
                      {
                        loop_counter[622]++;
                        j1 = ((((img->pix_c_y / 2) + jj) + joff) * f1_y) + bw_mv_array[jf][ifx][1];
                      }
                      else
                        j1 = (((((img->pix_c_y - img->mb_cr_size_y) / 2) + jj) + joff) * f1_y) + bw_mv_array[jf][ifx][1];

                    }

                    j1 += listX[1 + list_offset][bw_refframe]->chroma_vector_adjustment;
                    ii0 = 0 > ((i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1) ? 0 : (i1 / f1_x) < (img->width_cr - 1) ? i1 / f1_x : img->width_cr - 1;
                    jj0 = 0 > ((j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr) ? 0 : (j1 / f1_y) < max_y_cr ? j1 / f1_y : max_y_cr;
                    ii1 = 0 > (((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1) ? 0 : ((i1 + f2_x) / f1_x) < (img->width_cr - 1) ? (i1 + f2_x) / f1_x : img->width_cr - 1;
                    jj1 = 0 > (((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr) ? 0 : ((j1 + f2_y) / f1_y) < max_y_cr ? (j1 + f2_y) / f1_y : max_y_cr;
                    if1 = i1 & f2_x;
                    jf1 = j1 & f2_y;
                    if0 = f1_x - if1;
                    jf0 = f1_y - jf1;
                    bw_pred = ((((((if0 * jf0) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj0][ii0]) + ((if1 * jf0) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj0][ii1])) + ((if0 * jf1) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj1][ii0])) + ((if1 * jf1) * listX[1 + list_offset][bw_refframe]->imgUV[uv][jj1][ii1])) + f4) / f3;
                  }

                  if (img->apply_weights)
                  {
                    loop_counter[623]++;
                    if (((active_pps->weighted_pred_flag && ((img->type == P_SLICE) || (img->type == SP_SLICE))) || ((active_pps->weighted_bipred_idc == 1) && (img->type == B_SLICE))) && curr_mb_field)
                    {
                      loop_counter[624]++;
                      fw_ref_idx >>= 1;
                      bw_ref_idx >>= 1;
                    }

                    if (img->direct_spatial_mv_pred_flag && (direct_pdir == 1))
                    {
                      loop_counter[625]++;
                      img->mpr[ii + ioff][jj + joff] = ((((img->wp_weight[1][bw_ref_idx][uv + 1] * bw_pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[1][bw_refframe >> curr_mb_field][uv + 1]) > img->max_imgpel_value_uv ? img->max_imgpel_value_uv : ((((img->wp_weight[1][bw_ref_idx][uv + 1] * bw_pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[1][bw_refframe >> curr_mb_field][uv + 1]) < 0 ? 0 : (((img->wp_weight[1][bw_ref_idx][uv + 1] * bw_pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[1][bw_refframe >> curr_mb_field][uv + 1];
                    }
                    else
                      if (img->direct_spatial_mv_pred_flag && (direct_pdir == 0))
                    {
                      loop_counter[626]++;
                      img->mpr[ii + ioff][jj + joff] = ((((img->wp_weight[0][fw_ref_idx][uv + 1] * fw_pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[0][fw_refframe >> curr_mb_field][uv + 1]) > img->max_imgpel_value_uv ? img->max_imgpel_value_uv : ((((img->wp_weight[0][fw_ref_idx][uv + 1] * fw_pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[0][fw_refframe >> curr_mb_field][uv + 1]) < 0 ? 0 : (((img->wp_weight[0][fw_ref_idx][uv + 1] * fw_pred) + img->wp_round_chroma) >> img->chroma_log2_weight_denom) + img->wp_offset[0][fw_refframe >> curr_mb_field][uv + 1];
                    }
                    else
                    {
                      int wt_list_offset = active_pps->weighted_bipred_idc == 2 ? list_offset : 0;
                      int alpha_fw = img->wbp_weight[0 + wt_list_offset][fw_ref_idx][bw_ref_idx][uv + 1];
                      int alpha_bw = img->wbp_weight[1 + wt_list_offset][fw_ref_idx][bw_ref_idx][uv + 1];
                      img->mpr[ii + ioff][jj + joff] = (((((alpha_fw * fw_pred) + (alpha_bw * bw_pred)) + (1 << img->chroma_log2_weight_denom)) >> (img->chroma_log2_weight_denom + 1)) + (((img->wp_offset[wt_list_offset + 0][fw_ref_idx][uv + 1] + img->wp_offset[wt_list_offset + 1][bw_ref_idx][uv + 1]) + 1) >> 1)) > img->max_imgpel_value_uv ? img->max_imgpel_value_uv : (((((alpha_fw * fw_pred) + (alpha_bw * bw_pred)) + (1 << img->chroma_log2_weight_denom)) >> (img->chroma_log2_weight_denom + 1)) + (((img->wp_offset[wt_list_offset + 0][fw_ref_idx][uv + 1] + img->wp_offset[wt_list_offset + 1][bw_ref_idx][uv + 1]) + 1) >> 1)) < 0 ? 0 : ((((alpha_fw * fw_pred) + (alpha_bw * bw_pred)) + (1 << img->chroma_log2_weight_denom)) >> (img->chroma_log2_weight_denom + 1)) + (((img->wp_offset[wt_list_offset + 0][fw_ref_idx][uv + 1] + img->wp_offset[wt_list_offset + 1][bw_ref_idx][uv + 1]) + 1) >> 1);
                    }


                  }
                  else
                  {
                    if (img->direct_spatial_mv_pred_flag && (direct_pdir == 1))
                    {
                      loop_counter[627]++;
                      img->mpr[ii + ioff][jj + joff] = bw_pred;
                    }
                    else
                      if (img->direct_spatial_mv_pred_flag && (direct_pdir == 0))
                    {
                      loop_counter[628]++;
                      img->mpr[ii + ioff][jj + joff] = fw_pred;
                    }
                    else
                    {
                      img->mpr[ii + ioff][jj + joff] = ((fw_pred + bw_pred) + 1) / 2;
                    }


                  }

                }

              }

            }

          }

          if (!smb)
          {
            loop_counter[629]++;
            itrans(img, ioff, joff, cofuv_blk_x[yuv][b8 + uv_shift][b4], cofuv_blk_y[yuv][b8 + uv_shift][b4], 1);
            for (ii = 0; ii < 4; ii++)
            {
              loop_counter[630]++;
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[631]++;
                if (!residue_transform_flag)
                {
                  loop_counter[632]++;
                  dec_picture->imgUV[uv][j4 + jj][i4 + ii] = img->m7[ii][jj];
                }
                else
                {
                  mprRGB[uv + 1][ii + ioff][jj + joff] = img->mpr[ii + ioff][jj + joff];
                  rec_res[uv + 1][ii + ioff][jj + joff] = img->m7[ii][jj];
                }

              }

            }

          }

        }

      }

      if (smb)
      {
        loop_counter[633]++;
        itrans_sp_chroma(img, 2 * uv);
        for (j = 4; j < 6; j++)
        {
          loop_counter[634]++;
          joff = (j - 4) * 4;
          j4 = img->pix_c_y + joff;
          for (i = 0; i < 2; i++)
          {
            loop_counter[635]++;
            ioff = i * 4;
            i4 = img->pix_c_x + ioff;
            itrans(img, ioff, joff, (2 * uv) + i, j, 1);
            for (ii = 0; ii < 4; ii++)
            {
              loop_counter[636]++;
              for (jj = 0; jj < 4; jj++)
              {
                loop_counter[637]++;
                dec_picture->imgUV[uv][j4 + jj][i4 + ii] = img->m7[ii][jj];
              }

            }

          }

        }

      }

    }

  }

  if (residue_transform_flag)
  {
    loop_counter[638]++;
    if (currMB->mb_type != 13)
    {
      loop_counter[639]++;
      for (k = 0; k < 16; k++)
      {
        loop_counter[640]++;
        i = decode_block_scan[k] & 3;
        j = (decode_block_scan[k] >> 2) & 3;
        ioff = i * 4;
        i4 = img->block_x + i;
        joff = j * 4;
        j4 = img->block_y + j;
        mv_mode = currMB->b8mode[(2 * (j / 2)) + (i / 2)];
        pred_dir = currMB->b8pdir[(2 * (j / 2)) + (i / 2)];
        assert(pred_dir <= 2);
        if (mv_mode == 11)
        {
          loop_counter[641]++;
          if (intrapred(img, ioff, joff, i4, j4) == 1)
          {
            loop_counter[642]++;
            return 1;
          }

          for (ii = 0; ii < 4; ii++)
          {
            loop_counter[643]++;
            for (jj = 0; jj < 4; jj++)
            {
              loop_counter[644]++;
              mprRGB[0][ii + ioff][jj + joff] = img->mpr[ii + ioff][jj + joff];
            }

          }

        }

        for (jj = 0; jj < 4; jj++)
        {
          loop_counter[645]++;
          for (ii = 0; ii < 4; ii++)
          {
            loop_counter[646]++;
            temp = rec_res[0][ii + ioff][jj + joff] - (rec_res[2][ii + ioff][jj + joff] >> 1);
            residue_G = rec_res[2][ii + ioff][jj + joff] + temp;
            residue_B = temp - (rec_res[1][ii + ioff][jj + joff] >> 1);
            residue_R = residue_B + rec_res[1][ii + ioff][jj + joff];
            dec_picture->imgUV[0][(j4 * 4) + jj][(i4 * 4) + ii] = img->max_imgpel_value_uv < (0 > (residue_B + mprRGB[1][ii + ioff][jj + joff]) ? 0 : residue_B + mprRGB[1][ii + ioff][jj + joff]) ? img->max_imgpel_value_uv : 0 > (residue_B + mprRGB[1][ii + ioff][jj + joff]) ? 0 : residue_B + mprRGB[1][ii + ioff][jj + joff];
            dec_picture->imgY[(j4 * 4) + jj][(i4 * 4) + ii] = img->max_imgpel_value < (0 > (residue_G + mprRGB[0][ii + ioff][jj + joff]) ? 0 : residue_G + mprRGB[0][ii + ioff][jj + joff]) ? img->max_imgpel_value : 0 > (residue_G + mprRGB[0][ii + ioff][jj + joff]) ? 0 : residue_G + mprRGB[0][ii + ioff][jj + joff];
            dec_picture->imgUV[1][(j4 * 4) + jj][(i4 * 4) + ii] = img->max_imgpel_value_uv < (0 > (residue_R + mprRGB[2][ii + ioff][jj + joff]) ? 0 : residue_R + mprRGB[2][ii + ioff][jj + joff]) ? img->max_imgpel_value_uv : 0 > (residue_R + mprRGB[2][ii + ioff][jj + joff]) ? 0 : residue_R + mprRGB[2][ii + ioff][jj + joff];
          }

        }

      }

    }
    else
    {
      for (block8x8 = 0; block8x8 < 4; block8x8++)
      {
        loop_counter[647]++;
        ioff = 8 * (block8x8 % 2);
        joff = 8 * (block8x8 / 2);
        intrapred8x8(img, block8x8);
        for (ii = 0; ii < 8; ii++)
        {
          loop_counter[648]++;
          for (jj = 0; jj < 8; jj++)
          {
            loop_counter[649]++;
            mprRGB[0][ii + ioff][jj + joff] = img->mpr[ii + ioff][jj + joff];
          }

        }

        for (jj = 0; jj < 8; jj++)
        {
          loop_counter[650]++;
          for (ii = 0; ii < 8; ii++)
          {
            loop_counter[651]++;
            temp = rec_res[0][ii + ioff][jj + joff] - (rec_res[2][ii + ioff][jj + joff] >> 1);
            residue_G = rec_res[2][ii + ioff][jj + joff] + temp;
            residue_B = temp - (rec_res[1][ii + ioff][jj + joff] >> 1);
            residue_R = residue_B + rec_res[1][ii + ioff][jj + joff];
            dec_picture->imgUV[0][(img->pix_y + joff) + jj][(img->pix_x + ioff) + ii] = img->max_imgpel_value_uv < (0 > (residue_B + mprRGB[1][ii + ioff][jj + joff]) ? 0 : residue_B + mprRGB[1][ii + ioff][jj + joff]) ? img->max_imgpel_value_uv : 0 > (residue_B + mprRGB[1][ii + ioff][jj + joff]) ? 0 : residue_B + mprRGB[1][ii + ioff][jj + joff];
            dec_picture->imgY[(img->pix_y + joff) + jj][(img->pix_x + ioff) + ii] = img->max_imgpel_value < (0 > (residue_G + mprRGB[0][ii + ioff][jj + joff]) ? 0 : residue_G + mprRGB[0][ii + ioff][jj + joff]) ? img->max_imgpel_value : 0 > (residue_G + mprRGB[0][ii + ioff][jj + joff]) ? 0 : residue_G + mprRGB[0][ii + ioff][jj + joff];
            dec_picture->imgUV[1][(img->pix_y + joff) + jj][(img->pix_x + ioff) + ii] = img->max_imgpel_value_uv < (0 > (residue_R + mprRGB[2][ii + ioff][jj + joff]) ? 0 : residue_R + mprRGB[2][ii + ioff][jj + joff]) ? img->max_imgpel_value_uv : 0 > (residue_R + mprRGB[2][ii + ioff][jj + joff]) ? 0 : residue_R + mprRGB[2][ii + ioff][jj + joff];
          }

        }

      }

    }

  }

  return 0;
}


