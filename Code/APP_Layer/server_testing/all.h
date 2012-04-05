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
#include <sstream>
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

//Function prototypes
void diewithError(string message);
void printhelp(void);
int phy_setup(int port, struct hostent *server);
int count_words(char *str);

extern queue<string> dl_receive_q;
extern queue<string> app_send_q;
extern queue<string> app_receive_q;

#endif
