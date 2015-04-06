#!/usr/bin/python

import glob
import os

makefiles = glob.glob("Makefile.*")

# Save existing Makefile
os.system("mv Makefile Makefile_tmp")

for makefile in makefiles:
  os.system("cp %s Makefile" % (makefile))
  os.system("make clean")

# Restore old Makefile
os.system("mv Makefile_tmp Makefile")
