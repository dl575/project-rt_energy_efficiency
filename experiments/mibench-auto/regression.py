#!/usr/bin/python

from parse_lib import *
import numpy
import logging
from pylab import *

logging.basicConfig(level=logging.INFO)

if len(sys.argv) < 2:
  print "usage: regression.py file.txt"
  sys.exit()
filename = sys.argv[1]

# Parse data from file
times = parse_execution_times(filename)
times = numpy.array(times)
loop_data = parse(filename, "loop counter = \((.*)\)")
loop_data = [[int(y) for y in x.strip(', ').split(',')] for x in loop_data]
loop_data = numpy.array(loop_data)
logging.debug(times)
logging.debug(loop_data)

# Regression
# Remove constant metrics
loop_data = data_remove_constant_cols(loop_data)
# Add constant column
loop_data = numpy.hstack((numpy.array([[1]*loop_data.shape[0]]).T, loop_data))

# Perform linear regression
(coeffs, residuals, rank, s) = numpy.linalg.lstsq(loop_data, numpy.transpose(times))
logging.debug(coeffs)
logging.debug(residuals)
logging.debug(rank)
logging.debug(s)

# Calculate predicted times
predicted_times = [numpy.dot(loop_data[i][:], coeffs) for i in range(loop_data.shape[0])]
# Sort by actual times
# all_times = zip(times, predicted_times)
# all_times = sorted(all_times, key=lambda x:x[0])
# [times, predicted_times] = zip(*all_times)

# Predicted vs. actual execution time
figure()
suptitle(filename)
subplot(2, 2, 1)
plot(times, 'bo--')
plot(predicted_times, 'ro')
xlabel("Instance")
ylabel("Execution Time [us]")
legend(["Actual", "Predicted"])
# Zoomed in
#figure()
subplot(2, 2, 2)
plot(times, 'bo--')
plot(predicted_times, 'ro')
xlabel("Instance")
ylabel("Execution Time [us]")
xlim([0, 50])
legend(["Actual", "Predicted"])
# Accuracy
#figure()
subplot(2, 2, 3)
errors = [float(x - y)/y*100 for (x, y) in zip(predicted_times, times)]
plot(errors, 'bo')
xlabel("Instance")
ylabel("Normalized Error [%]")
# Histogram of errors
#plot_histogram(errors, xlabel="Normalized error [%]")
subplot(2, 2, 4)
n, bins, patches = hist(errors, bins=50)

show()
