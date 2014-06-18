#!/usr/bin/python

import sys
import re
from parse_lib import *
import math

if len(sys.argv) != 2:
  print "usage: frames.py filename"
  sys.exit()
filename = sys.argv[1]

f = open(filename, 'r')
frame_times = []
for line in f:
  res = re.search("dlo: Frame [0-9]+ = ([0-9]+)us", line)
  if res:
    frame_times.append(int(res.group(1)))
f.close()

print min(frame_times), average(frame_times), max(frame_times)
print len(frame_times)
plot_sequence(frame_times, figsize=figsize_doublecol)
plot_set_fonts()
pylab.tight_layout()
pylab.xlabel("Frame")
pylab.ylabel("Execution Time [us]")
#plot_show()
plot_to_pdf("frame_time.pdf")

# OMAP 3530
# voltages = [1.35, 1.27, 1.20, 1.06, 0.985]
# frequencies = [720, 550, 500, 250, 125]
# Intel Pentium M
voltages = [1.484, 1.420, 1.276, 1.164, 1.036, 0.956]
frequencies = [1600, 1400, 1200, 1000, 800, 600]
# Add fake pair
voltages.append(0.75)
frequencies.append(250)
v = voltages
f = frequencies
energies = [v[i]*v[i]/(v[0]*v[0]) for i in range(len(voltages))]
print "Energy: ", energies
overhead = [float(f[0])/f[i] for i in range(len(frequencies))]
print "Time: ", overhead

exec_time_threshold = 35000
# For each P state
for p in range(len(voltages)):
  violated_frames = 0
  # For each frame
  for f in frame_times:
    # Calculate frame time at this P state
    new_time = f*overhead[p]
    if new_time > exec_time_threshold:
      violated_frames += 1
  print p, "%f (%d%%)" % (len(frame_times)*energies[p], energies[p]*100), "%d (%.2f%%)" % (violated_frames, 100*float(violated_frames)/len(frame_times))


# Optimal scheme
# print
# energy = 0
# for f in frame_times:
#   for p in range(len(voltages)-1, -1, -1):
#     new_time = f*overhead[p]
#     # If within execution time threshold
#     if new_time <= exec_time_threshold:
#       # Add energy usage of this frame
#       energy += energies[p]
#       # Continue on to next frame
#       break
# print "Optimal energy: %f (%d%%)\n" % (energy, 100*float(energy)/len(frame_times))

"""
Given an execution time, return the optimal (voltage, frequency) assuming
continuous DVFS points.
"""
def optimal_dvfs(t):
  # Determine the optimal frequency scaling that is needed
  f_scale = float(t)/exec_time_threshold
  freq = math.ceil(frequencies[0]*f_scale) # Calculate absolute frequency
  # Assert that we are under the threshold
  assert(t*frequencies[0]/freq <= exec_time_threshold)
  # Find voltage for this frequency
  voltage = 0.0006*freq + 0.6092
  return (voltage, freq)


print
total_energy = 0
for t in frame_times:
  (voltage, freq) = optimal_dvfs(t)
  energy = voltage*voltage/(v[0]*v[0])
  total_energy += energy
print "Optimal energy: %f (%d%%)\n" % (total_energy, 100*float(total_energy)/len(frame_times))

# Adaptive scheme
print
energy = 0
violated_frames = 0
#p = 0
(voltage, freq) = optimal_dvfs(exec_time_threshold)
rolling_window = [exec_time_threshold]*10
for t in frame_times:
  # Use P state based on previous frame to determine energy and timing violation
  if t*frequencies[0]/freq > exec_time_threshold:
    violated_frames += 1
  energy += voltage*voltage/(v[0]*v[0])
  # if f*overhead[p] > exec_time_threshold:
  #   violated_frames += 1
  # energy += energies[p]

  # Determine voltage, freq for next frame
  rolling_window.pop(0)
  rolling_window.append(t)
  avg_time = max(rolling_window)
  (voltage, freq) = optimal_dvfs(avg_time*1.2)
  # # Determine p-state for next frame
  # for p in range(len(voltages)-1, -1, -1):
  #   if f*overhead[p] <= exec_time_threshold:
  #     # Continue on to next frame
  #     break
print "Adaptive energy: %f (%d%%)" % (energy, 100*float(energy)/len(frame_times))
print "Violated frames = %d (%.2f%%)" % (violated_frames, 100*float(violated_frames)/len(frame_times))



