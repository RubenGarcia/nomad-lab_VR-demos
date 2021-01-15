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

