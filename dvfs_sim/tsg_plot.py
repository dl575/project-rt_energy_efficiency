#! /usr/bin/env python
"""
Library for plotting.

Classes:
  PlotOptions
    Object passed to plotting function that defines options.

Functions:
  add_plot( opt )
  add_scatter_plot( ax, opt )
  add_bar_plot( ax, opt )
  set_legend( ax, opt, legend, legend_labels )
  set_common( ax, opt )
  main()
"""

import matplotlib
import numpy as np
import matplotlib.pyplot as plt
import random
import sys


class PlotOptions:

  def __init__( self ):

    # default values
    self.title = "test bar plot"
    self.ylabel = None
    self.xlabel = None
    self.bar_width = 0.70

    self.valid_plot_types = ["bar", "stacked_bar", "scatter_bar", "scatter"]
    self.plot_type = "bar"

    # Error bars, lists of 2 lists. The first contains the minimums to draw and the second contains the maximums to draw.
    self.errorbars = None

    # if true, draws an arrow from the earlier to later
    self.scatter_bar_arrow = False
    # only draw arrow head if the length is this long
    self.scatter_bar_arrow_minlen = 0.2

    # Can provide multiple file names seperated by spaces
    self.file_name = ""
    # option to rotate labels when there are many of them
    self.rotate_labels = False
    self.rotate_labels_angle = -45

    # set the font size for the rest of things
    self.fontsize = 12

    # set the font size for the labels
    self.labels_fontsize = 12

    # used to override default y range. you can provide the range in the
    # form of [min, max]
    self.yrange = None
    # Override default x range.
    self.xrange = None

    # if not None, normalize line draws a horizontal line at indicated y
    self.normalize_line = None

    self.fig = None
    # this is useful for drawing multiple plots in a page
    self.plot_idx = 1
    self.num_cols = 1
    self.num_rows = 1

    # if non-zero, shares axis with the previous plot. 1 indicates x axis
    # sharing, 2 indicates y axis sharing
    self.share_axis = 0

    # if true, just show the plot, don't save
    self.show = True

    # paper mode squishes everything
    self.paper_mode = False

    self.legend_enabled = True
    # the following are for fine tweaking of legend in paper mode
    self.legend_ncol = 1
    # Position of legend (x, y, width, height)
    self.legend_bbox = (0., 1.05, 1., 0.1)
    # Spacing and sizing of text/handles
    self.legend_columnspacing = None
    self.legend_handlelength = None
    self.legend_handletextpad = None


    self.figsize = (8, 6) # (width, height) in inches

    self.data = [ [1,3], [2,2], [3,1], [4,5], [1,3] ]

    # Individually label each point in scatter plot. Labels are
    # specified in self.labels[0]
    self.labels_enabled = False
    self.labels_x_off = 2
    self.labels_y_off = 2
    #self.labels = [ ["aa", "ab", "ac", "sfa", "asda"],
    #                [ "a", "b"] ]
    self.labels = None

    #self.colors = [ 'r', 'b', 'g', 'y', 'c', 'm', 'k', 'w' ]
    # Selected from colobrewer2.org for 9 categories, qualitative, print friendly
    self.colors = ['#e41a1c', '#377eb8', '#4daf4a', '#984ea3', '#ff7f00', '#ffff33', '#a65628', '#f781bf', '#999999']
    self.hatch = None
    self.symbols = [ 'o', 'd', '^', 's', 'p', '*', 'x' ]

    random.seed( 0xdeadbeef )

  def get_color( self, idx ):
    # get a color from colors array if idx is small, otherwise, gets a
    # random color
    if idx < len( self.colors ):
      return self.colors[idx]
    else:
      return "#{:06x}".format( random.randint( 0, 0xffffff ) )


def add_plot( opt ):
  """ Function to create a bar plot. """
  # Check for valid plot type
  if opt.plot_type not in opt.valid_plot_types:
    print "Unrecognized plot type: %s" % opt.plot_type
    sys.exit(1)

  if opt.paper_mode:
    # use a sans-serif font
    #plt.rcParams['pdf.use14corefonts'] = True
    plt.rcParams['font.size'] = 8
    plt.rcParams['font.family'] = 'sans-serif'
    plt.rcParams['font.sans-serif'] = ['Arial']

  if opt.fig == None:
    opt.fig = plt.figure( figsize=opt.figsize )

  if opt.share_axis == 1:
    # Share x-axis
    ax = opt.fig.add_subplot( opt.num_rows, opt.num_cols, opt.plot_idx, \
                              sharex=opt.ax )
  elif opt.share_axis == 2:
    # Share y-axis
    ax = opt.fig.add_subplot( opt.num_rows, opt.num_cols, opt.plot_idx, \
                              sharey=opt.ax )
  else:
    ax = opt.fig.add_subplot( opt.num_rows, opt.num_cols, opt.plot_idx )

  if opt.plot_type == "scatter":
    add_scatter_plot( ax, opt )
  else:
    add_bar_plot( ax, opt )
  opt.plot_idx += 1

  # move gridlines behind plot
  ax.set_axisbelow( True )

  opt.ax = ax

  if opt.paper_mode:
    plt.tight_layout()

  if opt.file_name != None and opt.file_name != "":
    for file in opt.file_name.split():
      print "saving", file
      if opt.paper_mode:
        plt.savefig( file, bbox_inches="tight" )
      else:
        plt.savefig( file )
  elif opt.show:
    plt.show()

def add_scatter_plot( ax, opt ):

  # for scatter plots, the normalize lines are a tuple for x, y
  if opt.normalize_line is not None:
    x, y = opt.normalize_line[:2]
    if x is not None:
      ax.axvline( x=x, zorder=1, color='k' )
    if y is not None:
      ax.axhline( y=y, zorder=1, color='k' )

    # more args to normalize line are arbitrary lines of format
    # (x1, y1, x2, y2)
    for line in opt.normalize_line[2:]:
      x1, y1, x2, y2 = line
      ax.plot( [x1, x2], [y1, y2], 'k-', lw=0.5, zorder=1 )

  scatters = []
  for i in xrange( len( opt.data ) ):
    c = opt.data[ i ]
    # we get x's and y's
    x, y = zip( *c )

    # zorder specifies the ordering of elements, the higher the more
    # visible
    scatters.append( ax.scatter( x, y, \
                               marker=opt.symbols[i], \
                               color=opt.get_color(i), \
                               zorder=5+i ) )

    if opt.scatter_bar_arrow and i > 1:
      for j in xrange( len( x ) ):
        start = ( opt.data[i-1][j][0], opt.data[i-1][j][1] )
        diff =  ( x[j] - start[0], y[j] - start[1] )

        # we only draw arrow if the diff is above minlen
        if abs( ( diff[0]**2 + diff[1]**2 )**0.5 ) \
                                > opt.scatter_bar_arrow_minlen:
          # temporarily disabling arrow head
          #head_width = 0.06
          #head_length = 0.06
          head_width = 0.00
          head_length = 0.00
        else:
          head_width = 0.0
          head_length = 0.0

        ax.arrow( start[0], start[1], \
                  diff[0], diff[1], shape="full", length_includes_head=True, \
                  head_width=head_width, head_length=head_length, \
                  lw=0.5, overhang=0.0, zorder=2, \
                  color="#808080")

    if opt.labels_enabled: #and i > 0:
      for j in xrange( len( x ) ):
        label = opt.labels[0][i][j]
        ax.annotate( label, xy=(x[j], y[j]), \
                     xytext=( opt.labels_x_off, opt.labels_y_off ), \
                     textcoords='offset points', \
                     #arrowprops=dict(arrowstyle="->", \
                     #                connectionstyle="arc3,rad=.2")
                   )

  if opt.yrange is not None:
    ax.set_ylim( opt.yrange )
  if opt.xrange is not None:
    ax.set_xlim( opt.xrange )

  legend = []
  legend_labels = []

  # if no labels specified, create blank entries to generate legend
  if not opt.labels:
    opt.labels = [[], [" "]*len(scatters)]
  for i in xrange( len( scatters ) ):
    if opt.labels[1][i] is not None and opt.labels[1][i] != "":
      legend.append( scatters[i] )
      legend_labels.append( opt.labels[1][i] )

  set_legend( ax, opt, legend, legend_labels )
  set_common( ax, opt )

  ax.xaxis.grid(True)
  ax.yaxis.grid(True)


def add_bar_plot( ax, opt ):

  # Determine number of categories based on labels
  #num_cat = len( opt.labels[0] )
  #num_subcat = len( opt.labels[1] )
  num_cat = len(opt.data)
  num_subcat = len(opt.data[0])

  ind = np.arange( num_cat )
  if opt.plot_type == "stacked_bar":
    width = opt.bar_width
  else:
    width = opt.bar_width / num_subcat
  ind = ind + (1-opt.bar_width)/2

  indexes = []
  bar_data = []

  for s in xrange( num_subcat ):
    if opt.plot_type == "stacked_bar":
      # stacked use the same x values
      indexes.append( ind )
    elif opt.plot_type == "scatter_bar":
      # for scatter bar, we need to put the point right at the center
      indexes.append( ind + opt.bar_width/2. )
    else:
      indexes.append( ind + s * width )

    tmp_list = []
    for c in xrange( num_cat ):
      tmp_list.append( opt.data[c][s] )

    bar_data.append( np.array( tmp_list ) )

  bars = []

  bottom = np.array( [0.0] * num_cat )

  # use narrower line widths for bars on paper mode
  bar_linewidth = 0.5 if opt.paper_mode else None
  for i in xrange( num_subcat ):
    if opt.plot_type == "scatter_bar":
      # bars are actually scatters
      bars.append( ax.scatter( indexes[i], bar_data[i], \
                               marker=opt.symbols[i], \
                               color=opt.get_color(i), \
                               s=40, zorder=i+5 ) )
      # we draw to this if arrows are allowed and the index is larger than
      # 0
      if opt.scatter_bar_arrow and i > 0:
        for j in xrange( len( indexes[i] ) ):
          start = ( indexes[i-1][j], bar_data[i-1][j] )
          ydiff = bar_data[i][j] - bar_data[i-1][j]
          if ydiff == 0:
            continue

          # we only draw arrow if the diff is above minlen
          if abs(ydiff) > opt.scatter_bar_arrow_minlen:
            # temporarily disabled arrowhead
            #head_width = 0.3
            #head_length = 0.1
            head_width = 0.0
            head_length = 0.0
          else:
            head_width = 0.0
            head_length = 0.0

          ax.arrow( start[0], start[1], \
                    0, ydiff, shape="full", length_includes_head=True, \
                    head_width=head_width, head_length=head_length, \
                    zorder=2, \
                    color='k')

    else:
      if opt.hatch:
        bars.append( ax.bar( indexes[i], bar_data[i], width, \
                             color=opt.get_color(i), \
                             linewidth=bar_linewidth, \
                             bottom=bottom,
                             hatch = opt.hatch[i]
                            ) )
      else:
        bars.append( ax.bar( indexes[i], bar_data[i], width, \
                             color=opt.get_color(i), \
                             linewidth=bar_linewidth, \
                             bottom=bottom
                            ) )

    if opt.plot_type == "stacked_bar":
      bottom += bar_data[i]

  if opt.errorbars:
    min_errors = np.array(opt.errorbars[0]).transpose()
    max_errors = np.array(opt.errorbars[1]).transpose()
    for i in range(len(indexes)):
      # Calculate center point of error bar
      error_indexes = [x + width/2.0 for x in indexes[i]]
      # Plot error bars
      plt.errorbar(error_indexes, bar_data[i], yerr=[min_errors[i], max_errors[i]], fmt='.', markersize=0, color='k')

  # we force that there is no empty space to the right
  ax.set_xlim( (0.0, 0.0 + num_cat ) )

  # scatter bar requires x axis grids to help viewers
  if opt.plot_type == "scatter_bar":
    ax.xaxis.grid(True)


  ax.set_xticks( ind + opt.bar_width/2. )
  ax.tick_params( labelsize=opt.fontsize )
  if opt.labels:
    if opt.rotate_labels:
      ax.set_xticklabels( opt.labels[0], \
                          verticalalignment="top", \
                          y=0.01, \
                          horizontalalignment="left" \
                                  if opt.rotate_labels_angle > 0 \
                                  else "right", \
                          rotation=-opt.rotate_labels_angle, \
                          fontsize=opt.labels_fontsize )
    else:
      ax.set_xticklabels( opt.labels[0], \
                          verticalalignment="top", \
                          y=0.01, \
                          fontsize=opt.labels_fontsize )

  # if yrange is specified, we set the value
  if opt.yrange is not None:
    ax.set_ylim( opt.yrange )

  # draw the normalization line
  if opt.normalize_line is not None:
    ax.axhline( y=opt.normalize_line, color='k' )

  # Add the legend stuff.
  if opt.labels:
    if num_subcat > 1:
      legend_labels = opt.labels[1]
      if opt.plot_type == "scatter_bar":
        # scatters just use the scatters
        legend_bars   = bars
      else:
        # bars use the first element
        legend_bars   = map( lambda x: x[0], bars )
      set_legend( ax, opt, legend_bars, legend_labels )

  set_common( ax, opt )

def set_legend( ax, opt, legend, legend_labels ):
  if not opt.legend_enabled:
    return

  if not opt.paper_mode:
    # using fancy translucent legend box
    leg = ax.legend( legend, \
                     legend_labels, loc='best', fancybox=True, \
                     prop={'size':opt.fontsize} , \
                     columnspacing=opt.legend_columnspacing, \
                     handlelength=opt.legend_handlelength, \
                     handletextpad=opt.legend_handletextpad)
    leg.get_frame().set_alpha( 0.5 )
  else:
    leg = ax.legend( legend, \
                     legend_labels, loc=8, \
                     bbox_to_anchor=opt.legend_bbox, \
                     ncol=opt.legend_ncol, \
                     borderaxespad=0., \
                     prop={'size':opt.fontsize} , \
                     columnspacing=opt.legend_columnspacing, \
                     handlelength=opt.legend_handlelength, \
                     handletextpad=opt.legend_handletextpad)
    # we dissappear the box
    leg.get_frame().set_color("white")


def set_common( ax, opt ):
  # Title and Legend stuff.
  if not opt.paper_mode:
    ax.set_title( opt.title )

  if opt.xlabel:
    ax.set_xlabel( opt.xlabel, fontsize=opt.fontsize )
  if opt.ylabel:
    ax.set_ylabel( opt.ylabel, fontsize=opt.fontsize )

  if opt.paper_mode:
    # enable grid
    ax.yaxis.grid(True)
    # turn off top and right border
    ax.spines['right'].set_visible(False)
    ax.spines['top'].set_visible(False)
    ax.xaxis.set_ticks_position('bottom')
    ax.yaxis.set_ticks_position('left')

  # when sharing axes, don't display labels
  if opt.share_axis == 1:
    plt.setp( ax.get_xticklabels(), visible=False )
  elif opt.share_axis == 2:
    plt.setp( ax.get_yticklabels(), visible=False )


