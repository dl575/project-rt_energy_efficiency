#!/usr/bin/python

import os
import sys
import random

"""
Takes a number to dial and output file as input. Returns a string with a say
command to generate dialing the passed number.
"""
def dial(number, filename):
  cmd = "say -o %s --data-format=I16@16000 dial" % filename
  silence = "[[slnc 100]]"
  for digit in str(number):
    cmd += " %s %s" % (silence, digit)
  return cmd

digits = {0 : "zero",
    1 : "one",
    2 : "two",
    3 : "three",
    4 : "four",
    5 : "five",
    6 : "six",
    7 : "seven",
    8 : "eight",
    9 : "nine"}

def single_digit_test():
  """
  Dial a single digit number. Dial each number 50 times.
  """
  # Setup wav output file
  filename = "temp.wav"
  # Setup corresponding filelist
  os.system("echo %s > temp_filelist.txt" % filename)
  # Command for running julius
  julius_cmd = "./julius.dSYM -C Sample.jconf -input file -filelist temp_filelist.txt"

  ntrials = 50
  for digit in range(10):
    for n in range(ntrials):
      #say_cmd = dial(digit, filename)
      say_cmd = "cp digits/%s.wav temp.wav" % (digits[digit])
      os.system(say_cmd)
      os.system(julius_cmd)

def generate_single_digits():
  for d in digits.iterkeys():
    filename = "digits/" + digits[d] + ".wav"
    say_cmd = dial(d, filename)
    os.system(say_cmd)

if __name__ == "__main__":
  if len(sys.argv) < 2:
    N = 50
  else:
    N = int(sys.argv[1])

  #generate_single_digits()
  #single_digit_test()

  # Setup wav output file
  filename = "temp.wav"
  # Setup corresponding filelist
  os.system("echo %s > temp_filelist.txt" % filename)
  # Command for running julius
  julius_cmd = "./julius.dSYM -C julian.jconf -input file -filelist temp_filelist.txt"

  for n in range(N):
    random_length = random.randint(1, 10)
    random_number = random.randint(0, 10**random_length)
    print "Dialing %d" % (random_number)
    say_cmd = dial(random_number, filename)
    os.system(say_cmd)
    os.system(julius_cmd)

  # Clean up
  os.system("rm -f temp.wav temp_filelist.txt")
