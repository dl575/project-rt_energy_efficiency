#!/usr/bin/env python

import os
from parse_lib import *
from dvfs_sim_lib import *

output_dir = "predict_times"

# Find all policies that were run
policies = []
for root, dirname, filenames in os.walk(output_dir):
  for filename in filenames:
    policy = filename.split("-")[0]
    if not policy in policies:
      policies.append(policy)

# Get oracle times
oracle_times = {}
for benchmark in benchmarks:
  predict_times = read_predict_file("%s/policy_oracle-%s.txt" % (output_dir, benchmark))
  oracle_times[benchmark] = predict_times

for policy in policies:
  for benchmark in benchmarks:
    predict_times = read_predict_file("%s/%s-%s.txt" % (output_dir, policy, benchmark))
    #errors = [float(x-y)/y*100 for x, y in zip(predict_times, oracle_times[benchmark])] # Normalized [%]
    errors = [float(x-y)/1000 for x, y in zip(predict_times, oracle_times[benchmark])] # Absolute [ms]
    print list_to_csv([policy, benchmark, average(errors), max(errors)])
