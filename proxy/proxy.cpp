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
#include <math.h>
#include <limits>

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
#include <sys/ioctl.h>
#include <sys/signal.h>

#define closesocket close
#define INVALID_SOCKET (-1)
//http://stackoverflow.com/questions/4291149/difference-between-string-h-and-strings-h
#endif
#include <string.h>
#include <stdlib.h>
#include <vector>

std::vector<int> sockfds;
std::vector<struct sockaddr_in> cli_addrs;
std::vector<struct sockaddr_in> udp_cli_addrs;
std::vector<float> cli_userpos;
std::vector<unsigned char> cli_colour;


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
	 char buffer[22];
	 buffer[0]='X';
	 tmp=cli_addrs.size();
	 memcpy(buffer+1, &tmp, sizeof(tmp));
	 buffer[5]='t';
	 tmp=htonl(state.timestep);
	 memcpy (buffer+6, &tmp, sizeof(state.timestep));
	 buffer[10]='i';
	 tmp=htonl(state.iso);
	 memcpy (buffer+11, &tmp, sizeof(state.iso));
	 buffer[15]='s';
	 buffer[16]=(char)state.showatoms;
	 buffer[17]='n';
	 tmp=htonl(state.ncfg);
	 memcpy (buffer+18, &tmp, sizeof(state.iso));

	 struct sockaddr_in cli_addr;
	 socklen_t clilen=sizeof(cli_addr);
     sockfds.push_back(accept(sockfds[0], (sockaddr*)&cli_addr, &clilen));
#ifdef WIN32
	if (sockfds.back() == INVALID_SOCKET) 
#else
     if (sockfds.back() < 0) 
#endif
          error("ERROR on accept");
	
	cli_addrs.push_back(cli_addr);
	for (int i=0;i<3;i++)
		cli_userpos.push_back(0);
	for (int i=0;i<3;i++)
		cli_colour.push_back((unsigned char)(rand()/RAND_MAX*256.f));
	tmp=htonl (secret);

	 n=recv(sockfds.back(), (char*)&rsec, sizeof(rsec), 0);
	 if (tmp!=rsec) {
		//verify if extraneous HTTP GET and redirect
		if (rsec==542393671){ // 'GET '
			printf ("extraneous HTTP GET, send to NOMAD VR web\n");
			const char* response="HTTP/1.1 302 Found\r\n"
				"Location: https://www.nomad-coe.eu/the-project/graphics/VR-prototype\r\n";

		n=send(sockfds.back(), response, strlen(response), 0);
		
		}
#ifdef WIN32
		closesocket (sockfds.back());
#else
		printf ("expected %d, got %d\n", secret, ntohl(rsec));
		printf ("tmp=%d, rsec=%d\n", tmp, rsec);
		char l[5];
		l[4]=0;
		memcpy(l, &rsec, 4);
		printf ("ascii %s\n",l);

		close (sockfds.back());
#endif
		return false;
	 }

	n = send(sockfds.back(), buffer , 22, 0);
	if (n < 0) {
		error("ERROR writing to socket");
		return false;
	}

	return true;
}

int myrecv(int sock,char *buffer,int len, int flags) {
	int n=len;
	char *mybuff=buffer;
	int r;
	do {
		r=recv (sock, mybuff, n, flags);
		if (r<0)
			return r;
		mybuff+=r;
		n-=r;
	} while (n>0);
	return len;
}

int main(int argc, char *argv[])
{
	static_assert( std::numeric_limits<double>::is_iec559, "IEEE 754 floating point" );

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
#else
signal(SIGPIPE, SIG_IGN);
#endif



     if (argc < 3) {
         fprintf(stderr,"Usage:\n%s <port> <secret>\nSecret is a 32 bit unsigned integer\n", argv[0]);
         exit(1);
     }
	 secret=atoi(argv[2]);
	printf ("NOMADVR proxy server, accepting connections with secret %d\n", secret);
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
	 for (int i=0;i<3;i++)
		cli_userpos.push_back(0); //unused
	 for (int i=0;i<3;i++)
		 cli_colour.push_back(0); //unused
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

  //now udp
	int udpSock=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (udpSock==INVALID_SOCKET) 
	{
		error("ERROR on udp socket creation");
	}
	if (bind(udpSock, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(udpSock,5);

	 FD_SET (udpSock, &active_fd_set);

     memset(buffer,0, 256);



for (;;) {
	read_fd_set=active_fd_set;

#ifdef WIN32
	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) == INVALID_SOCKET)
#else
	if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
#endif
		{
//		error ("select"); //descriptors not currently ready
		sleep(1);
		continue;
		}

	for (unsigned i=1;i<sockfds.size();i++)
		if (FD_ISSET (sockfds[i], &read_fd_set)) {
			//select activates this also on other conditions such as closed socket. Verify.
			int bytes_available;
			ioctl(sockfds[i],FIONREAD,&bytes_available);
			if (bytes_available<1) {
				fprintf (stderr, "socket %d, bytes=%d, disconnecting\n", 
					sockfds[i], bytes_available);
				FD_CLR(sockfds[i], &active_fd_set);
				closesocket(sockfds[i]);
				sockfds[i]=-1;
				continue;
			}
	     		n = myrecv(sockfds[i],buffer,1, 0);
//			fprintf (stderr, "received %d bytes from socket %d\n", n, i);
			if (n<0) { //disconnected
				printf  ("client closed socket\n");
				FD_CLR(sockfds[i], &active_fd_set);
				closesocket(sockfds[i]);
				sockfds[i]=-1;
				continue;
			}
			buffer[n]=0;
			if (n==0) {
				printf  ("client closed socket\n");
				FD_CLR(sockfds[i], &active_fd_set);
				closesocket(sockfds[i]);
				sockfds[i]=-1;
				continue;
			}

//			printf("Here is the message: '%s'\n",buffer);
		
			//update state
			if (buffer[0]=='t') {
				n = myrecv(sockfds[i],buffer+1,4, 0);
				if (n<0) { //disconnected
					FD_CLR(sockfds[i], &active_fd_set);
					closesocket(sockfds[i]);
					sockfds[i]=-1;
					continue;
				}
				int32_t time;
				memcpy(&time, buffer+1, sizeof(int32_t));
				time=ntohl(time);
				state.timestep=time;
				printf("Timestep: %d\n",time);
			} else if (buffer[0]=='i') {
				n = myrecv(sockfds[i],buffer+1,4, 0);
				if (n<0) { //disconnected
					FD_CLR(sockfds[i], &active_fd_set);
					closesocket(sockfds[i]);
					sockfds[i]=-1;
					continue;
				}
				int32_t iso;
				memcpy(&iso, buffer+1, sizeof(int32_t));
				iso=ntohl(iso);
				state.iso=iso;
				printf("Iso: %d\n",iso);
			} else if (buffer[0]=='n') {
				n = myrecv(sockfds[i],buffer+1,4, 0);
				if (n<0) { //disconnected
					FD_CLR(sockfds[i], &active_fd_set);
					closesocket(sockfds[i]);
					sockfds[i]=-1;
					continue;
				}
				int32_t ncfg;
				memcpy(&ncfg, buffer+1, sizeof(int32_t));
				ncfg=ntohl(ncfg);
				state.ncfg=ncfg;
				printf("ncfg: %d\n",ncfg);
			} else if (buffer[0]=='s') {
				n = myrecv(sockfds[i],buffer+1,1, 0);
				if (n<0) { //disconnected
					FD_CLR(sockfds[i], &active_fd_set);
					closesocket(sockfds[i]);
					sockfds[i]=-1;
					continue;
				}
				state.showatoms=buffer[1]!=0;
				printf("showatoms: %d\n",state.showatoms);
			} else if (buffer[0]=='p') { //local state, response also has identifier
				memcpy(buffer+1,&i, 4);
				n = myrecv(sockfds[i],buffer+5,sizeof(float)*3, 0);
				if (n<0) { //disconnected
					FD_CLR(sockfds[i], &active_fd_set);
					closesocket(sockfds[i]);
					sockfds[i]=-1;
					continue;
				}
				memcpy(&(cli_userpos[3*i]), buffer+5, sizeof(float)*3);
				n+=5;
				printf ("user pos\n");
			} else if (buffer[0]=='D') { //Dragging
				memcpy(buffer+1,&i, 4);
				n = myrecv(sockfds[i],buffer+5, sizeof(float)*6+4+1, 0);
				if (n<0) {
					FD_CLR(sockfds[i], &active_fd_set);
					closesocket(sockfds[i]);
					 sockfds[i]=-1;
					continue;
				}	
				n+=5;
				printf ("user drag\n");
			} else {
				fprintf (stderr, "Unknown state request '%c',%d\n", buffer[0], buffer[0]);
				error ("unknown state" );
			}

			for (unsigned j=1;j<sockfds.size();j++) {
				if (sockfds[j]>=0) {
					if (buffer[0]=='s')
						n=2;
					else if (buffer[0]=='p')
						n=4*3+5;
					else if (buffer[0]=='D')
						n=6*sizeof(float)+4+1+5;
					else
						n=5;
					n = send(sockfds[j], buffer , n, 0);
					if (n < 0) {
						fprintf(stderr, "ERROR writing to socket, closing\n");
						FD_CLR(sockfds[j], &active_fd_set);
						closesocket(sockfds[j]);
						sockfds[j]=-1;
					}
				}
			}
		}

	//unblockingly see if new connections were made and add to sockfds
	if (FD_ISSET (sockfds[0], &read_fd_set)) {
		if (initNewSocket (secret)) {
			printf ("Connected to new client, %d\n", sockfds.back());
			FD_SET (sockfds.back(), &active_fd_set);
		}
	}
	//now udp
	if (FD_ISSET (udpSock, &read_fd_set)) {
//		printf ("fd_isset on udpSock\n");
//datagrams can be reordered, so we cannot split type and payload
//reading a subset of the datagrams discards the rest, so we need all of them to be the same size, and read them in one chunk.
		struct sockaddr_in src_addr;
		socklen_t addrlen=sizeof(src_addr);
		n = recvfrom(udpSock,buffer,5+4+4*4*sizeof(float), 0, (sockaddr*)&src_addr, &addrlen);
		int found=-1;
		for (std::vector<int>::size_type i=0;i<udp_cli_addrs.size();i++) {
			if (src_addr.sin_port == udp_cli_addrs[i].sin_port &&
				src_addr.sin_addr.s_addr== udp_cli_addrs[i].sin_addr.s_addr) {
					found=i;
					break;	
			}
		}
		if (found==-1) {
			udp_cli_addrs.push_back(src_addr);
			found=udp_cli_addrs.size()-1;
                        printf ("adding new udp client %d, p=%d\n", found, 
				src_addr.sin_port);

		}
//		printf ("received bytes: %d\nOrder: %c\n", n, buffer[0]);
		if (n<5+3*4*sizeof(float)) {
			//may receive a 3x4 from the controllers
			printf ("error, continuing\n");
			continue; //error
		}
		else if (!(buffer[0]=='1' || buffer[0]=='2' || buffer[0]=='h')) {
			printf ("udp, unknown message type\n");
			continue;
		}
		uint32_t rem;
		memcpy (&rem, buffer+1, 4);
//		printf ("received udp '%c' from %d, retransmitting\n", buffer[0], rem);
		for (std::vector<int>::size_type i=0;i<udp_cli_addrs.size();i++) {
			//do not sent to originator
			if (i==found)
				continue;
//			printf ("sending to %d\n", i);
			if (n!=sendto(udpSock, buffer, n, 0, (sockaddr*)&(udp_cli_addrs[i]), sizeof(sockaddr_in))){
				printf ("Error sending to %d\n", i);
				udp_cli_addrs[i]=udp_cli_addrs.back();
				udp_cli_addrs.pop_back();
				if (found==udp_cli_addrs.size())
					found=i;
				i--; 
				continue;
			}
		}
	}
}
#ifdef WIN32
WSACleanup();
#endif
     return 0; 
}
