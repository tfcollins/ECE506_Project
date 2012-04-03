/*
 * dl_layer.cpp
 *
 *  Created on: Mar 20, 2012
 *      Author: six
 *
 *
 * Things to do:
 *		- Set up Timer
 *		- Continue to wrap head around piggyback
 *		- Queues, how to get buffer, push/pop
 *		- Queue to app_layer
 *		- Figure out how to reset, seq/ack counters *WHEN
 *		- Finalize how we want to encode/decode, packet/frame format
 *			i. setup basic frame in this
 */

#include "all.h"
#include <sys/time.h>

using namespace std;

//Window Size
#define MAX_SEQ 4
#define MAX_PKT 200
#define TIMEOUT_MAX 3000000 //fix later, 1 sec = 1000000

#define PHY 1  //RECEIVE
#define APP 2  //SEND
#define TIME_OUT 3

typedef struct{
	int type; //0 for non ACK, 1 for ACK
	int seq_NUM;
//	int ack_NUM;
	char *data;
} frame;


//function prototypes
static void send_data(int frame_to_send, int frame_expected, string buff, int type, int client);
int wait_for_event(int client);
static bool between(int a, int b, int c);
void *time_disp(void* num);
int timeouts(void);
frame deconstruct_frame(char *input);
long current_time();

//globals
long timers[4]={0};
int queued = 0;
int k;
string data;

queue<string> phy_send_q[20];
queue<string> phy_receive_q[20];
queue<string> dl_send_q[20];
queue<string> dl_receive_q[20];
queue<string> app_send_q[20];
queue<string> app_receive_q[20];
queue<string> window_q[20];

pthread_mutex_t mutex_phy_send[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_phy_receive[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_dl_send[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_dl_receive[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_socket[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_app_send_q[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_app_receive_q[20] = {PTHREAD_MUTEX_INITIALIZER};
pthread_mutex_t mutex_window_q[20] = {PTHREAD_MUTEX_INITIALIZER};

//int client;
int previous_frame_received[20]={3};

//Data Link Layer Master Thread
void *dl_layer_server(void *client_num){

	//Set User
	int *client_temp = (int *) client_num;
	int const client = *client_temp;
	//cout<<"DL Layer thread started for client: "<<client<<endl;
	
	pthread_mutex_t mutex_prev_seq_num = PTHREAD_MUTEX_INITIALIZER;
	int frame_to_send = 0;
	int frame_expected = 0;
	int ack_expected = 0;
	int rc;
	frame buffer;

	
	//Spawn Timers Status Thread, updates when timers change
	/*pthread_t thread;
        rc = pthread_create(&thread, NULL, time_disp , (void *) 1);
	if (rc){
		cout<<"Something bad happened with thread creation :("<<endl;
		exit(1);
	}*/
	string input;
	char *tmp;
	previous_frame_received[client]=3;
	//Wait for events to happen
	while (1) {
		//cout<<"Waiting for event (DL)"<<endl;
		//cout<<"PREV:"<<previous_frame_received[client]<<endl;
		int event=wait_for_event(client);
		//cout<<"Event Occurred: "<<event<<" (DL)"<<endl;
		//cout<<&event<<endl;
		switch (event) {

			//If PHY Layer receives message
			case (PHY):
				//cout<<"Last correct seq: "<<previous_frame_received<<endl;
				//bzero(&buffer.data,sizeof(buffer.data));
				pthread_mutex_lock(&mutex_phy_receive[client]);
				//cout<<"phy_r_q Client: "<<client<<endl;
				input = phy_receive_q[client].front();
				//cout<<"Comp"<<endl;
				pthread_mutex_unlock(&mutex_phy_receive[client]);

				tmp = &input[0];
				buffer = deconstruct_frame(tmp);
				//cout<<"Message Deconstructed"<<endl;
				//cout<<"Original: "<<phy_receive_q[client].front()<<endl;
				//cout<<"Data: "<<buffer.data<<" seq_NUM: "<<buffer.seq_NUM<<" Type: "<<buffer.type<<endl; 

				//ACK Received
				if (buffer.type){
					int start=frame_expected;
					int count=0;
					while(1){
						if (start==buffer.seq_NUM)
							break;
						start=(start+1)%4;
						count++;
					}
					for(int h=0;h<=count;h++){
						pthread_mutex_lock(&mutex_window_q[client]);
						window_q[client].pop();
						pthread_mutex_unlock(&mutex_window_q[client]);

						queued--;
						frame_expected=((frame_expected+1)%4);
					}
					pthread_mutex_lock(&mutex_phy_receive[client]);
					phy_receive_q[client].pop();
					pthread_mutex_unlock(&mutex_phy_receive[client]);

					}
				else{//Data Frame Received
		
					//Correct order in sequence
					pthread_mutex_lock(&mutex_prev_seq_num);
					cout<<"Message: "<<buffer.data<<"|"<<endl;
					//cout<<"Prev Seq: "<<previous_frame_received[client]<<" Recvd Seq: "<<buffer.seq_NUM<<" Client: "<<client<<endl;
					if (buffer.seq_NUM==((previous_frame_received[client]+1)%4)){
						//cout<<"Correctly Ordered Frame"<<endl;
						previous_frame_received[client]=((previous_frame_received[client]+1)%4);
						
						pthread_mutex_lock(&mutex_dl_receive[client]);
						dl_receive_q[client].push(buffer.data);
						pthread_mutex_unlock(&mutex_dl_receive[client]);
	
						pthread_mutex_lock(&mutex_phy_receive[client]);
						phy_receive_q[client].pop();
						pthread_mutex_unlock(&mutex_phy_receive[client]);

						send_data(buffer.seq_NUM, 9, "ACK", 1, client);//Send ACK
						//cout<<"Sending ACK (DL)"<<endl;
					}
					else{//Drop Packet
						cout<<"Data Frame out order, dropping (DL)"<<endl;
						pthread_mutex_lock(&mutex_phy_receive[client]);
						//cout<<"phy_r_q Client"<<client<<endl;
						phy_receive_q[client].pop();
						//cout<<"Comp"<<endl;
						pthread_mutex_unlock(&mutex_phy_receive[client]);
						//break;//not sure if needed
					}
					pthread_mutex_unlock(&mutex_prev_seq_num);
				
				}
				break;

			//If APP Layer wants to send message
			case (APP):
				pthread_mutex_lock(&mutex_dl_send[client]);
				data = dl_send_q[client].front();
				dl_send_q[client].pop();
				pthread_mutex_unlock(&mutex_dl_send[client]);

				pthread_mutex_lock(&mutex_window_q[client]);
				window_q[client].push(data);//Save if needed for retransmission
				pthread_mutex_unlock(&mutex_window_q[client]);
				//Send buffer to physical layer
				//Include seq number for packing
				send_data(frame_to_send, frame_expected, data, 0, client);
				frame_to_send++;
				queued++;//cycle to next q
				break;

			//If No ACK received, and timeout
			case (TIME_OUT):
				frame_to_send = ack_expected;
				//Reset N Frames
				if (queued==0){
					cout<<"Timeout incorrect Queue Size"<<endl;
					exit(1);
				}
				for (int i = 0; i < queued; i++){
					
					pthread_mutex_lock(&mutex_window_q[client]);
					data = window_q[client].front();//Get oldest data to send first
					//Cycle Queue, so we push just oldest message to back, it will reach the front once all windowed messages are sent
					window_q[client].push(window_q[client].front());
					window_q[client].pop();
					pthread_mutex_unlock(&mutex_window_q[client]);


					//data = dl_send_q();
					send_data(frame_to_send, frame_expected, data, 0, client);
					frame_to_send++;
					//Reset Timer(s)
					cout<<"Reseting Timer: "<<i<<endl;
					cout<<"Timer: "<<timers[i]<<" Current: "<<current_time()<<" Diff: "<<(current_time()-timers[i])<<endl;
					timers[i]=current_time();
					//clear the queue
				}
				break;
		} //switch(event)
		//cout<<"Event Completed (DL)"<<endl;//Done with that event

		if (queued > MAX_SEQ) cout << "FUCK (DL)"<<endl;
		//STOP putting stuff in the queue, or reset queue.

	} //while(1)
	cout<<"EXITTED WHILE(1) LOOP!!!!!!!!!!!!!! CLIENT: "<<client<<endl;
} //main

//SUPPORT FUNCTIONS//////////////////

//Trigger when event occurs
int wait_for_event(int client){
	int event=0;
	int client_temp;
	while(event<1){
	    client_temp=client;
	    //sleep(1);
	    //cout<<"ALIVE CLIENT: "<<client<<endl;
	    pthread_mutex_lock( &mutex_phy_receive[client] );	
	    pthread_mutex_lock( &mutex_dl_send[client] );
	    if (!phy_receive_q[client].empty())
		event=1;
	    else if (!dl_send_q[client].empty())
		event=2;
	    else if (timeouts())//Need a timeout function
		event=3;
	    pthread_mutex_unlock( &mutex_phy_receive[client] );
	    pthread_mutex_unlock( &mutex_dl_send[client] );
	    if(client_temp!=client)
		cout<<"EEEERRRRRORRRRRRRRRRRRRRRRRRRRRR"<<endl;
	}

	return event;
}

static void send_data(int frame_to_send, int frame_expected, string buff, int type, int client){
	
	//Convert Integers to Characters
	char frame_expected_c[20];
	char frame_to_send_c[20];
	char type_c[20];
	sprintf(frame_expected_c, "%d", frame_expected);
	sprintf(frame_to_send_c, "%d", frame_to_send);
	sprintf(type_c, "%d", type);
		
	string tosend = string(type_c) + '\a' + frame_to_send_c + '\a' + buff;

	pthread_mutex_lock(&mutex_phy_send[client]);
	//cout<<"phy_s_q Client: "<<client<<endl;
	phy_send_q[client].push(tosend);
	//cout<<"Comp"<<endl;
	pthread_mutex_unlock(&mutex_phy_send[client]);

}

//Returns true if a<=b<c, else false.
static bool between(int a, int b, int c){
	if (((a<=b)&&(b<c)) || ((c<a)&&(a<=b)) || ((b<c)&&(c<a)))
		return(true);
	else
		return(false);
}


//Check Timeout
int timeouts(void){

	long current=current_time();
	//Look at times
	for (int i=0;i<queued;i++)
		if ((current-timers[i])>TIMEOUT_MAX){
			cout<<"Timeout occured (DL)"<<endl;
			return 1;//Timeout occured
		}
	//cout<<"No timeouts"<<'\r';
	return 0;//No timeouts

}

//Print out timers
void *time_disp(void* num){
	int old_time[4]={0};
	//Update if times have changed
	while(1)
		if (old_time[0]!=timers[0] || old_time[1]!=timers[1] || old_time[2]!=timers[2] || old_time[3]!=timers[3]){
			cout<<"Timers 1:"<<timers[0]<<" Timers 2:"<<timers[0]<<" Timers 3:"<<timers[0]<<" Timers 4:"<<timers[0]<<'\r'; 
			for (int i=0;i<4;i++)
				old_time[i]=timers[i];//Update old times
		}
}

//Get current time
long current_time(){
	
	struct timeval tv;
        struct timezone tz;
        struct tm *tm;
        gettimeofday(&tv,&tz);
        tm=localtime(&tv.tv_sec);
	long total=(tm->tm_min*100000000+tm->tm_sec*1000000+tv.tv_usec);
	return total;
}


//Deconstruct Frame from PHY Layer
frame deconstruct_frame(char *input){

	char * split = strtok(input, "\a");
	frame buffer2;
	int t=1;
	while (split != NULL){
		if (t==1){
			buffer2.type=atoi(split);
			split=strtok(NULL,"\a");
			}
		else if (t==2){
			buffer2.seq_NUM=atoi(split);
			split=strtok(NULL,"\a");
			}
		else{
			buffer2.data=split;
			break;
		}
		t++;
	}

	return buffer2;
}

