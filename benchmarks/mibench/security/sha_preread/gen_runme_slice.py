#!/usr/bin/python

#input_range = range(1, 101)

input_range = range(1, 100)

output = "#!/bin/bash\n"
output += "rm output_slice.txt\n"
output += "./sha "
for j in xrange(0, 30):
  for i in input_range:
    #output += "../sha/input_files/input_random%d.asc " % (i)
    output += "../../../../datasets/sha-50ms/input%d.txt " % (i)
output += ">> output_slice.txt\n"

print output
