#include "all.h"

/*
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <list>
#include <cstdlib>
#include <queue>
#include <deque>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
*/
//using namespace std;

#define BUFFER_SIZE 256
//#define PORT 5005
//#define HOSTNAME "CCCWORK2"

void diewithError(string message) {
        cout<<message<<endl;
        exit(1);
}

//void *phy_send_t(void* num);
//void *phy_receive_t(void* num);
void *phy_layer_t(void* num);


//queues
//queue<string> phy_send_q;
//queue<string> phy_receive_q;

int alive=1;
int connected=1;//Socket connected=1
int socketfd;//Socket descriptor


//overall dl thread
void *phy_layer_t(void* num){
    
    fd_set read_flags,write_flags; // you know what these are
    struct timeval waitd;          
    int thefd;             // The socket
    char outbuff[256];     // Buffer to hold outgoing data
    char inbuff[256];      // Buffer to read incoming data into
    int err;	       // holds return values
    string temp;   
 
    memset(&outbuff,0,sizeof(outbuff)); // memset used for portability
    thefd=phy_setup(PORT,gethostbyname(HOSTNAME)); // Connect to the finger port
    if(thefd==-1) {
        printf("Could not connect to finger server\n");
        exit(0);
    }
    
    // Mark the socket as non-blocking, for safety.
    int x;
    x=fcntl(thefd,F_GETFL,0);
    fcntl(thefd,F_SETFL,x | O_NONBLOCK);
    connected=0; 
    cout<<"Sokcet_SETUP!!!"<<endl;
    
    while(1) {
        FD_ZERO(&read_flags); // Zero the flags ready for using
        FD_ZERO(&write_flags);
        FD_SET(thefd, &read_flags);
        if(!phy_send_q.empty()) FD_SET(thefd, &write_flags);
        err=select(thefd+1, &read_flags,&write_flags,
                   (fd_set*)0,&waitd);
        if(err < 0) continue;
        
        //READ SOMETHING
        //cout<<"Trying to read"<<endl;
        if(FD_ISSET(thefd, &read_flags)) { //Socket ready for reading
            FD_CLR(thefd, &read_flags);
            memset(&inbuff,0,sizeof(inbuff));
            if (read(thefd, inbuff, sizeof(inbuff)-1) <= 0) {
                close(thefd);
                diewithError("Socket READ Bug socket closed");
                break;
            }
            else{
                printf("Received %s\n",inbuff);
                pthread_mutex_lock( &mutex_phy_receive );
                phy_receive_q.push(inbuff);
                pthread_mutex_unlock( &mutex_phy_receive );
            }
        }
        
        //WRITE SOMETHING
        if (!phy_send_q.empty()){

            cout<<"SOMETHING IN Q"<<endl;    

            if(FD_ISSET(thefd, &write_flags)) { //Socket ready for writing
                FD_CLR(thefd, &write_flags);
	
		temp.clear();
		temp=phy_send_q.front();
		strcpy(outbuff,temp.c_str());    
		
                cout<<"Sending "<<"'"<<outbuff<<"'"<<" (PHY)"<<endl;
                write(thefd,outbuff,strlen(outbuff));
                cout<<"Sent (PHY)"<<endl;
                memset(&outbuff,0,sizeof(outbuff));
                
                pthread_mutex_lock( &mutex_phy_send );
                phy_send_q.pop();
                pthread_mutex_unlock( &mutex_phy_send );
            }
	    else
		cout<<"Socket not ready for writing"<<endl;
        }
        // now the loop repeats over again
    }
    
    
}



