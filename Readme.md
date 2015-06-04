# Prediction-Guided Performance-Energy Trade-offs

This repository contains code related to experiments for our work on using
execution time prediction to improve energy efficiency in soft real-time (e.g.,
interactive) systems.

Directory structure:
- benchmarks: Programs that have been modified to perform execution time prediction and instrumented for experiments.
- datasets: Datasets used for benchmark programs, including generation scripts.
- dvfs_sim: Code for training prediction model and analytical estimates of DVFS energy savings.
- lib: Common library code.
- power_monitor: Utility for measuring power on ODRIOD board.
- pycparser: Scripts to perform slicing based on pycparser Python library.
- tar: Original (unmodified) tarballs of code from other projects that were used.
