#!/usr/bin/python

import sys

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
#
sys.path.extend(['.', '..'])

from pycparser import parse_file
from pycparser import c_ast
from pycparser import c_generator

if __name__ == "__main__":
  if len(sys.argv) > 1:
    filename = sys.argv[1]
  else:
    filename = "c_files/test.c"

  ast = parse_file(filename, use_cpp=True)
  ast.show(nodenames=True)
