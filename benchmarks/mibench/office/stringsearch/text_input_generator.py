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
nlines = 1332
find_strings = []

print "char *find_strings[] = {"
for i in range(nlines):
  find_string = random.choice(words)
  print "\""+find_string+"\", "
  find_strings.append(find_string)
print "NULL};"

print "char *search_strings[] = {"
for i in range(nlines):
  nwords = random.randint(5,400) #ARCH_X86
  print "\"",
  for j in range(nwords):
    if random.randint(1, 100) == 1: # with very low possiblity, add find_string word
      print find_strings[i]+" ",
    print random.choice(words)+" ",
  print "\", "
print "NULL};"

