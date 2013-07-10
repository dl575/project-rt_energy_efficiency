#!/usr/bin/python

"""
parse_svm_test.py infile
"""

import re
import sys
import matplotlib.pyplot as plot

if len(sys.argv) != 2 and len(sys.argv) != 3:
  raise Exception(__doc__)
filename = sys.argv[1]
if len(sys.argv) == 3:
  experiment_name = sys.argv[2]
else:
  experiment_name = ""

f = open(filename, 'r')

thresholds = []
accuracy = []
slow_percentages = []
fast_percentages = []
# Percentage of each frame type correctly classified
slow_accuracies = []
fast_accuracies = []
for line in f:
  res = re.search("Threshold of ([0-9]+)", line)
  if res:
    thresholds.append(int(res.group(1)))

  res = re.search("Accuracy = ([0-9\.]+)", line)
  if res:
    accuracy.append(float(res.group(1)))

  res = re.search("Slow frames = ([0-9\.]+)", line)
  if res:
    slow_percentages.append(100*float(res.group(1)))
  res = re.search("Fast frames = ([0-9\.]+)", line)
  if res:
    fast_percentages.append(100*float(res.group(1)))

  res = re.search("Slow accuracy: [0-9]+/[0-9]+ = ([0-9\.]+)", line)
  if res:
    slow_accuracies.append(100*float(res.group(1)))
  res = re.search("Fast accuracy: [0-9]+/[0-9]+ = ([0-9\.]+)", line)
  if res:
    fast_accuracies.append(100*float(res.group(1)))

f.close()

best_percentages = [max(a, b) for (a, b) in zip(slow_percentages, fast_percentages)]

# for (threshold, accuracies) in data.iteritems():
#   print "%d, %f, %f" % (threshold, accuracies[0], accuracies[1])

print thresholds
print accuracy

# Plot
fig = plot.figure()

ax1 = plot.subplot(311)
ax1.plot(thresholds, accuracy, 'bo-')
ax1.plot(thresholds, best_percentages, 'rx', markeredgewidth=2)
ax1.set_ylim([50, 110])
ax1.set_xlabel("Threshold [us]")
ax1.set_ylabel("Accuracy [%]")
ax1.set_title(experiment_name)
ax1.legend(("SVM", "Constant guess"), loc="lower left")

ax2 = plot.subplot(312)
ax2.plot(thresholds, [(a - b) for (a, b) in zip(accuracy, best_percentages)], 'ko-')
ax2.set_title("SVM - Constant guess")
ax2.set_xlabel("Threshold [us]")
ax2.set_ylabel("Accuracy [%]")

ax3 = plot.subplot(313)
ax3.plot(thresholds, slow_accuracies, 'bo-')
ax3.plot(thresholds, fast_accuracies, 'go-')
ax3.plot(thresholds, [50 for x in range(len(thresholds))], 'r:')
ax3.set_xlabel("Threshold [us]")
ax3.set_ylabel("Accuracy [%]")
ax3.set_ylim([-5, 105])
ax3.legend(("Slow frames", "Fast frames"), loc="center left")

plot.tight_layout()
plot.show()
