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
        'colors' : ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#ff7f00", "#cab2d6", "#6a3d9a", "#ffff99", "#b15928", "#fdbf6f"],
        'bar_width' : 0.7,
        'figsize' : (7.5, 2),
        'fontsize' : 8,
        'labels_fontsize' : 8,
        'legend_ncol' : 3,
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
        'num_cols' : 3,
        'num_rows' : 1
    }
for name, value in attribute_dict.iteritems():
  setattr(opts, name, value)

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
    opts.labels = [benchmarks, policies]
    opts.ylabel = metric + " [%]"
    # Only show legend in middle plot
    if opts.plot_idx == 2:
      opts.legend_enabled = True
    else:
      opts.legend_enabled = False
    # Plot
    tsg_plot.add_plot(opts)

    # Reset for next metric
    data = []
f.close()

