typedef int pel_t;
typedef int StorablePicture;

void SetupFullFastPelSearch (short ref, int list)  // <--  reference frame parameter, list0 or 1
{
  short   pmv[2];
  pel_t   orig_blocks[256];
  pel_t *orgptr=orig_blocks;
  pel_t *refptr;
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

  //===== Use weighted Reference for ME ====
  if (apply_weights && input->UseWeightedReferenceME) {
    ref_pic       = ref_picture->imgY_11_w;
  } else {
    ref_pic       = ref_picture->imgY_11;
  }

  max_width     = ref_picture->size_x - 17;
  max_height    = ref_picture->size_y - 17;
  
  img_width     = ref_picture->size_x;
  img_height    = ref_picture->size_y;

  //===== get search center: predictor of 16x16 block =====
  SetMotionVectorPredictor (pmv, enc_picture->ref_idx, enc_picture->mv, ref, list, 0, 0, 16, 16);
  search_center_x[list][ref] = pmv[0] / 4;
  search_center_y[list][ref] = pmv[1] / 4;

  if (!input->rdopt)
  {
    //--- correct center so that (0,0) vector is inside ---
    search_center_x[list][ref] = max(-search_range, min(search_range, search_center_x[list][ref]));
    search_center_y[list][ref] = max(-search_range, min(search_range, search_center_y[list][ref]));
  }
  else {
  }

  search_center_x[list][ref] += img->opix_x;
  search_center_y[list][ref] += img->opix_y;

  offset_x = search_center_x[list][ref];
  offset_y = search_center_y[list][ref];

  //===== copy original block for fast access =====
  for   (y = img->opix_y; y < img->opix_y+16; y++) {
    for (x = img->opix_x; x < img->opix_x+16; x++) {
      *orgptr++ = imgY_org [y][x];
    }
  }


  //===== check if whole search range is inside image =====
  if (offset_x >= search_range && offset_x <= max_width  - search_range &&
      offset_y >= search_range && offset_y <= max_height - search_range   )
  {
    range_partly_outside = 0; PelYline_11 = FastLine16Y_11;
  }
  else
  {
    range_partly_outside = 1;
  }

  //===== determine position of (0,0)-vector =====
  if (!input->rdopt)
  {
    ref_x = img->opix_x - offset_x;
    ref_y = img->opix_y - offset_y;

    for (pos = 0; pos < max_pos; pos++)
    {
      if (ref_x == spiral_search_x[pos] &&
          ref_y == spiral_search_y[pos])
      {
        pos_00[list][ref] = pos;
        break;
      }
    }
  }

  //===== loop over search range (spiral search): get blockwise SAD =====
  for (pos = 0; pos < max_pos; pos++)
  {
    abs_y = offset_y + spiral_search_y[pos];
    abs_x = offset_x + spiral_search_x[pos];

    if (range_partly_outside)
    {
      if (abs_y >= 0 && abs_y <= max_height &&
          abs_x >= 0 && abs_x <= max_width    )
      {
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
      LineSadBlk0 = 0;
      LineSadBlk1 = 0;
      LineSadBlk2 = 0;
      LineSadBlk3 = 0;
      for (y = 0; y < 4; y++)
      {
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



  //===== combine SAD's for larger block types =====
  SetupLargerBlocks (list, ref, max_pos);


  //===== set flag marking that search setup have been done =====
  search_setup_done[list][ref] = 1;

}
