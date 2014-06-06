1. In ffmpeg-1.2/ do
$ ./configure --extra-cflags=-fdump-tree-original-raw
$ make
This doesn't seem to work on Mac OSX (use Linux instead).

On Mac OSX:
$ ./configure --extra-cflags=-fdump-tree-cfg

2. cp ffmpeg-1.2/libavcodec/h264.c.003t.original here
3. Pull out just the function of interest
$ ../../parse/parse_function.py h264.c.003t.original decode_slice > out
4. Run get_functions.py and get_conditionals.py on function AST.



### Directions for running experiments for ffmpeg.

1) Generate AST using gcc on hana.
```
hana$ ./configure --extra-cflags=-fdump-tree-original-raw --disable-yasm
hana$ make
```

2) Copy the generated AST here
```
hana$ cp h264.c.t02.original ~
$ scp hana:~/h264.c.t02.original .
```

3) Run parsing scripts to get conditional and function arguments
```
$ make auto_metrics.c
```

4) Manually contents of copy auto_metrics.c into the decode_slice function
in ffmpeg-1.2/libavcodec/h264.c. (The original decode_slice has been renamed
to decode_slice2 and wrapped with a new decode_slice which measures times
and prints metrics.)

5) Make ffmpeg
```
$ cd ../../ffmpeg-1.2
$ ./configure
$ make
```
> DEBUG: The following variables raise an error for the metrics print out: start_i, eos, h->gb, prev_status, h->cabac. Comment them out.

6) Get a video to convert (if needed) and run ffmpeg
```
$ scp hana:/research/data/dlo/videos/ironman3.mp4 .
$ ffmpeg -i ironman3.mp4 -benchmark -y -s 1000x1000 -an output.avi > ffmpeg_convert_out
```

7) Run SVM
```
$ svm_sweep_threshold.py ffmpeg_convert_out
```
Select [8] for ffmpeg_parse_auto_metrics.py
```
$ svm_plot_test.py svm_out.parse8
```

