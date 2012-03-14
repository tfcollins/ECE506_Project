#include "all.h"

#include <queue>

using namespace std;

//globals
//MUTEX

//Queues
queue<string> dl_send_q;
queue<string> dl_recv_q;
queue<string> phy_send_q;
queue<string> phy_recv_q;

//APP LAYER
int main(){

	//start all threads
	//phy receive
	//phy send
	//dl receive
	//dl send

	//APP LAYER
	while(1){
		//Do I have something to send?
		if (message2send>0)
			dl_snd(payload);//add to dl_send queue
		if (dl_recv())
				//do something with messages
	}
}


//App layer interface to data link layer (HELPER FUNCTION)
void dl_snd(string payload){
	dl_snd_q.push(payload);
}

//Data link layer interface to app layer (HELPER FUNCTION)
string dl_recv(){

	if (dl_recv_q.size()>0)
		payload=dl_recv_q.front();
		dl_recv_q.pop();
		return payload;
	else
		return "";
}

//Data link layer send thread always running
// Adds data from dl_snd_q (data link layer send queue) to phy_snd_q (physical layer send queue)
// Manages timers
int dl_snd_thread(){
	//run is a global

	//variables
	string payload;

	while(run){
		if(dl_snd_q.size()>0){
			payload=dl_snd_q.front();
			dl_snd_q.pop();//remove from queue

			//cut payload into 256 byte frames with seq #, frame delimiter and other things
			int frames=round(size(payload)/256);
			//add things to frames
			strcat(frame,seqNUM);

			//Add frame to physical layer send queue
			//mutex
			phy_snd_q.push(frame);
			//mutex

			//Set Timer for sequence #
		}
}

//Data link layer thread always running
//Process froms received from phyiscal layer from phy_recv_q (physical layer receive queue)
int dl_recv_thread(){
	int process;
	string buffer[256];

	while(1){
		//Wait until we get a frame from PHY
		if(phy_recv_q.size()>0){
			buffer=phy_recv_q.front();//get message off queue
			phy_recv_q.pop();//remove message from queue

			//SEND ACK
			phy_snd_q.push(ACK);

			//Look for end frame delimiator in buffer
			if (char_in_string(buffer,'\9'))
				process=1;
		}
		//Process frame
		if (process){
			process=0; //reset flag
			//remove delimiators UNSTUFF
			//add to dl receive queue for app layer
			dl_recv_q.push(buffer);
			bzero(buffer,256);//reset buffer
		}
	}
}

//Physical layer receive thread always running
//Add received frames to phy_recv_q (received queue)
int phy_recv_thread(){
	//SETUP SOCKET

	//GET MESSAGES
	while(socket_is_alive){
		//wait at read
		message=read(socket);

		//CRC Check

		//ADD MESSAGE TO PHY RECEIVE QUEUE
		//mutex
		phy_recv_q.push(message);
		//mutex
	}
}


//Physical Layer send thread always running
int phy_snd_thread(){

	//MAKE SURE SOCKET SETUP
	while(socket_is_alive){
		//WAIT FOR SOMETHING TO BE SENT
		if (phy_snd_q.size()>0){
			//GET MESSAGE OFF QUEUE
			message=phy_snd_q.front();
			phy_snd_q.pop();

			//CREATE CRC

			//SEND
			//MUTEX
			write(message);
			//MUTEX

			}
		}


	}

}


