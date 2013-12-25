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
slow_frames = []
slow_accuracies = []
fast_frames = []
fast_accuracies = []
train_slow_frames = []
train_slow_accuracies = []
train_fast_frames = []
train_fast_accuracies = []
# True if test set, False if training set data
test = False
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

  if "Testing set:" in line:
    test = True
  elif "Training set:" in line:
    test = False
  if test:
    res = re.search("Slow accuracy: [0-9]+/([0-9]+) = ([0-9\.]+)", line)
    if res:
      slow_frames.append(int(res.group(1)))
      slow_accuracies.append(100*float(res.group(2)))
    res = re.search("Fast accuracy: [0-9]+/([0-9]+) = ([0-9\.]+)", line)
    if res:
      fast_frames.append(int(res.group(1)))
      fast_accuracies.append(100*float(res.group(2)))
  else:
    res = re.search("Slow accuracy: [0-9]+/([0-9]+) = ([0-9\.]+)", line)
    if res:
      train_slow_frames.append(int(res.group(1)))
      train_slow_accuracies.append(100*float(res.group(2)))
    res = re.search("Fast accuracy: [0-9]+/([0-9]+) = ([0-9\.]+)", line)
    if res:
      train_fast_frames.append(int(res.group(1)))
      train_fast_accuracies.append(100*float(res.group(2)))

f.close()

best_percentages = [max(a, b) for (a, b) in zip(slow_percentages, fast_percentages)]

# Plot
fig = plot.figure()
subplot_layout = 410
subplot_count = 0

# SVM accuracy
subplot_count += 1
ax1 = plot.subplot(subplot_layout + 1)
ax1.plot(thresholds, accuracy, 'bo-')
# Mark the actual value of these accuracies
for i in range(len(accuracy)):
  plot.annotate(int(accuracy[i]), (thresholds[i], accuracy[i]))
ax1.plot(thresholds, best_percentages, 'rx', markeredgewidth=2)
# Labels
ax1.set_ylim([50, 110])
ax1.set_xlabel("Threshold [us]")
ax1.set_ylabel("Accuracy [%]")
ax1.set_title(experiment_name)
ax1.legend(("SVM", "Constant guess"), loc="upper left")

"""
# Difference in accuracy between SVM and guessing
subplot_count += 1
ax2 = plot.subplot(subplot_layout + subplot_count)
ax2.plot(thresholds, [(a - b) for (a, b) in zip(accuracy, best_percentages)], 'ko-')
ax2.set_title("SVM - Constant guess")
ax2.set_xlabel("Threshold [us]")
ax2.set_ylabel("Accuracy [%]")
"""

# Accuracy for slow/fast frames
subplot_count += 1
ax3 = plot.subplot(subplot_layout + subplot_count)
# Don't plot if no frames
slow_accuracies = [(slow_accuracies[i] if slow_frames[i] != 0 else -1) for i in range(len(slow_accuracies))]
ax3.plot(thresholds, slow_accuracies, 'ro-')
fast_accuracies = [(fast_accuracies[i] if fast_frames[i] != 0 else None) for i in range(len(fast_accuracies))]
ax3.plot(thresholds, fast_accuracies, 'go-')
ax3.plot(thresholds, [50 for x in range(len(thresholds))], 'r:')
# Labels
ax3.set_xlabel("Threshold [us]")
ax3.set_ylabel("Accuracy [%]")
ax3.set_ylim([-5, 105])
ax3.legend(("Slow frames", "Fast frames"), loc="center left")

# Bar plot showing percentages of slow/fast frames 
subplot_count += 1
ax4 = plot.subplot(subplot_layout + subplot_count)
import numpy
ind = numpy.array(thresholds) #numpy.arange(len(thresholds))
width = (thresholds[1] - thresholds[0])/3
# Test set frames
ax4.bar(ind, slow_frames, width, color='r');
ax4.bar(ind, fast_frames, width, color='g', bottom=slow_frames);
# Labels
ax4.set_title("Test Frames")
ax4.set_xlabel("Threshold [us]")
ax4.set_ylabel("Frames")
ax4.legend(("Slow frames", "Fast frames"))

# Bar plot showing percentage of slow/fast frames in train set
subplot_count += 1
ax5 = plot.subplot(subplot_layout + subplot_count)
ax5.bar(thresholds, train_slow_frames, width, color = 'r')
ax5.bar(thresholds, train_fast_frames, width, color = 'g', bottom=train_slow_frames)
# Labels
ax5.set_title("Train Frames")
ax5.set_xlabel("Threshold [us]")
ax5.set_ylabel("Frames")
ax5.legend(("Slow frames", "Fast frames"))

# Show plot
plot.tight_layout()
plot.show()

