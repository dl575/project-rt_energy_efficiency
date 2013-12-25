
"""
Functions for parsing -fdump-tree-original-raw from gcc.

class ast_node
  __init__(self, expr, children, id, typ)
  __repr__(self)
  get_children_exprs(self) 
  get_identifier_nodes(self) 
  get_var_args(self)
  get_variables(self)
  is_variable(self)
  is_constant(self)

get_argument(arg_name, expression)
get_op0(expression)
get_op1(expression)
get_fn(expression)
get_args(expression)
get_valu(expression)
get_chan(expression)
get_name(expression)
get_strg(expression)
get_low(expression)
get_high(expression)
get_type(expression)

parse2(line)
expand2(expression)

debug_print(i)
recursive_debug_print(i)

find_variables(i, variables)
"""

import re
import sys

objects = {}

class ast_node:
  def __init__(self, expr = None, children = [], id = -1, typ = None):
    self.expr = expr
    self.children = children
    # ID number from gcc output
    self.id = id
    # Return type of node
    self.typ = None

  def __repr__(self):
    # If statement
    if self.expr == "cond_expr":
      return "if (%s)" % (self.children[0])
    # Function call
    elif self.expr == "call_expr":
      return "%s(%s)" % (self.children[0], self.children[1])
    elif self.expr == "function_decl":
      return "%s" % (self.children[0])
    elif self.expr == "addr_expr":
      # FIXME: If proceeded by call_expr, then no need to add &. Otherwise,
      # should have &.
      #return "&%s" % (self.children[0])
      return "%s" % (self.children[0])
    elif self.expr == "tree_list":
      # Remove braces
      return str(self.children)[1:-1]
    # Arrays and object members
    elif self.expr == "array_ref":
      return "%s[%s]" % (self.children[0], self.children[1])
    elif self.expr == "component_ref":
      if self.children[0].expr == "indirect_ref":
        return "%s->%s" % (self.children[0], self.children[1])
      else:
        return "%s.%s" % (self.children[0], self.children[1])
    elif self.expr == "indirect_ref" or self.expr == "var_decl" or self.expr == "field_decl" or self.expr == "identifier_node" or self.expr == "convert_expr" or self.expr == "parm_decl":
      return "%s" % self.children[0]
    # Comparison expressions
    elif self.expr == "eq_expr":
      return "%s == %s" % (self.children[0], self.children[1])
    elif self.expr == "ne_expr":
      return "%s != %s" % (self.children[0], self.children[1])
    elif self.expr == "ge_expr":
      return "%s >= %s" % (self.children[0], self.children[1])
    elif self.expr == "gt_expr":
      return "%s > %s" % (self.children[0], self.children[1])
    elif self.expr == "le_expr":
      return "%s <= %s" % (self.children[0], self.children[1])
    elif self.expr == "lt_expr":
      return "%s < %s" % (self.children[0], self.children[1])
    elif self.expr == "truth_orif_expr":
      return "(%s || %s)" % (self.children[0], self.children[1])
    elif self.expr == "truth_andif_expr":
      return "(%s && %s)" % (self.children[0], self.children[1])
    # Arithmetic expressions
    elif self.expr == "plus_expr":
      return "(%s + %s)" % (self.children[0], self.children[1])
    elif self.expr == "minus_expr":
      return "(%s - %s)" % (self.children[0], self.children[1])
    elif self.expr == "mult_expr":
      return "(%s * %s)" % (self.children[0], self.children[1])
    elif self.expr == "trund_div_expr":
      return "(%s / %s)" % (self.children[0], self.children[1])
    elif self.expr == "bit_and_expr":
      return "(%s & %s)" % (self.children[0], self.children[1])
    elif self.expr == "lshift_expr":
      return "(%s << %s)" % (self.children[0], self.children[1])
    elif self.expr == "rshift_expr":
      return "(%s >> %s)" % (self.children[0], self.children[1])
    elif self.expr == "preincrement_expr":
      return "++%s" % (self.children[0])
    elif self.expr == "postincrement_expr":
      return "%s++" % (self.children[0])
    elif self.expr == "nop_expr":
      return "%s" % (self.children[0])
    elif self.expr == "pointer_plus_expr":
      return "%s + %s" % (self.children[0], self.children[1])
    elif self.expr == "modify_expr":
      return "%s = %s" % (self.children[0], self.children[1])
    # Constants
    elif self.expr == "integer_cst" or self.expr == "string_cst":
      return "%s" % (self.children[0])
    else:
      raise Exception("Unknown expression type: %s" % self.expr)

  """
  Return a list of the expression types of the children
  """
  def get_children_exprs(self):
    return [(c.expr if c else None) for c in self.children]

  """
  Return a list of ast_node objects which have an expr
  type of "identifier_node".
  """
  def get_identifier_nodes(self):
    id_nodes = []
    # Handle node
    if self.expr == "identifier_node":
      id_nodes.append(self)
    # Search children
    for c in self.children:
      if isinstance(c, ast_node):
        id_nodes += c.get_identifier_nodes()
    return id_nodes
  
  """
  Returns a list of the variable (non-constant) arguments of the function
  expression. Assumes that the node is a function expression.
  """
  """
  def get_var_args(self):
    # Only valid for function call
    assert(self.expr == "call_expr")

    args = []
    # Traverse tree
    tree_list = self.children[1]
    # While more arguments in tree
    while len(tree_list.children) == 2:
      # Variable (not constant) found
      #if not tree_list.children[0].is_constant():
      if tree_list.children[0].is_variable():
        args.append(tree_list.children[0])
      # Move to next argument
      tree_list = tree_list.children[1]
    # Handle final argument
    # Don't add constant values
    #if not tree_list.is_constant():
    if tree_list.is_variable():
      args.append(tree_list.children[0])
    return args
  """

  """
  Recursively get all variables contained in this expression. Return a list of strings of
  all variables.
  """
  def get_variables(self, variables=[]):
    # This is a variable, add to list and return
    if self.is_variable():
      if str(self) not in variables:
        variables.append(str(self))
      return variables
    # Constant, return existing list
    if self.is_constant():
      return variables

    # Recursively perform for all children
    for c in self.children:
      variables = c.get_variables(variables)
    return variables

  """
  Return true if node reduces to a variable.
  """
  def is_variable(self):
    if self.expr == "var_decl" or self.expr == "component_ref" or self.expr == "parm_decl":
      return True
    else:
      return False
        
  """
  Return true if node reduces to a constant, false otherwise.
  """
  def is_constant(self):
    # Strings, integers, and nodes for variable names, etc. are constants
    if self.expr == "string_cst" or self.expr == "integer_cst" or self.expr == "identifier_node":
      return True
    # Variables and function arguments are not constant
    elif self.expr == "parm_decl" or self.expr == "var_decl":
      return False
    # Multiple children implies non-constant node
    elif len(self.children) >=2:
      return False
    # Otherwise, follow the node down to find its fundamental type
    else:
      return self.children[0].is_constant()

def get_argument(arg_name, expression):
  # Does not work correctly for strings with more than one word
  res_string = "%s: (\S+)" % arg_name
  res = re.search(res_string, expression)
  if res:
    return res.group(1)
  else:
    return None
def get_op0(expression):
  op = get_argument("op 0", expression)
  return int(op[1:]) if op else None
def get_op1(expression):
  op = get_argument("op 1", expression)
  return int(op[1:]) if op else None
def get_fn(expression):
  op = get_argument("fn  ", expression)
  return int(op[1:]) if op else None
def get_args(expression):
  op = get_argument("args", expression)
  return int(op[1:]) if op else None
def get_valu(expression):
  op = get_argument("valu", expression)
  return int(op[1:]) if op else None
def get_chan(expression):
  op = get_argument("chan", expression)
  return int(op[1:]) if op else None
def get_name(expression):
  name = get_argument("name", expression)
  return int(name[1:]) if name else None
def get_strg(expression):
  name = get_argument("strg", expression)
  return name
def get_low(expression):
  low = get_argument("low ", expression)
  return int(low) if low else None
def get_high(expression):
  high = get_argument("high", expression)
  return int(high) if high else None
def get_type(expression):
  typ = get_argument("type", expression)
  return int(typ[1:]) if typ else None

"""
Take a string line and return an ast_node.
"""
def parse(line):
  # Pull out the type of expression
  id_num = int(line.split()[0].lstrip('@'))
  expr = line.split()[1]

  # Get arguments
  op0 = get_op0(line)
  op1 = get_op1(line)
  name = get_name(line)
  strg = get_strg(line)
  low = get_low(line)
  high = get_high(line)
  fn = get_fn(line)
  args = get_args(line)
  valu = get_valu(line)
  chan = get_chan(line)
  typ = get_type(line)

  # If statement
  if expr == "cond_expr":
    ast = ast_node("cond_expr", [parse(objects[op0])]) 
  # Function call
  elif expr == "call_expr":
    if args:
      ast = ast_node("call_expr", [parse(objects[fn]), parse(objects[args])])
    else:
      ast = ast_node("call_expr", [parse(objects[fn]), ast_node("string_cst", ["void"])])
  elif expr == "addr_expr":
    ast = ast_node("addr_expr", [parse(objects[op0])])
  elif expr == "function_decl":
    ast = ast_node("function_decl", [parse(objects[name])])
  elif expr == "tree_list":
    if chan:
      ast = ast_node("tree_list", [parse(objects[valu]), parse(objects[chan])])
    else:
      ast = ast_node("tree_list", [parse(objects[valu])])
  # Arrays and object members
  elif expr == "array_ref":
    ast = ast_node("array_ref", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "component_ref":
    ast = ast_node("component_ref", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "indirect_ref":
    ast = ast_node("indirect_ref", [parse(objects[op0])])
  elif expr == "var_decl":
    ast = ast_node("var_decl", [parse(objects[name])])
  elif expr == "parm_decl":
    ast = ast_node("parm_decl", [parse(objects[name])])
  elif expr == "field_decl":
    ast = ast_node("field_decl", [parse(objects[name])])
  elif expr == "convert_expr":
    ast = ast_node("convert_expr", [parse(objects[op0])])
  # Comparison expressions
  elif expr == "eq_expr":
    ast = ast_node("eq_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "ne_expr":
    ast = ast_node("ne_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "ge_expr":
    ast = ast_node("ge_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "gt_expr":
    ast = ast_node("gt_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "le_expr":
    ast = ast_node("le_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "lt_expr":
    ast = ast_node("lt_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "truth_orif_expr":
    ast = ast_node("truth_orif_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "truth_andif_expr":
    ast = ast_node("truth_andif_expr", [parse(objects[op0]), parse(objects[op1])])
  # FIXME
  elif expr == "truth_and_expr":
    ast = ast_node("string_cst", ["TRUE"])
  # Arithmetic expressions
  elif expr == "plus_expr":
    ast = ast_node("plus_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "minus_expr":
    ast = ast_node("minus_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "mult_expr":
    ast = ast_node("mult_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "trunc_div_expr":
    ast = ast_node("trund_div_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "bit_and_expr":
    ast = ast_node("bit_and_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "lshift_expr":
    ast = ast_node("lshift_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "rshift_expr":
    ast = ast_node("rshift_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "preincrement_expr":
    ast = ast_node("preincrement_expr", [parse(objects[op0])])
  elif expr == "postincrement_expr":
    ast = ast_node("postincrement_expr", [parse(objects[op0])])
  elif expr == "nop_expr":
    ast = ast_node("nop_expr", [parse(objects[op0])])
  elif expr == "pointer_plus_expr":
    ast = ast_node("pointer_plus_expr", [parse(objects[op0]), parse(objects[op1])])
  elif expr == "modify_expr":
    ast = ast_node("modify_expr", [parse(objects[op0]), parse(objects[op1])])
  # Constants
  elif expr == "integer_cst":
    # if high:
    #   ast = "%d" % ((-high << 64) + -low)
    # else:
    ast = ast_node("integer_cst", ["%d" % low])
  elif expr == "string_cst":
    ast = ast_node("string_cst", ["\"%s\"" % (strg)])
  elif expr == "identifier_node":
    ast = ast_node("identifier_node", [strg])
  else:
    print "Error: Unknown expression type %s" % line.split()[1]
    print "  "  + line,
    sys.exit()
    #raise Exception("[Unknown expression type: %s]" % line.split()[1])

  # Save node number
  ast.id = id_num
  # Get type of expression (in string format)
  ast.typ = objects[typ].split()[1] if typ else None
  return ast

"""
Print out information about the passed node number.
"""
def debug_print(i):
  print
  print objects[i]
  root = expand2(objects[i])
  print root
  print root.typ

"""
Print out information about the passed node number and all its
descendents.
"""
def debug_print_recursive(i, tab=""):
  print tab, objects[i]
  root = expand2(objects[i])
  print tab, root
  print tab, root.typ, root.is_constant(), root.is_variable()

  for c in root.children:
    try:
      debug_print_recursive(c.id, tab + "  ")
    except:
      pass
