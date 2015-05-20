#!/usr/bin/python

import os

resolution = "88x72"
output_dir = "temp/"
video_files = [
    "akiyo_qcif",
    "carphone_qcif",
    "claire_qcif",
    "coastguard_qcif",
    "container_qcif",
    "foreman_qcif",
    "hall_qcif",
    "mother-daughter_qcif",
    "news_qcif",
    "silent_qcif",
    ]

os.system("rm %s/*" % output_dir)

for video_file in video_files:
  cmd = "ffmpeg -i %s.264 -s %s -c:a copy %s/%s.h264" % (video_file, resolution, output_dir, video_file)
  os.system(cmd)
  os.system("mv %s/%s.h264 %s/%s.264" % (output_dir, video_file, output_dir, video_file))
