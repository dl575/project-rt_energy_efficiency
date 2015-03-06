#!/usr/bin/python

import sys
sys.path.extend(['../..'])
import tsg_plot
import math
import numpy

def parse(filename):
  f = open(filename, 'r')
  data = {}
  data['data'] = {}
  for line in f:
    # Empty line, skip
    if not line.strip():
      continue

    ls = line.strip().split(',')
    # metric label
    if len(ls) == 1:
      continue
    # Benchmarks line, no first entry
    elif not ls[0].strip():
      data['benchmarks'] = ls
    # Data line
    else:
      policy = ls[0]
      #d = [float(x)*100 for x in ls[1:]]
      d = [float(x)*100 for x in ls[-1:]]
      if policy not in data['data'].keys():
        data['data'][policy] = d
      else:
        data['data'][policy] = zip(data['data'][policy], d)
  f.close()
  return data

def plot(data):
  opts = tsg_plot.PlotOptions()
  opts.data = []
  opts.labels = [[], []]
  for (k, v) in data['data'].iteritems():
    opts.data.append(v)
    opts.labels[1].append(k)

  attribute_dict = \
      {
          'show' : False,
          'file_name' : 'lasso_scatter.pdf',
          'plot_type' : 'scatter',
          'paper_mode' : True,
          'figsize' : (3.5, 3.5),
          'legend_ncol' : 1,
          'xlabel' : 'Deadline Misses [%]',
          'ylabel' : 'Energy [%]',
          'fontsize' : 8,

          'symbols' : ['o']*15
      }

  for name, value in attribute_dict.iteritems():
    setattr( opts, name, value )
  tsg_plot.add_plot( opts )

if __name__ == "__main__":
  if len(sys.argv) == 2:
    filename = sys.argv[1]
  else:
    print "usage: graph_metrics.py filename"
    sys.exit()

  data = parse(filename)
  plot(data)
