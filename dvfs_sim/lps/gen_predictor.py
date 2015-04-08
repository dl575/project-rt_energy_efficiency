#!/usr/bin/python

import os

def get_coeffs(filename):
  coeffs = []
  f = open(filename, 'r')
  for line in f:
    if line[0] == 'b':
      coeffs.append(float(line.split()[1]))
  f.close()
  return coeffs

def gen_predictor(coeffs):
  predictor = "float exec_time;\n"
  predictor += "exec_time = "
  non_zero_indices = []
  for (ci, c) in enumerate(coeffs[:-1]):
    if c != 0:
      non_zero_indices.append(ci)
      predictor += "%f*loop_counter[%d] + " % (c, ci)
  predictor += "%f;\n" % coeffs[-1]
  predictor += "printf(\"predicted time = %f\\n\", exec_time);"
  print "// non-zero coeffs = %d" % len(non_zero_indices)
  print "// loop counters: ", ' '.join(["loop_counter[%d]" % (x) for x in non_zero_indices])
  return predictor

base_dir = "."

for root, dirnames, filenames in os.walk(base_dir):
  for filename in filenames:
    if filename.split('.')[-1] == "lps":
      print filename
      print
      coeffs = get_coeffs(filename)
      predictor = gen_predictor(coeffs)
      print predictor
      print
      print
