# Benchmarks
This directory contains various applications and kernels that we are using for benchmarks.

## 2048.c
C clone of the puzzle game 2048. To build, run `make`. Use WASD to play rather
than arrow keys because arrow keys generate extra button presses, and thus
extra "jobs".

Source: https://github.com/mevdschee/2048.c

## curseofwar
Terminal-based strategy game. To build terminal version, run `make`. To build
SDL version, run `make SDL=yes`.

Source: https://github.com/a-nikolaev/curseofwar

## freeciv
FreeCiv is a turn-based game based on the commercial game Civilization. To
build it, you can either run the `./configure` script with your own options, or
just run the `./run_configure.sh` script. After this, run `make`. `fcgui` is
the resulting binary to run.

Source: http://sourceforge.net/projects/freeciv/

## julius
Julius is a speech recognition program. To build, first go into
`benchmarks/julius/julius-4.3.1`. Run `./configure` followed by `make`. This
should create a binary `julius/julius.dSYM` or `julius/julius`. Copy this into
`benchmarks/julius/julius-3.5.2-quickstart-linux`. This directory contains
config and supporting files to run julius. `live.sh` will run julius taking
input from the microphone. Alternatively, run `generate_wav.py` which will
generate 50 random samples and run julius with them. This script depends on the
`say` command on Mac OSX and may need to be modified for other systems.

Note: This benchmark has been dropped in favor of pocketsphinx.

Source: http://julius.osdn.jp/en_index.php

## ldecode
H.264 decoder (version 9.4 from H.264/AVC Software Coordination). To build, run
`make`. To run, use `./bin/ldecod.dbg.exe`. The program will automatically
decode the file test.264.

Source: http://iphome.hhi.de/suehring/tml/

## mibench

Benchmarks from the MiBench benchmark suite. Look at MiBench documentation for
build information. Note there are two added benchmarks: sha_preread and
rijnadel_preread. These are version of sha and rijndael (respectively) where
the entire source file is first read into memory before running the crypto
algorithm.

Source: http://wwweb.eecs.umich.edu/mibench/

## pocketsphinx

Voice recognition program. Detailed instructions are in pocketsphinx/Readme.md.

Source: http://cmusphinx.sourceforge.net/

Some tutorial information: http://www.eecis.udel.edu/~bohacek/MobileComputing/Android/pocketSphinx.pptx

## shmupacabra

Shmup game. To build, run `make`. To run, `cd sos; ./sos`.

Source: https://github.com/snauts/shmupacabra

## uzbl

Web browser. Run `make` to build. May need to run `make install` to set up
config.

Source: https://github.com/uzbl/uzbl

## xpilot

XPilot is a game similar to asteroids. To build, run `xmkmf -a` followed by
`make` (Refer to `INSTALL.txt` if needed). The XPilot application consists of 2
parts: a server and a client. First run the server by running
`src/servre/xpilots`. Then, launch a client by running `src/client/xpilot`.

Source: http://www.xpilot.org/

