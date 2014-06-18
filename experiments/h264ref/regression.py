#!/usr/bin/python

from parse_lib import *
import numpy
import sys

if len(sys.argv) != 2:
  print "usage: regression.py filename"
  sys.exit()
filename = sys.argv[1]

# Parse execution times
times = numpy.array(parse_execution_times(filename))
# Parse metrics
metrics = numpy.array(parse_auto_metrics(filename))
# Remove constant metrics
metrics = data_remove_constant_cols(metrics)
# Add constant column
metrics = numpy.hstack((numpy.array([[1]*metrics.shape[0]]).T, metrics))
times = times[0:metrics.shape[0]]
print metrics
print times
print metrics.shape
print times.shape

# Linear regression
(x, residuals, rank, s) = numpy.linalg.lstsq(metrics, numpy.transpose(times))
print x
print residuals
print rank
print s

# pylab.scatter(metrics[:, 6], times)
# pylab.plot(range(100), [x[6]*y+x[0] for y in range(100)], 'r')
# pylab.show()

# Determine accuracies
errors = []
for row in range(metrics.shape[0]):
  predicted_time = numpy.dot(metrics[row][:], x)
  error = (predicted_time - times[row])/times[row]
  errors.append(error)
plot_histogram(errors)
# Accuracy if average time is used
errors = []
average_time = numpy.average(times)
for row in range(times.shape[0]):
  error = (average_time - times[row])/times[row]
  errors.append(error)
plot_histogram(errors)
pylab.show()
