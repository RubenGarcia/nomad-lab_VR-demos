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
#include <stdint.h>
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")
#else
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
//http://stackoverflow.com/questions/4291149/difference-between-string-h-and-strings-h
#endif
#include <string.h>
#include <stdlib.h>
#include <vector>

std::vector<int> sockfds;

void error(const char *msg)
{
perror(msg);
#ifdef WIN32
for (unsigned int i=0;i< sockfds.size();i++)
	if (sockfds[i]>=0 && sockfds[i]!=INVALID_SOCKET)
		closesocket (sockfds[i]);
#else
for (int i=0;i< sockfds.size();i++)
	if (sockfds[i]>=0)
		close (sockfds[i]);
#endif


#ifdef WIN32
WSACleanup();
#endif
    exit(1);
}

struct state_t {
	int32_t timestep;
	int32_t iso;
	bool showatoms;
	int32_t ncfg;
} state;

bool initNewSocket (unsigned int secret)
{
	int n;
	 int32_t tmp;
	 int32_t rsec;
	 char buffer[17];
	 buffer[0]='t';
	 tmp=htonl(state.timestep);
	 memcpy (buffer+1, &tmp, sizeof(state.timestep));
	 buffer[5]='i';
	 tmp=htonl(state.iso);
	 memcpy (buffer+6, &tmp, sizeof(state.iso));
	 buffer[10]='s';
	 buffer[11]=(char)true;
	 buffer[12]='n';
	 tmp=htonl(state.ncfg);
	 memcpy (buffer+13, &tmp, sizeof(state.iso));

	 //struct sockaddr_in cli_addr;
	 //socklen_t clilen;
     sockfds.push_back(accept(sockfds[0], nullptr, nullptr));
#ifdef WIN32
	if (sockfds.back() == INVALID_SOCKET) 
#else
     if (sockfds.back() < 0) 
#endif
          error("ERROR on accept");

	tmp=htonl (secret);

	 n=recv(sockfds.back(), (char*)&rsec, sizeof(rsec), 0);
	 if (tmp!=rsec) {
#ifdef WIN32
		closesocket (sockfds.back());
#else
		close (sockfds.back());
#endif
		return false;
	 }

	n = send(sockfds.back(), buffer , 17, 0);
	if (n < 0) {
		error("ERROR writing to socket");
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{
	unsigned int secret;
	//windows sockets are only very loosely based on unix sockets.
	//initialization and error management are different.
	//https://msdn.microsoft.com/en-us/library/windows/desktop/ms742213%28v=vs.85%29.aspx
	//http://tangentsoft.net/wskfaq/articles/bsd-compatibility.html

	state.timestep=0;
	state.iso=-1; //ISOS
	state.showatoms=true;
	state.ncfg=1;

	fd_set active_fd_set, read_fd_set;
     int portno;
	 std::vector<socklen_t> clilens;
     char buffer[256];
     struct sockaddr_in serv_addr;
	 std::vector<struct sockaddr_in> cli_addrs;
     int n;

#ifdef WIN32
	WORD wVersionRequested;
    WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        return 1;
    }
	
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return 1;
    }
#endif



     if (argc < 3) {
         fprintf(stderr,"Usage:\n%s <port> <secret>\nSecret is a 32 bit unsigned integer\n", argv[0]);
         exit(1);
     }
	 secret=atoi(argv[2]);
     sockfds.push_back (socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    
#ifdef WIN32
	if (sockfds[0] == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
#else
	  if (sockfds[0] < 0) {
#endif
        error("ERROR opening socket");
	 }
     memset((char *) &serv_addr, 0, sizeof(serv_addr));
	 cli_addrs.push_back(serv_addr);
	 cli_addrs.push_back(serv_addr);
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfds[0], (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfds[0],5);
	 clilens.push_back(sizeof(struct sockaddr_in));
	 clilens.push_back(sizeof(struct sockaddr_in));

  FD_ZERO (&active_fd_set);
  FD_SET (sockfds[0], &active_fd_set);

     memset(buffer,0, 256);



for (;;) {
	read_fd_set=active_fd_set;

#ifdef WIN32
	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) == INVALID_SOCKET)
#else
	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
#endif
		{
		error ("select");
		}

	for (unsigned i=1;i<sockfds.size();i++)
		if (FD_ISSET (sockfds[i], &read_fd_set)) {
     		n = recv(sockfds[i],buffer,255, 0);
			buffer[n]=0;
			if (n < 0) error("ERROR reading from socket");
			if (n==0) error ("client closed socket, exiting");

			printf("Here is the message: %s\n",buffer);
			//update state
			if (buffer[0]=='t') {
				int32_t time;
				memcpy(&time, buffer+1, sizeof(int32_t));
				time=ntohl(time);
				state.timestep=time;
			} else if (buffer[0]=='i') {
				int32_t iso;
				memcpy(&iso, buffer+1, sizeof(int32_t));
				iso=ntohl(iso);
				state.timestep=iso;
			} else if (buffer[0]=='n') {
				int32_t ncfg;
				memcpy(&ncfg, buffer+1, sizeof(int32_t));
				ncfg=ntohl(ncfg);
				state.timestep=ncfg;
			} else if (buffer[0]=='s') {
				state.showatoms=buffer[1]!=0;
			} else {
				fprintf (stderr, "Unknown state request %c\n", buffer[0]);
				error ("unknown state" );
			}

			for (unsigned j=1;j<sockfds.size();j++) {
				n = send(sockfds[j], buffer , n, 0);
				if (n < 0) error("ERROR writing to socket");
			}
		}

	//unblockingly see if new connections were made and add to sockfds
	if (FD_ISSET (sockfds[0], &read_fd_set)) {
		if (initNewSocket (secret));
			FD_SET (sockfds.back(), &active_fd_set);
	}
}
#ifdef WIN32
WSACleanup();
#endif
     return 0; 
}
