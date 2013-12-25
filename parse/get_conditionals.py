#!/usr/bin/python

"""
Print out the conditional expressions in the passed AST file.

get_conditionals.py filename
"""

import re
import sys
import gcc_parse

if len(sys.argv) != 2:
  print __doc__
  sys.exit()
filename = sys.argv[1]

f = open(filename, 'r')

# Dict of all nodes <node number>:<string expression>
objects = {}
# List of condtional expressions
cond_exprs = []
# Parse out node strings from file
for line in f:
  res = re.search("^@([0-9]+)", line)
  if res:
    obj_no = int(res.group(1))
    objects[obj_no] = line.strip()

  if "cond_expr" in line:
    cond_exprs.append(line.strip())

f.close()

# Feed gcc_parse library the object strings
gcc_parse.objects = objects
variables = []
# For each conditional expression
for line in cond_exprs:
  # Expand out conditional expressions
  root = gcc_parse.parse(line)
  #print root.id, root
  variables = root.get_variables(variables)

# Print out printf for each variable
for v in variables:
  print 'printf("%%d, ", (int)%s);' % (v)

