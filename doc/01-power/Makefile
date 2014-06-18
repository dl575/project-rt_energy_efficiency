paper.pdf: Makefile paper.tex frame_time.pdf figs/*.pdf
	pdflatex -shell-escape paper.tex

#dvfs.pdf:
#	./plots.py
#dvfs_energy.pdf:
#	./plots.py
frame_time.pdf:
	./frames.py ffmpeg_convert_out

clean:
	rm -f *.aux *.pdf *.log *.dot *.pyc

