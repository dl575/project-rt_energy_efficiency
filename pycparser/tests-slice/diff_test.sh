if [ $# -eq 0 ]
then
  echo "No arguments supplied"
  echo "usage: diff_test.sh test_name [inline|loop_counts|slice]"
  exit
fi

vimdiff $1/$1_$2.c $1/out/$1_$2.c
