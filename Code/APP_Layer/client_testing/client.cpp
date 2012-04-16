//Client APP_Layer
//Travis Collins and James Silvia

//Client
#include "all.h"
#include <cmath>

#define DELIM " "
#define MAX_BUFF 255
#define chunk 100

int PORT = 8783; /* Known port number */
char* HOSTNAME;
int vb_mode = 0;
int writing = 0;
int probability = 0;
queue<string> file_recv_q;

//File mutex locks
pthread_mutex_t mutex_file_recv = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_writing = PTHREAD_MUTEX_INITIALIZER;

//Function Prototypes
void *recv_display(void *num);
void send_dl(string message);
string get_string(void);
void split_up_message(string to_split);
void send_file(string name);
void receive_file(string name);

using namespace std;

//JES
int main(int argc, char *argv[]) {

	int sockfd, n;
	struct sockaddr_in serv_addr;

	//Ensure correct number of inputs
	if ((argc < 7) || (argc > 10)) {
		fprintf(
				stderr,
				"usage:\n %s <server> login <username> <age> <location> <hobby> <verbose> <port> <probability (0-10)>\n",
				argv[0]);
		exit(0);
	}
	if (argc >= 8)
		vb_mode = atoi(argv[7]);
	if (argc >= 9)
		PORT = atoi(argv[8]);
	if (argc == 10)
		probability = atoi(argv[9]);

	//If not login
	if (strcmp(argv[2], "login") != 0) {
		cout << "Please login!\n";
		fprintf(
				stderr,
				"usage:\n %s <server> login <username> <age> <location> <hobby> <verbose> <port> <probability (0-10)>\n",
				argv[0]);
		exit(0);
	}

	//Profile and username information
	string username(argv[3]);
	string age(argv[4]);
	string location(argv[5]);
	string hobby(argv[6]);

	//Build login and profile information buffer
	string buffer;
	buffer = "login " + username + " " + age + " " + location + " " + hobby
			+ "\x89";

	//Setup and connect to server
	HOSTNAME = argv[1];

	//Start Data Link and Physical Layers
	pthread_t dl_thread;
	int rc;
	rc = pthread_create(&dl_thread, NULL, dl_layer_client, (void *) 1);
	if (rc) {
		cout << "Data Link Layer Thread Failed to be created" << endl;
		exit(1);
	}

	//Send Login buffer
	verbose("Sending Login (APP)");
	if (buffer.size() > 256)
		split_up_message(buffer);
	else {
		send_dl(buffer);
	}

	//Wait for login confirmation
	bool logged_in = 0;
	while (1) {
		pthread_mutex_lock(&mutex_dl_receive);
		if (!dl_receive_q.empty()) {
			string received;
			pthread_mutex_unlock(&mutex_dl_receive);
			received = get_string();
			verbose("RECEIVED '" + received + "' (APP)");

			//Must be a confirmed login from server
			if (strcmp(received.c_str(), "loggedin") == 0) {
				//Start user display thread
				pthread_t recv_thread;
				int rc = pthread_create(&recv_thread, NULL, recv_display,
						(void *) 1);
				if (rc) {
					cout << "Receive thread failed to be created" << endl;
					exit(1);
				}

				logged_in = true;
				cout << "Welcome! Type 'help' for chatroom commands" << endl;
			}
			//Error message from server, username already exists, Login again.
			if (strcmp(received.c_str(), "ERROR! Username already exists.")
					== 0) {
				cout << "Username already exists! Please try again." << endl;
				sleep(3);
				pthread_cancel(dl_thread);
				exit(1);
			}
		} else
			pthread_mutex_unlock(&mutex_dl_receive);

		//Logged in
		while (logged_in) {
			buffer.clear();

			//Read in user input
			cout << "Enter input: ";
			getline(cin, buffer);

			istringstream iss(buffer);
			string command, input, tosend, message;
			iss >> command;

			while (iss >> input) {
				message = message + input + " ";
			}

			//Determine if possible input: Error Checking
			char *buffin = &buffer[0];
			int word = count_words(buffin);

			string filename;
			//flags for flow control
			int lessthan = 0;
			int morethan = 0;
			int upload = 0;
			int get = 0;
			int logout = 0;

			//Verifies correct input, else prints help function
			//Sets up message handling flow
			if ((command.compare("who") == 0) && (word == 1)) {
				lessthan = true;
				tosend = command;
			} else if ((command.compare("send") == 0) && (word >= 2)) {
				tosend = command + " " + message;
				if (tosend.size() > MAX_BUFF - 1)
					morethan = true;
				else
					lessthan = true;
			} else if ((command.compare("history") == 0) && (word == 1)) {
				writing = 1;
				tosend.clear();
				filename = "chat_history.txt ";
				tosend = "get chat_history.txt\x89";
			} else if ((command.compare("what") == 0) && (word == 2)) {
				lessthan = true;
				tosend = command + " " + message;
			} else if ((command.compare("upload") == 0) && (word == 2)) {
				filename = message;
				tosend = command + " " + message;
				send_dl(tosend + "\x89");
				send_file(filename);
				continue;
			} else if ((command.compare("get") == 0) && (word == 2)) {
				writing = 1; //enable writing
				filename = message;
				tosend = command + " " + message;
			} else if ((command.compare("logout") == 0) && (word == 1)) {
				logout = true;
				tosend = command;
				//pthread_cancel(recv_thread);
			} else
				printhelp();

			//Sends to data link layer based on flows from above
			//If no message splitting
			if (lessthan) {
				verbose("Sending '" + tosend + "' to server. (APP)");
				tosend = tosend + "\x89";
				send_dl(tosend);
				tosend.clear();
			}
			//If message splitting
			else if (morethan) {
				split_up_message(tosend);
			}
			//If logout
			else if (logout) {
				tosend = tosend + "\x89";
				send_dl(tosend);
				sleep(10);
				pthread_cancel(dl_thread);
				exit(0);
			} else if (writing) {
				tosend = tosend + "\x89";
				send_dl(tosend);
				receive_file(filename);
			}
		}
	}
	return 0;
}

//Basic diewithError Function
//JES
void diewithError(string message) {
	cout << message << endl;
	exit(1);
}

//Send string to DL
//JES
void send_dl(string message) {
	pthread_mutex_lock(&mutex_dl_send);
	dl_send_q.push(message);
	pthread_mutex_unlock(&mutex_dl_send);
}

//Split message and send to DL Layer
//JES
void split_up_message(string to_split) {

	pthread_mutex_lock(&mutex_dl_send);

	verbose("File being split, over 256 bytes (APP)");
	string tosend = "";
	int pieces = (int) ceil((double) to_split.size() / ((double) MAX_BUFF + 1));
	cout << "APP Pieces: " << pieces << endl;
	for (int i = 0; i < pieces - 1; i++) {
		tosend.clear();
		tosend = to_split.substr(i * MAX_BUFF, MAX_BUFF);
		cout << "Adding: " << tosend << endl;
		dl_send_q.push(tosend);
	}
	tosend.clear();
	tosend = to_split.substr((pieces - 1) * MAX_BUFF,
			to_split.length() % (MAX_BUFF));
	//cout<<"Piece: "<<tosend<<"|"<<endl;
	cout << "Adding: " << tosend << endl;
	tosend = tosend + "\x89";
	dl_send_q.push(tosend);

	cout << "Done Adding to queue" << endl;
	pthread_mutex_unlock(&mutex_dl_send);
}

//Help Function for UI
//JES
void printhelp(void) {
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
	cout << "	get <image.jpg>" << endl;
	cout << "To logout of the chatroom..." << endl;
	cout << "	logout" << endl;
	cout << "***************************" << endl;
}

//Used for error checking, returns number of STD inputs
//JES
int count_words(char *str) {

	int count = 0;
	char * token = strtok(str, " ");
	while (token != NULL) {
		count++;
		token = strtok(NULL, " ");
	}

	return count;
}

//Returns concatenated string when getting message from DL Layer
//JES
string get_string(void) {
	string str = "";
	while (1) {
		string temp = "";
		pthread_mutex_lock(&mutex_dl_receive);
		if (!dl_receive_q.empty()) {
			temp = dl_receive_q.front();
			//cout<<temp<<endl;
		} else {
			pthread_mutex_unlock(&mutex_dl_receive);
			continue;
		}
		dl_receive_q.pop();
		pthread_mutex_unlock(&mutex_dl_receive);
		//Find the DELIM and erase it, return full string
		if (temp.find("\x89") < 256) {
			temp.erase(temp.find('\x89'), 1);
			str = str + temp;
			return str;
		}
		//If not in this message, concatenate
		else {
			str = str + temp;
		}
	}
	//Never gets here
	return 0;
}

//Additional thread that manages server responses
//JES
void *recv_display(void *num) {
	verbose("\nStarted Recv_Display thread");
	while (1) {
		pthread_mutex_lock(&mutex_dl_receive);
		if (!dl_receive_q.empty()) {
			//Gets string from queue
			pthread_mutex_unlock(&mutex_dl_receive);
			string recvd = get_string();

			if (recvd.substr(0, 4) == "FILE") {
				pthread_mutex_lock(&mutex_file_recv);
				file_recv_q.push(recvd.substr(4, recvd.length() - 4));
				pthread_mutex_unlock(&mutex_file_recv);
			} else if (recvd.substr(0, 4) == "FILD")
				writing = 0;
			else if (strcmp(recvd.c_str(), "FILB") == 0) {
				cout << "That file does not exist, try again!" << endl;
				writing = 0;
			}
			//Display the received message from server to user
			else {
				cout << "\nMessage received from server!" << endl;
				cout << "'" + recvd + "'" << endl;
				cout << "Please continue...\nEnter input:" << endl;
			}
		} else
			pthread_mutex_unlock(&mutex_dl_receive);
	}
	//Never gets here
	return 0;
}

//Function used to read in and send file to server, UPLOAD
//JES
void send_file(string filename) {

	//Recreate structure

	//Set User
	char * Filename;
	filename = filename.substr(0, filename.length() - 1);
	Filename = new char[filename.size() + 1];
	strcpy(Filename, filename.c_str());

	ifstream Input;
	size_t Size = 0;

	Input.open(Filename, ios::in | ios::binary | ios::ate);
	Input.seekg(0, ios::end);
	Size = Input.tellg();
	Input.seekg(0, ios::beg);

	char * oData = new char[Size + 1];

	Input.read(oData, Size);
	oData[Size] = '\0';

	int pieces = (int) floor((double) Size / ((double) chunk));
	int remainder = Size % chunk;
	int index = 0;
	int index2 = 0;
	int bytes = 0;
	int done = 1;
	char * buffer;
	string tosend;
	string buffer2;

	sleep(1);
	/* Algorithm                               */
	string buffer_save = "";
	while (done) {
		if (index2 < pieces) {

			buffer = (char*) malloc(sizeof(char) * chunk + 1);
			memset(buffer, 0, strlen(buffer));
			buffer2.clear();
			for (int l = 0; l < chunk; l++) {
				//buffer[l]=oData[index];
				buffer2 = buffer2 + oData[index];
				index++;
			}
		}

		else {
			buffer = (char*) malloc(sizeof(char) * remainder + 1);
			memset(buffer, 0, strlen(buffer));
			buffer2.clear();
			for (int l = 0; l < remainder; l++) {
				//buffer[l]=oData[index];
				buffer2 = buffer2 + oData[index];
				index++;
			}
			done = 0;

		}
		//cout<<"BUFFER:"<<buffer2<<endl;
		tosend.clear();
		tosend.append("FILE");
		tosend = tosend + " " + string(buffer2);
		tosend.append("\x89");

		buffer_save.append(buffer);

		pthread_mutex_lock(&mutex_dl_send);
		dl_send_q.push(tosend);
		pthread_mutex_unlock(&mutex_dl_send);
		//sleep(1);
		index2++;

	}
	tosend = "FILD\x89";
	pthread_mutex_lock(&mutex_dl_send);
	dl_send_q.push(tosend);
	pthread_mutex_unlock(&mutex_dl_send);
	cout << "Upload complete" << endl;
	Input.close();

}

//Function used to receive file from server, GET
//Use this in recv_thread
//JES
void receive_file(string name) {

	int wait = 1;
	char C;
	char Filename[MAX_BUFF];
	string temp;

	name = name.substr(0, name.length() - 1);
	strcpy(Filename, name.c_str());
	ofstream Output(Filename, ios::out | ios::binary);

	verbose("File Opened");

//	Write chunks to file
	while (1) {
		pthread_mutex_lock(&mutex_file_recv);
		if (!file_recv_q.empty()) {
			temp.clear();
			temp = file_recv_q.front();
			temp = temp.substr(1, temp.length()-1);

			file_recv_q.pop();

			char * buffer;
			buffer = new char[temp.size()+1];
			strcpy(buffer, temp.c_str());
			Output << buffer;
		}
		pthread_mutex_unlock(&mutex_file_recv);

		pthread_mutex_lock(&mutex_file_recv);
		pthread_mutex_lock(&mutex_writing);

		// Algorithm
		if (writing || !file_recv_q.empty()) {
			pthread_mutex_unlock(&mutex_writing);
			pthread_mutex_unlock(&mutex_file_recv);
			continue;
		} else {
			pthread_mutex_unlock(&mutex_writing);
			pthread_mutex_unlock(&mutex_file_recv);
			break;
		}
	}
	cout << "Done receiving" << endl;
	Output.close();
}

