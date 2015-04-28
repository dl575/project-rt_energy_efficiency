#!/bin/bash

rm times.txt

videos=("akiyo_qcif" "carphone_qcif" "claire_qcif" "coastguard_qcif" "container_qcif" "foreman_qcif" "hall_qcif" "mother-daughter_qcif" "news_qcif" "silent_qcif")

for video in "${videos[@]}"
do
  cp -v videos/$video.264 test.264
  ./bin/ldecod.dbg.exe
done
