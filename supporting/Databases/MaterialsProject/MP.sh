#!/bin/bash

ID=$1
source api-key.sh
curl -X GET --header 'Accept:application/json' --header "X-Api-Key: $KEY" \
	"https://www.materialsproject.org/rest/v2/materials/mp-$ID/vasp" \
	-o data.json

