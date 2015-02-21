#!/usr/bin/python

from pylab import *
from parse_lib import *
from dvfs_sim_lib import *

def read_predict_file(filename):
  f = open(filename, 'r')
  data = []
  for line in f:
    data.append(float(line))
  f.close()
  return data

for benchmark in benchmarks:
  figure(figsize=(7.0, 3.0))
  suptitle(benchmark)

  # Original execution time
  times = parse_execution_times("data/%s/%s1.txt" % (benchmark, benchmark))

  # Predicted times
  predict_dir = "predict_times/"
  policies = ["policy_data_dependent_oracle", "policy_data_dependent_lp"]
  titles = {"policy_data_dependent_oracle" : "Least-Squares",
      "policy_data_dependent_lp" : "Conservative"}
  for (i, policy) in enumerate(policies):
    # Read in predicted times
    filename = predict_dir + "%s-%s.txt" % (policy, benchmark)
    predict_times = read_predict_file(filename)

    # Calculate errors
    errors = [float(x - y)/y for (x, y) in zip(predict_times, times)]
    # Bound errors
    errors = [min(1, x) for x in errors]
    errors = [max(-1, x) for x in errors]

    # Plot histogram
    subplot(1, 2, i+1)
    hist(errors, bins=50)
    title(titles[policy])
    xlabel("Normalized Error [%]")

  plot_set_fonts({"family" : "Arial",
    "size" : 8})
  tight_layout()
  # Write to file
  filename = "error_histogram_%s.pdf" % (benchmark)
  print "Writing plot to %s" % (filename)
  plot_to_pdf(filename)
