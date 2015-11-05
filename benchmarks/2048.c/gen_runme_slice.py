#!/usr/bin/python

output = "#!/bin/bash\n"
output += "rm times.txt\n"
output += "rm output_slice.txt\n"
output += "./2048\n"
output += "cp times.txt output_slice.txt"

print output
