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
#include <iomanip>
#include <fstream>

using namespace std;

//Globals
extern queue<string> phy_send_q;
extern queue<string> phy_receive_q;
extern queue<string> dl_send_q;
extern queue<string> dl_receive_q;
extern queue<string> app_send_q;
extern queue<string> app_receive_q;
extern queue<string> window_q;

extern pthread_mutex_t mutex_phy_send;
extern pthread_mutex_t mutex_phy_receive;
extern pthread_mutex_t mutex_dl_send;
extern pthread_mutex_t mutex_dl_receive;
extern pthread_mutex_t mutex_socket;
extern pthread_mutex_t mutex_app_send;
extern pthread_mutex_t mutex_app_receive;
extern pthread_mutex_t mutex_window_q;

extern int PORT;
extern int connected;
extern char *HOSTNAME;
extern int vb_mode;
extern int probability;

//Function prototypes
void diewithError(string message);
void verbose(string message);
void printhelp(void);
//int phy_setup(int port, struct hostent *server);
int count_words(char *str);
void *phy_layer_server(void *num);
int phy_setup(int port, struct hostent *serv);
int phy_setup_server1(int port);
void *phy_layer_t(void *num);
void *dl_layer_client(void *num);

#endif
