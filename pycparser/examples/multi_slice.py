#!/usr/bin/python

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
Identify all IDs in tree. Struct accesses are saved as struct + member (i.e., a->b).
Arrays should be identified by base name.
"""
class IDVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.IDs = []
    self.cgenerator = c_generator.CGenerator()
  def visit_ID(self, node):
    if node.name not in self.IDs:
      self.IDs.append(node.name)
  def visit_StructRef(self, node):
    struct_name = self.cgenerator.visit(node)
    self.IDs.append(struct_name)

"""
Identify all data dependencies of the passed variable var.
"""
class DataDependencyVisitor(c_ast.NodeVisitor):
  def __init__(self, var):
    self.var = var
    self.rvalues = set()
    self.id_visitor = IDVisitor()
    self.cgenerator = c_generator.CGenerator()
  # Mark node as sliced and add rvalues to saved set
  def slice_data(self, node):
    node.sliced = True
    self.id_visitor.visit(node)
    self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
  def get_ArrayRef_name(self, node):
    assert(isinstance(node, c_ast.ArrayRef) or isinstance(node, c_ast.ID))
    if isinstance(node, c_ast.ID):
      return node.name
    else:
      return self.get_ArrayRef_name(node.name)
  def visit_Assignment(self, node):
    # Direct variable assignment
    if isinstance(node.lvalue, c_ast.ID) and node.lvalue.name == self.var:
      self.slice_data(node)
    # Array assignment
    #elif isinstance(node.lvalue, c_ast.ArrayRef) and (node.lvalue.name.name.name == self.var):
    elif isinstance(node.lvalue, c_ast.ArrayRef) and (self.get_ArrayRef_name(node.lvalue) == self.var):
      self.slice_data(node)
      # Struct assignment
      """
      elif isinstance(node.lvalue, c_ast.StructRef) and (node.lvalue.name.name == self.var):
        self.slice_data(node)
      """
    elif isinstance(node.lvalue, c_ast.StructRef):
      if self.cgenerator.visit(node.lvalue) == self.var:
        self.slice_data(node)
      else:
        self.generic_visit(node)
    # Not part of slice, check children
    else:
      self.generic_visit(node)
  def visit_UnaryOp(self, node):
    """
    if isinstance(node.expr, c_ast.ID) and node.expr.name == self.var:
      self.slice_data(node)
    # Not part of slice, check children
    else:
      self.generic_visit(node)
    """
    if self.cgenerator.visit(node.expr) == self.var:
      self.slice_data(node)
    else:
      self.generic_visit(node)
  def visit_Decl(self, node):
    # For structs, only use base name
    base_name = self.var.split("->")[0]
    if (node.name == base_name):
      self.slice_data(node)
      """
      # Add pointer copies
      elif isinstance(node.type, c_ast.PtrDecl) and node.init and isinstance(node.init, c_ast.ID) and (node.init.name == base_name):
        node.sliced = True 
        #self.id_visitor.visit(node.init)
        #self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
        self.rvalues = self.rvalues.union(set([node.name]))
      """
    # Not part of slice, check children
    else:
      self.generic_visit(node)
  """
  Modify generic_visit to propagate slice information up tree.
  """
  def generic_visit(self, node):
    for c_name, c in node.children():
      self.visit(c)
      # If child is sliced, then mark this as well
      if c.sliced:
        node.sliced = True

  """
  Include values used in conditions as rvalues to find future dependencies.
  """
  def visit_For(self, node):
    self.generic_visit(node)
    # Include condition part of for loop (init and next should be captured as
    # data assignment/unaryops)
    if node.sliced:
      # self.id_visitor.visit(node.init)
      # self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
      self.id_visitor.visit(node.cond)
      self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
      # self.id_visitor.visit(node.next)
      # self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
  def visit_If(self, node):
    self.generic_visit(node)
    # Include if condition
    if node.sliced:
      self.id_visitor.visit(node.cond)
      self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
      self.generic_visit(node)
  def visit_While(self, node):
    self.generic_visit(node)
    if node.sliced:
      # Include loop condition
      self.id_visitor.visit(node.cond)
      self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
      self.generic_visit(node)
  def visit_DoWhile(self, node):
    self.generic_visit(node)
    # Include loop condition
    if node.sliced:
      self.id_visitor.visit(node.cond)
      self.rvalues = self.rvalues.union(set(self.id_visitor.IDs))
      self.generic_visit(node)

"""
Only print out nodes that are considered part of slice
"""
class PrintSliceVisitor(c_generator.CGenerator):
  def __init__(self):
    self.output = ''
    self.indent_level = 0

    self.sliced = False

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
          return ''
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
Return AST containing nodes which affect variable name var.
"""
def slice_ast(ast, slice_vars):
  # Set all nodes to sliced=False
  v = InitializeSlicedVisitor()
  v.visit(ast)
  for var in slice_vars:
    # Find immediate data dependencies
    v = DataDependencyVisitor(var)
    v.visit(ast)

  # Iteratively find all data dependencies
  rvalues = v.rvalues
  new_rvalues = rvalues
  changed = True
  # Keep propagating until no changes to data dependency list 
  while changed:
    changed = False
    # For each rvalue, find all dependencies for it
    for rvalue in rvalues:
      v = DataDependencyVisitor(rvalue)
      v.visit(ast)
      new_rvalues = new_rvalues.union(v.rvalues)
    # If data dependency list has increased, keep looping
    if len(new_rvalues) > len(rvalues):
      changed = True
    rvalues = new_rvalues

  """
  print '\n'.join(sorted(rvalues))
  print 
  print
  """

"""
Identify all IDs that are declared in function.
"""
class DeclVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.IDs = []
    self.id_visitor = IDVisitor()
  def visit_Decl(self, node):
    self.IDs.append(node.name)

class LValueVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.IDs = []
  def visit_Assignment(self, node):
    if isinstance(node.lvalue, c_ast.ID):
      self.IDs.append(node.lvalue.name)
  def visit_UnaryOp(self, node):
    if isinstance(node.expr, c_ast.ID):
      self.IDs.append(node.expr.name)

"""
Pretty print the passed AST/node.
"""
def print_node(node):
  generator = c_generator.CGenerator()
  print generator.visit(node)
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
    var_filename = sys.argv[2]
  else:
    filename = "c_files/mv-search2.c"
    var_filename = "slice_vars.txt"

  # Get slice variables
  slice_vars = []
  f = open(var_filename, 'r')
  for line in f:
    if line[0] != '#':
      slice_vars.append(line.strip())
  f.close()
  if len(slice_vars) == 0:
    print "No slice variables found!"
    sys.exit(1)

  # Generate AST
  ast = parse_file(filename, use_cpp=True)
  #ast.show(nodenames=True)
  # Generate slice
  slice_ast(ast, slice_vars)

  print_slice(ast)

