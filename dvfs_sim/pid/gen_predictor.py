#!/usr/bin/python

import os
import re

def read_PID(filename):
  f = open(filename, 'r')
  for line in f:
    res = re.search("([PID]) = ([0-9\.]+)", line)
    if res:
      if res.group(1) == 'P':
        P = float(res.group(2))
      elif res.group(1) == 'I':
        I = float(res.group(2))
      elif res.group(1) == 'D':
        D = float(res.group(2))
  f.close()
  return (P, I, D)

def pid_to_defines(pid):
  s = ""
  s += "#define PID_P %f\n" % (pid[0])
  s += "#define PID_I %f\n" % (pid[1])
  s += "#define PID_D %f\n" % (pid[2])
  return s

base_dir = "."

for root, dirnames, filenames in os.walk(base_dir):
  for filename in filenames:
    if filename.split('.')[-1] == "lps":
      print filename
      print
      pid = read_PID(filename)
      defines = pid_to_defines(pid)
      print defines
