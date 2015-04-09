sed -e '
/print_loop_counter:/ a\
  #if GET_PREDICT || DEBUG_EN
' -e '/print_loop_counter_end:/ a\
  #endif
' $1
