/*
 * dl_layer.cpp
 *
 *  Created on: Mar 20, 2012
 */

#include "all.h"
#include <sys/time.h>
#include <cmath>

using namespace std;

//Window Size
#define BUFFER_SIZE 128
#define MAX_SEQ 4
#define MAX_PKT 200
#define TIMEOUT_MAX 8000000 //fix later, 1 sec = 1000000

#define PHY 1  //RECEIVE
#define APP 2  //SEND
#define TIME_OUT 3

typedef struct{
	int type; //0 for non ACK, 1 for ACK
	int seq_NUM;
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
int message_cutter(int client);

//globals
long timers[4]={0};
int queued = 0;
int k;

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

int previous_frame_received[20]={3};

//TFC
//Data Link Layer Master Thread
void *dl_layer_server(void *client_num){

	//Set User
	int *client_temp = (int *) client_num;
	int const client = *client_temp;
	
	pthread_mutex_t mutex_prev_seq_num = PTHREAD_MUTEX_INITIALIZER;
	int frame_to_send = 0;
	int frame_expected = 0;
	int ack_expected = 0;
	int old_queued=0;
	int rc;
	frame buffer;
	string recv_temp_buff;
	string data;
	string input;
	char *tmp;
	previous_frame_received[client]=3;

	
	//Spawn Timers Status Thread, updates when timers change
	/*pthread_t thread;
        rc = pthread_create(&thread, NULL, time_disp , (void *) 1);
	if (rc){
		cout<<"Something bad happened with thread creation :("<<endl;
		exit(1);
	}*/


	//Wait for events to happen, then loop again
	while (1) {
		int event=wait_for_event(client);
		switch (event) {

			//If PHY Layer receives message
			case (PHY):
				pthread_mutex_lock(&mutex_phy_receive[client]);
				input = phy_receive_q[client].front();
				pthread_mutex_unlock(&mutex_phy_receive[client]);


				tmp = &input[0];
				buffer = deconstruct_frame(tmp);

				//ACK Received
				if (buffer.type){
					int start=frame_expected;
					int count=0;
					while(1){//Determine if acks need to be readjusted
						if (start==buffer.seq_NUM)
							break;
						start=(start+1)%4;
						count++;
					}
					//Remove ACK(s)
					for(int h=0;h<=count;h++){
						pthread_mutex_lock(&mutex_window_q[client]);
						if (!window_q[client].empty())
							window_q[client].pop();
						pthread_mutex_unlock(&mutex_window_q[client]);
						verbose("Queue Reduced in size (DL)");
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
					if ((buffer.seq_NUM==((previous_frame_received[client]+1)%4))||(buffer.seq_NUM==previous_frame_received[client])){
						//Correctly Ordered Frame
						//Duplicate	
						if(buffer.seq_NUM==previous_frame_received[client]){
						 	verbose("ERROR: Duplicate Message Received (DL)");
                                                        pthread_mutex_lock(&mutex_phy_receive[client]);
                                                        phy_receive_q[client].pop();
                                                        pthread_mutex_unlock(&mutex_phy_receive[client]);
							send_data(buffer.seq_NUM, 9, "ACK", 1, client);//Send ACK
							break;
						} 
						//increment frame number
						previous_frame_received[client]=((previous_frame_received[client]+1)%4);
						//Remove delimiters
						pthread_mutex_lock(&mutex_dl_receive[client]);
						if (buffer.data[strlen(buffer.data)-1]=='\x88'){
                                                        string temp1=string(buffer.data);
                                                        recv_temp_buff.append(temp1.substr(0,temp1.length()-1));
                                                }
                                                else
							recv_temp_buff.append(buffer.data);
                                                //check if endline character exists
                                                for (int u=0;u<recv_temp_buff.size();u++)
                                                        if (recv_temp_buff[u]=='\t'){
								string str2 = recv_temp_buff.substr (0,recv_temp_buff.length()-1);
                                                                dl_receive_q[client].push(str2);
							
                                                                recv_temp_buff.clear();
                                                                }
						pthread_mutex_unlock(&mutex_dl_receive[client]);
						//remove from queue	
						pthread_mutex_lock(&mutex_phy_receive[client]);
						phy_receive_q[client].pop();
						pthread_mutex_unlock(&mutex_phy_receive[client]);

						send_data(buffer.seq_NUM, 9, "ACK", 1, client);//Send ACK
					}
					else{//Drop Packet
						verbose("ERROR: Data Frame out order, dropping (DL)");
						pthread_mutex_lock(&mutex_phy_receive[client]);
						phy_receive_q[client].pop();
						pthread_mutex_unlock(&mutex_phy_receive[client]);
					}
					pthread_mutex_unlock(&mutex_prev_seq_num);
				
				}
				break;

			//If APP Layer wants to send message
			case (APP):
				if (queued >= MAX_SEQ){
                                        if (old_queued!=queued){
                                                verbose("Queue Maxed, must wait for ACK (DL)");
                                                old_queued=queued;//Used to display messages only when queue changes in size
                                        }
                                        break;
                                }
				pthread_mutex_lock(&mutex_dl_send[client]);
				data=dl_send_q[client].front();
				dl_send_q[client].pop();
				pthread_mutex_unlock(&mutex_dl_send[client]);

				pthread_mutex_lock(&mutex_window_q[client]);
				window_q[client].push(data);//Save if needed for retransmission
				pthread_mutex_unlock(&mutex_window_q[client]);
				//Send buffer to physical layer
				//Include seq number for packing
				timers[queued]=current_time();
				queued++;//cycle to next q
				send_data(frame_to_send, frame_expected, data, 0, client);
				frame_to_send=((frame_to_send+1)%4);
				break;

			//If No ACK received, and timeout
			case (TIME_OUT):
				//frame_to_send = ack_expected;
				//Reset N Frames
				if (queued==0){
					cout<<"ERROR: Timeout incorrect Queue Size"<<endl;
					exit(1);
				}
				//Determine how many messages need to be sent
				int start=0;
                                if (queued==1)
                                        start=(frame_to_send+3)%4;
                                else if (queued==2)
                                        start=(frame_to_send+2)%4;
                                else if (queued==3)
                                        start=(frame_to_send+1)%4;
                                else
                                        start=frame_to_send;
				//Send Queued messages
				for (int i = 0; i < queued; i++){
					
					pthread_mutex_lock(&mutex_window_q[client]);
					//Get oldest data to send first	
					if (!window_q[client].empty()){
						data = window_q[client].front();//Get oldest data to send first
						//Cycle Queue, so we push just oldest message to back, it will reach the front once all windowed messages are sent
						window_q[client].push(window_q[client].front());
						window_q[client].pop();
					}
					pthread_mutex_unlock(&mutex_window_q[client]);


					send_data(start+i, frame_expected, data, 0, client);
					//Reset Timer(s)
					timers[i]=current_time();
					//clear the queue
				}
				break;
		} //switch(event)
		//cout<<"Event Completed (DL)"<<endl;//Done with that event

		if (queued > MAX_SEQ) cout << "Something went terribly wrong (DL)"<<endl;
		//STOP putting stuff in the queue, or reset queue.

	} //while(1)
	cout<<"EXITTED WHILE(1) LOOP, Something went really really wrong on client: "<<client<<endl;
} //main

//SUPPORT FUNCTIONS//////////////////

//TFC
//Trigger when event occurs
int wait_for_event(int client){
	//cout<<"WAIT FOR EVENT"<<endl;
	int event=0;
	int client_temp;
	while(event<1){
	    client_temp=client;
	    //sleep(1);//debugging
	    pthread_mutex_lock( &mutex_phy_receive[client] );	
	    if (!phy_receive_q[client].empty()){
		event=1;
		//cout<<"Something in recevie QUEUE"<<endl;
	    	pthread_mutex_unlock( &mutex_phy_receive[client] );	
		break;
	    }
	    else{
	    	pthread_mutex_unlock( &mutex_phy_receive[client] );	
		//cout<<"Nothing in queue"<<endl;
	    }
	    pthread_mutex_lock( &mutex_dl_send[client] );
	    if (!dl_send_q[client].empty()){//Something to send for APP
		event=2;
		message_cutter(client);
	    	pthread_mutex_unlock( &mutex_dl_send[client] );
		break;
	    }
	    else{
	    	pthread_mutex_unlock( &mutex_dl_send[client] );
	    }
	    if (timeouts()){//Need a timeout function
		event=3;
	    }
	    else{
		//cout<<"No timeouts"<<endl;
	    }
	}
	//cout<<"Event Occured"<<endl;
	return event;
}

//TFC
//Add delimiters and send to phy layer
static void send_data(int frame_to_send, int frame_expected, string buff, int type, int client){
	
	//Convert Integers to Characters
	char frame_expected_c[20];
	char frame_to_send_c[20];
	char type_c[20];
	sprintf(frame_expected_c, "%d", frame_expected);
	sprintf(frame_to_send_c, "%d", frame_to_send);
	sprintf(type_c, "%d", type);
	
	//Add Demlimiters	
	string tosend = string(type_c) + '\a' + frame_to_send_c + '\a' + buff;

	pthread_mutex_lock(&mutex_phy_send[client]);
	phy_send_q[client].push(tosend);
	pthread_mutex_unlock(&mutex_phy_send[client]);

}

//Returns true if a<=b<c, else false.
static bool between(int a, int b, int c){
	if (((a<=b)&&(b<c)) || ((c<a)&&(a<=b)) || ((b<c)&&(c<a)))
		return(true);
	else
		return(false);
}

//TFC
//Check Timeout
int timeouts(void){

	long current=current_time();
	//Look at times
	for (int i=0;i<queued;i++)
		if ((current-timers[i])>TIMEOUT_MAX){
			verbose("ERROR: Timeout occured (DL)");
			return 1;//Timeout occured
		}
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

//TFC
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

//TFC
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

//TFC
//Message Cutter, for messages > 128
int message_cutter(int client){

        int E=dl_send_q[client].size();
        string message;
        string piece;

	for (int k=0;k<E;k++){
                message.clear();
                message=dl_send_q[client].front();
                dl_send_q[client].pop();
                int number_of_pieces=(int)ceil((double)message.size()/((double)BUFFER_SIZE+1));
                if (number_of_pieces>1)
                        verbose("Message being cut into Pieces");
                //Add Delimiters
                for (int i=0;i<number_of_pieces;i++){
                        piece.clear();
                        if (i==(number_of_pieces-1)){//Last piece

                                //Message has already been processed
                                if ((message.find("\t")<256)||(message.find("\x88")<256)){
                                        dl_send_q[client].push(message);
                                       // cout<<"Message Skipped"<<endl;
                                }
                                //Fresh Piece
                                else{
                                        piece=message.substr(i*(BUFFER_SIZE-1),i*BUFFER_SIZE+1+(message.size()%(BUFFER_SIZE+1)));
                                        piece.append("\t");//end marker
                                        dl_send_q[client].push(piece);
                                }
                        }
                        else{
                                string str=message.substr(0,BUFFER_SIZE-1);
                                //First Piece
                                dl_send_q[client].push(str.append("\x88"));//Mid message marker

                        }
                }

        }

        return 0;
}

