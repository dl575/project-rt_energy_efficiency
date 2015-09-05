#!/bin/bash

for (( i=0; i<10; i++ ));
do
    cp temp00$i.wav temp05$i.wav
    cp temp01$i.wav temp06$i.wav
    cp temp02$i.wav temp07$i.wav
    cp temp03$i.wav temp08$i.wav
    cp temp04$i.wav temp09$i.wav
done
