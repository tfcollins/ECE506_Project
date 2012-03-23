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

//Globals
queue<string> phy_send_q;
queue<string> phy_receive_q;
queue<string> dl_send_q;
queue<string> dl_receive_q;
queue<string> app_send_q;
queue<string> app_receive_q;
queue<string> window_q;

/*
extern pthread_mutex_t mutex_phy_send;// = PTHREAD_MUTEX_INITIALIZER;
extern pthread_mutex_t mutex_phy_receive;// = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_app_send_q = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_app_receive_q = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_window_q = PTHREAD_MUTEX_INITIALIZER;
*/

//Function prototypes
void diewithError(string message);
void printhelp(void);
//int phy_setup(int port, struct hostent *server);
int count_words(char *str);
void *phy_layer_server(void *num);
int phy_setup(int port, struct hostent *serv);
int phy_setup_server1(int port);


#endif
