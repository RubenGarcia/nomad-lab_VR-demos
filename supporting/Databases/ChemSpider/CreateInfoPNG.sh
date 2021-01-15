#!/bin/bash

 # Copyright 2016-2018 Ruben Jesus Garcia Hernandez
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #     http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.

#$1="Text"
#$2=size of png
#$3=filename
SIZE=$2
FILE=$3
if [[ "$FILE" == "" ]]; then
	FILE=info.png
fi
if [[ "$SIZE" == "" ]]; then
	SIZE=320
fi
convert -size ${SIZE}x${SIZE} xc:lightblue -pointsize 40 \
          -fill blue  -annotate +10+40 "$1" -rotate 180 -annotate +10+40 "$1" \
	$FILE

