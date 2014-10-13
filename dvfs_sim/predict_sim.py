#!/usr/bin/python

from dvfs_sim_lib import *
from pylab import *

deadline = 100
#times = step_time_trace()
times = markov_time_trace(N=100, start=25, sigma=3)
#times = [10]*50 + [10*x + 20 for x in range(1, 25)]

#predict_times = policy_average(times, window_size=5)
predict_times = policy_pid(times, P=1, I=0.5, D=0.01)
#predict_times = policy_data_dependent(times)

frequencies = [scale_frequency(int(t), deadline) for t in predict_times]
result_times = [dvfs_time(t, f) for (t, f) in zip(times, frequencies)]

ax = subplot(3, 1, 1)
plot(times, 'bo')
plot(predict_times, 'rx')
cmap = get_cmap("gist_rainbow")
for i in range(len(times)):
  plot(i, result_times[i], '*', color=cmap(frequencies[i]))
import matplotlib
sm = matplotlib.cm.ScalarMappable(cmap=cmap)
sm.set_array([0, 1])
colorbar(sm)
legend(["Max f Time", "Predicted Time", "DVFS Time"])

subplot(3, 1, 2)
plot([x - y for (x, y) in zip(predict_times, times)], 'ro-')
title("Prediction Error")

subplot(3, 1, 3)
plot([x - deadline for x in result_times], 'go')
title("Deadline Error")

show()

