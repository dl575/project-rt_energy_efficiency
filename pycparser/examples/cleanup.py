#!/usr/bin/python

"""
cleanup.py

Classes:
  CleanupVisitor

Functions:
  print_node
"""

import sys

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
#
sys.path.extend(['.', '..'])

from pycparser import parse_file
from pycparser import c_generator
from pycparser import c_ast

class CleanupVisitor(c_ast.NodeVisitor):
  def __init__(self):
    pass
  def generic_visit(self, node):
    # Visit children
    for c_name, c in node.children():
      self.visit(c)

    # Get rid of empty Compound statements
    # e.g.:
    # {
    #   {} // remove this
    # }
    if isinstance(node, c_ast.Compound):
      new_block_items = []
      for c_name, c in node.children():
        if isinstance(c, c_ast.Compound) and not c.block_items:
          # Empty
          pass
        else:
          new_block_items.append(c)
      node.block_items = new_block_items
    elif isinstance(node, c_ast.Case) or isinstance(node, c_ast.Default):
      new_stmts = []
      for c in node.stmts:
        if isinstance(c, c_ast.Compound) and not c.block_items:
          # Empty
          pass 
        else:
          new_stmts.append(c)
      node.stmts = new_stmts

def print_node(node):
  """
  Pretty print the passed AST/node.
  """
  generator = c_generator.CGenerator()
  print generator.visit(node)



if __name__ == "__main__":
  if len(sys.argv) > 1:
    filename = sys.argv[1]
  else:
    print "usage: cleanup.py filename"
    sys.exit()

  # Generate AST
  ast = parse_file(filename, use_cpp=True)
  # Run clean-up
  v = CleanupVisitor()
  v.visit(ast)
  # Output generated slice
  print_node(ast)
