//
//  non_blocking.cpp
//  
//
//  Created by Travis Collins on 3/19/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

using namespace std;

// this routine simply converts the address into an
// internet ip
unsigned long name_resolve(char *host_name)
{
    struct in_addr addr;
    struct hostent *host_ent;
    if((addr.s_addr=inet_addr(host_name))==(unsigned)-1) {
        host_ent=gethostbyname(host_name);
        if(host_ent==NULL) return(-1);
        memcpy(host_ent->h_addr, (char *)&addr.s_addr, host_ent->h_length);
    }
    return (addr.s_addr);
}

// The connect routine including the command to set
// the socket non-blocking.
int doconnect(char *address, int port)
{
    int x,s;
    struct sockaddr_in sin;
    
    s=socket(AF_INET, SOCK_STREAM, 0);
    x=fcntl(s,F_GETFL,0);              // Get socket flags
    fcntl(s,F_SETFL,x | O_NONBLOCK);   // Add non-blocking flag
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family=AF_INET;
    sin.sin_port=htons(port);
    sin.sin_addr.s_addr=name_resolve(address);
    if(sin.sin_addr.s_addr==NULL) return(-1);
    printf("ip: %s\n",inet_ntoa(sin.sin_addr));
    x=connect(s, (struct sockaddr *)&sin, sizeof(sin));
    if(x<0) return(-1);
    return(s);
}

int main (void)
{
    fd_set read_flags,write_flags; // you know what these are
    struct timeval waitd;          
    int thefd;             // The socket
    char outbuff[512];     // Buffer to hold outgoing data
    char inbuff[512];      // Buffer to read incoming data into
    int err;	       // holds return values
    
    memset(&outbuff,0,sizeof(outbuff)); // memset used for portability
    thefd=doconnect("203.1.1.1",79); // Connect to the finger port
    if(thefd==-1) {
        printf("Could not connect to finger server\n");
        exit(0);
    }
    strcat(outbuff,"jarjam\n"); //Add the string jarjam to the output
    //buffer
    while(1) {
        waitd.tv_sec = 1;     // Make select wait up to 1 second for data
        waitd.tv_usec = 0;    // and 0 milliseconds.
        FD_ZERO(&read_flags); // Zero the flags ready for using
        FD_ZERO(&write_flags);
        FD_SET(thefd, &read_flags);
        if(strlen(outbuff)!=0) FD_SET(thefd, &write_flags);
        err=select(thefd+1, &read_flags,&write_flags,
                   (fd_set*)0,&waitd);
        if(err < 0) continue;
        if(FD_ISSET(thefd, &read_flags)) { //Socket ready for reading
            FD_CLR(thefd, &read_flags);
            memset(&inbuff,0,sizeof(inbuff));
            if (read(thefd, inbuff, sizeof(inbuff)-1) <= 0) {
                close(thefd);
                break;
            }
            else printf("%s",inbuff);
        }
        if(FD_ISSET(thefd, &write_flags)) { //Socket ready for writing
            FD_CLR(thefd, &write_flags);
            write(thefd,outbuff,strlen(outbuff));
            memset(&outbuff,0,sizeof(outbuff));
        }
        // now the loop repeats over again
    }
}
