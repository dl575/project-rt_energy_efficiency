#!/usr/bin/python

"""
Calculate percentage of frames in slow/fast category from
libsvm format file.

slow_fast.py libsvmfile
"""

import sys

if len(sys.argv) != 2:
  raise Exception(__doc__)
filename = sys.argv[1]

f = open(filename, 'r')

slow_frames = 0
fast_frames = 0
for line in f:
  if line[0:2] == "-1":
    slow_frames += 1
  else:
    fast_frames += 1

slow_percent = float(slow_frames)/(slow_frames + fast_frames)
fast_percent = float(fast_frames)/(slow_frames + fast_frames)
print "Slow frames = %f" % (slow_percent)
print "Fast frames = %f" % (fast_percent)

f.close()
