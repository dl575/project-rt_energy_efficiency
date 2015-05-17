#!/usr/bin/python

import os
import random

# pocketsphinx word dictionary
dictionary = []
f = open("../benchmarks/pocketsphinx/pocketsphinx-5prealpha/model/en-us/cmudict-en-us.dict")
for line in f:
  dictionary.append(line.split(' ')[0].split('(')[0])
f.close()


def say_cmd(words, filename):
  cmd = "say -o %s --data-format=I16@16000" % filename
  silence = "[[slnc 100]]"
  for word in words:
    cmd += " %s \"%s\"" % (silence, word)
  return cmd

def get_nwords(n):
  words = []
  for i in range(n):
    words.append(random.choice(dictionary))
  return words

if __name__ == "__main__":
  """
  Sweep generator.
  """
  """
  j = 0
  for nwords in range(1, 10):
    for i in range(10):
      words = get_nwords(nwords)
      cmd = say_cmd(words, "temp%03d.wav" % (j))
      j += 1
      print cmd
      os.system(cmd)
  """  

  """
  Random generator.
  """
  nfiles = 50
  for i in range(nfiles):
    nwords = random.randint(1,3)
    words = get_nwords(nwords)
    cmd = say_cmd(words, "temp%03d.wav" % (i))
    print cmd
    os.system(cmd)
