# Copies generates files into the out directory for the specified test

if [ $# -eq 0 ]
then
  echo "No arguments supplied"
  echo "usage: update_test.sh test_name"
  exit
fi

cp -v $1/$1_inline.c $1/out/
cp -v $1/$1_loop_counts.c $1/out/
cp -v $1/$1_slice.c $1/out/
