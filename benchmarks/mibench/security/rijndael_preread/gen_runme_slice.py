#!/usr/bin/python

import random
import platform

random.seed(0)

def gen_key():
  """
  Generate a 256-bit key in hexadecimal format.
  """
  hex_digits = "0123456789abcdef"
  key = ""
  for i in range(64):
    key += random.choice(hex_digits)
  return key

def get_arch():
  arch = platform.machine()
  if arch == "armv7l":
    ret = "odroid"
  elif arch == "x86_64":
    ret = "x86"
  else:
    ret = "unknown"
  return ret

output = "#!/bin/bash\n"
output += "./rijndael "
input_range = range(1, 100)
for j in xrange(0, 10):
  for i in input_range:
    output += "../../../../datasets/"+get_arch()+"/rijndael-50ms/input%d.txt output_large.enc e %s " % (i, gen_key())
output += "> output_slice.txt\n"
print output

