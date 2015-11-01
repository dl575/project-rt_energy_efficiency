#!/usr/bin/python

import platform

def get_arch():
  arch = platform.machine()
  if arch == "armv7l":
    ret = "odroid"
  elif arch == "x86_64":
    ret = "x86"
  else:
    ret = "unknown"
  return ret


#input_range = range(1, 101)

input_range = range(1, 100)

output = "#!/bin/bash\n"
output += "./sha "
for j in xrange(0, 1):
  for i in input_range:
    output += "../../../../datasets/"+get_arch()+"/sha-50ms/input%d.txt " % (i)
output += "> output_slice.txt\n"

print output
