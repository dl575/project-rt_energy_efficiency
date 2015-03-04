#!/usr/bin/python

import sys
import subprocess
sys.path.extend([".."])
from parse_lib import *

def get_func_ptrs(filename):
  func_ptrs = parse(filename, "function pointer = \((.*)\)")
  func_ptrs = [x.strip().strip(',').split(',') for x in func_ptrs]
  func_ptrs = [map(lambda x: int(x, 16), x) for x in func_ptrs]
  return func_ptrs

def get_unique_func_ptrs(func_ptrs):
  unique_func_ptrs = list(set(reduce(lambda x, y: x+y, func_ptrs)))
  return unique_func_ptrs

def get_function_name(func_ptr, objdump):
  hex_addr = "%x" % (func_ptr)
  for line in objdump:
    if hex_addr in line:
      return line.split()[-1]

filename = "data/uzbl/uzbl0.txt"
index_offset = 3

# Find unique function pointers observed in profiling
func_ptrs = get_func_ptrs(filename)
unique_func_ptrs = get_unique_func_ptrs(func_ptrs)

# Get symbol table from binary
inary = "../../benchmarks/uzbl/uzbl-core"
objdump = subprocess.check_output(["objdump", "-tT", binary])
objdump = objdump.split('\n')

# Get function names
function_names = map(lambda x: get_function_name(x, objdump), unique_func_ptrs)
# Convert to define strings
function_names = ["ADDR_" + x.upper() for x in function_names]

# Write out loop counter code
f = open("function_addresses.c", 'w')
f.write("switch(func_ptr) {\n")
for (i, fn) in enumerate(function_names):
  f.write("  case %s :\n" % (fn))
  f.write("    loop_counter[%d] = 1;\n" % (i + index_offset))
  f.write("    break;\n")
f.write("  default :\n")
f.write("    loop_counter[%d] = 1;\n" % (i + index_offset + 1))
f.write("}\n")
f.close()

# Write out define file
f = open("function_addresses.h", 'w')
f.write("#ifndef __FUNCTION_ADDRESSES_H__\n")
f.write("#define __FUNCTION_ADDRESSES_H__\n\n")
for (fn, fp) in zip(function_names, unique_func_ptrs):
  f.write("#define %s 0x%x\n" % (fn, fp))
f.write("\n#endif // __FUNCTION_ADDRESSES_H__ \n")
f.close()
