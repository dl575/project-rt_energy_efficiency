#!/usr/bin/python

import sys
from pylab import *
from parse_lib import *
from dvfs_sim_lib import *
import tsg_plot
import numpy

if len(sys.argv) == 2:
  filename = sys.argv[1]
else:
  print "usage: graph_metrics.py filename"
  sys.exit()

opts = tsg_plot.PlotOptions()
# Common options for plots
attribute_dict = \
    {
        #'colors' : ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#ff7f00", "#cab2d6", "#6a3d9a", "#ffff99", "#b15928", "#fdbf6f"],
        #'colors' : ['#66c2a5', '#fc8d62', '#8da0cb', '#e78ac3'],
        'colors' : ['#d7191c', '#abd9e9', '#2c7bb6', '#fdae61'],
        'bar_width' : 0.7,
        'figsize' : (7.0, 3.0),
        'fontsize' : 10,
        'legend_ncol' : 2,
        'legend_bbox': (0.0, 1.1, 1.0, 0.1),
        'file_name' : 'plot_metrics.pdf',
        'rotate_labels' : True,
        'rotate_labels_angle' : -45,
        'xlabel' : '',
        'title' : '',
        'show' : False,
        'paper_mode' : True,
        'yrange' : (0, 100),
        'plot_idx' : 1,
        'num_cols' : 2,
        'num_rows' : 1,
        'legend_columnspacing' : 1.0,
        'legend_handlelength' : 1.0
    }
for name, value in attribute_dict.iteritems():
  setattr(opts, name, value)

def policy_to_label(policy):
  d = {
      "policy_tuned_pid" : "PID Controller",
      "policy_data_dependent_oracle" : "Least-Squares",
      "policy_data_dependent_lp" : "Conservative",
      "policy_oracle" : "Oracle"
      }
  policy = policy.strip()
  if policy in d.keys():
    return d[policy]
  else:
    return policy

f = open(filename, 'r')
data = []
subplot_index = 1
for line in f:
  if line != "\n":
    data.append(line.strip())
  # End of block of data, plot it
  elif data:
    # Parse data
    metric = data[0]
    benchmarks = data[1].split(',')[1:]
    policies = [d.split(',')[0] for d in data[2:]]
    data = [d.split(',')[1:] for d in data[2:]]
    data = [[100*float(dd) for dd in d] for d in data]
    data = numpy.array(data).transpose()
    # Add to options
    opts.data = data
    opts.labels = [benchmarks, map(policy_to_label, policies)]
    opts.ylabel = metric.replace('_', ' ') + " [%]"
    # Only show legend in middle plot
    #if opts.plot_idx == 1:
    #  opts.legend_enabled = True
    #else:
    #  opts.legend_enabled = False
    # Plot
    tsg_plot.add_plot(opts)

    # Reset for next metric
    data = []
f.close()

