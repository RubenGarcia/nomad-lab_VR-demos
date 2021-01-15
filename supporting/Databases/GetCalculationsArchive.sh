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

#Argument: $1: A, $2: B
#e.g.
#./GetCalculationsArchive.sh Ti Ca 

A=$1
B=$2


wget https://analytics-toolkit.nomad-coe.eu/api/query/single_configuration_calculation?filter=system_composition:$A${B}O3%20AND%20section_repository_info.repository_spacegroup_nr:221%20AND%20stats_meta_present:band_energies -O archive.json

ERROR=$?
I=0
while [[ "$ERROR" != "0" && "$I" != 3 ]]
	do
	echo "Connection error to analytics-toolkit.nomad-coe.eu, retrying"
	#echo "error $ERROR"
	sleep 2
	wget https://analytics-toolkit.nomad-coe.eu/api/query/single_configuration_calculation?filter=system_composition:$A${B}O3%20AND%20section_repository_info.repository_spacegroup_nr:221%20AND%20stats_meta_present:band_energies -O archive.json
	ERROR=$?
	I=`expr $I + 1`
	done

