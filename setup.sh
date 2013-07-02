if [[ ! ":$PATH:" =~ (^|:)"`pwd`/ffmpeg-1.2"(:|$) ]]; then
  export PATH=$PATH:`pwd`/ffmpeg-1.2
fi
