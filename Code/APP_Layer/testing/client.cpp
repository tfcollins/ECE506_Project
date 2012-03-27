//Generic APP_Layer testing
//Need queues
//JES

//Client
#include "all.h"
#define DELIM " "
#define PORTNO 8787		/* Known port number */

using namespace std;
void *recv_display(void *num);

int main(int argc, char *argv[]) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	//Ensure correct number of inputs
	if (argc != 7) {
		fprintf(stderr, "usage:\n %s server login <username> <location> <age> <hobby>\n", argv[0]);
		exit(0);
	}
	//If not login
	if (strcmp(argv[2],"login") != 0) {
		cout << "Please login!\n";
		fprintf(stderr, "usage:\n %s server login <username> <location> <age> <hobby>\n", argv[0]);
		exit(0);
	}

	//Profile and username information
	string username (argv[3]);
	string location (argv[4]);
	string job (argv[5]);
	string hobby (argv[6]);

	//Build login and profile information buffer
	string buffer;
	buffer = "#" + username + "#" + location + "#" + job + "#" + hobby;

	//Check if input longer than 256
	//Eventually split up if larger than 256
	if (buffer.size() > 256) diewithError("Greater than 256 bytes");

	//Setup and connect to server
	server = gethostbyname(argv[1]);
	if (server == NULL) diewithError("Error, no such host!");
	//sockfd = phy_setup(PORTNO, server);

	//TO DATA LINK LAYER
	//dl_send_q.push(buffer);

	//Wait for login confirmation
	bool logged_in;
	while (1) {
		//buffer = dl_recv();
		//Not sure what dl_recv will be, for now....
		string received ("loggedin");
		//received = dl_recv();
		if (strcmp(received.c_str(), "loggedin") == 0){
			pthread_t recv_thread;
			int rc = pthread_create(&recv_thread, NULL, recv_display, (void *) 1);
			if (rc){
				cout<<"Physical Layer Thread Failed to be created"<<endl;
				exit(1);
			}

			logged_in = true;
			cout << "Welcome! Type 'help' for chatroom commands" << endl;
		}

		while (logged_in){
			buffer.clear();
			//Read in user input
			cout << "Enter input: ";
			getline(cin,buffer);
			//cout << buffer << endl;

			istringstream iss(buffer);
			string command, input;
			iss >> command;

			//Determine if possible input
			char *buffin = &buffer[0];
			int word = count_words(buffin);

			int go=0;
			if((command.compare("who") == 0) && (word == 1)) go = true;
			else if ((command.compare("send") == 0) && (word >= 2)){
				go = true;
				iss >> input;
			}
			else if ((command.compare("history") == 0) && (word == 1)) go = true;
			else if ((command.compare("what") == 0) && (word == 2)) go = true;
			else if ((command.compare("upload") == 0) && (word == 2)) go = true;
			else if ((command.compare("get") == 0) && (word == 2)) go = true;
			else if ((command.compare("logout") == 0) && (word == 1)) go = true;
			else printhelp();

			if(go){
				cout << "Sending '" << input << "' to server." << endl;
				//dl_send_q.push(buffer);

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

void *recv_display(void *num){
	cout << " MY NIGGA " << endl;
	while(1){
		sleep(3);
		//cout << "\nSmelly" << endl;
	}
	return 0;
}
