#!/usr/bin/python

"""
usage: parse.py infile svmfile
"""

import sys
import re

"""
Regex search. Returns a function that assignes this data to a variable 
if it is found. If not found, returns function that does nothing.
"""
def res(regex, line):
  r = re.search(regex, line)
  if r:
    return lambda x: r.groups()
  else:
    return lambda x: x

if len(sys.argv) != 3 and len(sys.argv) != 4:
  raise Exception(__doc__)
filename = sys.argv[1]
svm_filename = sys.argv[2]
if len(sys.argv) == 4:
  threshold = int(sys.argv[3])
else:
  threshold = None

# Parse input file
f = open(filename, 'r')
metrics = 0
time = 0
data = []
# For calculating average time
total_time = 0
count = 0
for line in f:
  [metrics] = res("metrics = \[(.+)\]", line)([metrics])

  #time = res("Shift and count bits[\s]+> Time: ([0-9\.]+)", line)(time)
  #time = res("Time: ([0-9\.]+)", line)(time)
  [time] = res("Execution time = ([0-9]+)", line)([time])

  if time:
    data.append([int(time)] + [float(x) for x in metrics.split()])
    count += 1
    total_time += int(time)
    time = 0
f.close()

# Use average time as threshold if none given
avg_time = float(total_time)/count
if threshold == None:
  threshold = avg_time
# Print average time
print "Average time = %f us" % avg_time

import numpy
from sklearn.decomposition import PCA
# Create numpy array of metrics
np_data = numpy.array([x[1:-1] for x in data])
# Perform PCA 
pca = PCA(n_components=5)
pca.fit(np_data)
# print
print "PCA vectors:"
print pca.components_
print
print "PCA variance:"
print pca.explained_variance_ratio_
# Transform data
pca_data = pca.transform(np_data)

# Write SVM file
svm_file = open(svm_filename, 'w')
for (x, datum) in enumerate(data):
  # Classification
  time = datum[0]
  if time > threshold:
    svm_file.write("1 ")
  else:
    svm_file.write("-1 ")
  # Write out metrics
  #for i in range(1, len(datum)):
  #  svm_file.write("%d:%d " % (i, datum[i]))
  for y in range(pca_data.shape[1]):
    svm_file.write("%d:%f " % (y + 1, pca_data[x][y]))
  svm_file.write("\n")
svm_file.close()

# Plot
# import matplotlib.pyplot as plot
# 
# iterations = [x[1] for x in data]
# times = [x[0] for x in data]
# fig = plot.figure()
# ax1 = plot.subplot(111)
# ax1.plot(iterations, times, 'x')
# 
# plot.show()
