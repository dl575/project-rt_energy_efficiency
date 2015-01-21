#!/usr/bin/python

"""
Strip off times before initialization. Initialization is assumed to be over
after the last time that is over 100000 us. In addition, strip last line if it
is a loop counter line.
"""

import sys

filename = sys.argv[1]

f = open(filename, 'r')

out_lines = []
for line in f:
  # Skip empty lines
  if not line.strip():
    continue

  # Add line to output
  out_lines.append(line)

  # If we see a large time, reset output
  if line[0:4] == "time":
    time = int(line.split(' ')[3])
    if time > 100000:
      out_lines = []

f.close()

if "loop counter" in out_lines[-1]:
  out_lines = out_lines[:-1]

print ''.join(out_lines)
