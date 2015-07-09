plot_metrics.pdf: results.csv
	./plot_metrics.py results.csv

results.csv: predict_times/*.txt
	./run_dvfs.py > results.csv

predict_times/*.txt:
	./predict_times.py

clean:
	rm -rf predict_times/ temp.lps results.csv plot_metrics.pdf
