## Compilation

First build SphinxBase:
cd sphinxbase-5prealpha
./autogen.sh
./configure --prefix=`pwd`/../install
make
make install
cd ..

Then build PocketSphinx:
cd pocketsphinx-5prealpha
./autogen.sh
./configure --prefix=`pwd`/../install
make
make install
cd ..

## Running
cd test
./run.sh

Program output is in out.txt. Execution time prediction output is in times.txt

