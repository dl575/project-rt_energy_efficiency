#!/usr/bin/python

output = "#!/bin/bash\n"
output += "rm times.txt\n"
output += "rm output_slice.txt\n"

output += "videos=(\"akiyo_qcif\" \"carphone_qcif\" \"claire_qcif\" \
\"coastguard_qcif\" \"container_qcif\" \"foreman_qcif\" \"hall_qcif\" \
\"mother-daughter_qcif\" \"news_qcif\" \"silent_qcif\")\n"
output += "Index_videos=0\n"
output += "for video in \"${videos[@]}\"\n"
output += "  do\n"
output += "  echo $Index_videos\n"
output += "  #cp -v videos/$video.264 test.264\n"
output += "  cp -v ../../datasets/videos_88x72/$video.264 test.264\n"
output += "  ./bin/ldecod.dbg.exe $Index_videos\n"
output += "  ((++Index_videos))\n"
output += "done\n"
output += "cp times.txt output_slice.txt"

print output
