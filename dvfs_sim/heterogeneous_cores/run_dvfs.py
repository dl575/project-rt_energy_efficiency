#!/usr/bin/env python

import sys
sys.path.extend([".."])
from parse_lib import *
from dvfs_sim_lib import *

"""
Configuration
"""
margin = 1.1
# Scaling factor based on idle power at 1.4 GHz
#little_energy_factor = 0.04/0.35 # little energy = little_energy_factor * big energy at same frequency
# Scaling factor based on active power of rijndael at 1.4 GHz
little_energy_factor = 0.306/0.921 # littler energy = little energy_factor * big energy at same frequency
big_speedup_factor = 2 # times faster than little core at same frequency
switching_time = 0 # microseconds to switch between cores

# Normalized DVFS levels and execution time scaling function for little core
# 200 MHz to 1.4 GHz with 1.4 GHz normalized to 1
dvfs_levels_little = [x/14. for x in range(2, 15)]
dvfs_function_little = lambda t, f: float(t)/f
# Normalized DVFS levels and execution time scaling function for big core
dvfs_levels_big = [x/14. for x in range(2, 21)] # Normalized to 1 <-> 1.4 GHz
dvfs_function_big = lambda t, f: float(t)/(big_speedup_factor*f)

def run_dvfs_hetero(metric, times, predict_times, deadline, biglittle, policy):
  """
  Run DVFS simulation.
  """
  
  # Initialization
  frequencies = []
  result_times = []
  # Counts for running on each core and switches
  little_count = 0
  big_count = 0
  switch_count = 0
  # Whether big core exists
  big_core = "big" in biglittle
  little_core = "little" in biglittle
  # At least one core must exist
  assert(big_core or little_core)
  # Start on big core if it exists
  if big_core:
    last_core = "big"
  else:
    last_core = "little"

  # For each job,
  for (pt, t) in zip(predict_times, times):
    # Increase predicted time by margin
    #if not "worstcase" in policy:
    #  pt = margin*pt
    pt = margin*pt
    found = False

    # See if any little frequencies work
    if little_core:
      for f in dvfs_levels_little:
        # Calculate time for running on little core at this frequency
        time = dvfs_function_little(pt, f)
        # If switching cores, include switching overhead
        if last_core == "big":
          time += switching_time
        # If resulting time is less than the deadline, use this frequency
        if time <= deadline:
          frequencies.append(little_energy_factor*f)
          # Calculate resulting time based on actual time and freq used
          if last_core == "big":
            result_times.append(dvfs_function_little(t, f) + switching_time)
            switch_count += 1
          else:
            result_times.append(dvfs_function_little(t, f))
          # Update current core to little core
          last_core = "little"
          little_count += 1
          found = True
          break
      # little frequency works, move on to next point
      if found:
        continue
    assert(not found)

    # If no big core
    if not big_core:
      # Use highest frequency of small core
      frequencies.append(little_energy_factor*max(dvfs_levels_little))
      result_times.append(dvfs_function_little(t, max(dvfs_levels_little)))
      last_core = "little"
      little_count += 1
      continue

    if big_core:
      # If little frequencies don't work, use big core
      for f in dvfs_levels_big:
        # Calculate time for running on big core at this frequency
        time = dvfs_function_big(pt, f)
        # If switching cores, include switching overhead
        if last_core == "little":
          time += switching_time
        if time < deadline:
          # Found, break out
          break
      # Use found frequency, or if not found, use max frequency
      frequencies.append(f)
      # Calculate resulting time based on actual time and freq used
      if last_core == "little":
        result_times.append(dvfs_function_big(t, f) + switching_time)
        switch_count += 1
      else:
        result_times.append(dvfs_function_big(t, f))
      last_core = "big"
      big_count += 1

  # Calculate metric of interest
  if metric == "switch_count":
    metric_result = float(switch_count)/len(times)
  else:
    metric_result = metric(result_times, times, frequencies, deadline)
  return metric_result

if __name__ == "__main__":
  """
  Parse arguments
  """
  if len(sys.argv) < 2:
    print "usage: run_dvfs.py big|little [--no_test]"
    exit(1)
  if sys.argv[1] == "big":
    bigonly = True
    littleonly = False
  elif sys.argv[1] == "little":
    bigonly = False
    littleonly = True
  else:
    print "usage: run_dvfs.py big|little [--no_test]"
    exit(1)
  assert(bigonly != littleonly)
  no_test = False
  if "--no_test" in sys.argv:
    no_test = True

  """
  Directories
  """
  if bigonly:
    input_dir = "../data_big"
    output_dir = "predict_times_big"
  elif littleonly:
    input_dir = "../data_little"
    output_dir = "predict_times_little"

  # Find all policies that were run
  policies = []
  for root, dirname, filenames in os.walk(output_dir):
    for filename in filenames:
      policy = filename.split("-")[0]
      if not policy in policies:
        policies.append(policy)

  # For each metric of interest,
  for metric in [energy, deadline_misses, "switch_count"]:
    if isinstance(metric, str):
      print metric
    else:
      print metric.__name__
    print list_to_csv([""] + benchmarks + ["average"])

    # For heterogeneous core and switching time configurations
    if bigonly:
      biglittle_configurations = ["bigonly-60", "bigonly-80", "bigonly-100"]
    elif littleonly: 
      biglittle_configurations = ["littleonly-60", "littleonly-80", "littleonly-100"]
    biglittle_configurations += ["biglittle-60", "biglittle-80", "biglittle-100"]
    for biglittle in biglittle_configurations:
      # For each governor policy
      for policy in policies:
        print "%s-%s" % (policy, biglittle), ",", 
        sum_metric = 0

        # For each benchmark
        for benchmark in benchmarks:
          # Read in execution times and predicted times
          if no_test:
            times = parse_execution_times("%s/%s/%s0.txt" % (input_dir, benchmark, benchmark))
          else:
            times = parse_execution_times("%s/%s/%s1.txt" % (input_dir, benchmark, benchmark))
          predict_times = read_predict_file("%s/%s-%s.txt" % (output_dir, policy, benchmark))

          # Scale deadline based on worst-case execution time
          scaling = float(biglittle.split('-')[1])/100
          deadline = scaling*max(times)
          
          # Run DVFS simulation
          metric_result = run_dvfs_hetero(metric, times, predict_times, deadline, biglittle, policy)

          # Sum metric for average calculation
          sum_metric += metric_result
          # Output result for this configuration
          print metric_result, ",",
        # Output average
        print sum_metric/len(benchmarks)
    print
