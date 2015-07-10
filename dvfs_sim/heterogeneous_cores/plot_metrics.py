#!/usr/bin/env python

import sys
sys.path.extend([".."])
from pylab import *
from parse_lib import *
from dvfs_sim_lib import *
import tsg_plot
import numpy

def plot(data, benchmarks, metrics, policies):
  # Common options for plots
  opts = tsg_plot.PlotOptions()
  attribute_dict = \
      {
          'colors' : tsg_plot.colors['blue3'] + tsg_plot.colors['red3'],
          'bar_width' : 0.7,
          'figsize' : (10.0, 3.0),
          'fontsize' : 8,
          'legend_ncol' : 6,
          'file_name' : 'plot_metrics.pdf',
          'rotate_labels' : True,
          'rotate_labels_angle' : -45,
          'xlabel' : '',
          'title' : '',
          'plot_idx' : 1,
          'num_cols' : 3,
          'num_rows' : 1,
          'legend_columnspacing' : 1.0,
          'legend_handlelength' : 1.0,
      }
  for name, value in attribute_dict.iteritems():
    setattr(opts, name, value)

  for metric in metrics:
    metric_data = data[metric]
    opts.data = []
    opts.labels = [benchmarks, []]
    for policy in policies:
      opts.data.append(metric_data[policy])
      opts.labels[1].append(policy_to_label(policy))
    opts.data = numpy.array(opts.data).T
    opts.ylabel = metric.replace('_', ' ') + " [%]"
    if opts.plot_idx == 2:
      opts.legend_enabled = True
    else:
      opts.legend_enabled = False
    tsg_plot.add_plot(opts)

def policy_to_label(policy):
  """
  Convert policy function/configuration name to a more human readable format.
  """
  d = {
      "policy_tuned_pid" : "pid",
      "policy_least_squares" : "Least-Squares",
      "policy_conservative" : "prediction",
      "policy_cvx_conservative_lasso" : "prediction",
      "policy_oracle" : "oracle",
      }
  policy = policy.strip()
  result = []
  for word in policy.split('-'):
    if word in d.keys():
      result.append(d[word])
    elif "cvx_conservative_lasso_" in word:
      result.append("cvx-%s" % (policy.split('_')[4]))
    else:
      result.append(word)
  return '-'.join(result)

def parse(filename):
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
    print "usage: plot_metrics_biglittle.py filename"
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
        cvx_data = metric_data[policy]
        # Normalize to worstcase policy with biglittle available
        wc_policy = policy.replace("policy_cvx_conservative_lasso", "policy_worstcase").replace("littleonly", "biglittle").replace("bigonly", "biglittle")
        wc_data = metric_data[wc_policy]
        normalized_data = map(lambda x: x*100, normalize(cvx_data, wc_data))
        #normalized_data  = metric_data[policy]
        if metric not in plot_data.keys():
          plot_data[metric] = {}
        plot_data[metric][policy] = normalized_data
    # Otherwise, just copy data
    else:
      plot_data[metric] = {}
      for policy in policies:
        plot_data[metric][policy] = metric_data[policy]

  plot(plot_data, benchmarks, metrics, policies)
