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
#define PORT 5005
#define HOSTNAME "CCCWORK2"

void diewithError(string message) {
        cout<<message<<endl;
        exit(1);
}

void *phy_send_t(void* num);
void *phy_receive_t(void* num);


//queues
queue<string> phy_send_q;
queue<string> phy_receive_q;

int alive=1;
int connected=0;//Socket connected=1

pthread_mutex_t mutex_phy_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_phy_receive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;

int main(){

	pthread_t phy_send_thread, phy_receive_thread;
	int iret1, iret2;
	iret1 = pthread_create( &phy_send_thread, NULL, phy_send_t, (void*) 1);
	iret2 = pthread_create( &phy_receive_thread, NULL, phy_receive_t, (void*) 1);

	//Thread status info, consider moving to own thread
	while(1){
		sleep(1);
		cout<<"Phy Send Thread Size:"<<phy_send_q.size()<<endl;
		cout<<"Phy Receive Thread Size:"<<phy_receive_q.size()<<endl;

		if (!phy_receive_q.empty()){
			//Echo Back
			pthread_mutex_lock( &mutex_phy_send );
			phy_send_q.push(phy_receive_q.front());
			pthread_mutex_unlock( &mutex_phy_send );
			
			pthread_mutex_lock( &mutex_phy_receive );
			phy_receive_q.pop();	
			pthread_mutex_unlock( &mutex_phy_receive );
	
		}
	}

	//Stop threads
	alive=0;
	pthread_cancel(phy_send_thread);
	pthread_cancel(phy_receive_thread);

	return 0;

}


//Physical Layer Send thread
//Sends data from phy_send_q out through TCP
void *phy_send_t(void* num){

	int n;
	//bzero(buffer, BUFFER_SIZE);//Clear buffer

	//Setup Connection
	int socketfd=phy_setup_server(PORT);

	//Wait for messages to be sent
	while(alive){
		if (!phy_send_q.empty()){
			//Send first item in queue
			//buffer=phy_send_q.front();
			//n = write(socketfd, buffer, strlen(buffer)); //Send
			string buffer;
			buffer=phy_send_q.front();
			cout<<buffer<<endl;
			pthread_mutex_lock( &mutex_socket );
			n = write(socketfd, buffer, 256); //Send
			pthread_mutex_unlock( &mutex_socket );
                        if (n < 0) diewithError("ERROR writing to socket");
			cout<<"Message Sent:"<<phy_send_q.front()<<endl;
			//Remove sent item from queue
			pthread_mutex_lock( &mutex_phy_send );
			phy_send_q.pop();
			pthread_mutex_unlock( &mutex_phy_send );
			}
	}
	cout<<"phy_send_t (THREAD) Died"<<endl;
	close(socketfd);
}

//Phyiscal Layer Receive
//Receives data from TCP socket and places it in phy_receive_q
void *phy_receive_t(void* num){

	int n;
	sleep(1);

	//setup connection
	int socketfd=phy_setup_server(PORT+1);
	
	char buffer[BUFFER_SIZE];
	bzero(buffer, BUFFER_SIZE);//Clear buffer
	while(alive){
		//Wait for packet to be received
		n = read(socketfd, buffer, BUFFER_SIZE - 1);
		cout << n <<endl;
                if (n < 0) diewithError("ERROR reading from socket");
		cout<<"Message Received"<<endl;
		//Add frame to queue
		if (strcmp(buffer,"DONE"))//leave 
			alive=0;
		pthread_mutex_lock( &mutex_phy_receive );
		phy_receive_q.push(buffer);
		pthread_mutex_unlock( &mutex_phy_receive );

		cout<<phy_receive_q.front()<<endl;
	}
	cout<<"phy_receive_t (THREAD) Died"<<endl;
	close(socketfd);
}




