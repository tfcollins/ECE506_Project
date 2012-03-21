//Includes and diewithError 

#ifndef __ALL__
#define __ALL__

#include <iostream>
#include <stdio.h>              
#include <sys/socket.h>         
#include <arpa/inet.h>         
#include <stdlib.h>            
#include <string.h>
#include <string>
#include <unistd.h>   
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>          //For POSIX threads 
#include <list>
#include <queue>
#include <deque>
#include <time.h>
#include <fcntl.h>

using namespace std;

void diewithError(string message);
int phy_setup(int port, struct hostent *serv);
int phy_setup_server1(int port);
int phy_setup_server2(int port);
int doconnect(char *address, int port);
int SSconnect(int port);

extern int PORT;
extern char* HOSTNAME;

#endif
