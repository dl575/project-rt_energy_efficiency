#!/usr/bin/python

"""
parse_svm_test.py infile
"""

import re
import sys
import matplotlib.pyplot as plot

if len(sys.argv) != 2:
  raise Exception(__doc__)
filename = sys.argv[1]

f = open(filename, 'r')

thresholds = []
accuracy0 = []
accuracy1 = []
for line in f:
  res = re.search("Threshold of ([0-9]+)", line)
  if res:
    thresholds.append(int(res.group(1)))

  res = re.search("Accuracy = ([0-9\.]+)", line)
  if res:
    if len(accuracy0) == len(accuracy1):
      accuracy0.append(float(res.group(1)))
    else:
      accuracy1.append(float(res.group(1)))

f.close()

# for (threshold, accuracies) in data.iteritems():
#   print "%d, %f, %f" % (threshold, accuracies[0], accuracies[1])

# Plot
fig = plot.figure()
ax1 = plot.subplot(111)
ax1.plot(thresholds, accuracy0, 'bo-')
ax1.plot(thresholds, accuracy1, 'ro-')
ax1.set_ylim([50, 110])
ax1.set_xlabel("Threshold [us]")
ax1.set_ylabel("Accuracy [%]")
ax1.legend(["train0-predict1", "train1-predict0"], loc="lower left")
plot.show()
ax1.scatter
