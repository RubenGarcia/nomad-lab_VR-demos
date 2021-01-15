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

#$1="Crystal structure:\n YYYY\nBand Gap:\n XXXX eV\nComposition: TiCaO3"

#Encyclopedia
#$2=31455

#$3=ncfg file

#$4=png file

#$5=menubutton parameter

convert -size 512x512 xc:lightblue -pointsize 40 \
          -fill blue  -annotate +10+40 "$1" -rotate 180 -annotate +10+40 "$1" $4

cat > $3 <<END
server nomad.srv.lrz.de 3000 1234
menubutton $5
sidebuttontimestep 0
disablereloadreset
background 0 0 0
atomscaling 0.5
json "$2/material"
displaybonds
displayunitcell
showcontrollers
info 0 0 -4 2 0 "$4"
info  0 5 -4  2   0   "PE.png"
info 10 5 -4  2   0   "Info.png"
END
