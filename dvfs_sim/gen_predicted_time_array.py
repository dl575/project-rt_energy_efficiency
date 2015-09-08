#!/usr/bin/env python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *

if __name__ == "__main__":

    output_dir = "data"
    output_file = open('predicted_time_array.txt', 'w')
    output_file.close()

    for benchmark in benchmarks:
        line_cnt=0
        print benchmark
        # read execution time from out_file
        f = "%s/%s/%s0.txt" % (output_dir, benchmark, benchmark)
        predicted_times = parse_predicted_times(f)
        print predicted_times
        output_file = open('predicted_time_array.txt', 'a')
        output_file.write("\n\n#if _"+benchmark+"_\n")
        output_file.write("int predicted_times_arr[" + str(len(predicted_times)) + "]= {")
        for i in range(len(predicted_times)):
            line_cnt = line_cnt + 1
            output_file.write(str(predicted_times[i]))
            if line_cnt != len(predicted_times):
                output_file.write(", ")

        output_file.write("};\n#endif")
        output_file.close()

