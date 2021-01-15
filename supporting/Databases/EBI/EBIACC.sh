#!/bin/bash

ACC=$1
NUM=$2
SKIP=$3

curl -X GET --header 'Accept:application/json' \
	"https://www.ebi.ac.uk/proteins/api/proteins?offset=${SKIP}&size=${NUM}&accession=${ACC}" \
	-o data.json


