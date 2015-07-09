#!/usr/bin/python

import sys
import re

def read_features(filename):
  features = {}
  f = open(filename, 'r')
  for line in f:
    if ".lps" in line:
      benchmark = line.split('.')[0]
    res = re.search("non-zero coeffs =  \[(.*)\]", line)
    if res:
      features[benchmark] = map(int, res.group(1).split(','))
  f.close()
  return features

def diff_features(features1, features2):
  for benchmark, v1 in features1.iteritems():
    print benchmark
    v2 = features2[benchmark]
    v1_exclusive = list(set(v1) - set(v2))
    if v1_exclusive:
      print "  1 exclusive (%d): " % len(v1_exclusive), v1_exclusive
    v2_exclusive = list(set(v2) - set(v1))
    if v2_exclusive:
      print "  2 exclusive (%d): " % len(v2_exclusive), v2_exclusive

if len(sys.argv) < 3:
  print "usage: diff_features.py predictors1.c predictors2.c"
  exit(1)

little_features = read_features(sys.argv[1])
big_features = read_features(sys.argv[2])

diff_features(little_features, big_features)
