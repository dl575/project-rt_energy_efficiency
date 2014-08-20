#!/usr/bin/python

import sys

f = open(sys.argv[1], 'r')
for line in f:
	if line[0] != '#':
		print line,
f.close()
