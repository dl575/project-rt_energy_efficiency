help:
	@echo "Make options:"
	@echo "  svm_out.parse8"
	@echo "  plot_svm"
	@echo "  clean"

svm_out.parse8:
	svm_sweep_threshold.sh h264ref.log

plot_svm: svm_out.parse8
	svm_plot_test.py svm_out.parse8

clean:
	rm -f *.svm test test.* train train.* svm_out.* parsed.function *.pyc
