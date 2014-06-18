#!/usr/bin/python

"""
Compare results of SVM testing to actual classification. Outputs
percentage of correct results for each class. Old version assumed
only +1 or -1. This version generalizes to any number of classes.

compare_svm_results2.py truthfile resultsfile
"""

import sys

if len(sys.argv) != 3:
  raise Exception(__doc__)
truth_filename = sys.argv[1]
results_filename = sys.argv[2]

truth_file = open(truth_filename, 'r')
results_file = open(results_filename, 'r')

# correct[i] is number of correctly predicted data points for class i
correct = {}
# total[i] is the total number of data points in class i
total = {}
# Read from results file
for results_line in results_file:
  # Read from truth file
  truth_line = truth_file.readline()

  # Result and truth value
  result = int(results_line.strip().split()[0])
  truth = int(truth_line.split()[0])

  # Class not already encountered
  if truth not in total:
    total[truth] = 1
    # Correct prediction
    if result == truth: # or result + 1 == truth or result - 1 == truth:
      correct[truth] = 1
    # Incorrect prediction
    else:
      correct[truth] = 0
  else:
    total[truth] += 1
    # Correct prediction
    if result == truth: # or result + 1 == truth or result - 1 == truth:
      correct[truth] += 1

truth_file.close()
results_file.close()

print "Accuracy:"
for k in total.keys():
  print "  %d: %d/%d = %f" % (k, correct[k], total[k], float(correct[k])/total[k])
print "  Total: %d/%d = %f" % (sum(correct.values()), sum(total.values()), float(sum(correct.values()))/sum(total.values()))

