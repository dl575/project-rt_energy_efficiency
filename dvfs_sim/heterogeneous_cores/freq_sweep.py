#!/usr/bin/env python

import sys
sys.path.extend([".."])
import tsg_plot
from run_dvfs_abs import *

def plot(data, xlabel, ylabel, filename):
  opts = tsg_plot.PlotOptions()
  attribute_dict = {
      "plot_type" : "line",
      "data" : data,
      "labels" : ["little", "big"],
      "legend_ncol" : 2,
      "file_name" : filename,
      "figsize" : (3.5, 1.5),
      "fontsize" : 8,
      "xlabel" : xlabel,
      "ylabel" : ylabel,
      "colors" : [tsg_plot.colors['blue3'][2], tsg_plot.colors['qualitative_paired'][5]],
      "rotate_labels" : True,
      "rotate_labels_angle" : -30,
      }
  for k, v in attribute_dict.iteritems():
    setattr(opts, k, v)
  tsg_plot.add_plot(opts)

def normalize(l1, l2):
  """
  Normalize values in l1 and l2 to the max value across both arrays.
  """
  max_value = max(l1 + l2)
  l1 = [x/max_value for x in l1]
  l2 = [x/max_value for x in l2]
  return (l1, l2)

if __name__ == "__main__":

  # Scale frequencies to MHz
  dvfs_levels_little = [x*100 for x in dvfs_levels_little]
  dvfs_levels_big = [x*100 for x in dvfs_levels_big]

  # Little core
  power_little = []
  times_little = []
  for f in dvfs_levels_little:
    power_little.append(little_power_factor * f**3)
    times_little.append(big_speedup_factor / f)
  energy_little = [x*y for x, y in zip(power_little, times_little)]

  # Big core
  power_big = []
  times_big = []
  for f in dvfs_levels_big:
    power_big.append(f**3)
    times_big.append(1./f)
  energy_big = [x*y for x, y in zip(power_big, times_big)]

  # Normalize
  times_little, times_big = normalize(times_little, times_big)
  energy_little, energy_big = normalize(energy_little, energy_big)
  #import math
  #energy_little = map(math.log, energy_little)
  #energy_big = map(math.log, energy_big)

  plot([[dvfs_levels_little, times_little], [dvfs_levels_big, times_big]], "Frequency [MHz]", "Normalized Execution Time", "hetero_time_freq.pdf")
  plot([[dvfs_levels_little, energy_little], [dvfs_levels_big, energy_big]], "Frequency [MHz]", "Normalized Energy", "hetero_energy_freq.pdf")
  plot([[times_little, energy_little], [times_big, energy_big]], "Normalized Execution Time", "Normalized Energy", "hetero_energy_time.pdf")


