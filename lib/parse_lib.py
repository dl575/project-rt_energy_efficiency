#!/usr/bin/python
"""
This file includes useful functions for parsing, importing, exporting, and
plotting data.

Functions:
  average(l)
  geomean(l)
  normalize(l)
  median(l)
  list_to_int(l)

  data_remove_constant_cols(array)

  parse_execution_times(filename)
  parse_frame_times(filename)
  parse_metrics(filename)
  parse(filename, regex)

  plot_histogram(data, nbins, figsize, xlabel, ylabel)
  plot_sequence(data, figsize, xlabel, ylabel)
  plot_show()
  plot_to_pdf(filename)
  plot_set_fonts(font)
"""

import re
import pylab

# Default fonts for plots
default_font = {"family" : "Times New Roman",
    "size" : 9}
# Default figsize for viewing
figsize_default = (8.0, 6.0)
# Single column figure size
figsize_singlecol = (3.5, 3.0)
# Double column figure size
figsize_doublecol = (7.0, 3.0)

"""
Return the arithmetic mean of the passed list.
"""
def average(l):
  # Remove "None" entries
  l2 = []
  for ll in l:
    if ll != None:
      l2.append(ll)
  if not l2:
    return '-'
  else:
    return float(sum(l2))/len(l2)

"""
Return the geometric mean of the passed list.
"""
def geomean(l):
  prod = 1.0
  n = 0
  for ll in l:
    prod *= ll
    n += 1
  return prod**(1.0/n)

"""
Divide the entries from the first passed list by the values in the second
list.
"""
def normalize(data, baseline):
  assert len(data) == len(baseline), "Length of lists (%d != %d) do not match" % (len(data), len(baseline))
  return [float(data[i])/float(baseline[i]) for i in range(len(data))]



"""
Return the median of the pass list.
"""
def median(l):
  n = len(l)
  l = sorted(l)
  if n % 2 == 0:
    return float(l[n/2] + l[n/2 - 1])/2
  else:
    return l[n/2]

"""
Return passed list with all elements converted to integers.
"""
def list_to_int(l):
  return [int(ll) for ll in l]

"""
Remove columns from the passed numpy array which are always constant. The
passed array is assumed to be 2-dimensional.
"""
def data_remove_constant_cols(array):
  (nrows, ncols) = array.shape
  delete_cols = []
  non_delete_cols = []
  for c in range(ncols):
    same = True
    for r in range(1, nrows):
      # Non constant
      if array[r][c] != array[r-1][c]:
        same = False
        break
    # If constant, save column number to be deleted
    if same:
      delete_cols.append(c)
    else:
      non_delete_cols.append(c)
  # Delete columns
  ndeleted_cols = 0
  for c in delete_cols:
    array = pylab.delete(array, c-ndeleted_cols, axis = 1)
    # Reduce future column numbers now that array shape has changed
    ndeleted_cols += 1
  return array

"""
Parse based on the passed regular expression.
"""
def parse(filename, regex):
  f = open(filename, 'r')
  data = []
  for line in f:
    res = re.search(regex, line)
    if res:
      data.append(res.group(1))
  f.close()
  return data
"""
Parse based on the passed regular expression. Return list of ints.
"""
def parse_int(filename, regex):
  data = parse(filename, regex)
  return list_to_int(data)



"""
Parses the included files for execution times of frames. Returns a list of
these times.
Times are assumed to be recorded in the following format:
  Frame [0-9]+ = [0-9]+us
  $1 is the frame instance.
  $2 is the execution time in microseconds.
"""
def parse_frame_times(filename):
  f = open(filename, 'r')
  times = []
  for line in f:
    res = re.search("Frame ([0-9]+) = ([0-9]+)us", line)
    if res:
      times.append(int(res.group(2)))
  f.close()
  return times

"""
Parse the file for metrics. These are recorded in the following
format:
    .*metrics = (a, b, c, ...)
"""
def parse_metrics(filename):
  f = open(filename, 'r')
  metrics = []
  for line in f:
    res = re.search("metrics = \((.*)\)", line)
    if res:
      metrics.append([int(x[0:-1]) for x in res.group(1).split()])
  f.close()
  return metrics

"""
Parses the included files for execution times (of jobs). Returns a list of
these times.
Times are assumed to be recorded in the following format:
  time [0-9]+ = [0-9]+ us
  $1 is the job instance.
  $2 is the execution time in microseconds.
"""
def parse_execution_times(filename):
  f = open(filename, 'r')
  times = []
  for line in f:
    res = re.match("time ([0-9]+) = ([0-9]+) us", line)
    if res:
      times.append(int(res.group(2)))
  f.close()
  return times

"""
Plots the passed data into a histogram. data is a list of numbers.
If the number of bins is not specified, then the number of data items
divided by 100 is used.
"""
def plot_histogram(data, nbins=None, figsize=figsize_default, xlabel=None, ylabel=None):
  if not nbins:
    nbins = 50
  pylab.figure(figsize=figsize)
  n, bins, patches = pylab.hist(data, bins=nbins)
  if xlabel:
    pylab.xlabel(xlabel)
  if ylabel:
    pylab.ylabel(ylabel)
  return (n, bins, patches)

"""
Plot the passed data as a scatter plot. Items in data are plotted
sequentially.
"""
def plot_sequence(data, figsize=figsize_default, xlabel=None, ylabel=None):
  pylab.figure(figsize=figsize)
  pylab.plot(data, linestyle="none", marker="o", markersize=3)
  if xlabel:
    pylab.xlabel(xlabel)
  if ylabel:
    pylab.ylabel(ylabel)
  pylab.xlim(0, len(data) - 1)

"""
Plot the passed data as a scatter plot.
"""
def plot_scatter(x, y, figsize=figsize_default, xlabel=None, ylabel=None):
  pylab.figure(figsize=figsize)
  pylab.plot(x, y, marker="o", markersize=3)
  if xlabel:
    pylab.xlabel(xlabel)
  if ylabel:
    pylab.ylabel(ylabel)

"""
Show all plots that have been setup.
"""
def plot_show():
  pylab.tight_layout()
  pylab.show()

"""
Save the last plot to pdf.
"""
def plot_to_pdf(filename):
  pylab.savefig(filename, format="pdf")

"""
Set all fonts for plots. font is a dict with font information.
"""
def plot_set_fonts(font=default_font):
  pylab.rc("font", **font)
