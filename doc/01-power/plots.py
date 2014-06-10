#!/usr/bin/python

from parse_lib import *

# OMAP 3530
voltages = [1.35, 1.27, 1.20, 1.06, 0.985]
frequencies = [720, 550, 500, 250, 125]
# Intel Pentium M
# voltages = [1.484, 1.420, 1.276, 1.164, 1.036, 0.956]
# frequencies = [1600, 1400, 1200, 1000, 800, 600]

plot_set_fonts()
plot_scatter(voltages, frequencies, figsize=figsize_singlecol, xlabel="Voltage [V]", ylabel="Frequency [MHz]")
plot_to_pdf("dvfs.pdf")

v = voltages
f = frequencies
energies = [v[i]*v[i]/(v[0]*v[0]) for i in range(len(voltages))]
print "Energy: ", energies
times = [float(f[0])/f[i] for i in range(len(frequencies))]
print "Time: ", times

fig, ax1 = pylab.subplots(figsize=figsize_singlecol)
ax1.plot(v, times, 'ro-')
pylab.ylabel("Normalized Execution Time")
pylab.legend(["Execution Time"], "upper left")

ax2 = ax1.twinx()
ax2.plot(v, energies, 'bo-')
pylab.xlabel("Voltage [V]")
pylab.ylabel("Normalized Energy")
pylab.ylim(0.4, 1.2)
pylab.legend(["Energy"], "upper right")

plot_to_pdf("dvfs_energy.pdf")
