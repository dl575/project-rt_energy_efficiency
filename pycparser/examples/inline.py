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

def replace_node(parent, new_child, ci, c_name):
  """
  Replace node at ci/c_name in parent with the new_child.
  """
  typ = type(parent)
  # Depending on parent, handle insertion differently
  if typ in (c_ast.BinaryOp, c_ast.Assignment, c_ast.StructRef,
      c_ast.UnaryOp, c_ast.Cast, c_ast.TernaryOp, c_ast.If, c_ast.While,
      c_ast.DoWhile):
    exec("parent.%s = new_child" % c_name)
  elif typ == c_ast.Compound:
    parent.block_items[ci] = new_child
  elif typ == c_ast.Case:
    parent.stmts[ci - 1] = new_child
  else:
    print_node(parent)
    parent.show(nodenames=True, showcoord=True)
    raise Exception("Unsupported parent node type %s. Please implement." % (type(parent)))

def print_node(node):
  """
  Pretty print the passed AST/node.
  """
  generator = c_generator.CGenerator()
  print generator.visit(node)

class GetFuncCallVisitor(c_ast.NodeVisitor):
  """
  Determines if there is a function call in the visited node. If there is, then
  save the FuncCall node to self.function_call.
  """
  def __init__(self):
    self.function_call = []
  def new_visit(self, node):
    self.function_call = []
    self.visit(node)
  def visit_FuncCall(self, node):
    self.function_call.append(node)

class ReplaceFuncCallVisitor(c_ast.NodeVisitor):
  """
  Replace FuncCall with variable value.
  """
  def __init__(self):
    self.func_call = None
    self.replace_name = None
  def new_visit(self, func_call, replace_name, node):
    """
    Start a new visit to perform replacement. func_call_name is a string
    representing the function call name. replace_name is a string representing
    the variable to insert.
    """
    self.func_call = func_call
    self.replace_name = replace_name
    self.visit(node)
  def generic_visit(self, node):
    for ci, (c_name, c) in enumerate(node.children()):
      # If function call
      if isinstance(c, c_ast.FuncCall):
        # And call to the function to be replaced
        if c == self.func_call:
          # Handle replacement
          replace_node(node, c_ast.ID(self.replace_name), ci, c_name)
      self.visit(c)

class RemoveIfFunctionVisitor(c_ast.NodeVisitor):
  """
  Converts:
    if (func() == 1)
  to:
    int func_result;
    func_result = func();
    if (func_result == 1)
  """

  def __init__(self):
    self.visitor = GetFuncCallVisitor()
    self.replace_visitor = ReplaceFuncCallVisitor()
  def generic_visit(self, node):
    """
    Rewrite generic_visit to check for FuncCall nodes in the condition of If
    nodes.
    """
    for ci, (c_name, c) in enumerate(node.children()):

      # For conditional statement, 
      if isinstance(c, c_ast.If):
        # Determine if there is a function call in the condition
        self.visitor.new_visit(c.cond)
        if self.visitor.function_call:
          # For each function call found
          stmts = []
          for (fi, func_call) in enumerate(self.visitor.function_call):
            # Extract information about function call
            func_name = func_call.name.name
            result_name = func_name + "_result%d" % fi
            # Declare result variable
            it = c_ast.IdentifierType(["int"])
            td = c_ast.TypeDecl(result_name, [], it)
            d = c_ast.Decl(result_name, [], [], [], td, None, None)
            stmts.append(d)
            # Call function, assign into result variable
            a = c_ast.Assignment("=", c_ast.ID(result_name), func_call)
            stmts.append(a)
            # Modify conditional to use result variable instead of function call
            if isinstance(c.cond, c_ast.FuncCall) and (c.cond.name.name == func_name):
              c.cond = c_ast.ID(result_name)
            else:
              self.replace_visitor.new_visit(func_call, result_name, c.cond)
          # Put it all together in a new Compound block
          compound = c_ast.Compound(stmts + [c])
          # Replace original block with new block
          replace_node(node, compound, ci, c_name)

      self.visit(c)

class RenameVisitor(c_ast.NodeVisitor):
  """
  Renames variable old_name to new_name.
  """
  def __init__(self, old_name=None, new_name=None):
    self.old_name = old_name
    self.new_name = new_name

    self.cgenerator = c_generator.CGenerator()

    self.in_struct = False
  def set_names(self, old_name, new_name):
    """
    Set the old_name and new_name for renaming.
    """
    self.old_name = old_name
    # Ensure that new name is a string
    if isinstance(new_name, str):
      self.new_name = new_name
    else:
      self.new_name = self.cgenerator.visit(new_name)
  def new_visit(self, old_name, new_name, node):
    self.set_names(old_name, new_name)
    self.in_struct = False
    self.visit(node)

  def visit_ID(self, node):
    if node.name == self.old_name:
      node.name = self.new_name
  def visit_Decl(self, node):
    # Visit type decl but not init
    self.visit(node.type)
  def visit_TypeDecl(self, node):
    if node.declname == self.old_name:
      node.declname = self.new_name
  def visit_ArrayRef(self, node):
    self.visit(node.name)
    # Looking at subscript of ArrayRef should reset looking for struct bases
    self.in_struct = False
    self.visit(node.subscript)
  def visit_StructRef(self, node):
    """
    Only the base of a StructRef should be renameable (i.e., for a->b->c->d,
    only a should be renamable).
    """
    if not self.in_struct:
      struct_base_name = self.get_base_StructRef(node)
      if struct_base_name == self.old_name:
        self.set_base_StructRef(node, self.new_name)

      self.in_struct = True
      # Only visit fields that are not just members of the struct
      if not isinstance(node.name, c_ast.ID):
        self.visit(node.name)
      if not isinstance(node.field, c_ast.ID):
        self.visit(node.field)
      self.in_struct = False
    else:
      # Only visit fields that are not just members of the struct
      if not isinstance(node.name, c_ast.ID):
        self.visit(node.name)
      if not isinstance(node.field, c_ast.ID):
        self.visit(node.field)
  def get_base_StructRef(self, node):
    """
    Get the base of a (nested) StructRef
    """
    if isinstance(node, c_ast.ID):
      return node.name
    else:
      return self.get_base_StructRef(node.name)
  def set_base_StructRef(self, node, new_name):
    """
    Replace the base of a (nested) StructRef
    """
    if isinstance(node, c_ast.ID):
      node.name = new_name
    else:
      self.set_base_StructRef(node.name, new_name)

class ReturnToGotoVisitor(c_ast.NodeVisitor):
  """
  Replaces return statements with a goto.
  """
  def __init__(self):
    self.goto_name = None
  def new_visit(self, goto_name, node):
    self.goto_name = goto_name
    self.visit(node)
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
        # Replace original node with new node
        replace_node(node, insert_node, ci, c_name)
      # Non-return statement, continue visiting
      else:
        self.visit(c)

class ExpandFunctionVisitor(c_ast.NodeVisitor):
  """
  Used to take a function and inline all functions it calls.
  """
  def __init__(self, funcname, functions):
    self.funcname = funcname
    self.functions = functions
    self.expanded = False

    self.rtg_visitor = ReturnToGotoVisitor()
    self.return_counter = 0
    self.rename_visitor = RenameVisitor()
    self.rename_counter = 0
    self.cgenerator = c_generator.CGenerator()

  def visit_FuncDef(self, node):
    if (node.decl.name == self.funcname):
      # Find and replace function calls
      self.expand_visit(node)
  def expand_visit(self, node):
    """ 
    Replace node with full function.
    """
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
            replace_node(node, inline_function.body, ci, c_name)
            self.expanded = True

      #################################
      # Function call
      #################################
      elif isinstance(c, c_ast.FuncCall):
        # Look for function in function list
        for function in self.functions:
          # FIXME: Skip pointer referenced function call (i.e., jump table)
          if isinstance(c.name, c_ast.UnaryOp):
            break
          if c.name.name == function.decl.name:

            # Create in-lined version of function
            function_copy = self.create_inline_function(function, c)

            # Replace with full function
            replace_node(node, function_copy.body, ci, c_name)
            self.expanded = True

      #################################
      # Other - Continue visiting
      #################################
      else:
        self.expand_visit(c)
  def create_inline_function(self, function, caller):
    """
    Create a modified version of the passed function that can be inlined.
    """
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
            ptr_args.append((self.get_Decl_name(arg), self.cgenerator.visit(init)))
          elif isinstance(init, c_ast.ID):
            ptr_args.append((self.get_Decl_name(arg), init.name))
          elif isinstance(init, c_ast.ArrayRef):
            ptr_args.append((self.get_Decl_name(arg), init))
          else: 
            arg.show(nodenames=True, showcoord=True)
            init.show(nodenames=True, showcoord=True)
            raise Exception("Unsupported init type %s" % (type(init)))
        # Arrays also get renamed, no re-declare
        elif isinstance(arg.type, c_ast.ArrayDecl):
          ptr_args.append((self.get_Decl_name(arg), init))
        else:
          # Assign passed value to argument declaration
          arg.init = init
          # Save list of argument name for renaming
          args.append(self.get_Decl_name(arg))
          # Insert into start of function
          function_copy.body.block_items.insert(0, arg)

    # Rename arguments to prevent aliasing with upper-level
    for arg in args:
      self.rename_visitor.new_visit(arg, arg + "_rename%d" % self.rename_counter, function_copy)
    # Rename pointers to the passed pointer variable
    for arg in ptr_args:
      self.rename_visitor.new_visit(arg[0], arg[1], function_copy)
    # Find all declared variables and rename them
    self.decl_visitor = GetDeclVisitor()
    self.decl_visitor.visit(function_copy)
    for decl in self.decl_visitor.decls:
      self.rename_visitor.new_visit(decl, decl + "_rename%d" % self.rename_counter, function_copy)
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
    self.rtg_visitor.new_visit(return_label, function_copy)
    # Create label for goto
    function_copy.body.block_items.append(c_ast.Label(return_label, c_ast.EmptyStatement()))

    return function_copy

  def get_Function_type(self, func):
    """
    Returns the return type of the passed function (or decl) node. This is done
    by recursively descending into the node tree until the IdentifierType node is
    found.
    """
    if isinstance(func, c_ast.IdentifierType):
      return func.names[0]
    elif isinstance(func, c_ast.FuncDef):
      return self.get_Function_type(func.decl)
    else:
      return self.get_Function_type(func.type)
  def set_Decl_name(self, node, name):
    """
    Set the variable name for the passed decl node. This is done by recursively
    descending into the node tree until the TypeDecl node is found and changing
    its name.
    """
    if isinstance(node, c_ast.TypeDecl):
      node.declname = name
    else:
      self.set_Decl_name(node.type, name)

  def get_Decl_name(self, node):
    """
    Return the string name of the variable being declared
    """
    if isinstance(node, c_ast.TypeDecl):
      return node.declname
    else:
      return self.get_Decl_name(node.type)

class GetFunctionsVisitor(c_ast.NodeVisitor):
  """
  Find all function declarations.
  """
  def __init__(self):
    self.funcs = []
  def visit_FuncDef(self, node):
    self.funcs.append(node)

class GetDeclVisitor(c_ast.NodeVisitor):
  """
  Find all variable declarations.
  """
  def __init__(self):
    self.decls = []
  def visit_Decl(self, node):
    if node.name:
      self.decls.append(node.name)

if __name__ == "__main__":
  if len(sys.argv) > 2:
    filename = sys.argv[1]
    top_func = sys.argv[2]
  else:
    print "usage: ./inline.py file.c top_level_function"
    sys.exit()

  # Generate AST
  ast = parse_file(filename, use_cpp=True)

  # Find all defined functions
  v = GetFunctionsVisitor()
  v.visit(ast)
  functions = v.funcs

  # Remove function calls from being embedded in conditionals
  # v = RemoveIfFunctionVisitor()
  # v.visit(ast)
  # Inline functions
  v = ExpandFunctionVisitor(top_func, functions)
  v.visit(ast)
  # Keep performing until no changes
  while v.expanded:
    v.expanded = False
    v.visit(ast)

  # Print out non-function top-levels
  for c_name, c in ast.children():
    if not isinstance(c, c_ast.FuncDef):
      print_node(c) 
      print_node(c_ast.EmptyStatement())
  # Print out top-level function
  for function in functions:
    if function.decl.name == top_func:
      print_node(function)
