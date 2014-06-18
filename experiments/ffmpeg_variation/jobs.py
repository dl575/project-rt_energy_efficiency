#!/usr/bin/python

from parse_lib import *
import pylab

nruns = 10
times = [list() for x in range(nruns)]
# For each run
for i in range(nruns):
  times[i] = parse_execution_times("ff%d" % (i))
nframes = len(times[0])

# For each frame, calculate average time across runs
avg_times = []
for i in range(nframes):
  avg_times.append(median([times[x][i] for x in range(nruns)]))

# Plot all runs overlaid
pylab.figure(figsize=figsize_doublecol)
plot_set_fonts()
for i in range(nruns):
  pylab.plot(times[i])
pylab.xlabel("Frame #")
pylab.ylabel("Execution Time [us]")
#pylab.show()
#plot_to_pdf("frames_overlay.pdf")
pylab.savefig("frames_overlay.png", format="png")

# Calculate differences from average. Normalize to average time.
diffs = [list() for x in range(nruns)]
for f in range(nframes):
  for i in range(nruns):
    diff = times[i][f] - avg_times[f]
    #normalized_diff = float(diff)/avg_times[f]
    diffs[i].append(diff)
pylab.figure(figsize=figsize_doublecol)
for i in range(10):
  pylab.plot(diffs[i], color="gray", alpha=0.5)
pylab.plot(diffs[4], color="red")
pylab.xlabel("Frame #")
pylab.ylabel("Difference [us]")
plot_set_fonts()
#pylab.show()
pylab.savefig("frames_diff.png", format="png")
#plot_to_pdf("frames_diff.pdf")
pylab.ylim(-4000, 4000)
pylab.savefig("frames_diff_reduced.png", format="png")


