#!/usr/bin/python

input_range = range(1, 101)

output = "#!/bin/bash\n"
output += "rm output_slice.txt\n"
output += "./sha "
for i in input_range:
  output += "../sha/input_files/input_random%d.asc " % (i)
output += ">> output_slice.txt\n"

print output
