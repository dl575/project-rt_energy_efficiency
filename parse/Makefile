h264ref:
	./parse_function.py tests/mv-search.c.t02.original SetupFastFullPelSearch > parsed.function
	@./get_conditionals.py parsed.function
	@./get_functions.py parsed.function

h264:
	./parse_function.py tests/h264.c.003t.original decode_slice > parsed.function
	./get_conditionals.py parsed.function
	./get_functions.py parsed.function

%: %.c Makefile
	gcc-4.2 -fdump-tree-original-raw $<
	rm a.out
	./get_conditionals.py $(notdir $<).003t.original
	./get_functions.py $(notdir $<).003t.original

clean:
	rm -f *.pyc parsed.function *.c.003t.original 
