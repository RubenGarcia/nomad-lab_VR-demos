#!/bin/bash
#https://api.rsc.org/compounds/v1
#https://api.rsc.org/compounds/v1/filter/smiles

POLYMER=$1

#rubengarciahernandez / rubengarciahernandez@gmail.com
export KEY=7IXIW9G0kNP6AJAWCu5cBPfjAas4LoSw

curl --header "Content-Type: " --header "apikey: $KEY" -X POST \
	-d "{\"smiles\": \"$1\"}" \
	https://api.rsc.org/compounds/v1/filter/smiles -o Q.json

QID=`cut -f 4 -d \" Q.json`

if  [ "$QID" = "" ]; then
        echo No compound found in database, exiting
        exit 1
fi

sleep 10
curl --header "Content-Type: " --header "apikey: $KEY" \
	https://api.rsc.org/compounds/v1/filter/$QID/status -o QS.json


#old API,no message
STATUS=`cut -f 4 -d \" QS.json`
#new API, 2019, with message
STATUS=`cut -f 10 -d \" QS.json`

echo Status is $STATUS

while [[ "$STATUS" != "Complete" ]]
	do
	if [[ "$STATUS" == "Suspended" ]]; then
		echo "Suspended query, exiting"
		exit 2
	elif [[ "$STATUS" == "Failed" ]]; then
		echo "Failed query, exiting"
		exit 3
	elif [[ "$STATUS" == "Not Found" ]]; then
		echo "Compound not found, exiting"
		exit 4
	fi

	sleep 5
	curl --header "Content-Type: " --header "apikey: $KEY" \
		https://api.rsc.org/compounds/v1/filter/$QID/status -o QS.json
	STATUS=`cut -f 4 -d \" QS.json`
	done

COUNT=`cut -f 3 -d \: QS.json |cut -f 1 -d \,`

#rgh: gives empty result for benzene (!?)
#curl --header "Content-Type: " --header "apikey: $KEY" \
#	https://api.rsc.org/compounds/v1/filter/$QID/results/sdf -o data.sdf

curl -X GET --header "Content-Type: " --header "apikey: $KEY" \
	"https://api.rsc.org/compounds/v1/filter/$QID/results" -o QR.json

#get IDs. If COUNT=1, this is only one result, assume so for now
RID=`cut -f 2 -d \[ QR.json |cut -f 1 -d \]`

curl -X GET --header "Content-Type: " --header "apikey: $KEY" "https://api.rsc.org/compounds/v1/records/$RID/details?fields=Formula%2CMolecularWeight%2CCommonName%2CSmiles%2CMol3D" -o data.json

#obabel -i mol data.mol -O data.xyz

