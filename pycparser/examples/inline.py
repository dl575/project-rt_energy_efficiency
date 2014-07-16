#!/usr/bin/python

import copy
import sys

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
#
sys.path.extend(['.', '..'])

from pycparser import parse_file
from pycparser import c_ast
from pycparser import c_generator

"""
Replaces return statements with a goto.
"""
class ReturnToGotoVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.goto_ID = None
  def generic_visit(self, node):
    for ci, (c_name, c) in enumerate(node.children()):
      if isinstance(c, c_ast.Return):
        insert_node = c_ast.Compound([])
        if c.expr:
          insert_node.block_items.append(c_ast.Assignment("=", c_ast.ID("return_value"), c.expr))
        insert_node.block_items.append(c_ast.Goto(self.goto_name))
        if isinstance(node, c_ast.Compound):
          node.block_items[ci] = insert_node
        elif isinstance(node, c_ast.If):
          exec("node.%s = insert_node" % c_name)
        else:
          raise Exception("Unsupported parent node type %s. Please implement." % (type(node)))
      else:
        self.visit(c)

"""
Used to take a function and inline all functions it calls.
"""
class ExpandFunctionVisitor(c_ast.NodeVisitor):
  def __init__(self, funcname, functions):
    self.funcname = funcname
    self.functions = functions
    self.expanded = False

    self.rtg_visitor = ReturnToGotoVisitor()
    self.return_counter = 0
  def visit_FuncDef(self, node):
    if (node.decl.name == self.funcname):
      # Find and replace function calls
      self.expand_visit(node)
  """ 
  Replaced nodes with full function.
  """
  def expand_visit(self, node):
    # For each child
    for ci, (c_name, c) in enumerate(node.children()):

      #################################
      # Assignment
      #################################
      if isinstance(c, c_ast.Assignment):
        if isinstance(c.rvalue, c_ast.FuncCall):
          for function in self.functions:
            if c.rvalue.name.name == function.decl.name:
              inline_function = self.create_inline_function(function, c.rvalue)
              # Handle return value
              inline_function.body.block_items.append(c_ast.Assignment("=", c.lvalue, c_ast.ID("return_value")))
              if isinstance(node, c_ast.Compound):
                node.block_items[ci] = inline_function.body
              else:
                raise Exception("Unsupported parent node type %s. Please implement." % (type(node)))
              node.expanded = True

      #################################
      # Function call
      #################################
      elif isinstance(c, c_ast.FuncCall):
        # Look for function in function list
        for function in self.functions:
          if c.name.name == function.decl.name:

            # Create in-lined version of function
            function_copy = self.create_inline_function(function, c)

            # Replace with full function
            if isinstance(node, c_ast.Compound):
              node.block_items[ci] = function_copy.body
            elif isinstance(node, c_ast.If):
              exec("node.%s = function_copy.body" % c_name)
            elif isinstance(node, c_ast.Assignment):
              node.rvalue = function_copy.body
            else:
              raise Exception("Unsupported parent node type %s. Please implement." % (type(node)))
            self.expanded = True

      #################################
      # Other
      #################################
      else:
        self.expand_visit(c)
  """
  Create a modified version of the passed function that can be inlined.
  """
  def create_inline_function(self, function, caller):
    # Create a copy of the function
    function_copy = copy.deepcopy(function)
    ###################################
    # Function Arguments
    ###################################
    # If it has arguments
    if function_copy.decl.type.args:
      # Add those argument declarations to the start of the function
      for (arg, init) in zip(function_copy.decl.type.args.params, caller.args.exprs):
        arg.init = init
        function_copy.body.block_items.insert(0, arg)
    # Add declaration for return value (if function is non-void)
    if function.decl.type.type.type.names[0] != "void":
      td = c_ast.TypeDecl("return_value", None, function.decl.type.type.type)
      d = c_ast.Decl(None, None, None, None, td, None, None)
      function_copy.body.block_items.insert(0, d)
    function_copy.body.block_items.insert(0, c_ast.ID("// Inline function: %s" % (function_copy.decl.name)))
    ###################################
    # Return statements
    ###################################
    # Replace returns with goto statement
    return_label = "return%d" % (self.return_counter)
    self.return_counter += 1
    self.rtg_visitor.goto_name = return_label
    self.rtg_visitor.visit(function_copy)
    # Create label for goto
    function_copy.body.block_items.append(c_ast.Label(return_label, c_ast.EmptyStatement()))

    return function_copy

"""
Find all function declarations.
"""
class GetFunctionsVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.funcs = []
  def visit_FuncDef(self, node):
    self.funcs.append(node)

"""
Pretty print the passed AST/node.
"""
def print_node(node):
  generator = c_generator.CGenerator()
  print generator.visit(node)

if __name__ == "__main__":
  if len(sys.argv) > 2:
    filename = sys.argv[1]
    top_func = sys.argv[2]
  else:
    # filename = "c_files/h264ref.c"
    # top_func = "getLuma4x4Neighbour"
    filename = "c_files/test.c"
    top_func = "main"


  # Generate AST
  ast = parse_file(filename, use_cpp=True)
  #ast.show(nodenames=True)

  # Find all defined functions
  v = GetFunctionsVisitor()
  v.visit(ast)
  functions = v.funcs

  # Inline functions
  v = ExpandFunctionVisitor(top_func, functions)
  v.visit(ast)
  # Keep performing until no changes
  while v.expanded:
    v.expanded = False
    v.visit(ast)

  #print_node(ast)
  # Print out top-level function
  for function in functions:
    if function.decl.name == top_func:
      print_node(function)
