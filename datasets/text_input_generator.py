#!/usr/bin/python

import random

word_filename = "/usr/share/dict/words"
words = open(word_filename).read().splitlines()

def random_sentence():
  nwords = 15
  s = ""
  for word in range(nwords):
    s += random.choice(words) + ' '
  s += '\n'
  return s

"""
Sweep number of lines
"""
"""
for nlines in range(1, 25):
  f = open("input%d.txt" % (nlines), 'w')
  for line in range(1000*nlines):
    # Average number of words in a sentence
    nwords = 15
    for word in range(nwords):
      f.write(random.choice(words))
      f.write(' ')
    f.write('\n')
  f.close()
"""

"""
Random number of lines.
"""
nfiles = 100
for i in range(nfiles):
  filename = "input%d.txt" % (i)
  print "generating %s..." % (filename)
  f = open(filename, 'w')
  nlines = random.randint(1000, 26000)
  for j in range(nlines):
    f.write(random_sentence())
  f.close()
