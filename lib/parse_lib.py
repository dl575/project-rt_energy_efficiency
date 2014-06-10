#!/usr/bin/python
"""
This file includes useful functions for parsing, importing, exporting, and
plotting data.

Functions:
  average(l)
  geomean(l)

  parse_execution_times(filename)

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
    return sum(l2)/len(l2)

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
    res = re.search("time ([0-9]+) = ([0-9]+) us", line)
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
