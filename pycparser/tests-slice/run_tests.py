#!/usr/bin/python

import os
import filecmp

tests = ["arg_rename", "pass_array", "board_rename"]

errors = []

for test in tests:
  print test
  # Copy Makefile
  os.system("cp Makefile.%s Makefile" % test)
  # Run make
  os.system("make")
  # Run diff of outputs
  for file_extension in ["inline", "loop_counts", "slice"]:
    test_file = "%s/%s_%s.c" % (test, test, file_extension)
    truth_file = "%s/out/%s_%s.c" % (test, test, file_extension)
    if not filecmp.cmp(test_file, truth_file):
      errors.append("\033[91mError in %s \033[0m" % (test_file))
  # Clean-up
  os.system("make clean")

print
if errors:
  print '\n'.join(errors)
else:
  print "\033[92mAll tests passed!\033[0m"
