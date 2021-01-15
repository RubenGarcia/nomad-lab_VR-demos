#!/bin/bash

#Argument: $1: material, $2: calculation, $3: TOKEN
#e.g. 
#./GetProperties.sh 31455 60824 `cat token.txt`

M=$1
C=$2
TOKEN=$3

curl --user "$TOKEN:" https://encyclopedia.nomad-coe.eu/api/v1.0/materials/$M/calculations/$C?property=band_gap -o bandgapT.json \
	http://enc-testing-nomad.esc.rzg.mpg.de/v1.0/materials/$M/calculations/$C?property=band_gap_direct -o bandgapdirectT.json \
	http://enc-testing-nomad.esc.rzg.mpg.de/v1.0/materials/$M/calculations/$C?property=lattice_parameters -o latticeparametersT.json


