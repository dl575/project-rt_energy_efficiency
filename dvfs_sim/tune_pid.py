#!/usr/bin/python

import sys
import os
import itertools
from parse_lib import *
from dvfs_sim_lib import *
from pylab import *

PIDs = [[0.2*i for i in range(1, 10)], [0.2*i for i in range(10)], [0.01*i for i in range(100)]]
def run_pid(filename):
  # Read in execution times
  times = parse_execution_times(filename)
  deadline = max(times)

  dm = []
  e = []
  PID_configs = []
  # For each PID configuration, find deadline misses and energy
  for (P, I, D) in itertools.product(*PIDs):
    predict_times = policy_pid(times, P=P, I=I, D=D)
    margin = 1.1
    frequencies = [scale_frequency_perfect(margin*t, deadline) for t in predict_times]
    result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]

    dm.append(deadline_misses(result_times, deadline))
    e.append(energy(frequencies))
    PID_configs.append("%.2f,%.2f,%.2f" % (P, I, D))
  return (dm, e, PID_configs)

colors = ["#a6611a", "#dfc27d", "#f5f5f5", "#80cdc1", "#018571"]
def plot_dm_e(dm, e):
  for ci, (dd, ee) in enumerate(zip(dm, e)):
    #plot(dd, ee, 'o', color=colors[ci], markersize=10)
    plot(dd, ee, 'ko')

"""
for root, dirnames, filenames in os.walk("data/"):
  for filename in filenames:
    fullpath = os.path.join(root, filename)
    (dm, e) = run_pid(fullpath)
    plot_dm_e(dm, e)
"""
(dm, e, PID_configs) = run_pid("data/xpilot-doublenice.txt")
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
