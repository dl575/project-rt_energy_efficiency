if [ -z "$1" ]
then
  echo "No input file specified"
  exit
fi

ffmpeg -i $1 -benchmark -y -s 1000x1000 -an output.avi > ffmpeg_convert_out

