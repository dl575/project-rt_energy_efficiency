#!/usr/bin/python

from parse_lib import *
import numpy
import logging
from pylab import *
import sys

logging.basicConfig(level=logging.INFO)

if len(sys.argv) < 2:
  print "usage: regression.py file.txt"
  sys.exit()
filename = sys.argv[1]

# Parse data from file
times = parse_execution_times(filename)
times = [(1 if x == 0 else x) for x in times] # Replace 0 times with min (1)
times = numpy.array(times)
loop_data = parse(filename, "loop counter [0-9]+ = \((.*)\)")
loop_data = [[int(y) for y in x.strip(', ').split(',')] for x in loop_data]
loop_data = numpy.array(loop_data)
logging.debug(times)
logging.debug(loop_data)

# Parse callback addresses
f = open(filename, 'r')
callback_data = {}
for line in f:
  res = re.search("callback ([0-9]+) = ([0-9a-f]+)", line)
  if res:
    if res.group(1) in callback_data.keys():
      raise Exception("Multiple callbacks for one frame found.")
    callback_data[res.group(1)] = res.group(2)
f.close()
unique_addresses = list(set(callback_data.values()))
print unique_addresses
# Add extra columns to loop_data
translation = {}
for i in range(len(unique_addresses)):
  loop_data = numpy.hstack((loop_data, numpy.array([[0]*loop_data.shape[0]]).T))
  # Record which column the address maps to
  translation[unique_addresses[i]] = loop_data.shape[1] - 1
# Add callback counts
for (frame, address) in callback_data.iteritems():
  loop_data[int(frame)][translation[address]] += 1

# Cut off start and end due initialization and quitting outliers
times = times[500:-500]
loop_data = loop_data[500:-500]

# Regression
# Remove constant metrics
loop_data = data_remove_constant_cols(loop_data)
logging.debug("Constants removed:")
logging.debug(loop_data)
# Add constant column
loop_data = numpy.hstack((numpy.array([[1]*loop_data.shape[0]]).T, loop_data))

# Perform linear regression
logging.debug("Size of times")
logging.debug(times.shape)
logging.debug("Size of loop data")
logging.debug(loop_data.shape)
(coeffs, residuals, rank, s) = numpy.linalg.lstsq(loop_data, numpy.transpose(times))
logging.debug(coeffs)
logging.debug(residuals)
logging.debug(rank)
logging.debug(s)

# Calculate predicted times
predicted_times = [numpy.dot(loop_data[i][:], coeffs) for i in range(loop_data.shape[0])]
times_sorted = False
logging.debug("Actual times: %s" % times)
logging.debug("Predicted times: %s" % predicted_times)

# Predicted vs. actual execution time
figure()
suptitle(filename)
subplot(2, 2, 1)
plot(times, 'bo--')
plot(predicted_times, 'ro')
xlabel("Instance")
ylabel("Execution Time [us]")
legend(["Actual", "Predicted"])

# # Zoomed in
# #figure()
# subplot(2, 2, 2)
# plot(times, 'bo--')
# plot(predicted_times, 'ro')
# xlabel("Instance")
# ylabel("Execution Time [us]")
# xlim([0, 50])
# legend(["Actual", "Predicted"])

# Sorted
# Sort by actual times
#all_times = zip(times, predicted_times)
#all_times = sorted(all_times, key=lambda x:x[0])
#[times, predicted_times] = zip(*all_times)
#subplot(2, 2, 2)
#plot(times, 'bo--')
#plot(predicted_times, 'ro')
#xlabel("Instance")
#ylabel("Execution Time [us]")
#legend(["Actual", "Predicted"])
#times_sorted = True

# Predicted vs. Actual Scatter
subplot(2, 2, 2)
scatter(times, predicted_times)
plot(times, times, 'r--')
xlabel("Actual Time [us]")
ylabel("Predicted Time [us]")

# Accuracy
#figure()
subplot(2, 2, 3)
errors = [float(x - y)/y*100 for (x, y) in zip(predicted_times, times)]
plot(errors, 'bo')
if times_sorted:
  xlabel("Sorted Instance")
else:
  xlabel("Instance")
ylabel("Normalized Error [%]")

# Histogram of errors
#plot_histogram(errors, xlabel="Normalized error [%]")
subplot(2, 2, 4)
n, bins, patches = hist(errors, bins=50)


show()
