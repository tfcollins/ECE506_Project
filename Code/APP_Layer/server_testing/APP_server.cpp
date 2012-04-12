//Server APP_Layer
//JES

#include "all.h"

#define DELIM " "
#define MAX_LEN 256

int PORT = 9218;		/* Known port number */
int vb_mode = 0;

//User Entry
struct user_entry{
	int client_ID;
	string username;
	int age;
	string location;
	string hobby;
};
//Database of Users
static list<user_entry> database;

//Function Prototypes
void handle_client(int ID);
string get_string(const int client_ID);
bool add_entry(const int client_ID, const string &username, const int age, const string &location, const string &hobby);
void db_remove(const int id);
string return_username(const int ID);
void send_users(void);
void send_info(const int client_ID, const string &userin);
void send_to_all(string tosend);
void send_to_one(const int client_ID, string tosend);
void add_to_history(string message);
string convertInt(int number);

using namespace std;

int main(int argc, char *argv[]){

	if (argc > 2){
		fprintf(stderr, "usage:\n %s\n", argv[0]);
		exit(0);
	}
	if (argc == 2) vb_mode = atoi(argv[1]);

	//Initialize Physical Layer
    pthread_t phy_thread;
    int rc;
    rc = pthread_create(&phy_thread, NULL, phy_layer_server, (void *) 1);
    if (rc){
            cout<<"Physical Layer Thread Failed to be created"<<endl;
            exit(1);
    }

    while(1){
    	for(int ID = 0; ID < clients; ID++){
    		pthread_mutex_lock(&mutex_dl_receive[ID]);
    		if(!dl_receive_q[ID].empty()){
    			verbose("Handling client (APP)");
    			handle_client(ID);
    		}
    		pthread_mutex_unlock(&mutex_dl_receive[ID]);
    	}
    }
}

void handle_client(int client_ID){

	string buff = get_string(client_ID);

	char recv_buff[MAX_LEN] = {0};
	strcpy(recv_buff, buff.c_str());

	//Tokenize the string, first word is the command
	const char * command = strtok(recv_buff, DELIM);
	//If command is login, add to DB and send successful login to client
	if (strcmp(command, "login") == 0){
		char * user = strtok(NULL, DELIM);
		char * age = strtok(NULL, DELIM);
		char * loc = strtok(NULL, DELIM);
		char * hobby = strtok(NULL, DELIM);

		if(!user || !loc || !age || !hobby) diewithError("Login failed");
		if(!add_entry(client_ID, user, atoi(age), loc, hobby)) diewithError("Could not add Entry");
		verbose("Sending Login (APP)");

		pthread_mutex_lock(&mutex_dl_send[client_ID]);
		dl_send_q[client_ID].push("loggedin");
		pthread_mutex_unlock(&mutex_dl_send[client_ID]);
	}

	//Client wants to see who is online
	else if (strcmp(command,"who") == 0){
		verbose("Received 'who' from client (APP)");
		send_users();
	}

	//Client wants to send message to all other clients
	else if (strcmp(command,"send") == 0){
		verbose("Received 'send' from client (APP)");
		string message = "";

		//remove 'send' for copying
		command = strtok(NULL, DELIM);
		while (command != NULL){
			message = message + " " + command;
			command = strtok(NULL, DELIM);
		}
		string user = return_username(client_ID);
		send_to_all(user + " said:" + message);
		add_to_history(user + " said:" + message);
	}

	//Client wants to see user's profile info
	else if (strcmp(command,"what") == 0){
		verbose("Received 'what' from client (APP)");
		char * name = strtok(NULL,DELIM);
		send_info(client_ID,name);
	}

	//Client wants to logout
	else if (strcmp(command,"logout") == 0){
		verbose("Received logout");
		db_remove(client_ID);
	}

	verbose("Handled Client (APP)");
	return;
}
//Helper function to get messages from DL_Layer
//Received in 256 byte chunks
string get_string(const int client_ID){
	string str = "";
	while(1){
		string temp = "";
		temp = dl_receive_q[client_ID].front();
		dl_receive_q[client_ID].pop();

		if(temp.find("\x89") < 256){
			temp.erase(temp.find('\x89'),1);
			str = str + temp;
			return str;
			cout << str << endl;
		}
		else{
			str = str + temp;
		}
	}
	return 0;
}

//Adding an entry to the database
bool add_entry(const int client_ID, const string &username, const int age, const string &location, const string &hobby){

    //Check if username exists
    //if (get_entry(username))
        //return false;

    //Create a database entry and add to DB
    user_entry entry = {client_ID, username, age, location, hobby};
    database.push_back(entry);

    return true;
}

void db_remove(const int id){
	for (list<user_entry>::iterator entry = database.begin(); entry != database.end(); entry++){
		if (entry->client_ID == id){
			cout << "found ittttt" << endl;
			database.erase(entry);
			return;
		}
	}
}

//Get and send all users in database to all clients
void send_users(void){
	string to_users = "";
	//Iterate through the database
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if (to_users.compare("") == 0){
			to_users = "Users are:\n" + entry->username;
		}
		else to_users = to_users + "\n" + entry->username;
	}
	send_to_all(to_users);
}

//Get and send user's information to requesting client
void send_info(const int client_ID, const string &userin){
	string to_users = "";
	//Iterate through the database
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if ((entry->username).compare(userin) == 0){
			string ega = convertInt(entry->age);
			to_users = "username:" + entry->username + " age:" + ega + " location:" + entry->location + " hobby:" + entry->hobby;
			send_to_one(client_ID, to_users);
			return;
		}
		
	}
	send_to_one(client_ID, "'what' request error: That user does not exist");
}

string return_username(const int ID){
	string name = "";
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if (entry->client_ID == ID)
			name = entry->username;
	}

	return name;
}

//Send string to all connected clients
void send_to_all(string tosend){
	for(int i = 0; i < clients; i++){
		pthread_mutex_lock(&mutex_dl_send[i]);
		dl_send_q[i].push(tosend);
		pthread_mutex_unlock(&mutex_dl_send[i]);
	}
	verbose("PUSHED " + tosend + " (APP)");
}

//Send string to one connected client
void send_to_one(const int client_ID, string tosend){
	pthread_mutex_lock(&mutex_dl_send[client_ID]);
	dl_send_q[client_ID].push(tosend);
	pthread_mutex_unlock(&mutex_dl_send[client_ID]);

	verbose("PUSHED " + tosend + " (APP)");
}

//Add chat history to file
void add_to_history(string message){
	ofstream history ("chat_history.txt", ios::app);
	if (!history) diewithError("Failed to open chat_history.txt, re-build server!");

	history << message + "\n";
	history.close();
}

//Convert Integer to String
string convertInt(int number){
	stringstream ss;
	ss << number;
	return ss.str();
}


