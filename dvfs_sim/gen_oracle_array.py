#!/usr/bin/env python

import os
import sys
from parse_lib import *
from dvfs_sim_lib import *

if __name__ == "__main__":

    output_dir = "predict_times/"
    output_file = open('oracle_array.txt', 'w')
    output_file.close()

    for benchmark in benchmarks:
        line_cnt=0
        print benchmark
        # read execution time from out_file
        with open("%s/%s-%s.txt" % (output_dir, "policy_oracle", benchmark), 'r') as f:
            number_of_array = sum(1 for _ in f)

        f = open("%s/%s-%s.txt" % (output_dir, "policy_oracle", benchmark), 'r')
        output_file = open('oracle_array.txt', 'a')
        output_file.write("\n\n#if _"+benchmark+"_\n")
        output_file.write("int exec_time_arr[" + str(number_of_array) + "]= {")
        for line in f:
            line_cnt = line_cnt + 1
            output_file.write(line.strip())
            if line_cnt != number_of_array:
                output_file.write(", ")

        output_file.write("};\n#endif")
        output_file.close()
        f.close()

