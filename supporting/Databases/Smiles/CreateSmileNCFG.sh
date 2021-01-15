#!/bin/bash

GetSmile.sh $1 $2.xyz

PRINTSM=`echo $1 |sed -e "s/.\{10\}/&\n/g"`

convert -size 512x512 xc:lightblue -pointsize 40 \
          -fill blue  -annotate +10+40 "Smile\n$PRINTSM" \
	-rotate 180 -annotate +10+40 "Smile\n$PRINTSM" $2.png

cat >$2.ncfg <<END
#SMILE $1
xyzfile "$2.xyz"
info 0 0 -4 2 0 "$2.png"
menubutton Nothing
sidebuttontimestep 0
disablereloadreset
background 0 0 0
atomscaling 0.5
displaybonds
showcontrollers
END
