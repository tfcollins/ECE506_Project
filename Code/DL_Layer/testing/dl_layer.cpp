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
#define TIMEOUT_MAX 800000 //fix later

#define PHY 1
#define APP 2
#define TIME_OUT 3

//NOTES
/*




*/

typedef struct{
	int seq_NUM;
	int ack_NUM;
	char data[MAX_PKT];
} frame;

int timers[4]={0};
int queued = 0;



static void send_data(int frame_to_send, int frame_expected, char buff);
int wait_for_event(void);
static bool between(int a, int b, int c);
void *time_disp(void* num);
int timeouts(void);
frame deconstruct_frame(string input);


int main(){
	int frame_to_send = 0;
	int frame_expected = 0;
	int ack_expected = 0;
	int rc;
	frame buffer;

	//Spawn Timers Status Thread
	pthread_t thread;
        rc = pthread_create(&thread, NULL, time_disp , (void *) 1);
	if (rc){
		cout<<"Something bad happened with thread creation :("<<endl;
		exit(1);
	}

	while (1) {

		int event=wait_for_event();
		switch (event) {

			//If PHY Layer receives message
			case (PHY):
				buffer = deconstruct_frame(phy_receive_q.front());
				//If input is expected packet
				if (buffer.seq_NUM == frame_expected) {
					to_app_layer(buffer.data);
					frame_expected++;
				}
				//Check Cumulative ACKs
				int k=0;
				while (between(ack_expected, buffer.ack_NUM, frame_to_send)) {
					//stop timer
					timers[k]=999999999999;//reset timer
					k++;
					if (queued==0)
						queued=3;
					else
						queued--;
					ack_expected++;
					//phy_receive_q.pop();
				}
				break;

			//If APP Layer wants to send message
			case (APP):
				char data = dl_send_q.front();
				//Send buffer to physical layer
				//Include seq number for packing
				send_data(frame_to_send, frame_expected, data);
				frame_to_send++;
				queued=(queued++)%4;//cycle to next q
				break;

			//If No ACK received, and timeout
			case (TIME_OUT):
				frame_to_send = ack_expected;
				//Reset N Frames
				for (int i = 0; i <= queued; i++){
					data = dl_send_q();
					send_data(frame_to_send, frame_expected, data);
					frame_to_send++;
					//Reset Timer(s)
					timers[i]=current_time();
					//clear the queue
				}
				break;
		} //switch(event)

		if (queued > MAX_SEQ) cout << "FUCK"<<endl;
		//STOP putting stuff in the queue, or reset queue.

	} //while(1)
} //main


//Trigger when event occurs
int wait_for_event(void){
	int event=0;

    if (!phy_receive_q.empty())
        event=1;
    else if (!dl_send_q.empty())
        event=2;
    else if (timeouts())//Need a timeout function
        event=3;
    else
        wait_for_event();
return event;
}

static void send_data(int frame_to_send, int frame_expected, string buff){
	/*
	frame s;

	s.seq_NUM = frame_to_send;
	s.ack_NUM = frame_expected + MAX_SEQ;
	s.data = buff;

	string tosend;
	tosend = s.seq_NUM + s.ack_NUM + s.data;
	*/
	string tosend = itoa(frame_to_send) + '\a' + itoa(frame_expected) + '\a' + buff;
	//phy_send_q.push(tosend);
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

	int current=current_time();
	//Look at times
	for (int i=0;i<queued;i++)
		if ((current-timers[i])>TIMEOUT_MAX)
			return 1;//Timeout occured

	return 0;//No timeouts

}

//Print out timers
void *time_disp(void* num){
	int old_time[4]={0};
	//Update if times have changed
	while(1)
		if (old_time[0]!=timers[0] || old_time[1]!=timers[1] || old_time[2]!=timers[2] || old_time[3]!=timers[3]){
			cout<<"Timers 1:"<<timers[0]<<"Timers 2:"<<timers[0]<<"Timers 3:"<<timers[0]<<"Timers 4:"<<timers[0]<<'\r'; 
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
	long total=(tm->tm_min*1000000000+tm->tm_sec*1000000+tv.tv_usec);
	return total;
}


//Deconstruct Frame from PHY Layer
frame deconstruct_frame(string input){

        frame buffer;
        int i=0;
        char temp[200];

        //Find  Sequence Num
        for (i=0;i<input.size();i++){
                if (input[i]=='\a'){
                        cout<<atoi(temp)<<endl;
                        buffer.seq_NUM=atoi(temp);
                        cout<<"BREAK"<<endl;
                        break;
                }
                else
                        temp[i]=input[i];
        }
        //Find ACK Number
        char temp2[200];//clear
        i++;
        int j=i;
        while(i<input.size()){
                if (input[i]=='\a'){
                        cout<<atoi(temp2)<<endl;
                        buffer.ack_NUM=atoi(temp2);
                        cout<<"BREAK"<<endl;
                        i++;
                        break;
                }
                else{
                        temp2[i-j]=input[i];
                        i++;
                }
        }
        //Find Message
        j=i;
        while(i<(input.size())){
                cout<<input[i]<<":"<<(i-j)<<endl;
                buffer.data[i-j]=input[i];
                i++;
        }

        return buffer;
}



