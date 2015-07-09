#!/usr/bin/env python

import os
from parse_lib import *
from dvfs_sim_lib import *

output_dir = "predict_times"

## Find all policies that were run
#policies = []
#for root, dirname, filenames in os.walk(output_dir):
#  for filename in filenames:
#    policy = filename.split("-")[0]
#    if not policy in policies:
#      policies.append(policy)

policy1 = "policy_cvx_conservative_lasso"
policy2 = "policy_cvx_conservative_lasso_restricted"

for benchmark in benchmarks:
  predict_times1 = read_predict_file("%s/%s-%s.txt" % (output_dir, policy1, benchmark))
  predict_times2 = read_predict_file("%s/%s-%s.txt" % (output_dir, policy2, benchmark))
  diff = [abs(float(x-y)/y) for x, y in zip(predict_times1, predict_times2)]
  print list_to_csv([benchmark, min(diff), average(diff), max(diff)])
