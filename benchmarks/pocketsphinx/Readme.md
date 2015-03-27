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

