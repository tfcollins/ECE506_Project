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
#include <cmath>

using namespace std;

//Window Size
#define BUFFER_SIZE 128
#define MAX_SEQ 4
#define MAX_PKT 200
#define TIMEOUT_MAX 3000000 //fix later, 1 sec = 1000000

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
	int old_queued=0;
	int rc;
	frame buffer;
	string recv_temp_buff;
	string data;

	
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
		int event=wait_for_event(client);
		switch (event) {

			//If PHY Layer receives message
			case (PHY):
				pthread_mutex_lock(&mutex_phy_receive[client]);
				input = phy_receive_q[client].front();
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
					//cout<<"Message: "<<buffer.data<<"|"<<endl;
					//cout<<"Prev Seq: "<<previous_frame_received[client]<<" Recvd Seq: "<<buffer.seq_NUM<<" Client: "<<client<<endl;
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

						previous_frame_received[client]=((previous_frame_received[client]+1)%4);
						
						pthread_mutex_lock(&mutex_dl_receive[client]);
						if (buffer.data[strlen(buffer.data)-1]=='\x88'){
                                                        string temp1=string(buffer.data);
                                                        recv_temp_buff.append(temp1.substr(0,temp1.length()-1));

                                                }
                                                else
							recv_temp_buff.append(buffer.data);
                                                //check if endline character exists
                                                for (int u=0;u<recv_temp_buff.size();u++)
                                                        if (recv_temp_buff[u]=='\?'){
								//verbose("Found Delimiter (DL)");
								string str2 = recv_temp_buff.substr (0,recv_temp_buff.length()-1);
								verbose("Resulting message: "+str2+" (DL)");
                                                                dl_receive_q[client].push(str2);
							
                                                                recv_temp_buff.clear();
                                                                }
						pthread_mutex_unlock(&mutex_dl_receive[client]);
	
						pthread_mutex_lock(&mutex_phy_receive[client]);
						phy_receive_q[client].pop();
						pthread_mutex_unlock(&mutex_phy_receive[client]);

						send_data(buffer.seq_NUM, 9, "ACK", 1, client);//Send ACK
						//cout<<"Sending ACK (DL)"<<endl;
					}
					else{//Drop Packet
						verbose("ERROR: Data Frame out order, dropping (DL)");
						pthread_mutex_lock(&mutex_phy_receive[client]);
						phy_receive_q[client].pop();
						pthread_mutex_unlock(&mutex_phy_receive[client]);
						//break;//not sure if needed
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
				//cout<<"Wants to send: "<<data<<" (DL)"<<endl;
				dl_send_q[client].pop();
				pthread_mutex_unlock(&mutex_dl_send[client]);

				pthread_mutex_lock(&mutex_window_q[client]);
				//cout<<"Added to window"<<endl;
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
				frame_to_send = ack_expected;
				//Reset N Frames
				if (queued==0){
					//cout<<"ERROR: Timeout incorrect Queue Size"<<endl;
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
					//verbose("Reseting Timer: "+string(i));;
					//verbose("Timer: "+string(timers[i])+" Current: "+string(current_time())+" Diff: "+string(current_time()-timers[i]));
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
	//cout<<"WAIT FOR EVENT"<<endl;
	int event=0;
	int client_temp;
	while(event<1){
	    client_temp=client;
	    //sleep(1);
	    pthread_mutex_lock( &mutex_phy_receive[client] );	
	    if (!phy_receive_q[client].empty()){
		event=1;
	    }
	    else{
		//cout<<"Nothing in queue"<<endl;
	    }
	    pthread_mutex_unlock( &mutex_phy_receive[client] );	
	    pthread_mutex_lock( &mutex_dl_send[client] );
	    if (!dl_send_q[client].empty()){
		event=2;
		message_cutter(client);
	    	pthread_mutex_unlock( &mutex_dl_send[client] );
	    }
	    else{
		//cout<<"Nothing in queue 2"<<endl;
	    }
	    pthread_mutex_unlock( &mutex_dl_send[client] );
	    if (timeouts()){//Need a timeout function
		//cout<<"Event3";	
		event=3;
	    }
	    else{
		//cout<<"No timeouts"<<endl;
	    }
	    //pthread_mutex_unlock( &mutex_phy_receive[client] );
	    //pthread_mutex_unlock( &mutex_dl_send[client] );
	    //if(client_temp!=client)
	//	cout<<"EEEERRRRRORRRRRRRRRRRRRRRRRRRRRR"<<endl;
	//cout<<"DLLLLLLL WAIT LOOP\r";
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

	//cout<<"Sending to phy: "<<tosend<<" Size: "<<tosend.size()<<endl;
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


//Message Cutter
int message_cutter(int client){

        //pthread_mutex_lock(&mutex_dl_send[client]);
        int i=dl_send_q[client].size();
        string message;
        string piece;

        for (int k=0;k<i;k++){
                message.clear();
                message=dl_send_q[client].front();
                dl_send_q[client].pop();
		//cout<<"Message Size:"<<message.size();
                int number_of_pieces=(int)ceil((double)message.size()/(double)BUFFER_SIZE);
		//cout<<"Number of pieces: "<<number_of_pieces<<endl;
                for (int i=0;i<number_of_pieces;i++){
                        piece.clear();
                        if (i==(number_of_pieces-1)){
                                piece=message.substr(i*BUFFER_SIZE,(i)*BUFFER_SIZE+(message.size()%(BUFFER_SIZE+1)));
                                if (piece[piece.length()-1]!='\?')
					if (piece[piece.length()-1]!='\x88') 
						piece.append("\?");//end marker
                                dl_send_q[client].push(piece);
                        }
                        else{
				string str=message.substr(i*BUFFER_SIZE,(i+1)*BUFFER_SIZE);
                                dl_send_q[client].push(str.append("\x88"));
			}
                }

        }
        //pthread_mutex_unlock(&mutex_dl_send[client]);
	//cout<<"Size after cutter: "<<dl_send_q[client].size()<<endl;

        return 0;
}

