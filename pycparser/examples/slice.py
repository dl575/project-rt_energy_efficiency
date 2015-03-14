#!/usr/bin/python

"""
slice.py

Classes:
  InitializeSlicedVisitor
  MarkSlicedVisitor
  FuncDefRenameVisitor
  IDVisitor
  DataDependencyVisitor
  PrintSliceVisitory

Functions:
  slice_ast
  print_node
  str_node
  print_slice
"""

import sys

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
#
sys.path.extend(['.', '..'])

from pycparser import parse_file
from pycparser import c_generator
from pycparser import c_ast

"""
Initialize all nodes to not be part of program slice.
"""
class InitializeSlicedVisitor(c_ast.NodeVisitor):
  def __init__(self):
    pass
  def generic_visit(self, node):
    node.sliced = False
    for c_name, c in node.children():
      self.visit(c)
"""
Mark all nodes as part of program slice.
"""
class MarkSlicedVisitor(c_ast.NodeVisitor):
  def __init__(self):
    pass
  def generic_visit(self, node):
    node.sliced = True
    for c_name, c in node.children():
      self.visit(c)
"""
Rename function definition to append _slice. Also change to void type.
"""
class FuncDefRenameVisitor(c_ast.NodeVisitor):
  def __init__(self):
    pass
  def visit_FuncDef(self, node):
    # Change slice to void function
    node.decl.type.type.type.names = ["void"]
    if node.decl.type.type.declname.split('_')[-1] != "slice":
      node.decl.type.type.declname += "_slice"

"""
Identify all IDs in tree. 
Arrays should be identified by base name. (e.g., a[b] is identified by a).
Struct accesses are saved as all sub-portions. (e.g., a->b->c will save a->b->c, a->b, and a.)
"""
class IDVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.IDs = []
    self.cgenerator = c_generator.CGenerator()
  def reset(self):
    self.IDs = []
  """
  Reset list of IDs, then visit passed node.
  """
  def new_visit(self, node):
    self.reset()
    self.visit(node)
  def visit_ID(self, node):
    if node.name not in self.IDs:
      self.IDs.append(node.name)
  def visit_StructRef(self, node):
    # Recursively add struct reference and its base components
    while not isinstance(node, str):
      struct_name = self.cgenerator.visit(node)
      self.IDs.append(struct_name)
      node = node.name

"""
Identify all data dependencies of the passed variable var.
"""
class DataDependencyVisitor(c_ast.NodeVisitor):
  def __init__(self, var, specific_array_index=False):
    # Variable of interest
    self.var = var
    # Indicates that slice should be for a specific array index
    self.specific_array_index = specific_array_index
    # Set of rvalues to be used for future DataDependenceVisitors
    self.rvalues = []
    # Label to jump to on return statements
    self.end_label = "print_loop_counter"

    # Helper visitors
    self.id_visitor = IDVisitor()
    self.cgenerator = c_generator.CGenerator()
  """
  Reinitialize DataDependencyVisitor with a new target variable.
  """
  def reset(self, var, specific_array_index=False):
    self.var = var
    self.specific_array_index = specific_array_index
    self.rvalues = []
  """
  Reinitialize and run a new visit.
  """
  def new_visit(self, var, node, specific_array_index=False):
    self.reset(var, specific_array_index=specific_array_index)
    self.visit(node)

  """
  Mark node as sliced and add rvalues to saved set
  """
  def slice_data(self, node):
    node.sliced = True
    self.id_visitor.new_visit(node)
    if self.specific_array_index:
      for i in self.id_visitor.IDs:
        # Check that ID does not match base name when slicing for specific
        # array indices (should only be used for loop_counter[i].
        if i != self.var.split('[')[0]:
          self.rvalues.append(i)
    else:
      self.rvalues += self.id_visitor.IDs
    """
    print "Adding to slice:"
    print_node(node)
    print "because of data dependence %s" % self.var
    print
    """
  """
  Get base name of array without index
  """
  def get_ArrayRef_name(self, node):
    # If slicing is set up for specific array index, then compare based on
    # full string of array reference. This is meant to only be used for
    # slicing for specific loop_counter[i]
    if self.specific_array_index:
      return str_node(node)
    # Otherwise, we want to use the base variable of the array
    elif isinstance(node, c_ast.ID):
      return node.name
    elif isinstance(node, c_ast.StructRef):
      return node
    # Multi-dimensional array
    elif isinstance(node, c_ast.ArrayRef):
      return self.get_ArrayRef_name(node.name)
    # Cast
    elif isinstance(node, c_ast.Cast):
      return self.get_ArrayRef_name(node.expr)
    else:
      node.show(nodenames=True, showcoord=True)
      raise Exception("Unknown type for get_ArrayRef_name: %s" % (type(node)))
  def visit_Assignment(self, node):
    # Direct variable assignment
    if isinstance(node.lvalue, c_ast.ID) and node.lvalue.name == self.var:
      self.slice_data(node)
    # Array assignment
    elif isinstance(node.lvalue, c_ast.ArrayRef) and (self.get_ArrayRef_name(node.lvalue) == self.var):
      self.slice_data(node)
    # Struct assignment
    elif isinstance(node.lvalue, c_ast.StructRef):
      if self.cgenerator.visit(node.lvalue) == self.var:
        self.slice_data(node)
      else:
        self.generic_visit(node)
    # Not part of slice, check children
    else:
      self.generic_visit(node)
  def visit_UnaryOp(self, node):
    if self.cgenerator.visit(node.expr) == self.var:
      self.slice_data(node)
    elif isinstance(node.expr, c_ast.ArrayRef) and (self.get_ArrayRef_name(node.expr) == self.var):
      self.slice_data(node)
    else:
      self.generic_visit(node)
  def visit_Decl(self, node):
    # For arrays and structs, only use base name
    base_name = self.var.split("->")[0].split("[")[0]
    if (node.name == base_name):
      self.slice_data(node)
    # Not part of slice, check children
    else:
      self.generic_visit(node)
  def visit_FuncCall(self, node):
    self.generic_visit(node)
    # If sliced, add arguments to dependency list
    if node.sliced:
      self.id_visitor.new_visit(node)
      self.rvalues += self.id_visitor.IDs
  """
  Modify generic_visit to propagate slice information up tree.
  """
  def generic_visit(self, node):
    for ci, (c_name, c) in enumerate(node.children()):
      self.visit(c)
      # If child is sliced, then mark this as well
      if c.sliced:
        node.sliced = True
        # If adding an assignment, ensure that it is declared by adding lvalue
        # to dependency list
        # FIXME: Only need to declare this variable, not handle all its dependences
        if isinstance(node, c_ast.Assignment):
          self.id_visitor.new_visit(node.lvalue)
          self.rvalues += self.id_visitor.IDs
        """
        print "Adding to slice:"
        print_node(node)
        print "because child is in slice:"
        print_node(c)
        print
        """
      # Always include Returns, change them to Goto end of function
      if isinstance(c, c_ast.Return):
        # Create a new compound node
        insert_node = c_ast.Compound([])
        """
        # If there is a return value, add statement for return_value = value
        if c.expr:
          ret_id = c_ast.ID("return_value")
          ret_id.sliced = True
          assignment = c_ast.Assignment("=", ret_id, c.expr)
          assignment.sliced = True
          insert_node.block_items.append(assignment)
        """
        # Goto end of function
        goto = c_ast.Goto(self.end_label)
        goto.sliced = True
        insert_node.block_items.append(goto)
        insert_node.sliced = True
        # Add compound node back to function
        if isinstance(node, c_ast.Compound):
          node.block_items[ci] = insert_node
        elif isinstance(node, c_ast.If):
          exec("node.%s = insert_node" % c_name)
        elif isinstance(node, c_ast.Case):
          node.stmts[ci - 1] = insert_node
        else:
          print_node(node)
          node.show(nodenames=True, showcoord=True)
          raise Exception("Unsupported parent node type %s. Please implement." % (type(node)))

  """
  Include values used in conditions as rvalues to find future dependencies.
  """
  def slice_loop(self, node):
    # First visit inside loop/condition body
    self.generic_visit(node)
    # If this node gets marked as sliced,
    if node.sliced and node.cond:
      # Include variables used in the condition as part of slice
      self.id_visitor.new_visit(node.cond)
      self.rvalues += self.id_visitor.IDs
  def visit_For(self, node):
    self.slice_loop(node)
  def visit_If(self, node):
    self.slice_loop(node)
  def visit_While(self, node):
    self.slice_loop(node)
  def visit_DoWhile(self, node):
    self.slice_loop(node)
  def visit_Switch(self, node):
    self.slice_loop(node)

  """
  Always include continue/break and labels to ensure control flow.
  """
  def visit_Continue(self, node):
    node.sliced = True
  def visit_Break(self, node):
    node.sliced = True
  def visit_Label(self, node):
    node.sliced = True
    if isinstance(node.stmt, c_ast.EmptyStatement):
      node.stmt.sliced = True
    else:
      self.generic_visit(node)
  def visit_Goto(self, node):
    node.sliced = True
  def visit_Compound(self, node):
    # Mark entire print_loop_counter block as part of slice
    if node.block_items and isinstance(node.block_items[0], c_ast.Label) and node.block_items[0].name == "print_loop_counter":
      v = MarkSlicedVisitor()
      v.visit(node)
    else:
      self.generic_visit(node)

"""
Only print out nodes that are considered part of slice
"""
class PrintSliceVisitor(c_generator.CGenerator):
  def __init__(self):
    self.output = ''
    self.indent_level = 0

  def visit(self, node):
    method = "visit_" + node.__class__.__name__
    ret = getattr(self, method, self.generic_visit)(node)
    return ret
  def _generate_stmt(self, n, add_indent=False):
    """ Generation from a statement node. This method exists as a wrapper
            for individual visit_* methods to handle different treatment of
            some statements in this context.
    """
    typ = type(n)
    if add_indent: self.indent_level += 2
    indent = self._make_indent()
    if add_indent: self.indent_level -= 2

    if typ in (
        c_ast.Decl, c_ast.Assignment, c_ast.Cast, c_ast.UnaryOp,
        c_ast.BinaryOp, c_ast.TernaryOp, c_ast.FuncCall, c_ast.ArrayRef,
        c_ast.StructRef, c_ast.Constant, c_ast.ID, c_ast.Typedef,
        c_ast.ExprList):
      # These can also appear in an expression context so no semicolon
        # is added to them automatically
        #
        # Only print out expression if they are part of slice
        if n.sliced:
          return indent + self.visit(n) + ';\n'
        else:
          return indent + '{}\n'
    elif typ in (c_ast.Compound,):
      # No extra indentation required before the opening brace of a
        # compound - because it consists of multiple lines it has to
        # compute its own indentation.
        #
        return self.visit(n)
    else:
      if n.sliced:
        return indent + self.visit(n) + '\n'
      else:
        return ''

"""
Return AST containing nodes which affect variables in the list slice_vars.
"""
def slice_ast(ast, slice_vars):
  # Set all nodes to sliced=False
  v = InitializeSlicedVisitor()
  v.visit(ast)
  # Rename function
  v = FuncDefRenameVisitor()
  v.visit(ast)

  # Variables already sliced
  handled_rvalues = []
  # Variables that still need to be sliced
  rvalues = []
  # Create visitor
  v = DataDependencyVisitor(None)
  # Find immediate data dependencies
  for var in slice_vars:
    if '[' in var:
      v.new_visit(var, ast, specific_array_index=True)
    else:
      v.new_visit(var, ast, specific_array_index=False)
    # Add to list of sliced variables
    handled_rvalues.append(var)
    # Get list of dependent variables that need to be further sliced
    for rvalue in v.rvalues:
      if rvalue not in handled_rvalues and rvalue not in rvalues:
        rvalues.append(rvalue)

  # Iteratively find all data dependencies until no new variables to slice
  while rvalues:
    new_rvalues = []
    # For each rvalue
    for rvalue in rvalues:
      # Run DataDependencyVisitor to mark slice
      v.new_visit(rvalue, ast)
      # Add rvalue to list of sliced values
      handled_rvalues.append(rvalue)
      # Keep track of (possible) new rvalues 
      new_rvalues += v.rvalues
    # Get list of new rvalues that need to be handled
    rvalues = []
    for new_rvalue in new_rvalues:
      if new_rvalue not in handled_rvalues and new_rvalue not in rvalues:
        rvalues.append(new_rvalue)

"""
Pretty print the passed AST/node.
"""
def print_node(node):
  generator = c_generator.CGenerator()
  print generator.visit(node)
"""
Return a string of the passed AST/node.
"""
def str_node(node):
  generator = c_generator.CGenerator()
  return generator.visit(node)
"""
Pretty print only the sliced parts of the passed AST.
"""
def print_slice(node):
  v = PrintSliceVisitor()
  result = v.visit(ast)
  print result


if __name__ == "__main__":
  if len(sys.argv) > 2:
    filename = sys.argv[1]
    #var = sys.argv[2]
    slice_vars = sys.argv[2:]
  else:
    print "usage: slice.py filename var0 [var1 ...]"
    sys.exit()

  # Generate AST
  ast = parse_file(filename, use_cpp=True)
  # Generate slice
  slice_ast(ast, slice_vars)
  # Output generated slice
  print_slice(ast)

