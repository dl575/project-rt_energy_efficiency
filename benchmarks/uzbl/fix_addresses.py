#!/usr/bin/python

import re
import subprocess

def get_address(func_name, objdump):
  func_name = func_name.lower()
  for line in objdump:
    if line and func_name == line.split()[-1]:
      addr = line.split()[0]
  return addr

# Get symbol table from binary
binary = "../../benchmarks/uzbl/uzbl-core"
objdump = subprocess.check_output(["objdump", "-tT", binary])
objdump = objdump.split('\n')

# Find and replace addresses
include_file = []
fin = open("src/function_addresses.h", 'r')
fout = open("src/temp.h", 'w')
for line in fin:
  res = re.search("#define ADDR_(.*) ", line)
  if res:
    addr = get_address(res.group(1), objdump)
    fout.write("#define ADDR_%s 0x%s\n" % (res.group(1), addr))
  else:
    fout.write(line)
fin.close()
fout.close()
# Replace original file
subprocess.call(["mv", "src/temp.h", "src/function_addresses.h"])

