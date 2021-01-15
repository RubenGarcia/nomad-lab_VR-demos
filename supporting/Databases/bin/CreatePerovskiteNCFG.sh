#!/bin/bash
#$1="Crystal structure:\n YYYY\nBand Gap:\n XXXX eV\nComposition: TiCaO3"

#Encyclopedia
#$2=31455

#$3=ncfg file

#$4=png file

convert -size 512x512 xc:lightblue -pointsize 40 \
          -fill blue  -annotate +10+40 "$1" -rotate 180 -annotate +10+40 "$1" $4

cat >> $3 <<END
background 0 0 0
atomscaling 0.5
json "$2/material"
displaybonds
displayunitcell
showcontrollers
info 0 0 -3 2 0 "$4"
END
