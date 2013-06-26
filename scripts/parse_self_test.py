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
accuracy = []
for line in f:
  res = re.search("Threshold of ([0-9]+)", line)
  if res:
    thresholds.append(int(res.group(1)))

  res = re.search("Accuracy = ([0-9\.]+)", line)
  if res:
    accuracy.append(float(res.group(1)))

f.close()

# for (threshold, accuracies) in data.iteritems():
#   print "%d, %f, %f" % (threshold, accuracies[0], accuracies[1])

# Plot
fig = plot.figure()
ax1 = plot.subplot(111)
ax1.plot(thresholds, accuracy, 'bo-')
ax1.set_ylim([50, 110])
ax1.set_xlabel("Threshold [us]")
ax1.set_ylabel("Accuracy [%]")
ax1.set_title("mu-train100")
plot.show()
ax1.scatter
