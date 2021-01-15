#!/bin/bash

#Argument: $1: A, $2: B, $3:TOKEN
#e.g. 
#./GetPerovskite.sh Ti Ca `cat token.txt`

A=$1
B=$2
TOKEN="$3"

curl -H "Content-Type: application/json" --user "$TOKEN:" -X POST -d \
	"{\"search_by\": {\"exclusive\": \"1\", \"formula\": \"$A${B}O3\",\"pagination\": \"off\" }, \
 	\"has_band_structure\": \"True\", \"space_group\": [\"221\"] \
	, \"band_gap\": { \"min\": \"1e-35\" , \"max\": \"1000.0 \" } \
	}" \
	https://encyclopedia.nomad-coe.eu/api/v1.0/materials -o Perovskite.json

#curl -H "Content-Type: application/json" --user "$TOKEN:" -X POST -d \
#	"{\"search_by\": {\"exclusive\": \"1\", \"formula\": \"$A${B}O3\",\"pagination\": \"off\" },  \"has_band_structure\": \"True\", \"space_group\": [\"221\"], \"band_gap\": { \"min\": \"0.01\" } }" \
#	https://encyclopedia.nomad-coe.eu/api/v1.0/materials -o results.json



