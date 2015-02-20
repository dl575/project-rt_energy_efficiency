#!/usr/bin/python

"""
Iteratively runs make adding custom type declarations to the Makefile until no
errors are found.
"""

import os

ret = 1
# While make fails
while ret != 0:
  # Run make
  ret = os.system("make &> make.log")
  # Find line number where it failed
  f = open("make.log", 'r')
  for line in f:
    if line.split('.')[0] == "pycparser":
      print line,
      filename = line.split(' ')[1].split(':')[0]
      line_number = int(line.split(' ')[1].split(':')[1])
  f.close()

  # Find the line that failed
  f = open(filename, 'r')
  for (i, line) in enumerate(f):
    if i+1 == line_number:
      err_line = line
      print err_line,
      break
  f.close()

  # Assume that first word is the undefined type
  for word in err_line.strip().split(' '):
    if word != "extern":
      err_type = word
      break
  print err_type
  # Add to Makefile
  fin = open("Makefile", 'r')
  fout = open("Makefile.tmp", 'w')
  for line in fin:
    fout.write(line)
    if "# Define custom types" in line:
      fout.write("\t$(call td, \"%s\")\n" % err_type)
  fin.close()
  fout.close()
  # Use new Makefile
  os.system("mv Makefile.tmp Makefile")

  print
