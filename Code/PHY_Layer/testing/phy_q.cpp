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

#define BUFFER_SIZE 128
#define PORT 5000
#define HOSTNAME "CCCWORK1"

void diewithError(string message) {
        cout<<message<<endl;
        exit(1);
}

void *phy_send_t(void* num);
void *phy_receive_t(void* num);


//queues
queue<string> phy_send_q;
queue<string> phy_receive_q;

int connected=0;//Socket connected=1
int socketfd;//Socket descriptor

pthread_mutex_t mutex_phy_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_phy_receive = PTHREAD_MUTEX_INITIALIZER;

int main(){

	pthread_t phy_send_thread, phy_receive_thread;
	int iret1, iret2;
	iret1 = pthread_create( &phy_send_thread, NULL, phy_send_t, (void*) 1);
	iret2 = pthread_create( &phy_receive_thread, NULL, phy_receive_t, (void*) 1);

	//Thread status info, consider moving to own thread
	for(int i=0;i<12;i++){
		sleep(1);
		cout<<"Phy Send Thread Size:"<<phy_send_q.size()<<endl;
		cout<<"Phy Receive Thread Size:"<<phy_receive_q.size()<<endl;
		}
	return 0;

}


//Physical Layer Send thread
//Sends data from phy_send_q out through TCP
void *phy_send_t(void* num){

	int n;
	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);//Clear buffer

	//Setup Connection
	if (!connected){
		socketfd=phy_setup(PORT, gethostbyname(HOSTNAME));
		connected=1;
	}
	//Wait for messages to be sent
	while(1){
		if (!phy_send_q.empty()){
			//Send first item in queue
			n = write(socketfd, buffer, strlen(buffer)); //Send
                        if (n < 0) diewithError("ERROR writing to socket");
			//Remove sent item from queue
			pthread_mutex_lock( &mutex_phy_send );
			phy_send_q.pop();
			pthread_mutex_unlock( &mutex_phy_send );
			}
	}
	cout<<"phy_send_t (THREAD) Died"<<endl;
}

//Phyiscal Layer Receive
//Receives data from TCP socket and places it in phy_receive_q
void *phy_receive_t(void* num){

	int n;

	//setup connection
	if (!connected){
		socketfd=phy_setup(PORT,gethostbyname(HOSTNAME));
		connected=1;
	}
	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);//Clear buffer
	while(1){
		//Wait for packet to be received
		n = read(socketfd, buffer, BUFFER_SIZE - 1);
                if (n < 0) diewithError("ERROR reading from socket");
		//Add frame to queue
		pthread_mutex_lock( &mutex_phy_receive );
		phy_receive_q.push(buffer);
		pthread_mutex_unlock( &mutex_phy_receive );
	}
	cout<<"phy_receive_t (THREAD) Died"<<endl;
}




