all: results_big.pdf results_little.pdf

results_big.pdf: results_big.csv
	./plot_metrics.py results_big.csv
	mv plot_metrics.pdf results_big.pdf
results_little.pdf: results_little.csv
	./plot_metrics.py results_little.csv
	mv plot_metrics.pdf results_little.pdf

results_big.csv: predict_times_big/*.txt
	./run_dvfs.py big > results_big.csv
results_little.csv: predict_times_little/*.txt
	./run_dvfs.py little > results_little.csv

predict_times_big/*.txt:
	./predict_times.py big
predict_times_little/*.txt:
	./predict_times.py little

clean:
	rm -rf predict_times_big/ predict_times_little/ temp.lps results_big.csv results_little.csv results_big.pdf results_little.pdf
