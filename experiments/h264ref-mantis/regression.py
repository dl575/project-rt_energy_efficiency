#!/usr/bin/python

from parse_lib import *
import numpy
import sys
import math

if len(sys.argv) != 2:
  print "usage: regression.py filename"
  sys.exit()
filename = sys.argv[1]

# Parse execution times
times = numpy.array(parse_execution_times(filename))
# Parse metrics
metrics = numpy.array(parse_metrics(filename))
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
(coeffs, residuals, rank, s) = numpy.linalg.lstsq(metrics, numpy.transpose(times))
print coeffs
print residuals
print rank
print s

# pylab.scatter(metrics[:, 6], times)
# pylab.plot(range(100), [x[6]*y+x[0] for y in range(100)], 'r')
# pylab.show()

# Calculate predicted times
predicted_times = [numpy.dot(metrics[i][:], coeffs) for i in range(metrics.shape[0])]

# Predicted versus actual execution times
pylab.figure()
pylab.plot(times)
pylab.plot(predicted_times, 'r.')
pylab.xlim((0, 1000))
pylab.ylim((0, 2000))

# Determine accuracies
errors = []
for row in range(metrics.shape[0]):
  error = (predicted_times[row] - times[row])/times[row]
  errors.append(error)
plot_histogram(errors)
# Accuracy if average time is used
errors = []
average_time = numpy.average(times)
for row in range(times.shape[0]):
  error = (average_time - times[row])/times[row]
  errors.append(error)
plot_histogram(errors)

# Exec time vs. metrics
pylab.figure()
subplot_side = math.ceil(math.sqrt(metrics.shape[1]))
for i in range(metrics.shape[1]):
  pylab.subplot(subplot_side, subplot_side, i+1)
  pylab.plot(metrics[:, i], times, '.')

# pylab.figure()
# subplot_side = 4
# for i in range(subplot_side*subplot_side):
#   pylab.subplot(subplot_side, subplot_side, i+1)
#   pylab.plot([m % i for m in metrics[:, 3]], times, '.')


pylab.show()


