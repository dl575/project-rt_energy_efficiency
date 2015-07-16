#!/usr/bin/env python

import sys
sys.path.extend(['..'])
from parse_lib import *
from dvfs_sim_lib import *

margin = 1.1
little_active_power = 0.306
little_idle_power = 0.139
big_active_power = 0.921
big_idle_power = 0.363
little_power_factor = little_active_power / big_active_power
big_power_factor = 1.0
little_idle_power_factor = little_idle_power / little_active_power
big_idle_power_factor = big_idle_power / big_active_power
big_speedup_factor = 2.06
switching_time = 0
dvfs_levels_little = [ 1.0 * x for x in range(2, 15) ]
dvfs_levels_big = [ 1.0 * x for x in range(2, 21) ]

def find_best_frequency(predicted_time, dvfs_levels, dvfs_function, deadline, switch_core):
    for f in dvfs_levels:
        time = dvfs_function(predicted_time, f, switch_core)
        if time <= deadline:
            return f

    return max(dvfs_levels)


def energy(frequency, power_factor, time):
    """
    E = P*t
      = 1/2CVAf^2 * t
    With V = bf,
    E = cf^3 * t
    where c is some scaling factor
    """
    return power_factor * frequency ** 3 * time


def total_energy(frequency, power_factor, idle_power_factor, time, deadline):
    """
    Total energy = active energy + idle energy
    """
    return energy(frequency, power_factor, time)


def run_dvfs_hetero(metric, times, predict_times, deadline, biglittle, policy, normalize_big):
    """
    Run DVFS simulation.
    """
    energies = []
    deadline_misses = 0
    little_count = 0
    big_count = 0
    switch_count = 0
    big_core = 'big' in biglittle
    little_core = 'little' in biglittle
    if not big_core:
        assert little_core
    if big_core:
        last_core = 'big'
    else:
        last_core = 'little'
    for pt, t in zip(predict_times, times):
        pt = margin * pt
        found = False
        if little_core and big_core:
            little_f = find_best_frequency(pt, dvfs_levels_little, dvfs_function_little, deadline, last_core == 'big')
            big_f = find_best_frequency(pt, dvfs_levels_big, dvfs_function_big, deadline, last_core == 'little')
            little_actual_time = dvfs_function_little(t, little_f, last_core == 'big')
            big_actual_time = dvfs_function_big(t, big_f, last_core == 'little')
            little_energy = total_energy(little_f, little_power_factor, little_idle_power_factor, little_actual_time, deadline)
            big_energy = total_energy(big_f, big_power_factor, big_idle_power_factor, big_actual_time, deadline)
            if little_energy <= big_energy and little_actual_time <= deadline:
                energies.append(little_energy)
                deadline_misses += 1 if little_actual_time > deadline else 0
                switch_count += 1 if last_core == 'big' else 0
                last_core = 'little'
                little_count += 1
            else:
                energies.append(big_energy)
                deadline_misses += 1 if big_actual_time > deadline else 0
                switch_count += 1 if last_core == 'little' else 0
                last_core = 'big'
                big_count += 1
        elif little_core:
            little_f = find_best_frequency(pt, dvfs_levels_little, dvfs_function_little, deadline, False)
            actual_time = dvfs_function_little(t, little_f, False)
            energies.append(total_energy(little_f, little_power_factor, little_idle_power_factor, actual_time, deadline))
            if actual_time > deadline:
                deadline_misses += 1
            last_core = 'little'
            little_count += 1
        elif big_core:
            big_f = find_best_frequency(pt, dvfs_levels_big, dvfs_function_big, deadline, False)
            actual_time = dvfs_function_big(t, big_f, False)
            energies.append(total_energy(big_f, big_power_factor, big_idle_power_factor, actual_time, deadline))
            if actual_time > deadline:
                deadline_misses += 1
            last_core = 'big'
            big_count += 1

    if metric == 'energy':
        if normalize_big:
            max_energy = sum([ total_energy(max(dvfs_levels_big), big_power_factor, big_idle_power_factor, t, deadline) for t in times ])
        else:
            max_energy = sum([ total_energy(max(dvfs_levels_little), little_power_factor, little_idle_power_factor, t, deadline) for t in times ])
        return sum(energies) / max_energy
    elif metric == 'deadline_misses':
        return float(deadline_misses) / len(times)
    elif metric == 'switch_count':
        return float(switch_count) / len(times)
    elif metric == "big_count":
        assert metric == 'big_count' and big_count + little_count == len(times)
        return float(big_count) / len(times)
    else:
        raise Exception('Unknown metrics: %s' % metric)


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print 'usage: run_dvfs.py big|little [--no_test]'
        exit(1)
    if sys.argv[1] == 'big':
        bigonly = True
        littleonly = False
    elif sys.argv[1] == 'little':
        bigonly = False
        littleonly = True
    else:
        print 'usage: run_dvfs.py big|little [--no_test]'
        exit(1)
    assert bigonly != littleonly
    no_test = False
    if '--no_test' in sys.argv:
        no_test = True
    if bigonly:
        input_dir = '../data_big'
        output_dir = 'predict_times_big'
        dvfs_function_little = lambda t, f, switch: float(t) * max(dvfs_levels_big) / f * big_speedup_factor + (switching_time if switch else 0)
        dvfs_function_big = lambda t, f, switch: float(t) * max(dvfs_levels_big) / f + (switching_time if switch else 0)
    elif littleonly:
        input_dir = '../data_little'
        output_dir = 'predict_times_little'
        dvfs_function_little = lambda t, f, switch: float(t) * max(dvfs_levels_little) / f + (switching_time if switch else 0)
        dvfs_function_big = lambda t, f, switch: float(t) * max(dvfs_levels_little) / f * (1 / big_speedup_factor) + (switching_time if switch else 0)
    policies = []
    for root, dirname, filenames in os.walk(output_dir):
        for filename in filenames:
            policy = filename.split('-')[0]
            if policy not in policies:
                policies.append(policy)

    for metric in ['energy',
     'deadline_misses',
     'switch_count',
     'big_count']:
        if isinstance(metric, str):
            print metric
        else:
            print metric.__name__
        print list_to_csv([''] + benchmarks + ['average'])
        if bigonly:
            biglittle_configurations = ['bigonly-100', 'biglittle-100']
        elif littleonly:
            biglittle_configurations = ['littleonly-60',
             'littleonly-80',
             'littleonly-100',
             'biglittle-60',
             'biglittle-80',
             'biglittle-100']
        for biglittle in biglittle_configurations:
            for policy in policies:
                print '%s-%s' % (policy, biglittle), ',',
                sum_metric = 0
                for benchmark in benchmarks:
                    if no_test:
                        times = parse_execution_times('%s/%s/%s0.txt' % (input_dir, benchmark, benchmark))
                    else:
                        times = parse_execution_times('%s/%s/%s1.txt' % (input_dir, benchmark, benchmark))
                    predict_times = read_predict_file('%s/%s-%s.txt' % (output_dir, policy, benchmark))
                    scaling = float(biglittle.split('-')[1]) / 100
                    deadline = scaling * max(times)
                    metric_result = run_dvfs_hetero(metric, times, predict_times, deadline, biglittle, policy, bigonly)
                    sum_metric += metric_result
                    print metric_result, ',',

                print sum_metric / len(benchmarks)

        print
