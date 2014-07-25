typedef int PixelPos;
typedef int Macroblock;
typedef int pel_t;

/*!
 ***********************************************************************
 * \brief
 *    Setup the fast search for an macroblock
 ***********************************************************************
 */
void SetupFastFullPelSearch (short ref, int list)  // <--  reference frame parameter, list0 or 1
{
  short   pmv[2];
  pel_t   orig_blocks[256], *orgptr=orig_blocks, *refptr;
  int     offset_x, offset_y, x, y, range_partly_outside, ref_x, ref_y, pos, abs_x, abs_y, bindex, blky;
  int     LineSadBlk0, LineSadBlk1, LineSadBlk2, LineSadBlk3;
  int     max_width, max_height;
  int     img_width, img_height;

  StorablePicture *ref_picture;
  pel_t   *ref_pic;

  int**   block_sad     = BlockSAD[list][ref][7];
  int     search_range  = max_search_range[list][ref];
  int     max_pos       = (2*search_range+1) * (2*search_range+1);

  int     list_offset   = ((img->MbaffFrameFlag)&&(img->mb_data[img->current_mb_nr].mb_field))? img->current_mb_nr%2 ? 4 : 2 : 0;

  int     apply_weights = ( (active_pps->weighted_pred_flag && (img->type == P_SLICE || img->type == SP_SLICE)) ||
                            (active_pps->weighted_bipred_idc && (img->type == B_SLICE)));

  
  ref_picture     = listX[list+list_offset][ref];

  // dlo: metrics
#ifdef METRICS
  /*
  // These metrics were found and put in by hand
  printf("dlo: metrics = ( \
    %d, %d, %d, %d, %d, \
    %d, %d, %d, %d, %d, \
    %d, %d, %d, %d, %d, \
    %d, %d, %d, %d, %d, \
    %d, %d, %d, %d)\n", 
      // if and for loops 
      apply_weights, 
      input->UseWeightedReferenceME, 
      input->rdopt, 
      img->opix_y, 
      img->opix_x, 
      offset_x, 
      offset_y, 
      search_range, 
      max_width, 
      max_height,
      max_pos,
      ref_x, 
      ref_y,
      range_partly_outside,
      abs_y, 
      abs_x,
      // from functions 
      pmv,
      enc_picture->ref_idx,
      enc_picture->mv,
      ref,
      list,
      ref_pic,
      img_height,
      img_width
      );
  */

  /*
  printf("auto metrics = (");
  printf("%d, ", (int)apply_weights);
  printf("%d, ", (int)input->UseWeightedReferenceME);
  printf("%d, ", (int)input->rdopt);
  printf("%d, ", (int)img->opix_x);
  printf("%d, ", (int)x);
  printf("%d, ", (int)img->opix_y);
  printf("%d, ", (int)y);
  printf("%d, ", (int)offset_x);
  printf("%d, ", (int)search_range);
  printf("%d, ", (int)max_width);
  printf("%d, ", (int)offset_y);
  printf("%d, ", (int)max_height);
  printf("%d, ", (int)range_partly_outside);
  printf("%d, ", (int)blky);
  printf("%d, ", (int)pos);
  printf("%d, ", (int)max_pos);
  printf("%d, ", (int)abs_y);
  printf("%d, ", (int)abs_x);
  printf("%d, ", (int)spiral_search_x);
  printf("%d, ", (int)ref_x);
  printf("%d, ", (int)spiral_search_y);
  printf("%d, ", (int)ref_y);
  printf("%d, ", (int)img->MbaffFrameFlag);
  //printf("%d, ", (int)(img->mb_data + (img->current_mb_nr * 632))->mb_field);
  printf("%d, ", (int)img->current_mb_nr%20);
  printf("%d, ", (int)pmv);
  printf("%d, ", (int)enc_picture->ref_idx);
  printf("%d, ", (int)enc_picture->mv);
  printf("%d, ", (int)ref);
  printf("%d, ", (int)list);
  printf("%d, ", (int)max_pos);
  printf("%d, ", (int)PelYline_11);
  printf("%d, ", (int)ref_pic);
  printf("%d, ", (int)abs_y);
  printf("%d, ", (int)abs_x);
  printf("%d, ", (int)img_height);
  printf("%d, ", (int)img_width);
  */
  /*
  printf(")\n");
  */

  printf("\n");
  printf("mantis metrics = (");

  int loop_count = 0;

  // dlo: time this function
  start_timing();
#endif

#ifdef METRICS_SLICE
  printf("\n");

  /*
  // Calculate slice metrics
  printf("slice");
  start_timing();
  int blockshape_x = 16;
  int sm[4];
  sm[0] = slice_metrics(-1,  0);
  sm[1] = slice_metrics( 0, -1);
  sm[2] = slice_metrics(blockshape_x, -1);
  sm[3] = slice_metrics(-1, -1);
  stop_timing();

  // Print out metrics
  printf("slice metrics = (");
  printf("%d, ", sm[0]);
  printf("%d, ", sm[1]);
  printf("%d, ", sm[2]);
  printf("%d, ", sm[3]);
  printf(")\n");
  */

  int slice_metric = SetupFullFastPelSearch_slice_auto(ref, list);
  printf("slice metrics = (%d)\n", slice_metric);

  // dlo: time this function
  start_timing();
#endif

  //===== Use weighted Reference for ME ====
  if (apply_weights && input->UseWeightedReferenceME) {
    ref_pic       = ref_picture->imgY_11_w;
#ifdef METRICS
    printf("1, "); // 0
#endif
  } else {
    ref_pic       = ref_picture->imgY_11;
#ifdef METRICS
    printf("0, "); // 0
#endif
  }

  max_width     = ref_picture->size_x - 17;
  max_height    = ref_picture->size_y - 17;
  
  img_width     = ref_picture->size_x;
  img_height    = ref_picture->size_y;

  //===== get search center: predictor of 16x16 block =====
#ifdef METRICS_R1
  SetMotionVectorPredictor_print_metrics (pmv, enc_picture->ref_idx, enc_picture->mv, ref, list, 0, 0, 16, 16);
#else
  SetMotionVectorPredictor (pmv, enc_picture->ref_idx, enc_picture->mv, ref, list, 0, 0, 16, 16);
#endif
  search_center_x[list][ref] = pmv[0] / 4;
  search_center_y[list][ref] = pmv[1] / 4;

  if (!input->rdopt)
  {
    //--- correct center so that (0,0) vector is inside ---
    search_center_x[list][ref] = max(-search_range, min(search_range, search_center_x[list][ref]));
    search_center_y[list][ref] = max(-search_range, min(search_range, search_center_y[list][ref]));
#ifdef METRICS
    printf("1, "); // 1
#endif
  }
  else {
#ifdef METRICS
    printf("0, "); // 1
#endif
  }

  search_center_x[list][ref] += img->opix_x;
  search_center_y[list][ref] += img->opix_y;

  offset_x = search_center_x[list][ref];
  offset_y = search_center_y[list][ref];

  //===== copy original block for fast access =====
#ifdef METRICS
  loop_count = 0;
#endif
  for   (y = img->opix_y; y < img->opix_y+16; y++) {
    for (x = img->opix_x; x < img->opix_x+16; x++) {
      *orgptr++ = imgY_org [y][x];
#ifdef METRICS
      loop_count++;
#endif
    }
  }
#ifdef METRICS
  printf("%d, ", loop_count); // 2
#endif


  //===== check if whole search range is inside image =====
  if (offset_x >= search_range && offset_x <= max_width  - search_range &&
      offset_y >= search_range && offset_y <= max_height - search_range   )
  {
    range_partly_outside = 0; PelYline_11 = FastLine16Y_11;
#ifdef METRICS
    printf("1, "); // 3
#endif
  }
  else
  {
    range_partly_outside = 1;
#ifdef METRICS
    printf("0, "); // 3
#endif
  }

  //===== determine position of (0,0)-vector =====
#ifdef METRICS
  loop_count = 0;
#endif
  if (!input->rdopt)
  {
    ref_x = img->opix_x - offset_x;
    ref_y = img->opix_y - offset_y;
#ifdef METRICS
    printf("1, "); // 4
#endif

    for (pos = 0; pos < max_pos; pos++)
    {
      if (ref_x == spiral_search_x[pos] &&
          ref_y == spiral_search_y[pos])
      {
        pos_00[list][ref] = pos;
        break;
      }
#ifdef METRICS
      loop_count++;
#endif
    }
  }
#ifdef METRICS
    printf("0, "); // 4
    printf("%d, ", loop_count); // 5
#endif

  //===== loop over search range (spiral search): get blockwise SAD =====
#ifdef METRICS
  loop_count = 0;
  int loop_if_count = 0;
  int loop_if2_count = 0;
  int loop2_count = 0;
  int loop3_count = 0;
#endif
  for (pos = 0; pos < max_pos; pos++)
  {
#ifdef METRICS
    loop_count ++;
#endif
    abs_y = offset_y + spiral_search_y[pos];
    abs_x = offset_x + spiral_search_x[pos];

    if (range_partly_outside)
    {
#ifdef METRICS
      loop_if_count++;
#endif
      if (abs_y >= 0 && abs_y <= max_height &&
          abs_x >= 0 && abs_x <= max_width    )
      {
#ifdef METRICS
        loop_if2_count++;
#endif
        PelYline_11 = FastLine16Y_11;
      }
      else
      {
        PelYline_11 = UMVLine16Y_11;
      }
    }

    orgptr = orig_blocks;
    bindex = 0;
    for (blky = 0; blky < 4; blky++)
    {
#ifdef METRICS
      loop2_count++;
#endif
      LineSadBlk0 = LineSadBlk1 = LineSadBlk2 = LineSadBlk3 = 0;
      for (y = 0; y < 4; y++)
      {
#ifdef METRICS
        loop3_count++;
#endif
        refptr = PelYline_11 (ref_pic, abs_y++, abs_x, img_height, img_width);

        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk0 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk1 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk2 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
        LineSadBlk3 += byte_abs [*refptr++ - *orgptr++];
      }
      block_sad[bindex++][pos] = LineSadBlk0;
      block_sad[bindex++][pos] = LineSadBlk1;
      block_sad[bindex++][pos] = LineSadBlk2;
      block_sad[bindex++][pos] = LineSadBlk3;
    }
  }

#ifdef METRICS
  printf("%d, %d, %d, %d, %d, ", loop_count, loop_if_count, loop_if2_count, loop2_count, loop3_count); // 6 - 10
#endif


  //===== combine SAD's for larger block types =====
  SetupLargerBlocks (list, ref, max_pos);
#ifdef METRICS_R1
  printf("%d, ", max_pos); // 11
#endif


  //===== set flag marking that search setup have been done =====
  search_setup_done[list][ref] = 1;

  // dlo time this function
#ifdef METRICS
  printf(")\n");
  stop_timing();
#endif
#ifdef METRICS_SLICE
  printf("actual metrics = (%d)\n", LineSadBlk0);
#include <assert.h>
  if (slice_metric != LineSadBlk0) {
    printf("slice metric != acutal metric\n");
    exit(1);
  }
  stop_timing();
#endif
}

/*!
 ************************************************************************
 * \brief
 *    Set motion vector predictor
 ************************************************************************
 */
void SetMotionVectorPredictor (short  pmv[2],
                               short  ***refPic,
                               short  ****tmp_mv,
                               short  ref_frame,
                               int    list,
                               int    block_x,
                               int    block_y,
                               int    blockshape_x,
                               int    blockshape_y)
{
  int mb_x                 = 4*block_x;
  int mb_y                 = 4*block_y;
  int mb_nr                = img->current_mb_nr;

  int mv_a, mv_b, mv_c, pred_vec=0;
  int mvPredType, rFrameL, rFrameU, rFrameUR;
  int hv;

  PixelPos block_a, block_b, block_c, block_d;

  int SAD_a=0, SAD_b=0, SAD_c=0, SAD_d=0;
  int temp_pred_SAD[2];

  if (input->FMEnable) pred_SAD_space=0;

  getLuma4x4Neighbour(mb_nr, block_x, block_y,           -1,  0, &block_a);
  getLuma4x4Neighbour(mb_nr, block_x, block_y,            0, -1, &block_b);
  getLuma4x4Neighbour(mb_nr, block_x, block_y, blockshape_x, -1, &block_c);
  getLuma4x4Neighbour(mb_nr, block_x, block_y,           -1, -1, &block_d);

  if (mb_y > 0)
  {
    if (mb_x < 8)  // first column of 8x8 blocks
    {
      if (mb_y==8)
      {
        if (blockshape_x == 16)      block_c.available  = 0;
        else                         block_c.available &= 1;
      }
      else
      {
        if (mb_x+blockshape_x != 8)  block_c.available &= 1;
        else                         block_c.available  = 0;
      }
    }
    else
    {
      if (mb_x+blockshape_x != 16)   block_c.available &= 1;
      else                           block_c.available  = 0;
    }
  }

  if (!block_c.available)
  {
    block_c=block_d;
  }

  mvPredType = MVPRED_MEDIAN;

  if (!img->MbaffFrameFlag)
  {
    rFrameL    = block_a.available    ? refPic[list][block_a.pos_x][block_a.pos_y] : -1;
    rFrameU    = block_b.available    ? refPic[list][block_b.pos_x][block_b.pos_y] : -1;
    rFrameUR   = block_c.available    ? refPic[list][block_c.pos_x][block_c.pos_y] : -1;
  }
  else
  {
    if (img->mb_data[img->current_mb_nr].mb_field)
    {
      rFrameL    = block_a.available    ? 
        img->mb_data[block_a.mb_addr].mb_field ? 
        refPic[list][block_a.pos_x][block_a.pos_y]:
        refPic[list][block_a.pos_x][block_a.pos_y] * 2: 
        -1;
      rFrameU    = block_b.available    ? 
        img->mb_data[block_b.mb_addr].mb_field ? 
        refPic[list][block_b.pos_x][block_b.pos_y]:
        refPic[list][block_b.pos_x][block_b.pos_y] * 2: 
        -1;
      rFrameUR    = block_c.available    ? 
        img->mb_data[block_c.mb_addr].mb_field ? 
        refPic[list][block_c.pos_x][block_c.pos_y]:
        refPic[list][block_c.pos_x][block_c.pos_y] * 2: 
        -1;
    }
    else
    {
      rFrameL    = block_a.available    ? 
        img->mb_data[block_a.mb_addr].mb_field ? 
        refPic[list][block_a.pos_x][block_a.pos_y] >>1:
        refPic[list][block_a.pos_x][block_a.pos_y] : 
        -1;
      rFrameU    = block_b.available    ? 
        img->mb_data[block_b.mb_addr].mb_field ? 
        refPic[list][block_b.pos_x][block_b.pos_y] >>1:
        refPic[list][block_b.pos_x][block_b.pos_y] : 
        -1;
      rFrameUR    = block_c.available    ? 
        img->mb_data[block_c.mb_addr].mb_field ? 
        refPic[list][block_c.pos_x][block_c.pos_y] >>1:
        refPic[list][block_c.pos_x][block_c.pos_y] : 
        -1;
    }
  }

  /* Prediction if only one of the neighbors uses the reference frame
   * we are checking
   */
  if(rFrameL == ref_frame && rFrameU != ref_frame && rFrameUR != ref_frame)       mvPredType = MVPRED_L;
  else if(rFrameL != ref_frame && rFrameU == ref_frame && rFrameUR != ref_frame)  mvPredType = MVPRED_U;
  else if(rFrameL != ref_frame && rFrameU != ref_frame && rFrameUR == ref_frame)  mvPredType = MVPRED_UR;
  // Directional predictions 
  if(blockshape_x == 8 && blockshape_y == 16)
  {
    if(mb_x == 0)
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
    else
    {
      if( rFrameUR == ref_frame)
        mvPredType = MVPRED_UR;
    }
  }
  else if(blockshape_x == 16 && blockshape_y == 8)
  {
    if(mb_y == 0)
    {
      if(rFrameU == ref_frame)
        mvPredType = MVPRED_U;
    }
    else
    {
      if(rFrameL == ref_frame)
        mvPredType = MVPRED_L;
    }
  }

  for (hv=0; hv < 2; hv++)
  {
    if (!img->MbaffFrameFlag || hv==0)
    {
      mv_a = block_a.available  ? tmp_mv[list][block_a.pos_x][block_a.pos_y][hv] : 0;
      mv_b = block_b.available  ? tmp_mv[list][block_b.pos_x][block_b.pos_y][hv] : 0;
      mv_c = block_c.available  ? tmp_mv[list][block_c.pos_x][block_c.pos_y][hv] : 0;
    }
    else
    {
      if (img->mb_data[img->current_mb_nr].mb_field)
      {
        mv_a = block_a.available  ? img->mb_data[block_a.mb_addr].mb_field?
          tmp_mv[list][block_a.pos_x][block_a.pos_y][hv]:
          tmp_mv[list][block_a.pos_x][block_a.pos_y][hv] / 2: 
          0;
        mv_b = block_b.available  ? img->mb_data[block_b.mb_addr].mb_field?
          tmp_mv[list][block_b.pos_x][block_b.pos_y][hv]:
          tmp_mv[list][block_b.pos_x][block_b.pos_y][hv] / 2: 
          0;
        mv_c = block_c.available  ? img->mb_data[block_c.mb_addr].mb_field?
          tmp_mv[list][block_c.pos_x][block_c.pos_y][hv]:
          tmp_mv[list][block_c.pos_x][block_c.pos_y][hv] / 2: 
          0;
      }
      else
      {
        mv_a = block_a.available  ? img->mb_data[block_a.mb_addr].mb_field?
          tmp_mv[list][block_a.pos_x][block_a.pos_y][hv] * 2:
          tmp_mv[list][block_a.pos_x][block_a.pos_y][hv]: 
          0;
        mv_b = block_b.available  ? img->mb_data[block_b.mb_addr].mb_field?
          tmp_mv[list][block_b.pos_x][block_b.pos_y][hv] * 2:
          tmp_mv[list][block_b.pos_x][block_b.pos_y][hv]: 
          0;
        mv_c = block_c.available  ? img->mb_data[block_c.mb_addr].mb_field?
          tmp_mv[list][block_c.pos_x][block_c.pos_y][hv] * 2:
          tmp_mv[list][block_c.pos_x][block_c.pos_y][hv]: 
          0;
      }
    }

  if(input->FMEnable)
  {
    SAD_a = block_a.available ? ((list==1) ? all_bwmincost[block_a.pos_x][block_a.pos_y][0][FME_blocktype][0]:all_mincost[block_a.pos_x][block_a.pos_y][ref_frame][FME_blocktype][0]):0;
    SAD_b = block_b.available ? ((list==1) ? all_bwmincost[block_b.pos_x][block_b.pos_y][0][FME_blocktype][0]:all_mincost[block_b.pos_x][block_b.pos_y][ref_frame][FME_blocktype][0]):0;
    SAD_d = block_d.available ? ((list==1) ? all_bwmincost[block_d.pos_x][block_d.pos_y][0][FME_blocktype][0]:all_mincost[block_d.pos_x][block_d.pos_y][ref_frame][FME_blocktype][0]):0;
    SAD_c = block_c.available ? ((list==1) ? all_bwmincost[block_c.pos_x][block_c.pos_y][0][FME_blocktype][0]:all_mincost[block_c.pos_x][block_c.pos_y][ref_frame][FME_blocktype][0]):SAD_d;
  }

    switch (mvPredType)
    {
    case MVPRED_MEDIAN:
      if(!(block_b.available || block_c.available))
      {
        pred_vec = mv_a;
        if(input->FMEnable) temp_pred_SAD[hv] = SAD_a;
      }
      else
      {
        pred_vec = mv_a+mv_b+mv_c-min(mv_a,min(mv_b,mv_c))-max(mv_a,max(mv_b,mv_c));
      }
      if(input->FMEnable)
      {
         if (pred_vec == mv_a && SAD_a != 0) temp_pred_SAD[hv] = SAD_a;
         else if (pred_vec == mv_b && SAD_b!=0) temp_pred_SAD[hv] = SAD_b;
              else temp_pred_SAD[hv] = SAD_c;
      }
      break;
    case MVPRED_L:
      pred_vec = mv_a;
      if(input->FMEnable) temp_pred_SAD[hv] = SAD_a;
      break;
    case MVPRED_U:
      pred_vec = mv_b;
      if(input->FMEnable) temp_pred_SAD[hv] = SAD_b;
      break;
    case MVPRED_UR:
      pred_vec = mv_c;
      if(input->FMEnable) temp_pred_SAD[hv] = SAD_c;
      break;
    default:
      break;
    }

    pmv[hv] = pred_vec;
    
  }

  if(input->FMEnable) pred_SAD_space = temp_pred_SAD[0]>temp_pred_SAD[1]?temp_pred_SAD[1]:temp_pred_SAD[0];
}

void getLuma4x4Neighbour (int curr_mb_nr, int block_x, int block_y, int rel_x, int rel_y, PixelPos *pix)
{
  int x = 4* block_x + rel_x;
  int y = 4* block_y + rel_y;

  getNeighbour(curr_mb_nr, x, y, 1, pix);

  if (pix->available)
  {
    pix->x /= 4;
    pix->y /= 4;
    pix->pos_x /= 4;
    pix->pos_y /= 4;
  }
}

void getNeighbour(int curr_mb_nr, int xN, int yN, int luma, PixelPos *pix)
{
  if (curr_mb_nr<0)
    error ("getNeighbour: invalid macroblock number", 100);

  if (img->MbaffFrameFlag) {
    getAffNeighbour(curr_mb_nr, xN, yN, luma, pix);
  }
  else
    getNonAffNeighbour(curr_mb_nr, xN, yN, luma, pix);
}

void getAffNeighbour(unsigned int curr_mb_nr, int xN, int yN, int luma, PixelPos *pix)
{
  Macroblock *currMb = &img->mb_data[curr_mb_nr];
  int maxW, maxH;
  int yM = -1;

  if (luma)
  {
    maxW = 16;
    maxH = 16;
  }
  else
  {
    assert(img->yuv_format != 0);
    maxW = img->mb_cr_size_x;
    maxH = img->mb_cr_size_y;
  }

  // initialize to "not available"
  pix->available = 0;

  if(yN > (maxH - 1))
  {
    return;
  }
  if(xN > (maxW - 1) && yN >= 0 && yN < maxH)
  {
    return;
  }

  if (xN < 0)
  {
    if (yN < 0)
    {
      if(!currMb->mb_field)
      {
        // frame
        if (curr_mb_nr%2 == 0)
        {
          // top
          pix->mb_addr   = currMb->mbAddrD  + 1;
          pix->available = currMb->mbAvailD;
          yM = yN;
        }
        else
        {
          // bottom
          pix->mb_addr   = currMb->mbAddrA;
          pix->available = currMb->mbAvailA;
          if (currMb->mbAvailA)
          {
            if(!img->mb_data[currMb->mbAddrA].mb_field)
            {
               yM = yN;
            }
            else
            {
              (pix->mb_addr)++;
               yM = (yN + maxH) >> 1;
            }
          }
        }
      }
      else
      {
        // field
        if(curr_mb_nr % 2 == 0)
        {
          // top
          pix->mb_addr   = currMb->mbAddrD;
          pix->available = currMb->mbAvailD;
          if (currMb->mbAvailD)
          {
            if(!img->mb_data[currMb->mbAddrD].mb_field)
            {
              (pix->mb_addr)++;
               yM = 2 * yN;
            }
            else
            {
               yM = yN;
            }
          }
        }
        else
        {
          // bottom
          pix->mb_addr   = currMb->mbAddrD+1;
          pix->available = currMb->mbAvailD;
          yM = yN;
        }
      }
    }
    else
    { // xN < 0 && yN >= 0
      if (yN >= 0 && yN <maxH)
      {
        if (!currMb->mb_field)
        {
          // frame
          if(curr_mb_nr % 2 == 0)
          {
            // top
            pix->mb_addr   = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                 yM = yN;
              }
              else
              {
                if (yN %2 == 0)
                {
                   yM = yN>> 1;
                }
                else
                {
                  (pix->mb_addr)++;
                   yM = yN>> 1;
                }
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                (pix->mb_addr)++;
                 yM = yN;
              }
              else
              {
                if (yN %2 == 0)
                {
                   yM = (yN + maxH) >> 1;
                }
                else
                {
                  (pix->mb_addr)++;
                   yM = (yN + maxH) >> 1;
                }
              }
            }
          }
        }
        else
        {
          // field
          if (curr_mb_nr % 2 == 0)
          {
            // top
            pix->mb_addr  = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                if (yN < (maxH / 2))
                {
                   yM = yN << 1;
                }
                else
                {
                  (pix->mb_addr)++;
                   yM = (yN << 1 ) - maxH;
                }
              }
              else
              {
                 yM = yN;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr  = currMb->mbAddrA;
            pix->available = currMb->mbAvailA;
            if (currMb->mbAvailA)
            {
              if(!img->mb_data[currMb->mbAddrA].mb_field)
              {
                if (yN < (maxH / 2))
                {
                  yM = (yN << 1) + 1;
                }
                else
                {
                  (pix->mb_addr)++;
                   yM = (yN << 1 ) + 1 - maxH;
                }
              }
              else
              {
                (pix->mb_addr)++;
                 yM = yN;
              }
            }
          }
        }
      }
    }
  }
  else
  { // xN >= 0
    if (xN >= 0 && xN < maxW)
    {
      if (yN<0)
      {
        if (!currMb->mb_field)
        {
          //frame
          if (curr_mb_nr % 2 == 0)
          {
            //top
            pix->mb_addr  = currMb->mbAddrB;
            // for the deblocker if the current MB is a frame and the one above is a field
            // then the neighbor is the top MB of the pair
            if (currMb->mbAvailB)
            {
              if (!(img->DeblockCall == 1 && (img->mb_data[currMb->mbAddrB]).mb_field))
                pix->mb_addr  += 1;
            }
            
            pix->available = currMb->mbAvailB;
            yM = yN;
          }
          else
          {
            // bottom
            pix->mb_addr   = curr_mb_nr - 1;
            pix->available = 1;
            yM = yN;
          }
        }
        else
        {
          // field
          if (curr_mb_nr % 2 == 0)
          {
            // top
            pix->mb_addr   = currMb->mbAddrB;
            pix->available = currMb->mbAvailB;
            if (currMb->mbAvailB)
            {
              if(!img->mb_data[currMb->mbAddrB].mb_field)
              {
                (pix->mb_addr)++;
                 yM = 2* yN;
              }
              else
              {
                 yM = yN;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrB + 1;
            pix->available = currMb->mbAvailB;
            yM = yN;
          }
        }
      }
      else
      {
        // yN >=0
        // for the deblocker if this is the extra edge then do this special stuff
        if (yN == 0 && img->DeblockCall == 2)
        {
          pix->mb_addr  = currMb->mbAddrB + 1;
          pix->available = 1;
          yM = yN - 1;
        }

        else if ((yN >= 0) && (yN <maxH))
        {
          pix->mb_addr   = curr_mb_nr;
          pix->available = 1;
          yM = yN;
        }
      }
    }
    else
    { // xN >= maxW
      if(yN < 0)
      {
        if (!currMb->mb_field)
        {
          // frame
          if (curr_mb_nr % 2 == 0)
          {
            // top
            pix->mb_addr  = currMb->mbAddrC + 1;
            pix->available = currMb->mbAvailC;
            yM = yN;
          }
          else
          {
            // bottom
            pix->available = 0;
          }
        }
        else
        {
          // field
          if (curr_mb_nr % 2 == 0)
          {
            // top
            pix->mb_addr   = currMb->mbAddrC;
            pix->available = currMb->mbAvailC;
            if (currMb->mbAvailC)
            {
              if(!img->mb_data[currMb->mbAddrC].mb_field)
              {
                (pix->mb_addr)++;
                 yM = 2* yN;
              }
              else
              {
                yM = yN;
              }
            }
          }
          else
          {
            // bottom
            pix->mb_addr   = currMb->mbAddrC + 1;
            pix->available = currMb->mbAvailC;
            yM = yN;
          }
        }
      }
    }
  }
  if (pix->available || img->DeblockCall)
  {
    pix->x = (xN + maxW) % maxW;
    pix->y = (yM + maxH) % maxH;
    get_mb_pos(pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
    if (luma)
    {
      pix->pos_x += pix->x;
      pix->pos_y += pix->y;
    }
    else
    {
      pix->pos_x = pix->pos_x/(16/img->mb_cr_size_x) + pix->x;
      pix->pos_y = pix->pos_y/(16/img->mb_cr_size_y) + pix->y;
    }
  }
}

void getNonAffNeighbour(unsigned int curr_mb_nr, int xN, int yN, int luma, PixelPos *pix)
{
  Macroblock *currMb = &img->mb_data[curr_mb_nr];
  int maxW, maxH;

  if (luma)
  {
    maxW = 16;
    maxH = 16;
  }
  else
  {
    assert(img->yuv_format != 0);
    maxW = img->mb_cr_size_x;
    maxH = img->mb_cr_size_y;
  }

  if ((xN<0)&&(yN<0))
  {
    pix->mb_addr   = currMb->mbAddrD;
    pix->available = currMb->mbAvailD;
  }
  else if ((xN<0)&&((yN>=0)&&(yN<maxH)))
  {
    pix->mb_addr  = currMb->mbAddrA;
    pix->available = currMb->mbAvailA;
  }
  else if (((xN>=0)&&(xN<maxW))&&(yN<0))
  {
    pix->mb_addr  = currMb->mbAddrB;
    pix->available = currMb->mbAvailB;
  }
  else if (((xN>=0)&&(xN<maxW))&&((yN>=0)&&(yN<maxH)))
  {
    pix->mb_addr  = curr_mb_nr;
    pix->available = 1;
  }
  else if ((xN>=maxW)&&(yN<0))
  {
    pix->mb_addr  = currMb->mbAddrC;
    pix->available = currMb->mbAvailC;
  }
  else 
  {
    pix->available = 0;
  }

  if (pix->available || img->DeblockCall)
  {
    pix->x = (xN + maxW) % maxW;
    pix->y = (yN + maxH) % maxH;
    get_mb_pos(pix->mb_addr, &(pix->pos_x), &(pix->pos_y));
    if (luma)
    {
      pix->pos_x += pix->x;
      pix->pos_y += pix->y;
    }
    else
    {
      pix->pos_x = pix->pos_x/(16/img->mb_cr_size_x) + pix->x;
      pix->pos_y = pix->pos_y/(16/img->mb_cr_size_y) + pix->y;
    }
  }
}
