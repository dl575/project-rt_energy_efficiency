#!/usr/bin/python

import sys

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
#
sys.path.extend(['.', '..'])

from pycparser import parse_file
from pycparser import c_ast
from pycparser import c_generator

"""
Insert counters for all loops and conditionals.
"""
class LoopCountVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.loop_counter_index = 0
  def visit_For(self, node):
    stmt = self.create_loop_counter_stmt()
    self.insert_in_Loop(node, stmt)
    # Continue visiting for nested loops/conditionals
    self.generic_visit(node)
  def visit_While(self, node):
    stmt = self.create_loop_counter_stmt()
    self.insert_in_Loop(node, stmt)
    # Continue visiting for nested loops/conditionals
    self.generic_visit(node)
  def visit_If(self, node):
    stmt = self.create_loop_counter_stmt()
    self.insert_in_If(node, stmt)
    # Continue visiting for nested loops/conditionals
    self.generic_visit(node)
  def visit_Case(self, node):
    stmt = self.create_loop_counter_stmt()
    node.stmts.insert(0, stmt)
    self.generic_visit(node)
  def visit_Default(self, node):
    stmt = self.create_loop_counter_stmt()
    node.stmts.insert(0, stmt)
    self.generic_visit(node)
  """
  Create a new statement to increment a (new) loop counter.
  """
  def create_loop_counter_stmt(self):
    stmt = c_ast.UnaryOp("p++", c_ast.ID("loop_counter[%d]" % (self.loop_counter_index)))
    self.loop_counter_index += 1
    return stmt
  """
  Insert a statement into the beginning of a For/While loop.
  """
  def insert_in_Loop(self, node, stmt):
    # Insert at start of compound block
    if isinstance(node.stmt, c_ast.Compound):
      if node.stmt.block_items:
        node.stmt.block_items.insert(0, stmt)
      else:
        node.stmt.block_items = [stmt]
    # Replace empty statement
    elif isinstance(node.stmt, c_ast.EmptyStatement):
      node.stmt = stmt
    # Otherwise, replace with a compound block with stmt + original statment
    else:
      compound = c_ast.Compound([stmt, node.stmt])
      node.stmt = compound
  """
  Insert a statement into the beginning of a conditional block.
  """
  def insert_in_If(self, node, stmt):
    # Insert at start of compound block
    if isinstance(node.iftrue, c_ast.Compound):
      if node.iftrue.block_items:
        node.iftrue.block_items.insert(0, stmt)
      else:
        node.iftrue.block_items = [stmt]
    # Replace empty statement
    elif isinstance(node.iftrue, c_ast.EmptyStatement):
      node.iftrue = stmt
    else:
      compound = c_ast.Compound([stmt, node.iftrue])
      node.iftrue = compound

"""
Define loop counter at start of function. Print out loop counter values at
end of function.
"""
class LoopCountInitPrintVisitor(c_ast.NodeVisitor):
  def __init__(self, loop_counter_size, write_to_file = False):
    self.loop_counter_size = loop_counter_size
    # Write to file instead of printing to stdout
    self.write_to_file = write_to_file
  def visit_FuncDef(self, node):
    # Skip if no loop counters exist
    if self.loop_counter_size == 0:
      return

    # Create loop_counter declaration/initialization
    constants = [c_ast.Constant("int", '0') for i in range(self.loop_counter_size)]
    init_list = c_ast.InitList(constants)
    identifier_type = c_ast.IdentifierType(["int"])
    type_decl = c_ast.TypeDecl("loop_counter", [], identifier_type)
    dim = c_ast.Constant("int", str(self.loop_counter_size))
    array_decl = c_ast.ArrayDecl(type_decl, dim, [])
    decl = c_ast.Decl("loop_counter", [], [], [], array_decl, init_list, None) 
    node.body.block_items.insert(0, decl)

    # Label for return values to goto
    label = c_ast.Label("print_loop_counter", None)

    # Write to file
    if self.write_to_file:
      stmt = c_ast.ID("write_array(loop_counter, %d);\n" % (self.loop_counter_size))
      compound = c_ast.Compound([label, stmt])
    # Print to stdout
    else:
      # Add printf to the end of function
      # Start of printing
      stmt_start = c_ast.ID("printf(\"loop counter = (\")")

      # For loop
      # int i;
      identifier_type = c_ast.IdentifierType(["int"])
      type_decl = c_ast.TypeDecl("i", [], identifier_type)
      for_decl = c_ast.Decl("i", [], [], [], type_decl, [], None)
      # i = 0
      init = c_ast.Assignment("=", c_ast.ID("i"), c_ast.Constant("int", '0'))
      # i < self.loop_counter_size
      cond = c_ast.BinaryOp("<", c_ast.ID("i"), c_ast.Constant("int", str(self.loop_counter_size)))
      # i++
      next_stmt = c_ast.UnaryOp("p++", c_ast.ID("i"))
      # printf in for
      stmt = c_ast.ID("printf(\"%d, \", loop_counter[i])")
      # Cosntruct for loop
      stmt_for = c_ast.For(init, cond, next_stmt, stmt)

      # End of printing
      stmt_end = c_ast.ID("printf(\")\\n\")")
      
      compound = c_ast.Compound([label, stmt_start, for_decl, stmt_for, stmt_end])
    node.body.block_items.append(compound)


"""
Pretty print the passed AST/node.
"""
def print_node(node):
  generator = c_generator.CGenerator()
  print generator.visit(node)



if __name__ == "__main__":
  if len(sys.argv) > 1:
    filename = sys.argv[1]
  else:
    filename = "c_files/test_insert_loop_counts.c"
  if "--write_to_file" in sys.argv:
    write_to_file = True
  else:
    write_to_file = False

  # Generate AST
  ast = parse_file(filename, use_cpp=True)
  # Insert loop counts
  v = LoopCountVisitor()
  v.visit(ast)
  # Insert init/print
  v = LoopCountInitPrintVisitor(v.loop_counter_index, write_to_file)
  v.visit(ast)
  # Print out result
  print_node(ast)


