#!/bin/bash

i=1
echo -n NOMADViveT.exe > smile.bat

while (( "$#" )); do
	ARG=$1
	CreateSmileNCFG.sh $ARG $i
	echo -n " $i.ncfg " >> smile.bat
	i=`expr $i + 1`
	shift
done

echo >> smile.bat
