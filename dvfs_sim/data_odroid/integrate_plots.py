#!/usr/bin/python
import os
import sys
sys.path.extend(['../'])
import tsg_plot
import math
sys.path.extend(['/home/odroid/project-rt_energy_efficiency/lib'])
import parse_lib
import matplotlib.pyplot as plt

# Set up plotting options
opts = tsg_plot.PlotOptions()

def plot(data, labels, idx):
    opts.labels = labels

    attribute_dict = \
        {
            'bar_width' : 0.7,
            'figsize' : (7.0, 2),
            'fontsize' : 4,
            'labels_fontsize' : 4,
            'colors' : ["#a6cee3", "#1f78b4", "#b2df8a", "#33a02c", "#fb9a99", "#e31a1c", "#ff7f00", "#cab2d6", "#6a3d9a", "#ffff99", "#b15928", "#fdbf6f"],
            'paper_mode' : True,
            'show' : False,
            'paper_mode' : True,
            'legend_ncol' : 2,
            'rotate_labels' : True,
            'rotate_labels_angle' : -45,
            'num_rows' : 1,
            'num_cols' : 3
        }

    opts.xlabel = "hetero"
    if idx == 0 :
        opts.ylabel = "energy [%]"
        opts.legend_enabled = True
    elif idx == 1 :
        opts.ylabel = "deadline_misses [%]"
        opts.legend_enabled = False
        opts.file_name = "integrated_"+benchmark+".pdf"
    opts.data = data
    for name, value in attribute_dict.iteritems():
        setattr( opts, name, value )

    # Plot
    tsg_plot.add_plot( opts )

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "usage: integrate_plots.py benchmark [base_dir]"
        exit(0)
    else:
        benchmark=sys.argv[1]
        if len(sys.argv) >= 3:
            cur_path=sys.argv[2]
        else: 
            cur_path=os.getcwd()

    cmd=['./plot_energy.py big '+benchmark, './plot_energy.py little '+benchmark, './plot_energy_hetero.py hetero '+benchmark]
    big_little=['big', 'little', 'hetero']
    for n_cmd in xrange(0, len(cmd)):
        #execute plot scripts
        os.system(cmd[n_cmd]);

    for idx in range(2):
        line0 = []
        line1 = []

        for n_cmd in xrange(0, len(cmd)):
            f=open('_parsed_data'+str(idx)+'_'+big_little[n_cmd]+'.csv', 'r')

            first_line = []
            first_line = f.readline().strip().split(',')[1:]

            for i in xrange(0, len(first_line)): 
                line0.append(str(first_line[i])+'-'+str(big_little[n_cmd]))
            
            data_headers = []
            data = []
            for line in f:
                ls = line.split(',')
                data_headers.append(ls[0])
                data.append([float(x) for x in ls[1:]])
            f.close()
            labels = [data_headers, first_line]
            
            for i in xrange(0, len(data_headers)): 
                line1.append([])
            for i in xrange(0, len(data_headers)): 
                for j in xrange(0, len(first_line)): 
                    line1[i].append(data[i][j])
            
        #extra lists are added into line1 (need to solve later)
        print len(data_headers)*len(first_line)
        for i in xrange(0, len(data_headers)*len(first_line)): 
            line1.pop()
        print line1

        # Parse data and write to csv file
        f=open('integrated_parsed_data'+str(idx)+'.csv', 'w')
        num_governors = len(line0) / len(cmd)
        k = -1
        for i in xrange(0, len(line0)):
            if(i % len(cmd) == 0):
                k += 1
            f.write(',' + line0[(i%len(cmd)) * num_governors + k])
        f.write('\n')

        for j in xrange(0, len(line1)):
            norm_energy = 0.0
            f.write(data_headers[j])
            k = -1
            for i in xrange(0, len(line1[j])):
                if(i % len(cmd) == 0):
                    k += 1
                if idx == 0: #energy
                    if i == 0:
                        norm_energy = line1[j][(i%len(cmd)) * num_governors + k]
                    f.write(',' + str(line1[j][(i%len(cmd)) * num_governors + k]*100/norm_energy))
                elif idx == 1: #deadline misses
                    f.write(',' + str(line1[j][(i%len(cmd)) * num_governors + k]))
            f.write('\n')

        f.close()
    
    # Read csv files and plots
    for idx in range(2):
        f=open('integrated_parsed_data'+str(idx)+'.csv', 'r')
        _subcat = []
        _subcat = f.readline().strip().split(',')[1:]
        _cat = []
        _data = []
        for line in f:
            _ls = line.split(',')
            _cat.append(_ls[0])
            _data.append([float(x) for x in _ls[1:]])
        f.close()
        _labels = [_cat, _subcat]
        
        # Plot results
        plot(_data, _labels, idx)


