#!/usr/bin/python

import random

word_filename = "/usr/share/dict/words"
words = open(word_filename).read().splitlines()

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
