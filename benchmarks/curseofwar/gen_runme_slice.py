#!/usr/bin/python

output = "#!/bin/bash\n"
output += "rm output_slice.txt\n"
output += "./curseofwar-sdl > output_slice.txt\n"

print output
