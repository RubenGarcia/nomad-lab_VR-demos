#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "NOMADVRLib/ConfigFileAtoms.h"
#include "NOMADVRLib/atoms.hpp"
#include "PeriodicTable.h"
#include "exportXYZ.h"
#include "state.h"
#include "myrecv.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#define closesocket close
#endif

const int NUMATOMS=118;

int main (int argc, char ** argv)
{

if (argc!=4) {
	fprintf (stderr, "Parameters: <server> <port> <secret>\n");
	return 1;
}

state_t state;

numAtoms=new int[1];
numAtoms[0]=NUMATOMS;

atoms=new float* [1];
atoms[0]=new float[NUMATOMS*4];

CreatePeriodicTable(atoms[0], 5);

exportXYZ("A.xyz", "PeriodicTable", 1);

FILE *ncfg=fopen ("A.ncfg", "w");
fprintf (ncfg, "xyzfile A.xyz\n"
	"server %s %s %s\n"
	"menubutton Infobox\n"
	"sidebuttontimestep 0\n", argv[1], argv[2], argv[3]);
fclose (ncfg);

//connect to server, TCP

int sock;

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


char buffer[100];

uint32_t ID;
int found;
while (true) {
	n=myrecv(sock, buffer, 1, 0);
	if (n<1) {
			fprintf (stderr, "couldn't receive data command");
			return 5;
	}
	printf ("received order %c\n", buffer[0]);
	switch (buffer[0]) {
		case 'X': // ID
		n=myrecv(sock, &ID, 4, 0);
		if (n<4) {
			fprintf (stderr, "couldn't receive data X");
			return 5;
		}
		break;

		case 't': 
		n=myrecv(sock, &state.timestep, 4, 0);
		if (n<4) 
{
			fprintf (stderr, "couldn't receive data t");
			return 5;
		}
		break;

		case 'i':
		n=myrecv(sock, &state.iso, 4, 0);
		if (n<4) 
{
			fprintf (stderr, "couldn't receive data i\n");
			return 5;
		}
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
		n=myrecv(sock, &state.ncfg, 4, 0);
		if (n<4) 
{
			fprintf (stderr, "couldn't receive data n\n");
			return 5;
		}
		break;
		
		case 'A':
		//Selected atom, ignore
		int32_t tmp;
		n=myrecv(sock, &tmp, 4, 0);
		if (n<4) 
{
			fprintf (stderr, "couldn't receive data n\n");
			return 5;
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
			printf ("Starting Drag atom %d\n", found);
			//drag it
			for (int i=0;i<3;i++)
				atoms[0][found*4+i]+=dest[i]-orig[i];

			exportXYZ("A.xyz", "PeriodicTable", 1);
			//reload scene

			buffer[0]='n';
			memcpy(buffer+1, (char*)&state.ncfg, 4);
			n=send (sock, buffer, 5, 0);
			if (n<5) {
				fprintf (stderr, "could not send reload command\n");
				return 6;
			}
			printf ("Reload sent\n");
		}
		break;

		default: 
			fprintf (stderr, "Unknown command received: '%c', %d\n", buffer[0], buffer[0]);
			return -5;
	}
}

return 0;
}
