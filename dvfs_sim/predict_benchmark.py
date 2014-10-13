#!/usr/bin/python

import sys
from parse_lib import *
from dvfs_sim_lib import *
from pylab import *


filename = sys.argv[1]
times = parse_execution_times(filename)
deadline = max(times)

#predict_times = policy_average(times, window_size=5)
#predict_times = policy_pid(times, P=1, I=0.5, D=0.01)
metrics = parse(filename, "loop counter [0-9 ]*= \((.*)\)")
metrics = [x.strip().strip(',').split(',') for x in metrics]
metrics = [[int(y) for y in x] for x in metrics]
predict_times = policy_data_dependent(times, metrics)
#predict_times = policy_data_dependent_oracle(times, metrics)

conservative_margin = 1.1
frequencies = [scale_frequency_perfect(conservative_margin * t, deadline) for t in predict_times]
result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]

print deadline_misses(result_times, deadline)
print tardiness(result_times, deadline)
print energy(frequencies)

subplot(2, 1, 1)
plot(times, 'bo-')
plot(predict_times, 'ro-')
subplot(2, 1, 2)
title("DVFS, Deadline = %d" % (deadline))
cmap = get_cmap("gist_rainbow")
for i in range(len(times)):
  plot(i, result_times[i], 'o', color=cmap(frequencies[i]))
plot([deadline]*len(times), 'r')
show()
