export RT_SCHEDULING=`pwd`

# ffmpeg binaries
if [[ ! ":$PATH:" =~ (^|:)"$RT_SCHEDULING/ffmpeg-1.2"(:|$) ]]; then
  export PATH=$PATH:$RT_SCHEDULING/ffmpeg-1.2
fi

# scripts
if [[ ! ":$PATH:" =~ (^|:)"$RT_SCHEDULING/scripts"(:|$) ]]; then
  export PATH=$PATH:$RT_SCHEDULING/scripts
fi
