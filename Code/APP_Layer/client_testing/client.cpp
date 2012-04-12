//Client APP_Layer
//JES

//Client
#include "all.h"
#include <cmath>

#define DELIM " "
#define MAX_BUFF 256

int PORT = 9323;		/* Known port number */
char* HOSTNAME;
int vb_mode=0;

//Function Prototypes
void *recv_display(void *num);
void split_up_message(string to_split);

using namespace std;

int main(int argc, char *argv[]){

	int sockfd, n;
	struct sockaddr_in serv_addr;

	//Ensure correct number of inputs
	if ((argc < 7) || (argc > 8)) {
		fprintf(stderr, "usage:\n %s <server> login <username> <age> <location> <hobby>\n", argv[0]);
		exit(0);
	}
	if (argc == 8) vb_mode = atoi(argv[7]);

	//If not login
	if (strcmp(argv[2],"login") != 0) {
		cout << "Please login!\n";
		fprintf(stderr, "usage:\n %s <server> login <username> <age> <location> <hobby>\n", argv[0]);
		exit(0);
	}

	//Profile and username information
	string username (argv[3]);
	string age (argv[4]);
	string location (argv[5]);
	string hobby (argv[6]);

	//Build login and profile information buffer
	string buffer;
	buffer = "login " + username + " " + age + " " + location + " " + hobby + "\x89";

	//Check if input longer than 256
	if (buffer.size() > 256) diewithError("Login must be less than 256 bytes!");

	//Setup and connect to server
	HOSTNAME = argv[1];

	//Start Data Link and Physical Layers
	pthread_t dl_thread;
	int rc;
    rc = pthread_create(&dl_thread, NULL, dl_layer_client, (void *) 1);
    if (rc){
        cout<<"Data Link Layer Thread Failed to be created"<<endl;
        exit(1);
    }

	//TO DATA LINK LAYER
    verbose("Sending Login (APP)");
    pthread_mutex_lock(&mutex_app_send);
    dl_send_q.push(buffer);
    pthread_mutex_unlock(&mutex_app_send);

	//Wait for login confirmation
	bool logged_in=0;
	while (1) {
		pthread_mutex_lock(&mutex_dl_receive);
		if(!dl_receive_q.empty()){
			verbose("RECEIVE_Q NOT EMPTY (APP)");
			string received;
			received = dl_receive_q.front();
			verbose("RECEIVED '" + received + "' (APP)");
			dl_receive_q.pop();

			//Must be a confirmed login from server
			if (strcmp(received.c_str(), "loggedin") == 0){
				pthread_t recv_thread;
				int rc = pthread_create(&recv_thread, NULL, recv_display, (void *) 1);
				if (rc){
					cout<<"Receive thread failed to be created"<<endl;
					exit(1);
				}

				logged_in = true;
				cout << "Welcome! Type 'help' for chatroom commands" << endl;
			}
		}
		pthread_mutex_unlock(&mutex_dl_receive);

		//Logged in
		while (logged_in){
			buffer.clear();

			//Read in user input
			cout << "Enter input: ";
			getline(cin,buffer);

			istringstream iss(buffer);
			string command, input, tosend, message;
			iss >> command;

			while (iss >> input){
				message = message + input +" ";
			}

			//Determine if possible input: Error Checking
			char *buffin = &buffer[0];
			int word = count_words(buffin);

			//Verifies correct input, else prints help function
			int lessthan=0;
			int morethan=0;
			if((command.compare("who") == 0) && (word == 1)){
				lessthan = true;
				tosend = command;
			}
			else if ((command.compare("send") == 0) && (word >= 2)){
				tosend = command + " " + message;
				if (tosend.size() > MAX_BUFF-1) morethan = true;
				else lessthan = true;
			}
			else if ((command.compare("history") == 0) && (word == 1)){
				lessthan = true;
				tosend = command;
			}
			else if ((command.compare("what") == 0) && (word == 2)){
				lessthan = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("upload") == 0) && (word == 2)){
				lessthan = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("get") == 0) && (word == 2)){
				lessthan = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("logout") == 0) && (word == 1)){
				//go = true;
				tosend = command;
				//pthread_cancel(recv_thread);
				exit(0);
			}
			else printhelp();

			//Sends to data link layer if input acceptable
			//Will have to add additional sections, based on message size
			if(lessthan){
				verbose("Sending '" + tosend + "' to server. (APP)");
				pthread_mutex_lock(&mutex_dl_send);
				dl_send_q.push(tosend + "\x89");
				pthread_mutex_unlock(&mutex_dl_send);
				tosend.clear();
			}
			if(morethan){
				split_up_message(tosend);
			}
		}
	}
	return 0;
}

void diewithError(string message) {
        cout << message << endl;
        exit(1);
}

void printhelp(void){
	cout << "***************************" << endl;
	cout << "Possible commands and usage:" << endl;
	cout << "To view other chatroom users..." << endl;
	cout << "	who" << endl;
	cout << "To send chat messages..." << endl;
	cout << "	send <message>" << endl;
	cout << "To receive chat history..." << endl;
	cout << "	history" << endl;
	cout << "To see online user's details..." << endl;
	cout << "	what <username>" << endl;
	cout << "To upload a file..." << endl;
	cout << "	upload <image.jpg>" << endl;
	cout << "To download hosted files..." << endl;
	cout << "	get <image.jpg>" <<endl;
	cout << "To logout of the chatroom..." << endl;
	cout << "	logout" << endl;
	cout << "***************************" << endl;
}

int count_words(char *str){

	int count=0;
	char * token = strtok(str," ");
	while (token != NULL){
		count++;
		token = strtok(NULL," ");
	}

	return count;
}

void split_up_message(string to_split){
	string tosend = "";

	int pieces=(int)ceil((double)to_split.size()/(double)MAX_BUFF);

	pthread_mutex_lock(&mutex_dl_send);
	for(int i = 0; i < pieces -1; i++){
		tosend.clear();
		tosend = to_split.substr(i*MAX_BUFF,(i+1)*MAX_BUFF - 1);
		dl_send_q.push(tosend);
	}
	tosend.clear();
	tosend = to_split.substr((pieces-1)*MAX_BUFF, to_split.length());
	tosend = tosend + '\x87';
	cout << tosend << endl;
	dl_send_q.push(tosend);
	pthread_mutex_unlock(&mutex_dl_send);
}

//Additional thread that checks and displays messages to user
void *recv_display(void *num){
	verbose("\nStarted Recv_Display thread");
	while(1){
		pthread_mutex_lock(&mutex_dl_receive);
		if(!dl_receive_q.empty()){

			string recvd = dl_receive_q.front();
			dl_receive_q.pop();

			cout << "\nMessage received from server!" << endl;
			cout << "'" + recvd + "'" << endl;
			cout << "Please continue...\nEnter input:" << endl;
		}
		pthread_mutex_unlock(&mutex_dl_receive);
		//sleep(3);
		//cout << "\nSmelly" << endl;
	}
	return 0;
}
