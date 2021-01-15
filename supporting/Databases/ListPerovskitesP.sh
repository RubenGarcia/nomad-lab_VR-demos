#!/bin/bash

#Argument: $1: TOKEN
#e.g. 
#./ListPerovskitesP.sh `cat token.txt`

TOKEN="$1"

curl -H "Content-Type: application/json" --user "$TOKEN:" -X POST -d \
	"{\"search_by\": {\"exclusive\": \"0\", \"pagination\": \"off\" }, \
 	\"has_thermal_properties\": \"True\" \
	}" \
	https://encyclopedia.nomad-coe.eu/api/v1.0/materials -o PerovskiteThermal.json




