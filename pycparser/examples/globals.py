#!/usr/bin/python

"""
globals.py

Classes:
  GetEnumsVisitor
  FindFuncDefVisitor
  GetGlobalVarsVisitor
  GetDeclVisitor

Functions:
  print_node(node)
  get_decl_name(node)
  set_decl_name(node, name)
  generate_array_ref(decl, subscript_base)
  rename_array_decl(decl)
  nested_for_loop(node, inner_loop_body, loop_iterator_base, rename_index)
  rename_array_args(funcdef)
  rename_global_vars(funcdef)
"""

import sys
import copy

# This is not required if you've installed pycparser into
# your site-packages/ with setup.py
#
sys.path.extend(['.', '..'])

from pycparser import parse_file
from pycparser import c_generator
from pycparser import c_ast

import inline

class GetEnumsVisitor(c_ast.NodeVisitor):
  """
  Find all enums.
  """
  def __init__(self):
    self.enums = []
  def visit_Enumerator(self, node):
    self.enums.append(node.name)


class FindFuncDefVisitor(c_ast.NodeVisitor):
  """
  Find the FuncDef node for the specified function name.
  """
  def __init__(self, function_name):
    # Target function
    self.function_name = function_name
    self.funcdef = None
  def visit_FuncDef(self, node):
    if node.decl.name == self.function_name:
      self.funcdef = node

class GetGlobalVarsVisitor(c_ast.NodeVisitor):
  """
  Identify all IDs in tree. 
  Arrays and structs are identified by base name.
  """
  def __init__(self, declnames=[]):
    # All variable names
    self.IDs = []
    # Variables which are declared
    self.declnames = declnames
    self.cgenerator = c_generator.CGenerator()
  def reset(self):
    self.IDs = []
  def new_visit(self, node):
    """
    Reset list of IDs, then visit passed node.
    """
    self.reset()
    self.visit(node)
  def get_globals(self):
    return list(set(self.IDs) - set(self.declnames))
  def visit_ID(self, node):
    if node.name not in self.IDs:
      self.IDs.append(node.name)
  def visit_StructRef(self, node):
    if isinstance(node, str):
      self.IDs.append(node)
    else:
      self.visit(node.name)
  def visit_TypeDecl(self, node):
    self.declnames.append(node.declname)
  def visit_FuncCall(self, node):
    """
    Don't include function calls as variable IDs.
    """
    pass
  def visit_Cast(self, node):
    pass
  def visit_Enumerator(self, node):
    """
    Mark enumerated IDs as declared.
    """
    self.declnames.append(node.name)

class GetDeclVisitor(c_ast.NodeVisitor):
  """
  Get global Decl nodes.
  """
  def __init__(self):
    self.decls = []
  def visit_Decl(self, node):
    self.decls.append(node)
  def visit_FuncDef(self, node):
    """
    Don't visit into function declarations. Only care about globally
    declared variables.
    """
    pass

def print_node(node):
  """
  Pretty print the passed AST/node.
  """
  generator = c_generator.CGenerator()
  print generator.visit(node)

def get_decl_name(node):
  if isinstance(node, c_ast.TypeDecl):
    return node.declname
  else:
    return get_decl_name(node.type)

def set_decl_name(node, name):
  if isinstance(node, c_ast.TypeDecl):
    node.declname = name
  else:
    set_decl_name(node.type, name)

def generate_array_ref(decl, subscript_base):
  """
  Generate ArrayRef from Decl.
  For example:
    int array[1][2][3];
  Should become:
    array[subscript_base_i0][subscript_base_i1][subscript_base_i2]
  """
  if isinstance(decl.type.type, c_ast.ArrayDecl):
    # Multi-dimensional, recurse
    name = generate_array_ref(decl.type, subscript_base)
    # Subscript index is one more than the node below
    subscript_index = int(name.subscript.name[-1]) + 1
    if subscript_index >= 10:
      raise Exception("Only single-digit subscript indicies currently supported. Please fix.")
  else:
    # Single or last dimension
    name = c_ast.ID(decl.type.type.declname)
    # Base node has subscript index of 0
    subscript_index = 0

  # Create ArrayRef
  array_ref = c_ast.ArrayRef(
    name,
    c_ast.ID("%s%d" % (subscript_base, subscript_index))
    )
  return array_ref

def rename_array_decl(decl):
  """
  Pass a Decl node for an array and generate the needed declaration to create a
  copy of the array.
  For example:
    int array[4];
  Should become:
    int array_rename[4];
    int array_i;
    for (array_i = 0; array_i < 4; array_i++) {
      array_rename[array_i] = array[array_i];
    }

  Returns a list of the [renamed decl, loop iterator decl, for loop]
  """
  assert(isinstance(decl, c_ast.Decl))
  assert(isinstance(decl.type, c_ast.ArrayDecl))

  # Names to use
  base_name = decl.name
  arg_name = decl.name + "_rename"
  loop_iterator = base_name + "_i"

  # Generate ArrayRef for array to use in body of function
  arg_array = generate_array_ref(decl, loop_iterator)
  # Rename declaration for passed argument
  set_decl_name(decl, arg_name)
  # Generate ArrayRef for array passed as argument
  body_array = generate_array_ref(decl, loop_iterator)
  # Assignment to copy arg_array to body_array
  array_copy_assignment = c_ast.Assignment('=', body_array, arg_array)

  # Generate (nested) for loopo for array copy
  (loop_iterator_decl, for_loop) = nested_for_loop(decl, array_copy_assignment, base_name)

  return [decl, loop_iterator_decl, for_loop]

def nested_for_loop(node, inner_loop_body, loop_iterator_base, rename_index=0):
  """
  Recursively create nested for loops for copying a multi-dimensional array.
  """
  if isinstance(node.type.type, c_ast.ArrayDecl):
    # Multi-dimensional array, recurse to generate inner loop
    for_loop_body = nested_for_loop(node.type, inner_loop_body, loop_iterator_base, rename_index + 1)
  else:
    # Single or last dimension of array
    for_loop_body = [inner_loop_body]

  # Declare iterator
  loop_iterator = c_ast.ID("%s_i%d" % (loop_iterator_base, rename_index))
  loop_iterator_decl = c_ast.Decl(loop_iterator.name, [], [], [], c_ast.TypeDecl(loop_iterator.name, [], c_ast.IdentifierType(["int"])), None, None)
  # For loop
  array_size = node.type.dim
  for_loop = c_ast.For(
    c_ast.Assignment('=', loop_iterator, c_ast.Constant("int", "0")),
    c_ast.BinaryOp('<', loop_iterator, array_size),
    c_ast.UnaryOp('p++', loop_iterator),
    c_ast.Compound(for_loop_body)
    )
  return [loop_iterator_decl, for_loop]

def rename_array_args(funcdef):
  """
  Rename and copy arrays passed as arguments to funcdef.
  """
  # For each argument
  for param in funcdef.decl.type.args.params:
    if isinstance(param.type, c_ast.ArrayDecl):
      # Rename and copy array
      arg_decl = copy.deepcopy(param)
      # Rename array
      v = inline.RenameVisitor()
      v.new_visit(get_decl_name(param), get_decl_name(param) + "_rename", funcdef.body)
      # Add copy and declarations
      funcdef.body.block_items = rename_array_decl(arg_decl) + funcdef.body.block_items
    elif isinstance(param.type, c_ast.TypeDecl):
      # Simple variable passing, don't need to handle
      pass
    elif isinstance(param.type, c_ast.PtrDecl):
      """
      Param of form: type *var
      is copied in the function body using:
        type var_rename _temp = *var;
        type *var_rename = &var_rename_temp;
      """
      # General pointer arguments
      old_name = get_decl_name(param)
      new_name = old_name + "_rename"
      temp_name = new_name + "_temp"
      # Rename variable use in function body
      v = inline.RenameVisitor()
      v.new_visit(old_name, new_name, funcdef.body)

      # type var_rename_temp = *var;
      decl1 = c_ast.Decl(temp_name, None, None, None,
        c_ast.TypeDecl(temp_name, None, param.type.type.type),
        c_ast.UnaryOp('*', c_ast.ID(old_name)),
        None
        )

      # type *var_rename = &var_rename_temp;
      decl2 = c_ast.Decl(new_name, None, None, None,
        c_ast.PtrDecl([],
          c_ast.TypeDecl(new_name, None, param.type.type.type)
        ),
        c_ast.UnaryOp('&', c_ast.ID(temp_name)),
        None
        )

      # Insert into function body
      funcdef.body.block_items.insert(0, decl2)
      funcdef.body.block_items.insert(0, decl1)
    else:
      print_node(param)
      param.show(nodenames=True, showcoord=True)
      raise Exception("Unhandled argument type %s. Implement or verify that it can be ignored." % (type(param.type)))

def rename_global_vars(funcdef):
  """
  Rename and copy all global variables for the specified function funcdef.
  """
  # Get enums so they can be included as declared variables
  v = GetEnumsVisitor()
  v.visit(ast)
  enums = v.enums
  # Get all declarations so we know types for global variables
  v = GetDeclVisitor()
  v.visit(ast)
  decls = v.decls
  # Find all global variables
  v = GetGlobalVarsVisitor(enums + ignore_vars)
  v.visit(funcdef)
  global_vars = v.get_globals()

  # For each global variable
  for global_var in global_vars:
    global_var_rename = global_var + "_rename"
    # Rename in function 
    v = inline.RenameVisitor()
    v.new_visit(global_var, global_var_rename, funcdef)

    # Find corresponding declaration
    global_var_decl = None
    for decl in decls:
      if decl.name == global_var:
        global_var_decl = copy.deepcopy(decl)
    if not global_var_decl:
      raise Exception("Global variable declaration for %s not found" % global_var)

    # Modify declaration to declare renamed version
    if isinstance(global_var_decl.type, c_ast.ArrayDecl):
      funcdef.body.block_items = rename_array_decl(global_var_decl) + funcdef.body.block_items
    elif isinstance(global_var_decl.type, c_ast.TypeDecl):
      # Rename declaration
      set_decl_name(global_var_decl, global_var_rename)
      # Add init as original name
      global_var_decl.init = c_ast.ID(global_var)
      funcdef.body.block_items.insert(0, global_var_decl)
    elif isinstance(global_var_decl.type, c_ast.PtrDecl) and isinstance(global_var_decl.type.type.type, c_ast.Struct):
      # Handle struct passed by pointer
      old_name = get_decl_name(global_var_decl)
      new_name = old_name + "_rename"
      struct_type = global_var_decl.type.type.type.name
      # Rename struct
      v = inline.RenameVisitor()
      v.new_visit(old_name, new_name, funcdef.body)
      # Add declaration
      decl = c_ast.Decl(new_name, None, None, None,
        c_ast.PtrDecl([],
          c_ast.TypeDecl(new_name, None, global_var_decl.type.type.type),
        ),
        c_ast.ID(old_name),
        None 
        )
      funcdef.body.block_items.insert(0, decl)
    else:
      print_node(global_var_decl)
      global_var_decl.show(nodenames=True)
      raise Exception("Unhandled type %s for global_var_decl" % (type(global_var_decl.type)))



if __name__ == "__main__":
  if len(sys.argv) > 2:
    filename = sys.argv[1]
    function = sys.argv[2]
    if len(sys.argv) > 3:
			ignore_vars = sys.argv[3:]
    else:
      ignore_vars = []
  else:
    print "usage: globals.py filename function [ignore_var0 ...]"
    sys.exit()

  # Generate AST
  ast = parse_file(filename, use_cpp=True)

  # Get the FuncDef node of the function we are interested in
  v = FindFuncDefVisitor(function)
  v.visit(ast)
  funcdef = v.funcdef
  if not funcdef:
    raise Exception("Function %s not found" % (function))

  # Rename and copy array arguments
  rename_array_args(funcdef)
  # Rename and copy global variables
  rename_global_vars(funcdef)
 
 	# Output result
  print_node(ast)
