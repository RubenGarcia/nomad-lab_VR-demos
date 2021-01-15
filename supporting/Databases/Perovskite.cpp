/*
 # Copyright 2016-2018 Ruben Jesus Garcia-Hernandez
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
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>

#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h"

#include "NOMADVRLib/ConfigFileAtoms.h"
#include "NOMADVRLib/atoms.hpp"
#include "PeriodicTable.h"
#include "exportXYZ.h"
#include "state.h"
#include "myrecv.h"
#include "Archive.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#define closesocket close
#endif

const int NUMATOMS=118;

char *token;
int maxconfigs;

char buffer[100];
int sock;

int SendReload() 
{
static int l=0;
int n;
buffer[0]='n';
uint32_t tmp=htonl(state.ncfg);
memcpy(buffer+1, (char*)&tmp, 4);
n=send (sock, buffer, 5, 0);
if (n<5) {
	fprintf (stderr, "could not send reload command\n");
	return -1;
}
printf ("Reload %d sent, value %d\n", l++, state.ncfg);
return 0;
}


int ReceiveDatasets(int A, int B, const char* menubutton)
{
char command[1024];
int loadedcalculations;

printf ("Starting data download, %s%sO3\n", atomNames[A], atomNames[B]);
//query Encyclopedia database for ABO3, 221
sprintf (command, "GetPerovskite.sh %s %s %s", atomNames[A], atomNames[B], token);
system (command);
//now parse Perovskite.json
FILE *pfile=fopen ("Perovskite.json", "r");
char readBuffer[65536];
rapidjson::FileReadStream is(pfile, readBuffer, sizeof(readBuffer));
rapidjson::Document json;
json.ParseStream(is);
fclose(pfile);

int total_results;
if (!json.HasParseError() && json.HasMember("total_results") && json["total_results"].IsInt())
	total_results=json["total_results"].GetInt();
else
	return -8;
if (!json.HasMember("results"))
	return -8;
const rapidjson::GenericValue<rapidjson::UTF8<> > &results=json["results"];
int cresult=1;
for (int i=0;i<total_results;i++) {
	const rapidjson::GenericValue<rapidjson::UTF8<> > &result=results[i];
	if (!result.HasMember("nr_of_calculations_matching_criteria"))
		return -8;
	const rapidjson::GenericValue<rapidjson::UTF8<> > &calcs=
			result["calculations_list_matching_criteria"];
	int matching=result["nr_of_calculations_matching_criteria"].GetInt();
	int id=result["id"].GetInt();
	const char *formula=result["formula"].GetString();
	int space_group=result["space_group"].GetInt();
	//download material
	sprintf (command, "wget http://nomad.srv.lrz.de/cgi-bin/NOMAD/material?%d -O %d.zip", id, id);
	system (command);
	sprintf (command, "unzip -o %d.zip", id);
	system (command);
	for (int j=0;j<matching;j++) {
		int calc=calcs[j].GetInt();
		//download band gap
		sprintf (command, "GetProperties.sh %d %d %s", id, calc, token);
		system (command);
	
		//read bandgap
		FILE *file=fopen ("bandgap.json", "r");
		char readBuffer[65536];
		rapidjson::FileReadStream is2(file, readBuffer, sizeof(readBuffer));
		rapidjson::Document json;
		json.ParseStream(is2);
		fclose(file);
		double bandgap;
		if (!json.HasParseError() && json.HasMember("band_gap") && 
			json["band_gap"].IsDouble()) 
			bandgap=json["band_gap"].GetDouble();
		else 
			return -8;
		//read bandgapdirect
		file=fopen ("bandgapdirect.json", "r");
		rapidjson::FileReadStream is4(file, readBuffer, sizeof(readBuffer));
		json.ParseStream(is4);
		fclose(file);
		bool direct;
		if (!json.HasParseError() && json.HasMember("band_gap_direct") && 
			json["band_gap_direct"].IsBool()) 
			direct=json["band_gap_direct"].GetBool();
		else 
			return -8;
		//read latticeparameters
		file=fopen ("latticeparameters.json", "r");
		rapidjson::FileReadStream is3(file, readBuffer, sizeof(readBuffer));
		json.ParseStream(is3);
		fclose(file);
		const char *latticeParameters;
		if (!json.HasParseError() && json.HasMember("lattice_parameters") && 
			json["lattice_parameters"].IsString()) 
			latticeParameters=json["lattice_parameters"].GetString();
		else 
			return -8;

		float lp[3];
		sscanf (latticeParameters, "(%f,%f,%f)", &(lp[0]), &(lp[1]), &(lp[2]));
		for (int k=0;k<3;k++)
			lp[k]*=1e10; //display: °Angstrom
		//parse into png file
		sprintf (command, "CreatePerovskiteNCFG.sh \"Space group: %d\nBand Gap (Direct=%c):\n"
			" %f eV\nLattice parameters (Å):\n (%0.02f,%0.02f,%0.02f)\nComposition: %s\" %d %d.ncfg %d.png %s", 
				space_group, direct?'T':'F', bandgap*6.2415096471204E+18, lp[0], lp[1], lp[2], formula, 
				id, cresult, cresult, menubutton);
		system (command);
		cresult++;
	}//j: [0..matching[
}//i: [0..results[ (open set)
//end of query encyclopedia
//Query Archive
sprintf (command, "GetCalculationsArchive.sh %s %s", atomNames[A], atomNames[B]);
system (command);
int res=ParseArchiveResults(A, B, cresult, menubutton);
if (res<0)
	return res;
cresult=res;
//end of query archive
//reload scene
loadedcalculations=cresult-1;
if (loadedcalculations>maxconfigs)
	printf ("%d calculations, only first %d will be shown in VR\n", 
		loadedcalculations, maxconfigs);
else
	printf ("%d calculations loaded\n", loadedcalculations);
return loadedcalculations;
}

void CreateNCFGPeriodicTable (char **argv)
{
FILE *infoboxes=fopen ("PT.ncfg", "r");
FILE *ncfg=fopen ("1.ncfg", "w");
char *buffer[2048];
int n;
do {
	n=fread (buffer, 1, 2048, infoboxes);
	fwrite (buffer, n, 1, ncfg);
} while (n>0);
fprintf (ncfg, "xyzfile PeriodicTable.xyz\n"
	"server %s %s %s\n"
	"menubutton %s\n"
	"sidebuttontimestep 0\n"
	"showcontrollers\n"
	"disablereloadreset\n"
	"info  0 5 0  2   0   \"PE.png\"\n"
	"info 10 5 0  2   0   \"Info.png\"\n", argv[1], argv[2], argv[3], argv[5]);
fclose (ncfg);
}

void CreateCleanNCFG(int i, char **argv)
{
char name [100];
sprintf (name, "%d.ncfg", i);
FILE *ncfg=fopen (name, "w");
fprintf (ncfg, 
	"server %s %s %s\n"
	"menubutton %s\n"
	"sidebuttontimestep 0\n"
	"showcontrollers\n"
	"disablereloadreset\n", argv[1], argv[2], argv[3], argv[5]);
fclose (ncfg);
}

void ResetNCFGs(char **argv) 
{
char command[300];
CreateNCFGPeriodicTable(argv);
sprintf (command, "CreateInfoPNG.sh \"Perovskite\n ABO3\nPull atom\nto fix A\"");
system(command);

for (int i=2;i<=maxconfigs; i++)
	CreateCleanNCFG(i, argv);
}

int main (int argc, char ** argv)
{

char command[1024];

int A=-1, B=-1;
int mode =0; //0: periodic table; 1: perovskite
int numconfigs=1;

if (argc!=6) {
	fprintf (stderr, "Parameters: <server> <port> <secret> <maxconfigs> <menubutton option: Record, Infobox, Nothing>\n");
	return 1;
}

maxconfigs=atoi(argv[4]);

ResetNCFGs(argv);

struct stat stat_buf;


int len;
system ("GetToken.sh");
//https://stackoverflow.com/questions/5840148/how-can-i-get-a-files-size-in-c
int rc = stat("token.txt", &stat_buf);
if (rc == 0) {
	len=stat_buf.st_size;
} else {
	printf ("Could not get NOMAD Auth\n");
	return 7;
}
token=new char[len+1];
FILE *f=fopen ("token.txt", "r");
fread (token, len, 1, f);
token[len]='\0';
fclose(f);

numAtoms=new int[1];
numAtoms[0]=NUMATOMS;

atoms=new float* [1];
atoms[0]=new float[NUMATOMS*4];

FILE *pt=fopen ("PT.ncfg", "w");
CreatePeriodicTable(atoms[0], 5, pt);
fclose (pt);
exportXYZ("PeriodicTable.xyz", "PeriodicTable", 1);

f=fopen ("run.bat", "w");
fprintf (f, "NOMADViveT.exe ");
for (int i=1;i<=maxconfigs;i++)
	fprintf (f, "%d.ncfg ", i);
fprintf (f, "\n");
fclose(f);

//connect to server, TCP

struct hostent *he;
struct sockaddr_in serv_addr;

	if ( (he = gethostbyname(argv[1]) ) == nullptr ) {
		fprintf (stderr, "Connect to server, could not get host name %s\n", argv[1]);
      return 2; /* error */
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[2]));
	sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if ( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
				fprintf (stderr, "Connect to server, could not get connection %s\n", argv[1]);
      return 3; /* error */
	}

int n;
	int32_t tmp;
	tmp=htonl (atoi(argv[3])); //secret
	n = send(sock, (char*)&tmp , sizeof(tmp), 0);
	if (n<sizeof(tmp)) {
		closesocket(sock);
		sock=INVALID_SOCKET;
		fprintf (stderr, "couldn't send secret\n");
		return 4;
	}




uint32_t ID;
int found;
char prev1=0, prev2=0;
while (true) {
	n=myrecv(sock, buffer, 1, 0);
	if (n<1) {
			fprintf (stderr, "couldn't receive data command");
			return 5;
	}
	if (prev1!=buffer[0]) {
		prev2=prev1;
		prev1=buffer[0];
		printf ("received order %c\n", buffer[0]);
	} else if (prev2!=prev1) {
		printf ("received (possibly multiple) order %c\n", buffer[0]);
		prev2=prev1;
		prev1=buffer[0];		
	}
	switch (buffer[0]) {
		case 'X': // ID
		n=myrecv(sock, &ID, 4, 0);
		if (n<4) {
			fprintf (stderr, "couldn't receive data X");
			return 5;
		}
		break;

		case 't': 
		n=myrecv(sock, &tmp, 4, 0);
		if (n<4) 
		{
			fprintf (stderr, "couldn't receive data t");
			return 5;
		}
		state.timestep=ntohl(tmp);
		break;

		case 'i':
		n=myrecv(sock, &tmp, 4, 0);
		if (n<4) 
		{
			fprintf (stderr, "couldn't receive data i\n");
			return 5;
		}
		state.iso=ntohl(tmp);
		break;

		case 's': 
		n=myrecv(sock, &state.showatoms, 1, 0);
		if (n<1) 
		{
			fprintf (stderr, "couldn't receive data s\n");
			return 5;
		}
		break;

		case 'n': 
		
		n=myrecv(sock, &tmp, 4, 0);
		if (n<4) 
		{
			fprintf (stderr, "couldn't receive data n\n");
			return 5;
		}
		state.ncfg=ntohl(tmp);
		printf ("state.ncfg is %d\n", state.ncfg);
		if (mode==0) {//periodic table
			if(state.ncfg!=1) {
				state.ncfg=1;
				SendReload();
			}
		} else {
			if (state.ncfg>numconfigs) {
				state.ncfg=numconfigs;
				SendReload();
			}
		}
		break;

		case 'p': // user position
		n=myrecv (sock, buffer+1, sizeof(float)*3 + 4, 0);
		if (n<sizeof(float)*3 + 4) {
			fprintf (stderr, "couldn't receive data p\n");
			return 5;
		}
		break;

		case 'd': //disconnect
		n=myrecv (sock, buffer+1, 4, 0);
		if (n<4) 
{
			fprintf (stderr, "couldn't receive data d\n");
			return 5;
		}
		break;

		case 'D': //Drag
		n=myrecv (sock, buffer+1, 6*sizeof(float) + 4*2 +1, 0);
		if (n<6*sizeof(float) + 4*2 +1) 
{
			fprintf (stderr, "couldn't receive data D\n");
			return 5;
		}
		//find the nearest atom
		//test, use atom 0
		if (mode==0) {
			float orig[3];
			float dest[3];
			uint32_t ID, time;
			char button;
			memcpy (&ID, buffer+1, 4);
			memcpy (orig, buffer+5, 3*sizeof(float));
			memcpy (dest, buffer+5+3*sizeof(float), 3*sizeof(float));
			memcpy (&time, buffer+5+6*sizeof(float), 4);
			button=buffer[1+6*sizeof(float)+8];
			if (button!=2) {
				printf ("intermediate, discarding\n");
				continue;
			}
			found=-1;
			for (int i=0;i<NUMATOMS;i++) {
				float sum=0;
				for (int j=0;j<3;j++) {
					const float tmp=atoms[0][i*4+j]-orig[j];
					sum+=tmp*tmp;
				}
				const float r=atomRadius(static_cast<int>(atoms[0][i*4+3]));
				if (sum < r*r) {
					found=i;
					break;
				}
			
			}
			if (found!=-1) {
				if (A==-1) {
					A=found;
					//put A in a box
					sprintf (command, "CreateInfoPNG.sh \"Perovskite ABO3\nA=%s\nPull atom to fix B\"", 
						atomNames[A]);
					system(command);
					printf ("Selected A is %s\n", atomNames[A]);
				}
				if (B==-1 && A!=found) {
					B=found;
					printf ("Selected A is %s and B is %s\n", atomNames[A], atomNames[B]);
					sprintf (command, "CreateInfoPNG.sh \"Perovskite ABO3\nA=%s,B=%s\nLoading\"", 
						atomNames[A], atomNames[B]);
					system(command);
					SendReload();
					numconfigs=ReceiveDatasets(A,B, argv[5]);
					if(numconfigs<0) {
						sprintf (command, "CreateInfoPNG.sh \"Error getting\n datasets, exiting\"");
						system(command);
						printf ("error getting datasets, exiting");
						return -8;
					} else if (numconfigs==0) {
						sprintf (command, "CreateInfoPNG.sh \"Material not\n in database\"");
						system(command);
						printf ("the material is not in the database\n");
						A=-1;
						B=-1;
					} else { //all ok
						sprintf (command, "CreateInfoPNG.sh \"%s%sO3\n\n"
								"Pull anywhere to \nreturn to periodic table\"", 
								atomNames[A], atomNames[B]);
						system(command);
						mode=1;
					}
				}
			
			
		} else { //mode ==1
			//showing perovskite, go back to periodic table after user drags on box
			A=-1;
			B=-1;
			state.ncfg=1;
			ResetNCFGs(argv);
		}
			SendReload();
		}
		break;

		default: 
			fprintf (stderr, "Unknown command received: '%c', %d\n", buffer[0], buffer[0]);
			return -5;
	}
}

return 0;
}
