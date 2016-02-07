#!/usr/bin/python

import re
import os
import sys

cur_path=os.getcwd()
big_litte= sys.argv[1]

sweep = sys.argv[2]
governor_files = sys.argv[3]
#governor_files = ["performance", "interactive", "conservative", "ondemand", "powersave", "prediction_with_overhead", "prediction_wo_overhead"]

f = open(cur_path+"/"+big_litte+"/uzbl/uzbl-"+sweep+"/"+governor_files, 'r')

start_printing = False
initial_end = False
for line in f:
    if start_printing:
        print line,

    # When it meets "keycmd_promt" first time
    #if ("time 579 = " in line) | ("time_slice 579 = " in line):
    if ("csl" in line):
        initial_end = True

    if ("deadline time" in line) & (initial_end):
        start_printing = True

f.close()
