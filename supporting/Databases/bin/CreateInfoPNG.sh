#!/bin/bash
#$1="Text"

convert -size 512x512 xc:lightblue -pointsize 40 \
          -fill blue  -annotate +10+40 "$1" -rotate 180 -annotate +10+40 "$1" Info.png

