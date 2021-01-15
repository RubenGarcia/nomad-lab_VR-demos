#!/bin/bash

#Argument: $1: TOKEN
#e.g. 
#./ListPerovskites.sh `cat token.txt`

TOKEN="$1"

curl -H "Content-Type: application/json" --user "$TOKEN:" -X POST -d \
	"{\"search_by\": {\"exclusive\": \"0\", \"element\":\"O\", \"pagination\": \"off\" }, \
 	\"has_band_structure\": \"True\", \"space_group\": [\"221\"] \
	, \"band_gap\": { \"min\": \"1e-35\" , \"max\": \"1000.0 \" } \
	}" \
	https://encyclopedia.nomad-coe.eu/api/v1.0/materials -o Perovskite.json




