help:
	@echo "Make options"
	@echo "  ff"
	@echo "  clean"

ff:
	for i in {0..9}; do \
		ffmpeg -i ironman3.mp4 -benchmark -y -s 1000x1000 -an output.avi > ff$$i; \
	done

clean:
	rm -f ff[0-9] *.png output.avi *.pyc
