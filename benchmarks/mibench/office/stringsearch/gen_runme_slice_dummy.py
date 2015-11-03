#!/usr/bin/python

output = "#!/bin/bash\n"
output += "rm output_slice.txt\n"
output += "./search_large "
output += "> output_slice.txt\n"

print output
