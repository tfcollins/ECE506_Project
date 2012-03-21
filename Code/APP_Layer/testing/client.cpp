//Generic APP_Layer testing
//Need queues
//JES

//Client
#include "all.h"
#define PORTNO 8787		/* Known port number */

using namespace std;

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
	//dl_send(buffer);

	//Wait for login confirmation
	//If successful, logged in
	bool logged_in;
	while (1) {
		//buffer = dl_recv();
		//Not sure what dl_recv will be, for now....
		string received ("loggedin");
		//received = dl_recv();
		if (strcmp(received.c_str(), "loggedin") == 0){
			logged_in = true;
			cout << "Welcome! Type 'help' for chatroom commands" << endl;
		}

		while (logged_in){
			buffer.clear();
			cout << "Enter input: ";
			getline(cin,buffer);
			if (buffer.compare("help") == 0) printhelp();
			else if (buffer.compare("who") == 0) cout << "who";
			else if (buffer.compare("history") == 0) cout << "history";
			else cout << buffer << endl;
			//Check buffer
			int word = count_words(buffer.c_str());
			istringstream totalString( buffer );
			string command, input1;
			totalString >> command >> input1;
			cout << command << endl;
			cout<< input1 << endl;
		}
	}
	/*
	//Get echo back
	cout << "Waiting for response" << endl;
	//Get whatever is in the queue and then a message sent from server
	for (int j = 0; j < 2; j++) {
		bzero(buffer, 256);
		n = read(sockfd, buffer, 255);
		if (n < 0) {
			printf("ERROR reading from socket");
			exit(0);
		}
		//Print received message
		printf("Response: %s\n", buffer);
	}
	close(sockfd);
	*/
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

	int count=1;
	char * token = strtok(str," ");
	while (token != NULL){
		count++;
		token = strtok(NULL," ");
	}

	return count;
}
