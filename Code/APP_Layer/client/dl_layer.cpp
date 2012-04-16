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
#define MAX_SEQ 4
#define MAX_PKT 200
#define BUFFER_SIZE 128
#define TIMEOUT_MAX 5000000 //fix later, 1 sec = 1000000

#define PHY 1
#define APP 2
#define TIME_OUT 3

typedef struct{
	int type; //0 for non ACK, 1 for ACK
	int seq_NUM;
	char *data; //[MAX_PKT];
} frame;


//function prototypes
static void send_data(int frame_to_send, int frame_expected, string buff, int type);
int wait_for_event(void);
static bool between(int a, int b, int c);
void *time_disp(void* num);
int timeouts(void);
frame deconstruct_frame(string input);
long current_time();
int message_cutter();

//globals
long timers[5]={0};
int queued = 0;
int k;
string data;

queue<string> phy_send_q;
queue<string> phy_receive_q;
queue<string> dl_send_q;
queue<string> dl_receive_q;
queue<string> app_send_q;
queue<string> app_receive_q;
queue<string> window_q;

pthread_mutex_t mutex_phy_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_phy_receive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_dl_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_dl_receive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_app_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_app_receive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_window_q = PTHREAD_MUTEX_INITIALIZER;

//TFC
//Data Link Layer Master Thread
void *dl_layer_client(void *num){
	int frame_to_send = 0;
	int frame_expected = 0;
	int ack_expected = 0;
	int previous_frame_received=3;
	int rc;
	int old_queued=0;
	frame buffer;
	string recv_temp_buff;

	//Initalize Physical Layer
	pthread_t phy_thread;
	rc = pthread_create(&phy_thread, NULL, phy_layer_t, (void *) 1);
	if (rc){
		cout<<"Physical Layer Thread Failed to be created"<<endl;
		exit(1);
	}
	
	//Spawn Timers Status Thread, updates when timers change
	/*pthread_t thread;
        rc = pthread_create(&thread, NULL, time_disp , (void *) 1);
	if (rc){
		cout<<"Something bad happened with thread creation :("<<endl;
		exit(1);
	}*/
	
	//Wait for connected
	verbose("Waiting for connection (DL)");
	while(connected)
		continue;

	//Wait for events to happen
	while (1) {
		int event=wait_for_event();
		switch (event) {

			//If PHY Layer receives message
			case (PHY):
				buffer = deconstruct_frame(phy_receive_q.front());

				//ACK Received
				if (buffer.type){

					//Compare ACK seq number with older seq num in window
					int start=ack_expected;
					int count=0;
					while(1){
						if(start==buffer.seq_NUM)
							break;
						start=(start+1)%4;
						count++;		
					}
					if (count>=3){
						verbose("Duplicate ACK (DL)");
						pthread_mutex_lock(&mutex_phy_receive);
                                        	phy_receive_q.pop();
                                        	pthread_mutex_unlock(&mutex_phy_receive);	
						break;
					}
					if(count>0)
						verbose("Readjusting Known ACKS (DL)");
					for(int h=0;h<=count;h++){
						pthread_mutex_lock(&mutex_window_q);
						window_q.pop();
						pthread_mutex_unlock(&mutex_window_q);
						if (queued==0)
							verbose("Queue Error (DL)");
						queued--;
						ack_expected=(ack_expected+1)%4;
					}
					pthread_mutex_lock(&mutex_phy_receive);
					phy_receive_q.pop();
					pthread_mutex_unlock(&mutex_phy_receive);
				}
				else{//Data Frame Received
		
					//Correct order in sequence
					if ((buffer.seq_NUM==(previous_frame_received+1)%4)||(buffer.seq_NUM==previous_frame_received)){
						//Duplicate
                                                if(buffer.seq_NUM==previous_frame_received){
							verbose("Duplicate Message Received (DL)");
                                                	pthread_mutex_lock(&mutex_phy_receive);
							phy_receive_q.pop();
                                                	pthread_mutex_unlock(&mutex_phy_receive);
                                                        send_data(buffer.seq_NUM, 9, "ACK", 1);//Send ACK
                                                        break;
                                                }
						previous_frame_received=((previous_frame_received+1)%4);
						pthread_mutex_lock(&mutex_phy_receive);
						if (buffer.data[strlen(buffer.data)-1]=='\x88'){
							string temp1=string(buffer.data);
							recv_temp_buff.append(temp1.substr(0,temp1.length()-1));

						}
						else
							recv_temp_buff.append(buffer.data);
						//check if endline character exists
						for (int u=0;u<recv_temp_buff.size();u++)
							if (recv_temp_buff[u]=='\t'){//End of cutup message found
								string str2 = recv_temp_buff.substr (0,recv_temp_buff.length()-1);
								dl_receive_q.push(str2);
								recv_temp_buff.clear();
								}

						phy_receive_q.pop();
						pthread_mutex_unlock(&mutex_phy_receive);
						
						send_data(buffer.seq_NUM, 9, "ACK", 1);//Send ACK
					}
					else{//Drop Packet
						verbose("Data Frame out order, dropping (DL)");
						pthread_mutex_lock(&mutex_phy_receive);
						phy_receive_q.pop();
						pthread_mutex_unlock(&mutex_phy_receive);
						break;
					}
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
				pthread_mutex_lock(&mutex_dl_send);
				data = dl_send_q.front();
				dl_send_q.pop();
				pthread_mutex_unlock(&mutex_dl_send);
				
				pthread_mutex_lock(&mutex_window_q);
				window_q.push(data);//Save if needed for retransmission
				pthread_mutex_unlock(&mutex_window_q);

				//Send buffer to physical layer
				//Include seq number for packing
				timers[queued]=current_time();
				queued++;//cycle to next q
				send_data(frame_to_send, frame_expected, data, 0);
				frame_to_send=((frame_to_send+1)%4);
				break;

			//If No ACK received, timeout, and resend
			case (TIME_OUT):
				//Reset N Frames
				if (queued==0){
					verbose("ERROR Timeout incorrect Queue Size (DL)");
					exit(1);
				}
				int start=0;
				if (queued==1)
					start=(frame_to_send+3)%4;
				else if (queued==2)
					start=(frame_to_send+2)%4;
				else if (queued==3)
					start=(frame_to_send+1)%4;
				else
					start=frame_to_send;
				
				for (int i = 0; i < queued; i++){
					
					pthread_mutex_lock(&mutex_window_q);
					data = window_q.front();//Get oldest data to send first
					//Cycle Queue, so we push just oldest message to back, it will reach the front once all windowed messages are sent
					window_q.push(window_q.front());
					window_q.pop();
					pthread_mutex_unlock(&mutex_window_q);


					//data = dl_send_q();
					send_data((start+i)%4, frame_expected, data, 0);

					//Reset Timer(s)
					verbose("Reseting Timer");
					timers[i]=current_time();
					//clear the queue
				}
				break;
		} //switch(event)
		//cout<<"Event Completed (DL)"<<endl;//Done with that event

		//STOP putting stuff in the queue, or reset queue.

	} //while(1)
} //main

//SUPPORT FUNCTIONS//////////////////

//JES
//Trigger when event occurs
int wait_for_event(void){
	int event=0;
	while(event<1){
	    if (!phy_receive_q.empty())
		event=1;
	    else if (!dl_send_q.empty()){
		event=2;
		message_cutter();
	   }
	    else if (timeouts())//Need a timeout function
		event=3;
	}

	return event;
}

static void send_data(int frame_to_send, int frame_expected, string buff, int type){
	
	//Convert Integers to Characters
	char frame_expected_c[20];
	char frame_to_send_c[20];
	char type_c[20];
	sprintf(frame_expected_c, "%d", frame_expected);
	sprintf(frame_to_send_c, "%d", frame_to_send);
	sprintf(type_c, "%d", type);
		
	string tosend = string(type_c) + '\a' + frame_to_send_c + '\a' + buff;


	pthread_mutex_lock(&mutex_phy_send);
	phy_send_q.push(tosend);
	pthread_mutex_unlock(&mutex_phy_send);
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
			verbose("Timeout occured (DL)");
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
frame deconstruct_frame(string input){
	char * cstr, *split;
	cstr = new char [input.size()+1];
  	strcpy (cstr, input.c_str());
	frame buffer2;
	split = strtok (cstr,"\a");
	int t=1;
	while (split != NULL){
		if (t==1){
			buffer2.type=atoi(split);
			split = strtok (NULL,"\a");
		}
		else if (t==2){
			buffer2.seq_NUM=atoi(split);
			split = strtok (NULL,"\a");
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
//Message Cutter
int message_cutter(){

	pthread_mutex_lock(&mutex_dl_send);
	int E=dl_send_q.size();
	string message;
	string piece;

//	cout<<"Started Cutting"<<endl;

	for (int k=0;k<E;k++){
		message.clear();
		message=dl_send_q.front();
		dl_send_q.pop();
		int number_of_pieces=(int)ceil((double)message.size()/((double)BUFFER_SIZE+1));
		if (number_of_pieces>100)
			verbose("Message being cut into Pieces");
		//Add Delimiters
		for (int i=0;i<number_of_pieces;i++){
			piece.clear();
			if (i==(number_of_pieces-1)){//Last piece

				//Message has already been processed
				if ((message[message.length()-1]=='\t')||(message[message.length()-1]=='\x88')){
					dl_send_q.push(message);
				}
				//Fresh Piece
				else{
					piece=message.substr(i*(BUFFER_SIZE-1),i*BUFFER_SIZE+1+(message.size()%(BUFFER_SIZE+1)));
					piece.append("\t");//end marker
					dl_send_q.push(piece);
				}
			}
			else{
				string str=message.substr(0,BUFFER_SIZE-1);
				//cout<<"First Piece: "<<str<<"|"<<endl;
				dl_send_q.push(str.append("\x88"));//Mid message marker

			}
		}
			
	}
	pthread_mutex_unlock(&mutex_dl_send);

	return 0;
}

