
"""
Functions for parsing -fdump-tree-original-raw from gcc.
"""

import re

objects = {}

class ast_node:
  def __init__(self, expr = None, children = [], id = -1):
    self.expr = expr
    self.children = children
    self.id = id

  def __repr__(self):
    # If statement
    if self.expr == "cond_expr":
      return "if (%s)" % (self.children[0])
    # Function call
    elif self.expr == "call_expr":
      return "%s(%s)" % (self.children[0], self.children[1])
    elif self.expr == "addr_expr" or self.expr == "function_decl":
      return "%s" % (self.children[0])
    elif self.expr == "tree_list":
      # Remove braces
      return str(self.children)[1:-1]
    # Arrays and object members
    elif self.expr == "array_ref":
      return "%s[%s]" % (self.children[0], self.children[1])
    elif self.expr == "component_ref":
      return "%s->%s" % (self.children[0], self.children[1])
    elif self.expr == "indirect_ref" or self.expr == "var_decl" or self.expr == "field_decl" or self.expr == "identifier_node" or self.expr == "convert_expr":
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
    elif self.expr == "nop_expr":
      return "%s" % (self.children[0])
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
  Return a list of ast_node objects which are have an expr
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
  def get_var_args(self):
    assert(self.expr == "call_expr")
    args = []
    # Traverse tree
    tree_list = self.children[1]
    while len(tree_list.children) == 2:
      if not tree_list.children[0].is_constant():
        args.append(tree_list.children[0])
      tree_list = tree_list.children[1]
    if not tree_list.is_constant():
      args.append(tree_list.children[0])
    return args

  """
  Return true if node reduces to a constant, false otherwise.
  """
  def is_constant(self):
    if self.expr == "string_cst" or self.expr == "integer_cst":
      return True
    elif self.expr == "identifier_node":
      return False
    elif len(self.children) >=2:
      return False
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

def parse(line):
  HOST_BITS_PER_INT = 64

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
  # If statement
  if "cond_expr" in line:
    return "if (@%d)" % op0
  # Function call
  elif "call_expr" in line:
    return "@%s(@%s)" % (fn, args)
  elif "addr_expr" in line:
    return "@%s" % (op0)
  elif "function_decl" in line:
    return "@%s" % (name)
  elif "tree_list" in line:
    if chan:
      return "@%s, @%s" % (valu, chan)
    else:
      return "@%s" % (valu)
  # Arrays and object members
  elif "array_ref" in line:
    return "@%s[@%s]" % (op0, op1)
  elif "component_ref" in line:
    return "@%s->@%s" % (op0, op1)
  elif "indirect_ref" in line:
    return "@%s" % (op0)
  elif "var_decl" in line:
    return "@%s" % (name)
  elif "field_decl" in line:
    return "@%s" % (name)
  elif "identifier_node" in line:
    return "%s" % (strg)
  elif "convert_expr" in line:
    return "@%s" % (op0)
  # Comparison expressions
  elif "eq_expr" in line:
    return "@%s == @%s" % (op0, op1)
  elif "ne_expr" in line:
    return "@%s != @%s" % (op0, op1)
  elif "ge_expr" in line:
    return "@%s >= @%s" % (op0, op1)
  elif "gt_expr" in line:
    return "@%s > @%s" % (op0, op1)
  elif "le_expr" in line:
    return "@%s <= @%s" % (op0, op1)
  elif "lt_expr" in line:
    return "@%s < @%s" % (op0, op1)
  elif "truth_orif_expr" in line:
    return "(@%s) || (@%s)" % (op0, op1)
  elif "truth_andif_expr" in line:
    return "(@%s) && (@%s)" % (op0, op1)
  # Arithmetic expressions
  elif "plus_expr" in line:
    return "(@%s + @%s)" % (op0, op1)
  elif "minus_expr" in line:
    return "(@%s - @%s)" % (op0, op1)
  elif "mult_expr" in line:
    return "(@%s * @%s)" % (op0, op1)
  elif "trunc_div_expr" in line:
    return "(@%s / @%s)" % (op0, op1)
  elif "bit_and_expr" in line:
    return "(@%s & @%s)" % (op0, op1)
  elif "lshift_expr" in line:
    return "(@%s << @%s)" % (op0, op1)
  elif "rshift_expr" in line:
    return "(@%s >> @%s)" % (op0, op1)
  elif "preincrement_expr" in line:
    return "++@%s" % (op0)
  elif "nop_expr" in line:
    return "@%s" % (op0)
  # Constants
  elif "integer_cst" in line:
    # if high:
    #   return "%d" % ((-high << 64) + -low)
    # else:
    return "%d" % low
  elif "string_cst" in line:
    return "\"%s\"" % (strg)
  else:
    return "[Unknown expression type: %s]" % line.split()[1]


"""
Take a string line and return an ast_node.
"""
def parse2(line):
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

  # If statement
  if expr == "cond_expr":
    ast = ast_node("cond_expr", [parse2(objects[op0])]) 
  # Function call
  elif expr == "call_expr":
    if args:
      ast = ast_node("call_expr", [parse2(objects[fn]), parse2(objects[args])])
    else:
      ast = ast_node("call_expr", [parse2(objects[fn]), ast_node("string_cst", ["void"])])
  elif expr == "addr_expr":
    ast = ast_node("addr_expr", [parse2(objects[op0])])
  elif expr == "function_decl":
    ast = ast_node("function_decl", [parse2(objects[name])])
  elif expr == "tree_list":
    if chan:
      ast = ast_node("tree_list", [parse2(objects[valu]), parse2(objects[chan])])
    else:
      ast = ast_node("tree_list", [parse2(objects[valu])])
  # Arrays and object members
  elif expr == "array_ref":
    ast = ast_node("array_ref", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "component_ref":
    ast = ast_node("component_ref", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "indirect_ref":
    ast = ast_node("indirect_ref", [parse2(objects[op0])])
  elif expr == "var_decl":
    ast = ast_node("var_decl", [parse2(objects[name])])
  elif expr == "field_decl":
    ast = ast_node("field_decl", [parse2(objects[name])])
  elif expr == "convert_expr":
    ast = ast_node("convert_expr", [parse2(objects[op0])])
  # Comparison expressions
  elif expr == "eq_expr":
    ast = ast_node("eq_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "ne_expr":
    ast = ast_node("ne_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "ge_expr":
    ast = ast_node("ge_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "gt_expr":
    ast = ast_node("gt_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "le_expr":
    ast = ast_node("le_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "lt_expr":
    ast = ast_node("lt_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "truth_orif_expr":
    ast = ast_node("truth_orif_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "truth_andif_expr":
    ast = ast_node("truth_andif_expr", [parse2(objects[op0]), parse2(objects[op1])])
  # Arithmetic expressions
  elif expr == "plus_expr":
    ast = ast_node("plus_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "minus_expr":
    ast = ast_node("minus_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "mult_expr":
    ast = ast_node("mult_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "trunc_div_expr":
    ast = ast_node("trund_div_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "bit_and_expr":
    ast = ast_node("bit_and_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "lshift_expr":
    ast = ast_node("lshift_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "rshift_expr":
    ast = ast_node("rshift_expr", [parse2(objects[op0]), parse2(objects[op1])])
  elif expr == "preincrement_expr":
    ast = ast_node("preincrement_expr", [parse2(objects[op0])])
  elif expr == "nop_expr":
    ast = ast_node("nop_expr", [parse2(objects[op0])])
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
    raise Exception("[Unknown expression type: %s]" % line.split()[1])

  ast.id = id_num
  return ast

def expand(expression, objects):
  res = re.search("@([0-9]+)", expression)
  while res:
    obj_id = res.group(1)
    # Only replace first instance to avoid incorrectly overwriting others.
    # For example replacing @355 when trying to replace @3
    # FIXME: not really true
    expression = expression.replace('@' + obj_id, parse(objects[int(obj_id)]), 1)
    res = re.search("@([0-9]+)", expression)
  return expression

def expand2(expression):
  # Create root node
  root = parse2(expression)
  
  return root
