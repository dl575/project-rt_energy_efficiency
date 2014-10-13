#!/usr/bin/python

import sys
from pylab import *
from parse_lib import *

policies = ["policy_average", "policy_pid", "policy_data_dependent", "policy_data_dependent_oracle"]
colors = ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c"]

def graph_deadline_misses(filename):
  f = open(filename, 'r')
  benchmarks = f.readline().split(",")[1:]
  benchmarks.append("average")
  data = {}
  for line in f:
    ls = line.strip().strip(',').split(',')
    key = ls[0].strip()
    data[key] = [100*float(x) for x in ls[1:]]
    data[key].append(average(data[key]))
  f.close()
  print benchmarks
  print data

  index = 0
  for (bi, benchmark) in enumerate(benchmarks):
    for (pi, policy) in enumerate(policies):
      bar(index, data[policy][bi], color=colors[pi])
      index += 1
    index += 1
  # Set x-axis benchmark labels
  step = len(policies) + 1
  xticks([x*step + 0.5*step for x in range(len(benchmarks))], benchmarks, rotation=30, horizontalalignment="right")
  # y-axis
  ylabel("Deadline Misses [%]")
  ylim([0, 100])
  # Legend
  legend(policies, loc="upper center", ncol=2)
  title(filename)

def graph_tardiness(filename):
  f = open(filename, 'r')
  benchmarks = f.readline().split(",")[1:]
  benchmarks.append("average")
  data = {}
  for line in f:
    ls = line.strip().strip(',').split(',')
    key = ls[0].strip()
    data[key] = [100*float(x) for x in ls[1:]]
    data[key].append(average(data[key]))
  f.close()
  print benchmarks
  print data

  index = 0
  for (bi, benchmark) in enumerate(benchmarks):
    for (pi, policy) in enumerate(policies):
      bar(index, data[policy][bi], color=colors[pi])
      index += 1
    index += 1
  # Set x-axis benchmark labels
  step = len(policies) + 1
  xticks([x*step + 0.5*step for x in range(len(benchmarks))], benchmarks, rotation=30, horizontalalignment="right")
  # y-axis
  if "avg" in filename:
    ylabel("Average Tardiness [%]")
  elif "max" in filename:
    ylabel("Worst-Case Tardiness [%]")
  else:
    raise Exception()
  # Legend
  legend(policies, loc="upper center", ncol=2)
  title(filename)

def graph_energy(filename):
  f = open(filename, 'r')
  benchmarks = f.readline().split(",")[1:]
  benchmarks.append("average")
  data = {}
  for line in f:
    ls = line.strip().strip(',').split(',')
    key = ls[0].strip()
    data[key] = [100*float(x) for x in ls[1:]]
    data[key].append(average(data[key]))
  f.close()
  print benchmarks
  print data

  index = 0
  for (bi, benchmark) in enumerate(benchmarks):
    for (pi, policy) in enumerate(policies):
      bar(index, data[policy][bi], color=colors[pi])
      index += 1
    index += 1
  # Set x-axis benchmark labels
  step = len(policies) + 1
  xticks([x*step + 0.5*step for x in range(len(benchmarks))], benchmarks, rotation=30, horizontalalignment="right")
  # y-axis
  ylabel("Normalized Energy [%]")
  ylim([0, 100])
  # Legend
  legend(policies, loc="upper center", ncol=2)
  title(filename)

if len(sys.argv) == 2:
  filename = sys.argv[1]
else:
  filename = "results/deadline_misses.csv"

if "deadline_misses" in filename:
  graph_deadline_misses(filename)
elif "tardiness" in filename:
  graph_tardiness(filename)
elif "energy" in filename:
  graph_energy(filename)
else:
  print "Unsupported file. Please implement"
  sys.exit()

# Show graph
tight_layout()
show()

