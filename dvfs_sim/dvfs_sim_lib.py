"""
dvfs_sim_lib.py

This file includes function used for simulating execution time prediction and
its use for setting DVFS levels.

Functions:
  average(l)
  regression(Y, X)

  random_time_trace(N, mu, sigma)
  markov_time_trace(N, start, sigma)
  step_time_trace()

  dvfs_time(time, frequency)
  scale_frequency(predicted_time, deadline, dvfs_levels)
  scale_frequency_perfect(predicted_time, deadline)

  policy_average(times, window_size, metrics)
  policy_pid(times, P, I, D, metrics)
  policy_data_dependent(times, metrics)
  policy_data_dependent_oracle(times, metrics)

  deadline_misses(times, deadline)
  tardiness(times, deadline)
  normalized_tardiness(times, deadline)
  energy(frequencies)
"""

import math
import random
import numpy

default_dvfs_levels = [x*.1 for x in range(1, 11)]

def average(l):
  return float(sum(l))/len(l)

def regression(Y, X):
  """
  Performs multiple linear regression to calculate the least squares
  coefficients A for y = Ax.  Y is a numpy vector of y values and X is a numpy
  matrix where each row is an x vector.
  """
  # Add a constant column to X
  X = numpy.hstack((numpy.array([[1]*X.shape[0]]).T, X))
  (coeffs, residuals, rank, s) = numpy.linalg.lstsq(X, numpy.transpose(Y))
  return coeffs

def random_time_trace(N=300, mu=30, sigma=5):
  """
  Generate a set of random execution times. Generate N points randomly selected
  from a normal distribution with mean mu and standard deviation sigma.
  """
  times = [random.gauss(mu, sigma) for i in range(N)]
  return times

def markov_time_trace(N=300, start=30, sigma=5):
  """
  Generate a set of random execution times. Time[0] is equal to start. Each
  time is the previous time plus a value sampled from a normal distribution
  with mean 0 and standard deviation sigma. N points are generated and a
  minimum time of 1 is enforced.
  """
  times = [start]*N
  for i in range(1, N):
    times[i] = times[i-1] + random.gauss(0, sigma)
    if times[i] < 1:
      times[i] = 1
  return times

def step_time_trace():
  times = []
  for i in range(1, 3):
    times += [10*i for x in range(30)]
  for i in range(3, 0, -1):
    times += [10*i for x in range(30)]
  return times

def dvfs_time(time, frequency):
  """
  Apply frequency scaling to the passed time. It is assumed that the passed
  time is the execution time to run at a normalized frequency of 1. The
  execution time scaled by the frequency is returned. Frequencies above 1
  correspond to higher frequencies and thus lower execution time while
  frequencies below 1 correspond to increased execution time.
  """
  return float(time)/frequency

def scale_frequency(predicted_time, deadline, dvfs_levels=default_dvfs_levels):
  """
  Attempt to scale frequency in order to get the passed predicted_time to be
  less than or equal to the deadline. The lowest frequency possible is used.
  """
  desired_frequency = float(predicted_time)/deadline
  for dvfs_level in dvfs_levels:
    if dvfs_level >= desired_frequency:
      break
  return dvfs_level

def scale_frequency_perfect(predicted_time, deadline):
  """
  Return the exact frequency needed to meet deadline.
  """
  desired_frequency = float(predicted_time)/deadline
  if predicted_time <= 0:
    return -1
  return desired_frequency

def policy_average(times, window_size=10, metrics=None):
  predicted_times = list(times)
  for i in range(window_size, len(times)):
    window = times[i-window_size:i]
    predicted_times[i] = average(window)
  return predicted_times

def policy_pid(times, P=1, I=0.5, D=0.01, metrics=None):
  # Initialize predicted_times
  predicted_times = [0]*len(times)
  # Initialize errors
  error = 0
  i_error = 0
  d_error = 0
  
  for i in range(1, len(times)):
    predicted_times[i] = predicted_times[i-1] + P*error + I*i_error + D*d_error

    d_error = (times[i] - predicted_times[i]) - error
    error = times[i] - predicted_times[i]
    i_error += error
  return predicted_times

def policy_data_dependent(times, metrics):
  window_size = 5
  # Initialize predicted times
  predicted_times = [0]*len(times)

  # For each task,
  for i in range(1, len(times)):
    # Perform regression using passed metrics
    y = numpy.array([times[:i]])
    x = numpy.array(metrics[:i])
    coeffs = regression(y, x)
    # Use regression coefficients to predict next frame
    x = [1] + metrics[i]
    predicted_times[i] = numpy.dot(x, coeffs)[0]
    if predicted_times[i] > 2*max(times):
      predicted_times[i] = 2*max(times)
    elif predicted_times[i] < 0:
      predicted_times[i] = -1 

  return predicted_times

def policy_data_dependent_oracle(times, metrics):
  """
  Use all times and metrics to perform regression first. Then, use complete model
  to predict times.
  """
  y = numpy.array([times])
  x = numpy.array(metrics)
  coeffs = regression(y, x)

  predicted_times = [0]*len(times)
  for i in range(len(times)):
    x = [1] + metrics[i]
    predicted_times[i] = numpy.dot(x, coeffs)[0]
  return predicted_times

def deadline_misses(times, deadline):
  misses = [(1 if x > deadline else 0) for x in times]
  return float(sum(misses))/len(times)

def tardiness(times, deadline):
  tardiness = [max(x - deadline, 0) for x in times]
  return (float(sum(tardiness))/len(times), max(tardiness))

def normalized_tardiness(times, deadline):
  tardiness = [float(max(x - deadline, 0))/deadline for x in times]
  return (sum(tardiness)/len(times), max(tardiness))

def energy(frequencies):
  energies = [x*x for x in frequencies]
  return average(energies)
