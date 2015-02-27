# Benchmarks

This directory contains various applications and kernels that we are using for benchmarks.

## FreeCiv

FreeCiv is a turn-based game based on the commercial game Civilization. To build it, you can either run the `./configure` script with your own options, or just run the `./run_configure.sh` script. After this, run `make`. `fcgui` is the resulting binary to run.

## Julius
Julius is a speech recognition program. To build, first go into `benchmarks/julius/julius-4.3.1`. Run `./configure` followed by `make`. This should create a binary `julius/julius.dSYM` or `julius/julius`. Copy this into `benchmarks/julius/julius-3.5.2-quickstart-linux`. This directory contains config and supporting files to run julius. `live.sh` will run julius taking input from the microphone. Alternatively, run `generate_wav.py` which will generate 50 random samples and run julius with them. This script depends on the `say` command on Mac OSX and may need to be modified for other systems.

## XPilot

XPilot is a game similar to asteroids. To build, run `xmkmf -a` followed by `make` (Refer to `INSTALL.txt` if needed). The XPilot application consists of 2 parts: a server and a client. First run the server by running `src/servre/xpilots`. Then, launch a client by running `src/client/xpilot`.

## 2048.c

C clone of the puzzle game 2048. To build, run `make`. Use WASD to play rather than arrow keys because arrow keys generate extra button presses, and thus extra "jobs".

Source: https://github.com/mevdschee/2048.c

## curseofwar

Terminal-based strategy game. To build, run `make`.

## uzbl

Web browser. Run `make` to build. May need to run `make install` to set up config, but can run `make uninstall` and still run binary. Also, run `source setup.sh` to setup paths needed.

Source: https://github.com/uzbl/uzbl
