#!/usr/bin/python

from parse_lib import *
import sys

if (len(sys.argv) != 2):
  print "usage: times.py filename"
  sys.exit()
filename = sys.argv[1]

# Get execution times
times = parse_execution_times(filename)

# Plot
plot_set_fonts()
plot_histogram(times, xlabel="Time [us]", ylabel="Jobs")
plot_to_pdf("hist.pdf")
plot_sequence(times, xlabel="Job", ylabel="Time [us]")
plot_to_pdf("times.pdf")
