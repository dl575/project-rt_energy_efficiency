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
iterations = 0
time = 0
function = ""
# List of found bitcnt functions
functions = []
data = []
# For calculating average time
total_time = 0
count = 0
for line in f:
  [iterations] = res("iterations = ([0-9]+)", line)([iterations])
  #time = res("Shift and count bits[\s]+> Time: ([0-9\.]+)", line)(time)
  #time = res("Time: ([0-9\.]+)", line)(time)
  [function, time] = res("(.+) > Time: ([0-9\.]+)", line)([function, time])

  if time:
    # If function not found before, add it
    if not function in functions:
      functions.append(function)
    data.append([int(float(time)*1000000), int(iterations), functions.index(function)])
    count += 1
    total_time += int(float(time)*1000000)
    time = 0
f.close()

# Use average time as threshold if none given
avg_time = float(total_time)/count
if threshold == None:
  threshold = avg_time
# Print average time
print "Average time = %f us" % avg_time

# Write SVM file
svm_file = open(svm_filename, 'w')
for (time, iterations, function) in data:
  if time > threshold:
    svm_file.write("1 ")
  else:
    svm_file.write("-1 ")
  svm_file.write("1:%d " % iterations)
  # functions starts at 0, shift to start at 2nd feature
  svm_file.write("%d:1 " % (function + 2))
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
