//Server APP_Layer
//Travis Collins and James Silvia

#include "all.h"
#include <cmath>

#define chunk 100
#define DELIM " "
#define MAX_BUFF 255

int PORT = 8783; /* Known port number */
int vb_mode = 0;
int writing = 0;

queue<string> file_recv_q;
pthread_mutex_t mutex_file_recv = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_writing = PTHREAD_MUTEX_INITIALIZER;

//Structure for file download thread
typedef struct {
	string filename;
	int client;
	int size;
} upload_info;

//User Entry
struct user_entry {
	int client_ID;
	string username;
	int age;
	string location;
	string hobby;
};

//Database of User Entries
static list<user_entry> database;

//Database for available files
static list<string> file_db;

//Function Prototypes
void handle_client(int ID);
string get_string(const int client_ID);
bool add_entry(const int client_ID, const string &username, const int age,
		const string &location, const string &hobby);
bool name_check(const string name, const int client_ID);
void db_remove(const int id);
string return_username(const int ID);
void send_users(const int client_ID);
void send_info(const int client_ID, const string &userin);
void split_send_all(string tosplit);
void split_send_one(const int client_ID, string to_split);
void send_to_all(string tosend);
void send_to_one(const int client_ID, string tosend);
void add_to_history(string message);
void send_history(const int client_ID, string buff);
string convertInt(int number);
void *receive_file(void* info);
void *send_file(void* info);
bool check_filedb(string file);

using namespace std;

int main(int argc, char *argv[]) {

	if (argc > 3) {
		fprintf(stderr, "usage:\n %s\n", argv[0]);
		exit(0);
	}
	if (argc == 2 || argc == 3)
		vb_mode = atoi(argv[1]);
	if (argc == 3)
		PORT = atoi(argv[2]);

	//Initialize Physical Layer
	pthread_t phy_thread;
	int rc;
	rc = pthread_create(&phy_thread, NULL, phy_layer_server, (void *) 1);
	if (rc) {
		cout << "Physical Layer Thread Failed to be created" << endl;
		exit(1);
	}

	//Always should be in the file db
	file_db.push_front("chat_history.txt");

	//Monitor queues for connected clients, Handle Client when queue not empty
	while (1) {
		for (int ID = 0; ID < clients; ID++) {
			pthread_mutex_lock(&mutex_dl_receive[ID]);
			if (!dl_receive_q[ID].empty()) {
				pthread_mutex_unlock(&mutex_dl_receive[ID]);
				verbose("Handling client (APP)");
				handle_client(ID);
			} else
				pthread_mutex_unlock(&mutex_dl_receive[ID]);
		}
	}
}

//Handles each client based on queue
//JES
void handle_client(int client_ID) {

	string command;
	string buff = get_string(client_ID);
	istringstream iss(buff);
	iss >> command;

	//Client must login first
	if (strcmp(command.c_str(), "login") == 0) {

		string user, age, loc, hobby;
		iss >> user;
		iss >> age;
		iss >> loc;
		iss >> hobby;
		int ega = atoi(age.c_str());

		//If username already exists
		if (!add_entry(client_ID, user, ega, loc, hobby))
			verbose("Username Exists! (APP)");
		//If not, send logged in to client
		else {
			verbose("Sending Login (APP)");
			send_to_one(client_ID, "loggedin\x89");
		}
	}

	//Client wants to see who is online
	else if (strcmp(command.c_str(), "who") == 0) {
		verbose("Received 'who' from client (APP)");
		send_users(client_ID);
	}

	//Client wants to send message to all other clients
	else if (strcmp(command.c_str(), "send") == 0) {
		verbose("Received 'send' from client (APP)");
		string message = "";

		string temp;
		while (iss >> temp) {
			message = message + temp + " ";
		}
		string user = return_username(client_ID);
		string tosend = user + " said:" + message;
		//Add to chat history
		add_to_history(user + " said:" + message);

		//If message sent over MAX_BUFF, send to split, else send.
		if (tosend.size() > MAX_BUFF - 1)
			split_send_all(tosend);
		else
			send_to_all(tosend + "\x89");
	}

	//Client wants chat_history.txt
	else if (strcmp(command.c_str(), "history") == 0) {
		verbose("Received 'history' from client (APP)");
		send_history(client_ID, buff);
	}

	//Client wants to see user's profile info
	else if (strcmp(command.c_str(), "what") == 0) {
		verbose("Received 'what' from client (APP)");
		string name;
		iss >> name;
		//Send info to client
		send_info(client_ID, name);
	}

	//Client wants to logout, remove from DB
	else if (strcmp(command.c_str(), "logout") == 0) {
		verbose("Received logout");
		db_remove(client_ID);
	}

	//Client wants to upload a file
	else if (strcmp(command.c_str(), "upload") == 0) {
		pthread_mutex_lock(&mutex_writing);
		writing = 1;
		pthread_mutex_unlock(&mutex_writing);

		//Start file writing thread
		pthread_t file_thread;
		upload_info file_up;
		iss >> file_up.filename;
		file_up.client = client_ID;
		file_up.size = sizeof(file_up) + 1;

		//Server DB of available files
		file_db.push_front(file_up.filename);

		int rc2;
		rc2 = pthread_create(&file_thread, NULL, receive_file, &file_up);
		if (rc2) {
			cout << "File Upload Thread Failed to be created" << endl;
			exit(1);
		} else
			verbose("File upload thread started (APP)");
		//receive_file(client_ID);
	}

	//Received a piece of a file
	else if (strcmp(command.c_str(), "FILE") == 0) {
		string message = "";
		string temp;
		while (iss >> temp) {
			message = message + temp + " ";
		}
		pthread_mutex_lock(&mutex_file_recv);
		file_recv_q.push(buff.substr(5, buff.length() - 5));
		pthread_mutex_unlock(&mutex_file_recv);
		//receive_file(client_ID);
		return;
	}
	//Done with file transfer
	else if (command.find("FILD") < 256) {
		verbose("Client is done uploading file (APP)");
		pthread_mutex_lock(&mutex_writing);
		;
		writing = 0;
		pthread_mutex_unlock(&mutex_writing);
		//receive_file(client_ID);
	}
	//If client wants to download a file
	else if (strcmp(command.c_str(), "get") == 0) {
		verbose("Client wants to download a file");
		//Construct Filename
		string message = "";
		string temp;
		while (iss >> temp) {
			message = message + temp + " ";
		}

		//Spawn thread to upload file to client
		pthread_t get_thread;
		int rc3;
		//Thread info to be passed
		upload_info file_up;
		file_up.filename = buff.substr(4, buff.length() - 5);
		file_up.client = client_ID;
		file_up.size = sizeof(file_up) + 1;
		//cout<<file_up.filename<<endl;
		//cout<<file_up.client<<endl;

		if (!check_filedb(file_up.filename))
			send_to_one(client_ID, "FILB\x89");
		else {
			//Create thread
			//send_to_one(client_ID,"Exists\x89");
			rc3 = pthread_create(&get_thread, NULL, send_file, &file_up);
			if (rc3) {
				cout << "File Upload Thread Failed to be created" << endl;
				exit(1);
			} else
				verbose("File upload thread started (APP)");
		}
	}
	//Successfully handled Client
	verbose("Handled Client (APP)");
	return;
}

//Returns concatenated string when getting message from DL Layer
//JES
string get_string(const int client_ID) {
	string str = "";
	while (1) {
		string temp = "";
		pthread_mutex_lock(&mutex_dl_receive[client_ID]);
		if (!dl_receive_q[client_ID].empty())
			temp = dl_receive_q[client_ID].front();
		else {
			pthread_mutex_unlock(&mutex_dl_receive[client_ID]);
			continue;
		}
		dl_receive_q[client_ID].pop();
		pthread_mutex_unlock(&mutex_dl_receive[client_ID]);

		//Find the DELIM and erase it, return full string
		if (temp.find("\x89") < 257) {
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

//Adding an entry to the database
//JES
bool add_entry(const int client_ID, const string &username, const int age,
		const string &location, const string &hobby) {

	//Check if username exists
	if (!name_check(username, client_ID))
		return false;

	//Create a database entry and add to DB
	user_entry entry = { client_ID, username, age, location, hobby };
	database.push_back(entry);
	verbose("Added " + username + " to server DB (APP)");

	return true;
}

//Check if name exists in the database
//JES
bool name_check(const string name, const int client_ID) {
	for (list<user_entry>::iterator entry = database.begin();
			entry != database.end(); entry++) {
		if ((strcmp(entry->username.c_str(), name.c_str())) == 0) {
			string tosend = "ERROR! Username already exists.\x89";
			send_to_one(client_ID, tosend);
			return false;
		}
	}
	return true;
}

//Removes entry from the database
//JES
void db_remove(const int id) {
	for (list<user_entry>::iterator entry = database.begin();
			entry != database.end(); entry++) {
		if (entry->client_ID == id) {
			verbose("Removed " + entry->username + " from server DB (APP)");
			database.erase(entry);
			return;
		}
	}
}

//Get and send all users in database to requesting client
//JES
void send_users(const int client_ID) {
	string to_users = "";
	//Iterate through the database
	for (list<user_entry>::const_iterator entry = database.begin();
			entry != database.end(); entry++) {
		if (to_users.compare("") == 0) {
			to_users = "Users are:\n" + entry->username;
		} else
			to_users = to_users + "\n" + entry->username;
	}
	//If message needs to be split, else send to client
	if (to_users.size() > MAX_BUFF - 1)
		split_send_one(client_ID, to_users);
	else
		send_to_one(client_ID, to_users + "\x89");
}

//Get and send user's information to requesting client
//JES
void send_info(const int client_ID, const string &userin) {
	string to_users = "";
	//Iterate through the database
	for (list<user_entry>::const_iterator entry = database.begin();
			entry != database.end(); entry++) {
		if ((entry->username).compare(userin) == 0) {
			string ega = convertInt(entry->age);
			to_users = "username:" + entry->username + " age:" + ega
					+ " location:" + entry->location + " hobby:" + entry->hobby;

			//If message needs to be split, else send to client
			if (to_users.size() > MAX_BUFF - 1)
				split_send_one(client_ID, to_users);
			else
				send_to_one(client_ID, to_users + "\x89");
			return;
		}
	}
	//If user does not exist in the server DB, return this to requesting client
	send_to_one(client_ID,
			"'what' request error: That user does not exist\x89");
}

//Find the username of the requesting client_ID
//JES
string return_username(const int ID) {
	string name = "";
	for (list<user_entry>::const_iterator entry = database.begin();
			entry != database.end(); entry++) {
		if (entry->client_ID == ID)
			name = entry->username;
	}
	return name;
}

//Split up message over 256 bytes, and send to all clients
//JES
void split_send_all(string to_split) {

	verbose("File being split, over 256 bytes (APP)");
	string tosend = "";
	int pieces = (int) ceil((double) to_split.size() / ((double) MAX_BUFF + 1));

	for (int i = 0; i < pieces - 1; i++) {
		tosend.clear();
		tosend = to_split.substr(i * MAX_BUFF, MAX_BUFF);
		send_to_all(tosend);
		//cout<<"PIECE APP: "<<tosend<<endl;
	}
	tosend.clear();
	tosend = to_split.substr((pieces - 1) * MAX_BUFF,
			to_split.length() % (MAX_BUFF));
	tosend = tosend + "\x89";
	send_to_all(tosend);
}

//Split up messages over 256 bytes, and send to one requesting client
//JES
void split_send_one(const int client_ID, string to_split) {

	verbose("File being split, over 256 bytes (APP)");
	string tosend = "";
	int pieces = (int) ceil((double) to_split.size() / (double) MAX_BUFF);

	for (int i = 0; i < pieces - 1; i++) {
		tosend.clear();
		tosend = to_split.substr(i * MAX_BUFF, MAX_BUFF);
		send_to_one(client_ID, tosend);
	}
	tosend.clear();
	tosend = to_split.substr((pieces - 1) * MAX_BUFF,
			to_split.length() % MAX_BUFF);
	tosend = tosend + "\x89";
	send_to_one(client_ID, tosend);
}

//Send string to all connected clients
//JES
void send_to_all(string tosend) {
	int cnt = 0;
	for (list<user_entry>::const_iterator entry = database.begin();
			entry != database.end(); entry++) {
		cnt = entry->client_ID;
		pthread_mutex_lock(&mutex_dl_send[cnt]);
		dl_send_q[cnt].push(tosend);
		pthread_mutex_unlock(&mutex_dl_send[cnt]);
	}
	verbose("PUSHED " + tosend + " to all (APP)");
}

//Send string to one requesting client
//JES
void send_to_one(const int client_ID, string tosend) {
	pthread_mutex_lock(&mutex_dl_send[client_ID]);
	dl_send_q[client_ID].push(tosend);
	pthread_mutex_unlock(&mutex_dl_send[client_ID]);

	verbose("PUSHED " + tosend + " to one (APP)");
}

//Add chat history to file
//JES
void add_to_history(string message) {
	ofstream history("chat_history.txt", ios::app);
	if (!history)
		diewithError("Failed to open chat_history.txt, re-build server!");

	history << message + "\n";
	history.close();
}

//History file transfer
//JES
void send_history(const int client_ID, string buff) {

	verbose("Client wants the chat history");
	//Construct Filename
	//Spawn thread to upload file to client
	pthread_t get_thread;
	int rc3;

	//Thread info to be passed
	upload_info file_up2;
	file_up2.filename = buff.substr(9, buff.length() - 10);
	file_up2.client = client_ID;
	file_up2.size = sizeof(file_up2) + 1;
	//Create thread
	rc3 = pthread_create(&get_thread, NULL, send_file, &file_up2);
	if (rc3) {
		cout << "File Upload Thread Failed to be created" << endl;
		exit(1);
	} else
		verbose("History thread started (APP)");

}

//Convert Integer to String
//JES
string convertInt(int number) {
	stringstream ss;
	ss << number;
	return ss.str();
}

bool check_filedb(string file) {
	for (list<string>::const_iterator entry = file_db.begin();
			entry != file_db.end(); entry++) {
		if (file.compare(*entry) == 0) {
			return true;
		}
	}
	return false;
}

//Receive File from user
//JES
void *receive_file(void *info) {

	//Recreate structure
	int client = (*((upload_info*) (info))).client;

	//Set User
	int size = (*((upload_info*) (info))).size;
	char * name_char = new char[size + 1];
	strcpy(name_char, (*((upload_info*) (info))).filename.c_str());
	string name = name_char;
	string temp;

	//Filename
	char * Filename;
	Filename = new char[name.size() + 1];
	strcpy(Filename, name.c_str());

	ofstream Output(Filename,ios::out | ios::binary);

	verbose("File Opened");

	while (1) {
		pthread_mutex_lock(&mutex_file_recv);
		if (!file_recv_q.empty()) {
			temp.clear();
			temp = file_recv_q.front();
			temp = temp.substr(0, temp.length());
			//cout<<"Writing: "<<temp<<"|"<<endl;

			file_recv_q.pop();

			char * buffer;
			buffer = new char[temp.size() + 1];
			//buffer = (char*) malloc (sizeof(char)*temp.length());
			strcpy(buffer, temp.c_str());
			//fwrite(buffer,1,temp.length(),Output);
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
	//Send to everyone a new file has been added to server
	string person = return_username(client);
	send_to_all(
			person + " uploaded " + name
					+ ", and it is now available for download!\x89");

	verbose("Done receiving (APP)");
	Output.close();
}

//Send file to client
//JES
void *send_file(void *info) {

	//Recreate structure
	int client = (*((upload_info*) (info))).client;

	//Set User
	int size = (*((upload_info*) (info))).size;
	char * name_char = new char[size + 1];
	strcpy(name_char, (*((upload_info*) (info))).filename.c_str());
	string name = name_char;

	//Filename
	char * Filename;
	Filename = new char[name.size() + 1];
	strcpy(Filename, name.c_str());

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
	//cout<<"Pieces to send: "<<pieces<<endl;
	int remainder = Size % chunk;
	//cout<<"Remainder; "<<remainder<<endl;
	int index = 0;
	int index2 = 0;
	int bytes = 0;
	int done = 1;
	char * buffer;
	string tosend;
	string buffer2;

	/* Algorithm                               */
	string buffer_save = "";
	while (done) {
		if (index2 < pieces) {
			buffer = (char*) malloc(sizeof(char) * chunk);
			memset(buffer, 0, strlen(buffer));
			buffer2.clear();
			for (int l = 0; l < chunk; l++) {
				//buffer[l]=oData[index];
				buffer2 = buffer2 + oData[index];
				index++;
			}
		} else {
			buffer = (char*) malloc(sizeof(char) * remainder);
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
		tosend.append("FILE");//FILE Notifier
		tosend = tosend + " " + string(buffer2);
		tosend.append("\x89");//Delim

		buffer_save.append(buffer);

		pthread_mutex_lock(&mutex_dl_send[client]);
		dl_send_q[client].push(tosend);
		pthread_mutex_unlock(&mutex_dl_send[client]);
		//cout<<"Sending File chunck"<<endl;
		//sleep(1);
		index2++;
	}
	tosend = "FILD\x89";
	pthread_mutex_lock(&mutex_dl_send[client]);
	dl_send_q[client].push(tosend);
	pthread_mutex_unlock(&mutex_dl_send[client]);
	verbose("Done sending (APP)");
	Input.close();
}
