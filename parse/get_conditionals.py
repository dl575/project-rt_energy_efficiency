#!/usr/bin/python

"""
Print out the conditional expressions in the passed AST file.

get_conditionals.py filename
"""

import re
import sys
import gcc_parse

filename = sys.argv[1]

f = open(filename, 'r')

objects = {}
cond_exprs = []
for line in f:
  res = re.search("^@([0-9]+)", line)
  if res:
    obj_no = int(res.group(1))
    objects[obj_no] = line.strip()

  if "cond_expr" in line:
    cond_exprs.append(line.strip())

f.close()

gcc_parse.objects = objects
for line in cond_exprs:
  #p = gcc_parse.parse(line)
  #print gcc_parse.expand(p, objects)

  root = gcc_parse.expand2(line)
  print root


