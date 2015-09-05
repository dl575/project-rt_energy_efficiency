"""
parse_lib.py

Functions:
  average

  parse
  parse_int
  parse_float
  find
  
  deadline_misses
"""

import re

def average(l):
  return float(sum(l))/len(l)

def parse(filename, regex, start=None):
  """
  Run the regex passed on each line in filename. Values for group(1) of the
  regex are stored in a list and returned. start specifies a regex to search
  for before starting to save data.
  """
  f = open(filename, 'r')

  data = []
  if start:
    enabled = False
  else:
    enabled = True
  for line in f:
    # Not enabled, check start condition
    if not enabled:
      res = re.search(start, line)
      if res:
        enabled = True
    # Enabled, store data
    else:
      res = re.search(regex, line)
      if res:
        data.append(res.group(1))

  f.close()

  return data

def parse_int(filename, regex, start=None):
  data = parse(filename, regex, start)
  data = map(int, data)
  return data
def parse_float(filename, regex, start=None):
  data = parse(filename, regex, start)
  data = map(float, data)
  return data

def find(filename, regex):
  """
  Find first instance of regex match in filename and return group(1).
  If not found, return None.
  """
  f = open(filename, 'r')
  data = None
  for line in f:
    res = re.search(regex, line)
    if res:
      data = res.group(1)
      break
  f.close()
  return data

def deadline_misses(times, deadline, margin=1.0):
  misses = 0.0
  for t in times:
    if t > deadline*margin:
      misses += 1
  return 100*misses/len(times)

def deadline_misses_proactive(times, deadline, jumps, groups, margin=1.0):
  misses = 0.0
  t_sum = 0.0
  cnt = 0
  #global_cnt = 0
  for t, jump, group in zip(times, jumps, groups):
    if jump == 0 & group == 1:
      t_sum = 0.0
      cnt = 0
      if t > deadline*margin:
        misses += 1
        #print "jump zero"+str(global_cnt)
    else:
      t_sum += t
      cnt += 1
      if t_sum > cnt*deadline*margin:
        misses += 1
        #print "jump not zero"+str(global_cnt)
    #global_cnt +=1
  return 100*misses/len(times)

def tardiness(times, deadline, margin=1.0):
  tardiness_total = 0.0
  misses = 0
  for t in times:
    if t > deadline*margin:
      misses += 1
      tardiness_total += t - deadline*margin
  if misses:
    return tardiness_total/misses
  else:
    return 0

