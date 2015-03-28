## Compilation

First build SphinxBase:
```shell
cd sphinxbase-5prealpha
./autogen.sh
./configure --prefix=`pwd`/../install
make
make install
cd ..
```

Then build PocketSphinx:
```shell
cd pocketsphinx-5prealpha
./autogen.sh
./configure --prefix=`pwd`/../install
make
make install
cd ..
```

## Running
To run tests:
```shell
cd test
./run.sh
```
Program output is in out.txt. Execution time prediction output is in times.txt

You can update which input files are used by modifying ctlFile.txt. gen_ctlfile.py is a simple script to help generate ctlFile.txt.

