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
Renames variable old_name to new_name.
"""
class RenameVisitor(c_ast.NodeVisitor):
  def __init__(self, old_name=None, new_name=None):
    self.old_name = old_name
    self.new_name = new_name

    self.cgenerator = c_generator.CGenerator()

    self.in_struct = False
  """
  Set the old_name and new_name for renaming.
  """
  def set_names(self, old_name, new_name):
    self.old_name = old_name
    # Ensure that new name is a string
    if isinstance(new_name, str):
      self.new_name = new_name
    else:
      self.new_name = self.cgenerator.visit(new_name)
  def visit_ID(self, node):
    if node.name == self.old_name:
      node.name = self.new_name
  def visit_TypeDecl(self, node):
    if node.declname == self.old_name:
      node.declname = self.new_name
  def visit_StructRef(self, node):
    if not self.in_struct:
      struct_base_name = self.get_base_StructRef(node)
      if struct_base_name == self.old_name:
        self.set_base_StructRef(node, self.new_name)

      self.in_struct = True
      # Only visit fields that are not just members of the struct
      if not isinstance(node.name, c_ast.ID):
        self.generic_visit(node.name)
      if not isinstance(node.field, c_ast.ID):
        self.generic_visit(node.name)
      self.in_struct = False
    else:
      # Only visit fields that are not just members of the struct
      if not isinstance(node.name, c_ast.ID):
        self.generic_visit(node.name)
      if not isinstance(node.field, c_ast.ID):
        self.generic_visit(node.name)
  def get_base_StructRef(self, node):
    if isinstance(node, c_ast.ID):
      return node.name
    else:
      return self.get_base_StructRef(node.name)
  def set_base_StructRef(self, node, new_name):
    if isinstance(node, c_ast.ID):
      node.name = new_name
    else:
      self.set_base_StructRef(node.name, new_name)

"""
Replaces return statements with a goto.
"""
class ReturnToGotoVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.goto_ID = None
  def generic_visit(self, node):
    for ci, (c_name, c) in enumerate(node.children()):
      # Return statement
      if isinstance(c, c_ast.Return):
        # Create a new compound node
        insert_node = c_ast.Compound([])
        # If there is a return value, add statement for return_value = value
        if c.expr:
          insert_node.block_items.append(c_ast.Assignment("=", c_ast.ID("return_value"), c.expr))
        # Add goto to end of function block
        insert_node.block_items.append(c_ast.Goto(self.goto_name))
        # Depending on parent, handle insertion differently
        if isinstance(node, c_ast.Compound):
          node.block_items[ci] = insert_node
        elif isinstance(node, c_ast.If):
          exec("node.%s = insert_node" % c_name)
        else:
          raise Exception("Unsupported parent node type %s. Please implement." % (type(node)))
      # Non-return statement, continue visiting
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

    self.rename_visitor = RenameVisitor()
    self.rename_counter = 0

  def visit_FuncDef(self, node):
    if (node.decl.name == self.funcname):
      # Find and replace function calls
      self.expand_visit(node)
  """ 
  Replace node with full function.
  """
  def expand_visit(self, node):
    # For each child
    for ci, (c_name, c) in enumerate(node.children()):

      ##########################################
      # Assignment with function call as rvalue
      ##########################################
      if isinstance(c, c_ast.Assignment) and isinstance(c.rvalue, c_ast.FuncCall):
        # Find matching function body
        for function in self.functions:
          if c.rvalue.name.name == function.decl.name:
            # Create the inline version of the function body
            inline_function = self.create_inline_function(function, c.rvalue)
            # Set assignment lvalue to return value
            inline_function.body.block_items.append(c_ast.Assignment("=", c.lvalue, c_ast.ID("return_value")))
            # Replace node in parent
            if isinstance(node, c_ast.Compound):
              node.block_items[ci] = inline_function.body
            elif isinstance(node, c_ast.Case):
              node.stmts[ci - 1] = inline_function.body
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
            elif isinstance(node, c_ast.Case):
              node.stmts[ci - 1] = function_copy.body
            else:
              raise Exception("Unsupported parent node type %s. Please implement." % (type(node)))
            self.expanded = True

      #################################
      # Other - Continue visiting
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
    function_copy.body.block_items.insert(0, c_ast.ID("// End of arguments"))
    args = []
    ptr_args = []
    # If it has arguments
    if function_copy.decl.type.args:
      # For each function argument and passed value
      for (arg, init) in zip(function_copy.decl.type.args.params, caller.args.exprs):
        # Pointers just get renamed, no re-declare
        if isinstance(arg.type, c_ast.PtrDecl):
          # Add pointer name to list
          if isinstance(init, c_ast.UnaryOp):
            ptr_args.append((self.get_Decl_name(arg), init.expr.name))
          elif isinstance(init, c_ast.StructRef):
            ptr_args.append((self.get_Decl_name(arg), "%s->%s" % (init.name.name, init.field.name)))
          elif isinstance(init, c_ast.ID):
            ptr_args.append((self.get_Decl_name(arg), init.name))
          elif isinstance(init, c_ast.ArrayRef):
            ptr_args.append((self.get_Decl_name(arg), init))
          else: 
            raise Exception("Unsupported init type %s" % (type(init)))
        else:
          # Assign passed value to argument declaration
          arg.init = init
          # Save list of argument name for renaming
          args.append(self.get_Decl_name(arg))
          # Insert into start of function
          function_copy.body.block_items.insert(0, arg)

    # Rename arguments to prevent aliasing with upper-level
    for arg in args:
      self.rename_visitor.set_names(arg, arg + "_rename%d" % self.rename_counter)
      self.rename_visitor.visit(function_copy)
    # Rename pointers to the passed pointer variable
    for arg in ptr_args:
      self.rename_visitor.set_names(arg[0], arg[1])
      self.rename_visitor.visit(function_copy)
    # Find all declared variables and rename them
    self.decl_visitor = GetDeclVisitor()
    self.decl_visitor.visit(function_copy)
    for decl in self.decl_visitor.decls:
      self.rename_visitor.set_names(decl, decl + "_rename%d" % self.rename_counter)
      self.rename_visitor.visit(function_copy)
    # Increment rename counter to ensure unique names
    self.rename_counter += 1

    # Add declaration for return value (if function is non-void)
    if self.get_Function_type(function_copy) != "void":
      # Copy the function type declaration
      typedecl = copy.deepcopy(function.decl.type.type)
      # Create a variable declaration using the function type
      decl = c_ast.Decl(None, None, None, None, typedecl, None, None)
      # Replace the variable name as return_value
      self.set_Decl_name(decl, "return_value")
      # Insert the return_value declaration at the start of the function
      function_copy.body.block_items.insert(0, decl)
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
  Returns the return type of the passed function (or decl) node. This is done
  by recursively descending into the node tree until the IdentifierType node is
  found.
  """
  def get_Function_type(self, func):
    if isinstance(func, c_ast.IdentifierType):
      return func.names[0]
    elif isinstance(func, c_ast.FuncDef):
      return self.get_Function_type(func.decl)
    else:
      return self.get_Function_type(func.type)
  """
  Set the variable name for the passed decl node. This is done by recursively
  descending into the node tree until the TypeDecl node is found and changing
  its name.
  """
  def set_Decl_name(self, node, name):
    if isinstance(node, c_ast.TypeDecl):
      node.declname = name
    else:
      self.set_Decl_name(node.type, name)


  """
  Return the string name of the variable being declared
  """
  def get_Decl_name(self, node):
    if isinstance(node, c_ast.TypeDecl):
      return node.declname
    else:
      return self.get_Decl_name(node.type)

"""
Find all function declarations.
"""
class GetFunctionsVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.funcs = []
  def visit_FuncDef(self, node):
    self.funcs.append(node)
"""
Find all variable declarations.
"""
class GetDeclVisitor(c_ast.NodeVisitor):
  def __init__(self):
    self.decls = []
  def visit_Decl(self, node):
    self.decls.append(node.name)

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
  # Print out non-function top-levels
  for c_name, c in ast.children():
    if not isinstance(c, c_ast.FuncDef):
      print_node(c) 
      print_node(c_ast.EmptyStatement())
  # Print out top-level function
  for function in functions:
    if function.decl.name == top_func:
      print_node(function)
