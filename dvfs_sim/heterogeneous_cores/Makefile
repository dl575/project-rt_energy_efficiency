all: hetero_big.csv hetero_little.csv plot_metrics.py
	./plot_metrics.py hetero_big.csv
	./plot_metrics.py hetero_little.csv
	./freq_sweep.py

hetero_big.csv: predict_times_big/*.txt run_dvfs.py
	./run_dvfs.py big > hetero_big.csv
hetero_little.csv: predict_times_little/*.txt run_dvfs.py
	./run_dvfs.py little > hetero_little.csv

predict_times_big/*.txt:
	./predict_times.py big
predict_times_little/*.txt:
	./predict_times.py little

clean:
	rm -rf predict_times_big/ predict_times_little/ temp.lps *.pdf *.csv *.pyc
