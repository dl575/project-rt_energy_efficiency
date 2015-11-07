#!/usr/bin/python

output = "#!/bin/bash\n"
output += "rm times.txt\n"
output += "rm output_slice.txt\n"
output += "../install/bin/pocketsphinx_batch -argfile argFile.txt -cepdir \
../../../datasets/pocketsphinx_small/ -ctl ctlFile.txt -cepext .wav \
-adcin true -hyp out.txt\n"
output += "cp times.txt output_slice.txt"

print output
