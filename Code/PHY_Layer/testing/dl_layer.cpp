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
using namespace std;

//Window Size
#define MAX_SEQ 4
#define MAX_PKT 200

#define PHY 1
#define APP 2
#define TIME_OUT 3

typedef struct{
	int seq_NUM;
	int ack_NUM;
	char data[MAX_PKT];
} frame;

static void send_data(int frame_to_send, int frame_expected, char buff);
int wait_for_event(void);
static bool between(int a, int b, int c);

int main(){
	int frame_to_send = 0;
	int frame_expected = 0;
	int ack_expected = 0;
	int queued = 0;
	frame buffer;

	while (1) {

		int event=wait_for_event();
		switch (event) {

			//If PHY Layer receives message
			case (PHY):
				buffer = phy_receive_q();
				//If input is expected packet
				if (buffer.seq_NUM == frame_expected) {
					to_app_layer(buffer.data);
					frame_expected++;
				}
				//Check Cumulative ACKs
				while (between(ack_expected, buffer.ack_NUM, frame_to_send)) {
					queued = queued--;
					stop_timer(ack_expected);
					ack_expected++;
					phy_receive_q.pop();
				}
				break;

			//If APP Layer wants to send message
			case (APP):
				char data = dl_send_q();
				//Send buffer to physical layer
				//Include seq number for packing
				send_data(frame_to_send, frame_expected, data);
				frame_to_send++;
				break;

			//If No ACK received, and timeout
			case (TIME_OUT):
				frame_to_send = ack_expected;
				for (int i = 0; i <= queued; i++){
					data = dl_send_q();
					send_data(frame_to_send, frame_expected, data);
					frame_to_send++;
					//clear the queue
				}
				break;
		} //switch(event)

		if (queued > MAX_SEQ) cout >> "FUCK";
		//STOP putting stuff in the queue, or reset queue.

	} //while(1)
} //main

int wait_for_event(void){
	 int event=0;

    if (!phy_receive_q.empty())
        event=1;
    else if (!dl_send_q.empty())
        event=2;
    else if (timeout)//Need a timeout function
        event=3;
    else
        wait_for_event();
return event;
}

static void send_data(int frame_to_send, int frame_expected, char buff){
	/*
	frame s;

	s.seq_NUM = frame_to_send;
	s.ack_NUM = frame_expected + MAX_SEQ;
	s.data = buff;

	string tosend;
	tosend = s.seq_NUM + s.ack_NUM + s.data;
	*/
	string tosend = frame_to_send + "#" + frame_expected + "#" + buff;
	phy_send_q.push(tosend);
}

//Returns true if a<=b<c, else false.
static bool between(int a, int b, int c){
	if (((a <= b)&&(b<c)) || ((c<a)&&(a<=b)) || ((b<c)&&(c<a)))
		return(true);
	else
		return(false);
}







