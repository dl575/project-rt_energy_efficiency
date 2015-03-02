#!/usr/bin/python

import os
import filecmp
import glob

#tests = ["arg_rename", "pass_array", "board_rename"]
makefiles = glob.glob("Makefile.*")
tests = [x.split(".")[1] for x in makefiles]

test_results = []

for test in tests:
  print test
  # Copy Makefile
  os.system("cp Makefile.%s Makefile" % test)
  # Run make
  os.system("make")
  # Run diff of outputs
  errors = None
  for file_extension in ["inline", "loop_counts", "slice"]:
    test_file = "%s/%s_%s.c" % (test, test, file_extension)
    truth_file = "%s/out/%s_%s.c" % (test, test, file_extension)
    if not filecmp.cmp(test_file, truth_file):
      errors = file_extension
      break
  if errors:
    test_results.append("\033[91m%s_%s failed\033[0m" % (test, file_extension))
  else:
    test_results.append("\033[92m%s passed\033[0m" % (test))
  # Clean-up
  os.system("make clean")
os.system("rm Makefile")

print
print '\n'.join(test_results)
