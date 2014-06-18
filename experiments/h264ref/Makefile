help:
	@echo "Make options:"
	@echo "  auto_metrics.c"
	@echo "  svm_out.parse8"
	@echo "  plot_svm"
	@echo "  clean"


auto_metrics.c: mv-search.c.t02.original
	../../parse/parse_function.py mv-search.c.t02.original SetupFastFullPelSearch > parsed.function
	../../parse/get_conditionals.py parsed.function > auto_metrics.c
	../../parse/get_functions.py parsed.function >> auto_metrics.c

svm_out.parse8:
	svm_sweep_threshold.sh h264ref.log

plot_svm: svm_out.parse8
	svm_plot_test.py svm_out.parse8

clean:
	rm -f *.svm test test.* train train.* svm_out.* parsed.function *.pyc
