#!/bin/bash

SMILE=$1
XYZ=$2

curl -A "Opera 11.0" --referer "https://cactus.nci.nih.gov/translate/" \
	-F smiles="$SMILE" \
	-F format=mol \
	-F astyle=kekule \
	-F dim=3D \
	-F file="" \
	https://cactus.nci.nih.gov/cgi-bin/translate.tcl > tmp.html

FILE=`grep href tmp.html |cut -f 2 -d \"`
curl https://cactus.nci.nih.gov/$FILE > file.mol

obabel file.mol -O $XYZ

rm -f file.mol tmp.html
