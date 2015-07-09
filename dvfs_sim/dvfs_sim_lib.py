"""
dvfs_sim_lib.py

This file includes function used for simulating execution time prediction and
its use for setting DVFS levels.

Functions:
  average(l)
  regression(Y, X)
  print_dict(d, tabs)

  random_time_trace(N, mu, sigma)
  markov_time_trace(N, start, sigma)
  step_time_trace()

  dvfs_time(time, frequency)
  scale_frequency(predicted_time, deadline, dvfs_levels)
  scale_frequency_perfect(predicted_time, deadline)

  policy_average(train_times, train_metrics, test_times, test_metrics, window_size)
  policy_worstcase(train_times, train_metrics, test_times, test_metrics)
  policy_pid(times, P, I, D, metrics)
  policy_pid_timeliness(train_times, train_metrics, test_times, test_metrics)
  policy_pid_energy(train_times, train_metrics, test_times, test_metrics)
  policy_least_squares(train_times, train_metrics, test_times, test_metrics)
  policy_conservative(train_times, train_metrics, test_times, test_metrics)
  policy_lasso(train_times, train_metrics, test_times, test_metrics);
  policy_cvx(train_times, train_metrics, test_times, test_metrics, obj, constraints):
  policy_cvx_conservative_lasso(train_times, train_metrics, test_times, test_metrics, underpredict_penalty, lasso_weight):
  policy_oracle(train_times, train_metrics, test_times, test_metrics)

  deadline_misses(times, frequencies, deadline)
  tardiness(times, frequencies, deadline)
  normalized_tardiness(times, frequencies, deadline)
  avg_normalized_tardiness(times, frequencies, deadline)
  max_normalized_tardiness(times, frequencies, deadline)
  energy(times, frequencies, deadline)

  read_predict_file(filename)
"""

import itertools
import math
import random
import numpy
import os
import cvxpy
import sklearn.linear_model

benchmarks = ["pocketsphinx", "sha_preread", "rijndael_preread",
"xpilot_slice", "2048_slice", "curseofwar_slice_sdl", "uzbl", "ldecode"]

default_dvfs_levels = [.1*x for x in range(1, 11)]

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

def print_dict(d, tabs=0):
  for k, v in d.iteritems():
    print '  '*tabs, k
    if isinstance(v, dict):
      print_dict(v, tabs+1)
    else:
      print '  '*(tabs+1), v

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
  min_frequency = 0.1
  max_frequency = 1.0
  if predicted_time <= 0:
    return min_frequency
  if desired_frequency > max_frequency:
    return max_frequency
  if desired_frequency < min_frequency:
    return min_frequency
  return desired_frequency

def scale_frequency_new(predicted_time, deadline, dvfs_function, freq_levels):
  """
  Find the minimum frequency in freq_levels such that the execution time will
  be less than the deadline. predicted_time is the execution time at
  max(freq_levels) and dvfs_function describes how execution time varies with
  different frequencies.
  """
  for f in freq_levels:
    if dvfs_function(predicted_time, f) < deadline:
      return f
  return f

def policy_average(train_times, train_metrics=None, test_times=None, test_metrics=None, window_size=10):
  """
  Use the average of the last [window_size] frames as the predicted time for the next frame.
  """
  # Training is done online with test set
  if test_times:
    times = test_times
  else:
    times = train_times

  predicted_times = list(times)
  for i in range(window_size, len(times)):
    window = times[i-window_size:i]
    predicted_times[i] = average(window)
  return predicted_times

def policy_worstcase(train_times, train_metrics=None, test_times=None, test_metrics=None):
  """
  Predict all frames as the worst-case time seen. This is a conservative
  prediction that should ensure no deadline misses.
  """
  # If no test set, then train and test on same set
  if not test_times:
    test_times = train_times

  worstcase = max(train_times)
  return [worstcase]*len(train_times)

def policy_tuned_pid(train_times, train_metrics=None, test_times=None, test_metrics=None):
  """
  Tune PID weights using train set, then run PID controller to predict times on
  test set. Tuning attempts to minimize deadline misses. If another controller
  is found with the same deadline misses, then energy is minimized.
  """
  # If no test set, then train and test on same set
  if not test_times:
    test_times = train_times

  PIDs = [[0.05*i for i in range(1, 40)], [0.1*i for i in range(25)], [0.05*i for i in range(10)]]
  margin = 1.1
  deadline = max(train_times)
  best_misses = 1
  best_energy = 1
  best_PID = None
  for (P, I, D) in itertools.product(*PIDs):
    predict_times = policy_pid(train_times, P=P, I=I, D=D)
    # Skip non-stable controllers
    if max(predict_times) > 10*deadline:
      continue
    # Heuristic to skip non-stable/marginally stable controllers
    # If average difference in predicted times is greater than standard deviation, filter out
    diffs = 0
    for i in range(1, len(predict_times)):
      diffs += abs(predict_times[i] - predict_times[i-1])
    if float(diffs)/(len(predict_times) - 1) > numpy.std(predict_times):
      continue

    frequencies = [scale_frequency_perfect(margin*t, deadline) for t in predict_times]
    result_times = [dvfs_time(t, f) for (t, f) in zip(train_times, frequencies)]
    misses = deadline_misses(result_times, train_times, frequencies, deadline) 
    # If better on deadline misses
    if misses < best_misses:
      best_misses = misses
      best_energy = energy(result_times, train_times, frequencies, deadline)
      best_PID = (P, I, D)
    # If equal on misses, check if better on energy
    elif misses == best_misses:
      e = energy(result_times, train_times, frequencies, deadline)
      if e < best_energy:
        best_misses = misses
        best_energy = energy(result_times, train_times, frequencies, deadline)
        best_PID = (P, I, D)
  #best_PID = (1, 0.5, 0.01)

  # Write PID parameters out to file
  f = open("temp.lps", 'w')
  f.write("P = %f\n" % best_PID[0])
  f.write("I = %f\n" % best_PID[1])
  f.write("D = %f\n" % best_PID[2])
  f.close()

  # Use parameters to run PID on test set
  return policy_pid(test_times, P=best_PID[0], I=best_PID[1], D=best_PID[2])

def policy_pid(times, P=1, I=0.5, D=0.01, metrics=None):
  """
  PID controller for predicting execution times.
  """
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

def policy_pid_timeliness(train_times, train_metrics=None, test_times=None, test_metrics=None):
  # Training is done online with test set
  if test_times:
    times = test_times
  else:
    times = train_times

  # Found for continuous DVFS
  return policy_pid(times, P=0.40, I=0, D=0.03)

  # With discrete levels of [0.25, 0.50, 0.75, 1.00]
  #return policy_pid(times, P=1.80, I=1.80, D=0.83)

  #default_dvfs_levels = [0.05, 0.1, 0.15, 0.2, 0.25, 0.50, 0.75, 1.00]
  #return policy_pid(times, P=0.40, I=0.00, D=0.21)

  #default_dvfs_levels = [.1*x for x in range(1, 11)]
  #return policy_pid(times, P=0.60, I=0.20, D=0.02)

def policy_pid_energy(train_times, train_metrics=None, test_times=None, test_metrics=None):
  # Training is done online with test set
  if test_times:
    times = test_times
  else:
    times = train_times

  # continuous DVFS and discrete levels of [0.25, 0.50, 0.75, 1.00]
  return policy_pid(times, P=0.20, I=0, D=0)

  #default_dvfs_levels = [0.05, 0.1, 0.15, 0.2, 0.25, 0.50, 0.75, 1.00]
  #return policy_pid(times, P=0.20, I=0.00, D=0.14)

  #default_dvfs_levels = [.1*x for x in range(1, 11)]
  #return policy_pid(times, P=0.20, I=0.00, D=0.35)

def policy_least_squares(train_times, train_metrics, test_times=None, test_metrics=None):
  """
  Prediction based on linear least-squares regression.
  """
  if not test_times:
    test_times = train_times
    test_metrics = train_metrics

  y = numpy.array([train_times])
  x = numpy.array(train_metrics)
  coeffs = regression(y, x)

  f = open("temp.lps", 'w')
  f.write(', '.join([str(x[0]) for x in coeffs]))
  f.close()

  predicted_times = [0]*len(test_times)
  for i in range(len(test_times)):
    x = [1] + test_metrics[i]
    predicted_times[i] = numpy.dot(x, coeffs)[0]
  return predicted_times

def policy_conservative(train_times, train_metrics, test_times=None, test_metrics=None):
  """
  This predictor constructs a LP problem to solve for a linear model mapping
  metrics to execution times. The LP problem includes constraints so that the
  solution is always conservative (i.e., predicted times >= observed times).
  """
  # If no test times, test and train on same set
  if not test_times:
    test_times = train_times
    test_metrics = train_metrics
    
  # Define residuals (errors)
  residuals = [""] * len(train_times)
  for i in range(len(train_times)):
    residuals[i] = "r%d = " % (i)
    for b in range(len(train_metrics[i])):
      residuals[i] += "%d b%d + " % (train_metrics[i][b], b)
    residuals[i] += "b - %d;" % (train_times[i])
  # Allow coeffs to be negative
  free_vars = [""] * len(train_metrics[0])
  for b in range(len(train_metrics[i])):
    free_vars[b] = "free b%d;" % (b)
  free_vars.append("free b;")

  # Force underprediction
  underprediction = [""] * len(residuals)
  for r in range(len(residuals)):
    underprediction[r] = "r%d >= 0;" % (r)

  # Optimization objective
  optimize = "min: "
  for r in range(len(residuals)):
    optimize += "r%d + " % (r)
  optimize += "0;"

  # Full LP formulation
  lp = optimize + "\n"
  lp += '\n'.join(residuals) + '\n'
  lp += '\n'.join(underprediction) + '\n'
  lp += '\n'.join(free_vars)

  # Write to file and solve
  out_file = open("temp.lp", 'w')
  out_file.write(lp)
  out_file.close()
  os.system("lp_solve -s5 temp.lp > temp.lps")
  # Get coefficients
  solve_file = open("temp.lps", 'r')
  coeffs = []
  for line in solve_file:
    if line[0] == 'b':
      coeffs.append(float(line.split()[1]))

  # Predict times
  predicted_times = [0]*len(test_times)
  for i in range(len(test_times)):
    x = test_metrics[i] + [1]
    predicted_times[i] = numpy.dot(x, coeffs)
  return predicted_times

def policy_lasso(train_times, train_metrics, test_times=None, test_metrics=None):
  """
  LASSO-based regression.
  """

  # Fit Lasso model based on training set
  y = numpy.array([train_times]).transpose()
  x = numpy.array(train_metrics)
  clf = sklearn.linear_model.Lasso()
  clf.fit(x, y)
  # Write model out
  f = open("temp.lps", "w")
  f.write(str(clf.intercept_[0]))
  f.write(", ")
  f.write(', '.join(map(str, clf.coef_)))
  f.close()

  # Apply prediction to test set
  predicted_times = clf.predict(test_metrics)
  return predicted_times

def policy_cvx(train_times, train_metrics, test_times, test_metrics, obj, constraints=[]):
  """
  Core for doing CVX optimization. obj and constraints are strings defining the
  optimization objective and constraints to be used. y are the execution times,
  X is the matrix of features, and b is the coefficients.
  """
  if not test_times:
    test_times = train_times
    test_metrics = train_metrics

  # Problem data
  # Add constant term
  train_metrics = [[1] + x for x in train_metrics]
  X = numpy.array(train_metrics)
  y = numpy.array(train_times)

  # Construct the problem
  b = cvxpy.Variable(X.shape[1])
  for i in range(len(constraints)):
    constraints[i] = eval(constraints[i])
  obj = eval(obj)
  prob = cvxpy.Problem(obj, constraints)

  # Solve
  prob.solve()
  if prob.status != cvxpy.OPTIMAL:
    print "Problem not solved optimally: ", prob.status
    print "  Trying with CVXOPT..."
    prob.solve(solver=cvxpy.SCS)
    print "  Result: ", prob.status
  # Write out results
  coeffs = numpy.array(b.value)
  f = open("temp.lps", 'w')
  f.write("b\t\t" + str(coeffs[0][0]) + "\n")
  f.write('\n'.join(["b%d\t\t" % (i) + str(x[0]) for (i, x) in enumerate(coeffs[1:])]))
  #f.write('\n'.join([str(x[0]) for x in coeffs]))
  f.close()

  # Perform prediction
  predicted_times = [0]*len(test_times)
  for i in range(len(test_times)):
    x = [1] + test_metrics[i]
    predicted_times[i] = numpy.dot(x, coeffs)[0]
  return predicted_times

def policy_cvx_conservative_lasso(train_times, train_metrics, test_times=None, test_metrics=None, underpredict_penalty=100, lasso_weight=0):
  """
  Penalty function is sum of underpredict difference, overpredict difference,
  and L1-norm of coefficients. underpredict_penalty gives a weight to
  underprediction penalty and lasso_weight gives a weight to L1-norm of
  coefficients (excluding b[0] which is the constant offset).
  """
  if not test_times:
    test_times = train_times
    test_metrics = train_metrics

  constraints = []
  #for i in range(len(train_times)):
  #  constraints.append("(X*b - y)[%d] >=0" % (i))
  obj = "cvxpy.Minimize(cvxpy.sum_entries(cvxpy.scalene(X*b - y, 1, %d)) + %d*cvxpy.norm(b[1:], 1))" % (underpredict_penalty, lasso_weight)
  return policy_cvx(train_times, train_metrics, test_times, test_metrics, obj, constraints)

def policy_oracle(train_times, train_metrics=None, test_times=None, test_metrics=None):
  """
  Perfect prediction. Returns times as predicted times.
  """
  if test_times:
    return test_times
  else:
    return train_times

def deadline_misses(times, baseline_times, frequencies, deadline):
  misses = [(1 if x > deadline else 0) for x in times]
  return float(sum(misses))/len(times)

def tardiness(times, baseline_times, frequencies, deadline):
  tardiness = [max(x - deadline, 0) for x in times]
  return (float(sum(tardiness))/len(times), max(tardiness))

def normalized_tardiness(times, baseline_times, frequencies, deadline):
  tardiness = [float(max(x - deadline, 0))/deadline for x in times]
  return (sum(tardiness)/len(times), max(tardiness))
def avg_normalized_tardiness(times, baseline_times, frequencies, deadline):
  return normalized_tardiness(times, baseline_times, frequencies, deadline)[0]
def max_normalized_tardiness(times, baseline_times, frequencies, deadline):
  return normalized_tardiness(times, baseline_times, frequencies, deadline)[1]

def energy(times, baseline_times, frequencies, deadline):
  energies = [f*f*float(t)/bt for (f, t, bt) in zip(frequencies, times, baseline_times)]
  #energies = [f*f for (f, t, bt) in zip(frequencies, times, baseline_times)]
  return average(energies)

def read_predict_file(filename):
  """
  Return list of predicted times from filename.
  """
  f = open(filename, 'r')
  data = []
  for line in f:
    data.append(float(line))
  f.close()
  return data

def get_nonzero_coeffs(filename, benchmark):
  benchmark_found = False
  f = open(filename, 'r')
  for line in f:
    if benchmark_found:
      if "non-zero coeffs" in line:
        coeffs_string = line
        break
    elif benchmark in line:
      benchmark_found = True
  f.close()
  coeffs_array = coeffs_string.split('[')[1].split(']')[0]
  coeffs = map(int, coeffs_array.split(','))
  return coeffs
