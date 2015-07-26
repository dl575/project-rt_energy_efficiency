#!/usr/bin/env python

import sys
sys.path.extend([".."])
from pylab import *
from parse_lib import *
from dvfs_sim_lib import *
import tsg_plot
import numpy

def plot_common(data, benchmarks, metrics, policies, filename):
  """
  Common plot options.
  """
  opts = tsg_plot.PlotOptions()
  attribute_dict = \
      {
          'figsize' : (5.8, 2.0),
          'fontsize' : 8,
          'num_rows' : 1,
          'num_cols' : 2,
          'labels' : [benchmarks, []],
          'legend_ncol' : 6,
          'legend_handlelength' : 1.0,
          'legend_columnspacing' : 0.8,
          'legend_handletextpad' : 0.5,
          'legend_bbox' : [0.05, 1.10, 2., 0.1],
          'rotate_labels' : True,
          'rotate_labels_angle' : -30,
      }
  for k, v in attribute_dict.iteritems():
    setattr(opts, k, v)

  return opts

def plot_em(data, benchmarks, metrics, policies, filename):
  """
  Plot energy and deadline misses.
  """
  # Common options
  opts = plot_common(data, benchmarks, metrics, policies, filename)

  # Colors based on number of configurations
  if len(policies) == 6:
    opts.colors = tsg_plot.colors['blue3']*2
    opts.hatch = ['']*3 + ['////']*3
  elif len(policies) == 2:
    opts.colors = [tsg_plot.colors['blue3'][0], tsg_plot.colors['blue3'][2]]
  else:
    raise Exception("Unsuported number of policies = %d" % len(policies))

  opts.file_name = filename.split('.')[0] + "_em.pdf"

  for metric in  ["energy", "deadline_misses"]:
    if metric == "energy":
      opts.ylabel = "Energy [%]"
    elif metric == "deadline_misses":
      opts.ylabel = "Misses [%]"
    # Restructure data
    metric_data = data[metric]
    opts.data = []
    for policy in policies:
      opts.data.append(metric_data[policy])
      opts.labels[1].append(policy_to_label(policy))
    opts.data = numpy.array(opts.data).T

    # Use one legend for both plots
    if opts.plot_idx == 1:
      opts.legend_enabled = True
    else:
      opts.legend_enabled = False

    tsg_plot.add_plot(opts)
 
def plot_core_counts(data, benchmarks, metrics, policies, filename):
  """
  Plot number of core switches and number of jobs on big core.
  """
  # Common options
  opts = plot_common(data, benchmarks, metrics, policies, filename)

  # Colors based on number of configurations
  if len(policies) == 6: 
    opts.colors = tsg_plot.colors['blue3']
    opts.hatch = ['']*3
  elif len(policies) == 2:
    opts.colors = [tsg_plot.colors['blue3'][2]]
  else:
    raise Exception("Unsuported number of policies = %d" % len(policies))

  opts.file_name = filename.split('.')[0] + "_counts.pdf"

  for metric in  ["switch_count", "big_count"]:
    if metric == "switch_count":
      opts.ylabel = "Core Switches [%]"
    elif metric == "big_count":
      opts.ylabel = "Big Core Jobs [%]"
    # Restructure data
    metric_data = data[metric]
    opts.data = []
    for policy in policies:
      if "biglittle" in policy:
        opts.data.append(metric_data[policy])
        opts.labels[1].append(policy_to_label(policy))
    opts.data = numpy.array(opts.data).T

    # Use one legend for both plots
    if opts.plot_idx == 1:
      opts.legend_enabled = True
    else:
      opts.legend_enabled = False

    tsg_plot.add_plot(opts)
 
def policy_to_label(policy):
  """
  Convert policy function/configuration name to a more human readable format.
  """
  policy = policy.strip().split('-')[1:]
  policy[-1] = "%.1f" % (float(policy[-1])/100)
  return '-'.join(policy)

def parse(filename):
  """
  Parse raw data file.
  """
  data = {"metadata" : {"metrics" : [], "policies" : [], "benchmarks" : []}}

  f = open(filename, 'r')
  current_metric = None
  for line in f:
    # Skip empty lines
    if not line.strip():
      continue

    # Comma-delimited
    ls = line.strip().split(',')
    # Metrics line
    if len(ls) == 1:
      current_metric = ls[0]
      data[current_metric] = {}
      data["metadata"]["metrics"].append(current_metric)
    # Benchmarks line
    elif not ls[0]:
      data["metadata"]["benchmarks"] = ls[1:]
    # Data line
    else:
      policy = ls[0].strip()
      data[current_metric][policy] = map(lambda x: float(x)*100, ls[1:])
      # Store policies on first metric
      if "policy_cvx_conservative_lasso" in policy and len(data["metadata"]["metrics"]) == 1:
        data["metadata"]["policies"].append(policy)
  f.close()

  return data


if __name__ == "__main__":
  if len(sys.argv) == 2:
    filename = sys.argv[1]
  else:
    print "usage: plot_metrics.py filename"
    sys.exit()

  data = parse(filename)

  # Reformat for plotting
  plot_data = {}
  benchmarks = [x.split('_')[0] for x in data["metadata"]["benchmarks"]]
  policies = data["metadata"]["policies"]
  metrics = data["metadata"]["metrics"]
  for metric, metric_data in data.iteritems():
    if metric == "metadata":
      continue
    # Normalize energy numbers to WC
    elif metric == "energy":
      for policy in policies:
        if metric not in plot_data.keys():
          plot_data[metric] = {}
        plot_data[metric][policy] = metric_data[policy]
    # Otherwise, just copy data
    else:
      plot_data[metric] = {}
      for policy in policies:
        plot_data[metric][policy] = metric_data[policy]

  plot_em(plot_data, benchmarks, metrics, policies, filename)
  plot_core_counts(plot_data, benchmarks, metrics, policies, filename)
