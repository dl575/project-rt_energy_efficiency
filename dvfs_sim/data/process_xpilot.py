#!/usr/bin/python

import re
import sys

filename = sys.argv[1]

f = open(filename, 'r')
start_printing = False
for line in f:
  if start_printing:
    print line,

  # Filter out times before client joins
  if "Welcome Root" in line:
    start_printing = True

f.close()
