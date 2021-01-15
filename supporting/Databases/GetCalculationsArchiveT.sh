#!/bin/bash
#Argument: $1: A, $2: B
#e.g.
#./GetCalculationsArchive.sh Ti Ca 

A=$1
B=$2

SERVER="https://labdev-nomad.esc.rzg.mpg.de/api/query"

#QUERY="single_configuration_calculation?filter=system_composition:$A${B}O3%20AND%20section_repository_info.repository_spacegroup_nr:221%20AND%20stats_meta_present:band_energies"
QUERY="single_configuration_calculation?filter=system_composition%3D%22$A${B}O3%22"

wget $SERVER/$QUERY -O archiveT.json
ERROR=$?

I=0

while [[ "$ERROR" != "0" && "$I" != "3" ]] ;
	do
	echo "Connection error to analytics-toolkit.nomad-coe.eu, retrying"
	#echo "error $ERROR"
	sleep 2
	wget $SERVER/$QUERY -O archiveT.json
	ERROR=$?
	I=`expr $I + 1`
	done

