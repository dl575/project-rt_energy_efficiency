#!/usr/bin/python

import sys
import os
import itertools
from parse_lib import *
from dvfs_sim_lib import *
from pylab import *

PIDs = [[0.05*i for i in range(1, 40)], [0.1*i for i in range(25)], [0.05*i for i in range(10)]]
def run_pid(filename):
  # Read in execution times
  times = parse_execution_times(filename)
  deadline = max(times)

  dm = []
  e = []
  PID_configs = []
  margin = 1.1
  # For each PID configuration, find deadline misses and energy
  for (P, I, D) in itertools.product(*PIDs):
    predict_times = policy_pid(times, P=P, I=I, D=D)
    frequencies = [scale_frequency_perfect(margin*t, deadline) for t in predict_times]
    #frequencies = [scale_frequency(margin*t, deadline) for t in predict_times]
    result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]

    dm.append(deadline_misses(result_times, frequencies, deadline))
    e.append(energy(result_times, frequencies, deadline))
    PID_configs.append("%.2f,%.2f,%.2f" % (P, I, D))
  return (dm, e, PID_configs)

def plot_dm_e(dm, e):
  for ci, (dd, ee) in enumerate(zip(dm, e)):
    plot(dd, ee, 'ko')

"""
for root, dirnames, filenames in os.walk("data/"):
  for filename in filenames:
    fullpath = os.path.join(root, filename)
    (dm, e) = run_pid(fullpath)
    plot_dm_e(dm, e)
"""

(dm, e, PID_configs) = run_pid("data/xpilot.txt")
pareto_front = []
for ((dd, ee), PID) in zip(zip(dm, e), PID_configs):
  # Skip non-stable configurations
  if dd <=1 and dd >= 0 and ee <= 1 and ee >= 0:
    optimal = True
    for (ddd, eee) in zip(dm, e):
      if (ddd <= dd and eee < ee) or (ddd < dd and eee <= ee):
        optimal = False
        break
    if optimal:
      pareto_front.append([dd, ee, PID])
plot_dm_e(dm, e)
for (dd, ee, PID) in pareto_front:
  plot(dd, ee, 'ro')
print "\n".join([str(x) for x in pareto_front])

xlabel("Normalized Deadline Misses")
ylabel("Normalized Energy")
#legend(PID_configs)
xlim([0, 1])
ylim([0, 1])

show()

"""
figure()
times = parse_execution_times("data/xpilot.txt")
deadline = max(times)
P_range = range(1, 30)
I_range = range(10)
for pi in P_range:
  P = 0.1*pi
  for ii in I_range:
    I = 0.1*ii
    predict_times = policy_pid(times, P=0.1, I=P, D=I)
    margin = 1.1
    frequencies = [scale_frequency_perfect(margin*t, deadline) for t in predict_times]
    result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]
    dm = deadline_misses(result_times, frequencies, deadline)
    #dm = energy(result_times, frequencies, deadline)

    if dm >= 0.25:
      plot(P, I, 'ro', markersize=int(50*dm))
    else:
      plot(P, I, 'ko', markersize=int(50*dm))

xlim([0, max(P_range)*0.1])
ylim([-.1, max(I_range)*0.1])

xlabel("I")
ylabel("D")
title("Deadline Misses (Red >= 0.25), P=0.1")
show()
"""
