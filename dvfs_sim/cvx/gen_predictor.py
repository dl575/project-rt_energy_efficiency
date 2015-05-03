#!/usr/bin/python

import os

def get_coeffs(filename):
  coeffs = []
  constant = None
  f = open(filename, 'r')
  for line in f:
    if line[0] == 'b':
      if line[1] in "0123456789":
        coeffs.append(float(line.split()[1]))
      else:
        constant = float(line.split()[1])
  f.close()
  if not constant:
    raise Exception("Constant term not found")
  return (constant, coeffs)

min_threshold = 1e-6
def gen_predictor(constant, coeffs):
  predictor = "float exec_time;\n"
  predictor += "exec_time = "
  non_zero_indices = []
  for (ci, c) in enumerate(coeffs):
    if c >= min_threshold:
      non_zero_indices.append(ci)
      predictor += "%f*loop_counter[%d] + " % (c, ci)
  predictor += "%f;\n" % (constant)
  predictor += "printf(\"predicted time = %f\\n\", exec_time);"
  print "// non-zero coeffs = ", non_zero_indices
  print "// loop counters: ", ' '.join(["loop_counter[%d]" % (x) for x in non_zero_indices])
  print "// count = %d" % len(non_zero_indices)
  return predictor

base_dir = "."

for root, dirnames, filenames in os.walk(base_dir):
  for filename in filenames:
    if filename.split('.')[-1] == "lps":
      print filename
      print
      (constant, coeffs) = get_coeffs(filename)
      predictor = gen_predictor(constant, coeffs)
      print predictor
      print
      print
