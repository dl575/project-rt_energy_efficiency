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

if len(sys.argv) != 3:
  raise Exception(__doc__)
filename = sys.argv[1]
svm_filename = sys.argv[2]

# Parse input file
f = open(filename, 'r')
iterations = 0
time = 0
data = []
# For calculating average time
total_time = 0
count = 0
for line in f:
  iterations = res("iterations = ([0-9]+)", line)(iterations)
  #time = res("Shift and count bits[\s]+> Time: ([0-9\.]+)", line)(time)
  time = res("Time: ([0-9\.]+)", line)(time)

  if time:
    data.append([int(float(time[0])*1000000), int(iterations[0])])
    count += 1
    total_time += int(float(time[0])*1000000)
    time = 0
f.close()

# Print average time
threshold = float(total_time)/count
print threshold

# Write SVM file
svm_file = open(svm_filename, 'w')
for (time, iterations) in data:
  if time > threshold:
    svm_file.write("1 ")
  else:
    svm_file.write("-1 ")
  svm_file.write("1:%d" % iterations)
  svm_file.write("\n")
svm_file.close()

# Plot
import matplotlib.pyplot as plot

iterations = [x[1] for x in data]
times = [x[0] for x in data]
fig = plot.figure()
ax1 = plot.subplot(111)
ax1.plot(iterations, times, 'x')

plot.show()
