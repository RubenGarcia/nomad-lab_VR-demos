#include <sys/socket.h>

#include "myrecv.h"

int myrecv(int sock,void *buffer,int len, int flags) {
        int n=len;
        char *mybuff=(char*)buffer;
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

