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
#include <fstream>
#include <iomanip>

using namespace std;

//Globals
extern queue<string> phy_send_q[20];
extern queue<string> phy_receive_q[20];
extern queue<string> dl_send_q[20];
extern queue<string> dl_receive_q[20];
extern queue<string> app_send_q[20];
extern queue<string> app_receive_q[20];
extern queue<string> window_q[20];

extern pthread_mutex_t mutex_phy_send[20];
extern pthread_mutex_t mutex_phy_receive[20];
extern pthread_mutex_t mutex_dl_send[20];
extern pthread_mutex_t mutex_dl_receive[20];
extern pthread_mutex_t mutex_socket[20];
extern pthread_mutex_t mutex_app_send_q[20];
extern pthread_mutex_t mutex_app_receive_q[20];
extern pthread_mutex_t mutex_window_q[20];

extern int PORT;
extern int previous_frame_received[20];
extern pthread_t dl_thread[20];
extern int clients;
extern int vb_mode;

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
void *dl_layer_server(void *client_num);

#endif
