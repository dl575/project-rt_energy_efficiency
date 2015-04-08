#!/usr/bin/python

import os
import subprocess
from parse_lib import *

# Get benchmarks
os.chdir("lps")
coeffs = subprocess.check_output(["./gen_predictor.py"])
os.chdir("..")
benchmarks = []
for line in coeffs.split('\n'):
  if ".lps" in line:
    benchmarks.append(line.strip().rstrip(".lps"))
print list_to_csv([""] + benchmarks)

# Get number of coefficients
#for d in ["lps", "regression_coeffs", "lasso"]:
for d in ["predictors"]:
  os.chdir(d)
  coeffs = subprocess.check_output(["./gen_predictor.py"])
  os.chdir("..")

  num_coeffs = []
  for line in coeffs.split('\n'):
    if "non-zero" in line:
      num_coeffs.append(line.split(' ')[-1])
  print list_to_csv([d] + num_coeffs) 
