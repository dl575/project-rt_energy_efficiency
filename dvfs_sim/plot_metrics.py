#!/usr/bin/python

import sys
from pylab import *
from parse_lib import *
from dvfs_sim_lib import *

colors = ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#ff7f00", "#cab2d6", "#6a3d9a", "#ffff99", "#b15928", "#fdbf6f"]

def plot_data(data, display_legend=False):
  # Reformat data into a dict
  metric = data[0]
  benchmarks = data[1].strip(',').split(',')
  data_dict = {}
  policies = []
  for d in data[2:]:
    ds = d.split(',')
    policy = ds[0].strip()
    policies.append(policy)
    # Convert data to percent
    data_dict[policy] = [float(x)*100 for x in ds[1:]]

  # Plot data
  index = 0
  for bi in range(len(benchmarks)):
    for (pi, policy) in enumerate(policies):
      bar(index, data_dict[policy][bi], color=colors[pi])
      index += 1
    index += 1
  # Set x-axis benchmark labels
  step = len(policies) + 1
  xticks([x*step + 0.5*step for x in range(len(benchmarks))], benchmarks, rotation=30, horizontalalignment="right")
  # y-axis
  if data[0] == "deadline_misses":
    ylabel("Deadline Misses [%]")
    ylim([0, 100])
  elif data[0] == "avg_normalized_tardiness":
    ylabel("Average Normalized Tardiness [%]")
  elif data[0] == "energy":
    ylabel("Normalized Energy [%]")
    ylim([0, 100])
  # Legend
  if display_legend:
    legend(policies, loc="upper center", ncol=2)

  tight_layout()

if len(sys.argv) == 2:
  filename = sys.argv[1]
else:
  print "usage: graph_metrics.py filename"
  sys.exit()

f = open(filename, 'r')
data = []
subplot_index = 1
for line in f:
  if line != "\n":
    data.append(line.strip())
  else:
    #figure()
    subplot(2, 2, subplot_index)
    plot_data(data, display_legend=(subplot_index==1))
    subplot_index += 1
    data = []
f.close()

show()

