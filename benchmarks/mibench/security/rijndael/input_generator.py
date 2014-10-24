#!/usr/bin/python

import random

word_filename = "/usr/share/dict/words"
words = open(word_filename).read().splitlines()

nlines = random.randint(1, 1000)
for line in range(nlines):
  nwords = random.randint(1, 1000)
  for word in range(nwords):
    print random.choice(words),
  print
