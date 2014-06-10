help:
	@echo "Make options"
	@echo "  auto_metrics.c"
	@echo "  ffmpeg_convert_out"
	@echo "  svm_out.parse8"
	@echo "  svm_plot"
	@echo "  clean"

auto_metrics.c: h264.c.t02.original
	../../parse/parse_function.py h264.c.t02.original decode_slice > parsed.function
	echo "// Conditionals" > auto_metrics.c
	../../parse/get_conditionals.py parsed.function >> auto_metrics.c
	echo "// Function arguments" >> auto_metrics.c
	../../parse/get_functions.py parsed.function >> auto_metrics.c

ffmpeg_convert_out:
	ffmpeg -i ironman3.mp4 -benchmark -y -s 1000x1000 -an output.avi > ffmpeg_convert_out

svm_out.parse8:
	svm_sweep_threshold.sh ffmpeg_convert_out
svm_plot: svm_out.parse8
	svm_plot_test.py svm_out.parse8

clean:
	rm -f parsed.function auto_metrics.c output.avi ffmpeg_convert_out ffmpeg_convert_out.svm svm_out.* test test.* train train.* *.pdf
