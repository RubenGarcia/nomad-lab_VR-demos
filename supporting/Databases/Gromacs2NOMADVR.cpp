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

#include <signal.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>

#include "rapidjson/document.h" 
#include "rapidjson/filereadstream.h"

#include "myrecv.h"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#define closesocket close
#endif

char buffer[100];
int sock;

int SendReload() 
{
static int l=0;
int n;
buffer[0]='n';
uint32_t tmp=htonl(1);
memcpy(buffer+1, (char*)&tmp, 4);
n=send (sock, buffer, 5, 0);
if (n<5) {
	fprintf (stderr, "could not send reload command\n");
	return -1;
}
printf ("Reload %d sent, value %d\n", l++, 1);
return 0;
}

int main (int argc, char ** argv)
{
char command [200];

if (argc != 7) {
	fprintf (stderr, "Usage: %s <pid of gromacs mdrun> <time interval> <xtc/tpr basename>"
		" <server> <port> <secret>\n", argv[0]);
	return 1;
}

int pid;
pid=atoi (argv[1]);

float interval=atof(argv[2])*60.0f;

struct timespec req;
struct timespec rem;
req.tv_sec=(int)interval;
req.tv_nsec=(interval - req.tv_sec)* 1000000000;

//xtc does not have atom symbols, just positions
//sprintf (command, "obabel -i xtc %s -O 1.xyz", argv[3]);
sprintf (command, "GetXTCLastFrame.sh %s", argv[3]);

FILE *pfile=fopen ("1.ncfg", "w");
fprintf (pfile, "menubutton InfoBox\n"
	"sidebuttontimestep 0\n"
	"showcontrollers\n"
	"disablereloadreset\n"
	"server %s %s %s\n"
	"xyzfile \"%s.xyz\"\n", argv[4], argv[5], argv[6], argv[3]);
fclose (pfile);

float time=0;

struct hostent *he;
struct sockaddr_in serv_addr;

	if ( (he = gethostbyname(argv[4]) ) == nullptr ) {
		fprintf (stderr, "Connect to server, could not get host name %s\n", argv[1]);
      return 2; /* error */
	}
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[5]));
	sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if ( connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) {
				fprintf (stderr, "Connect to server, could not get connection %s\n", argv[1]);
      return 3; /* error */
	}

int n;
	int32_t tmp;
	tmp=htonl (atoi(argv[6])); //secret
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

while (0 == kill(pid, 0)) { //process exists

	n=myrecv(sock, buffer, 1, 0);
	if (n<1) {
			fprintf (stderr, "couldn't receive data command");
			return 5;
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
		break;

		case 'i':
		n=myrecv(sock, &tmp, 4, 0);
		if (n<4) 
		{
			fprintf (stderr, "couldn't receive data i\n");
			return 5;
		}
		break;
		case 's': 
		n=myrecv(sock, &tmp, 1, 0);
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
		break;
		default: 
			fprintf (stderr, "Unknown command received: '%c', %d\n", buffer[0], buffer[0]);
			return -5;
	}
	nanosleep (&req, &rem);
	system (command);
	SendReload();
	time++;
}
return 0;
}
