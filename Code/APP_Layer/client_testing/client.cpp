//Client APP_Layer
//JES

//Client
#include "all.h"
#define DELIM " "

int PORT = 8787;		/* Known port number */
char* HOSTNAME;
int vb_mode=0;

using namespace std;
void *recv_display(void *num);

int main(int argc, char *argv[]) {
	int sockfd, n;
	struct sockaddr_in serv_addr;

	//Ensure correct number of inputs
	if ((argc < 7) || (argc > 8)) {
		fprintf(stderr, "usage:\n %s <server> login <username> <age> <location> <hobby>\n", argv[0]);
		exit(0);
	}
	if (argc == 8) vb_mode = atoi(argv[8]);

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
	buffer = "login " + username + " " + age + " " + location + " " + hobby;

	//Check if input longer than 256
	//Eventually split up if larger than 256
	if (buffer.size() > 256) diewithError("Greater than 256 bytes");

	//Setup and connect to server
	HOSTNAME = argv[1];
	//if (HOSTNAME == NULL) diewithError("Error, no such host!");

	pthread_t dl_thread;
	int rc;
    rc = pthread_create(&dl_thread, NULL, dl_layer_client, (void *) 1);
    if (rc){
        cout<<"Data Link Layer Thread Failed to be created"<<endl;
        exit(1);
    }

	//TO DATA LINK LAYER
    cout<<"Sending Login (APP)"<<endl;
    pthread_mutex_lock(&mutex_app_send);
    dl_send_q.push(buffer);
    pthread_mutex_unlock(&mutex_app_send);

	//Wait for login confirmation
	bool logged_in;
	while (1) {
		pthread_mutex_lock(&mutex_dl_receive);
		if(!dl_receive_q.empty()){

			string received;
			received = dl_receive_q.front();
			dl_receive_q.pop();

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
			pthread_mutex_unlock(&mutex_dl_receive);
		}

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

			//Determine if possible input
			char *buffin = &buffer[0];
			int word = count_words(buffin);

			//Verifies correct input, else prints help function
			int go=0;
			if((command.compare("who") == 0) && (word == 1)){
				go = true;
				tosend = command;
			}
			else if ((command.compare("send") == 0) && (word >= 2)){
				go = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("history") == 0) && (word == 1)){
				go = true;
				tosend = command;
			}
			else if ((command.compare("what") == 0) && (word == 2)){
				go = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("upload") == 0) && (word == 2)){
				go = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("get") == 0) && (word == 2)){
				go = true;
				tosend = command + " " + message;
			}
			else if ((command.compare("logout") == 0) && (word == 1)){
				go = true;
				tosend = command;
			}
			else printhelp();

			//Sends to data link layer if input acceptable
			//Will have to add additional sections, based on message size
			if(go){
				cout << "Sending '" << tosend << "' to server. (APP)" << endl;
				tosend = tosend + '\f';
				cout<<"Sending Login (APP)"<<endl;
				pthread_mutex_lock(&mutex_dl_send);
				dl_send_q.push(tosend);
				pthread_mutex_unlock(&mutex_dl_send);
				tosend.clear();
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

//Additional thread that checks and displays messages to user
void *recv_display(void *num){
	cout << "\nStarted Recv_Display thread" << endl;
	while(1){
		if(!dl_receive_q.empty()){
			pthread_mutex_lock(&mutex_dl_receive);
			string recvd = dl_receive_q.front();
			dl_receive_q.pop();
			pthread_mutex_unlock(&mutex_dl_receive);

			cout << "\nMessage received from server!" << endl;
			cout << "'" + recvd + "'" << endl;
			cout << "Please continue...\nEnter input:";
		}
		//sleep(3);
		//cout << "\nSmelly" << endl;
	}
	return 0;
}
