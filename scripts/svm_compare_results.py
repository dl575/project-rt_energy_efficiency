#!/usr/bin/python

"""
Compare results of SVM testing to actual classification. Outputs
percentage of correct results for each class.

compare_svm_results.py truthfile resultsfile
"""

import sys

if len(sys.argv) != 3:
  raise Exception(__doc__)
truth_filename = sys.argv[1]
results_filename = sys.argv[2]

truth_file = open(truth_filename, 'r')
results_file = open(results_filename, 'r')

slow_correct = 0
slow_total = 0
fast_correct = 0
fast_total = 0
for results_line in results_file:
  truth_line = truth_file.readline()

  result = results_line.strip()
  truth = truth_line.split()[0]

  # Slow frames
  if truth == "1" or truth == "+1":
    slow_total += 1
    if result == truth:
      slow_correct += 1
  # Fast frames
  elif truth == "-1":
    fast_total += 1
    if result == truth:
      fast_correct += 1
  else:
    raise Exception("Unknown result")

truth_file.close()
results_file.close()

if slow_total:
  print "Slow accuracy: %d/%d = %f" % (slow_correct, slow_total, float(slow_correct)/slow_total)
else:
  print "Slow accuracy: 0/0 = 0"
if fast_total:
  print "Fast accuracy: %d/%d = %f" % (fast_correct, fast_total, float(fast_correct)/fast_total)
else:
  print "Fast accuracy: 0/0 = 0"
