//Server APP_Layer
//JES

#include "all.h"
#include <cmath>

#define DELIM " "
#define MAX_BUFF 255

int PORT = 8786;		/* Known port number */
int vb_mode = 0;

//User Entry
struct user_entry{
	int client_ID;
	string username;
	int age;
	string location;
	string hobby;
};
//Database of User Entries
static list<user_entry> database;

//Function Prototypes
void handle_client(int ID);
string get_string(const int client_ID);
bool add_entry(const int client_ID, const string &username, const int age, const string &location, const string &hobby);
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
void send_history(const int client_ID);
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
    	//Monitor queues for connected clients, Handle Client when queue not empty
    	for(int ID = 0; ID < clients; ID++){
    		pthread_mutex_lock(&mutex_dl_receive[ID]);
    		if(!dl_receive_q[ID].empty()){
    			pthread_mutex_unlock(&mutex_dl_receive[ID]);
    			verbose("Handling client (APP)");
    			handle_client(ID);
			cout<<"DONESZ"<<endl;
    		}
		else
    			pthread_mutex_unlock(&mutex_dl_receive[ID]);
    	}
    }
}

//Handles each client based on queue
void handle_client(int client_ID){

	string buff = get_string(client_ID);
	char recv_buff[MAX_BUFF] = {0};
	strcpy(recv_buff, buff.c_str());

	//Tokenize the string, first word is the command
	const char * command = strtok(recv_buff, DELIM);
	cout<<"Command: "<<command<<endl;
	//If command is login, try to add user to DB
	if (strcmp(command, "login") == 0){
		char * user = strtok(NULL, DELIM);
		char * age = strtok(NULL, DELIM);
		char * loc = strtok(NULL, DELIM);
		char * hobby = strtok(NULL, DELIM);

		if(!user || !loc || !age || !hobby) diewithError("Something went wrong");
		//If username already exists
		if(!add_entry(client_ID, user, atoi(age), loc, hobby)) verbose("Username Exists! (APP)");
		//If not, send logged in to client
		else {
			verbose("Sending Login (APP)");
			send_to_one(client_ID,"loggedin\x89");
		}
	}

	//Client wants to see who is online
	else if (strcmp(command,"who") == 0){
		verbose("Received 'who' from client (APP)");
		send_users(client_ID);
	}

	//Client wants to send message to all other clients
	else if (strcmp(command,"send") == 0){
		verbose("Received 'send' from client (APP)");
		string message = "";

		//Prepare message to be sent to all clients
		command = strtok(NULL, DELIM);
		while (command != NULL){
			message = message + " " + command;
			command = strtok(NULL, DELIM);
		}
		string user = return_username(client_ID);
		string tosend = user + " said:" + message;
		add_to_history(user + " said:" + message);

		//If message sent over MAX_BUFF, send to split, else send.
		tosend="HELLO";
		if (tosend.size() > MAX_BUFF-1) split_send_all(tosend);
		else send_to_all(tosend+"\x89");
	}

	//Client wants chat_history.txt
	else if (strcmp(command, "history") == 0){
		verbose("Received 'history' from client (APP)");
		send_history(client_ID);
	}

	//Client wants to see user's profile info
	else if (strcmp(command,"what") == 0){
		verbose("Received 'what' from client (APP)");
		char * name = strtok(NULL,DELIM);
		send_info(client_ID,name);
	}

	//Client wants to logout, remove from DB
	else if (strcmp(command,"logout") == 0){
		verbose("Received logout");
		db_remove(client_ID);
	}

	//Client wants to upload a file
	else if (strcmp(command,"upload") == 0){
		verbose("Client wants to upload file (APP)");
		//writing=1;
		//receive_file(client_ID);
	}

/*	//Received a piece of a file
	else if (strcmp(command.substr(0,4),"FILE") == 0){
		verbose("Client wants to upload file (APP)");
                pthread_mutex_lock(&mutex_file_recv);
		file_recv_q.push(command.substr(5,command.length()-1));
                pthread_mutex_lock(&mutex_file_recv);
		//receive_file(client_ID);
	}
	//Done with file xfer
	else if (strcmp(command.substr(0,4),"FILD") == 0){
		verbose("Client wants to upload file (APP)");
		writing=0;
		//receive_file(client_ID);
	}
	*/
	//Successfully handled Client
	verbose("Handled Client (APP)");
	sleep(10);
	cout<<"RETURNING"<<endl;
	return;
}

//Returns concatenated string when getting message from DL Layer
string get_string(const int client_ID){
	string str = "";
	//string temp = "";
	while(1){
		string temp="";
		pthread_mutex_lock(&mutex_dl_receive[client_ID]);
		if (!dl_receive_q[client_ID].empty())
			temp = dl_receive_q[client_ID].front();
		else{
			pthread_mutex_unlock(&mutex_dl_receive[client_ID]);
			continue;
		}
		dl_receive_q[client_ID].pop();
		pthread_mutex_unlock(&mutex_dl_receive[client_ID]);
		//Find the DELIM and erase it, return full string
		if(temp.find("\x89") < 256){
			temp.erase(temp.find('\x89'),1);
			str = str + temp;
			return str;
		}
		//If not in this message, concatenate
		else{
			str = str + temp;
		}
	}
	//Never gets here
	return 0;
}

//Adding an entry to the database
bool add_entry(const int client_ID, const string &username, const int age, const string &location, const string &hobby){

    //Check if username exists
    if (!name_check(username, client_ID))
        return false;

    //Create a database entry and add to DB
    user_entry entry = {client_ID, username, age, location, hobby};
    database.push_back(entry);
    verbose("Added " + username + " to server DB (APP)");

    return true;
}

//Check if name exists in the database
bool name_check(const string name, const int client_ID){
	for (list<user_entry>::iterator entry = database.begin(); entry != database.end(); entry++){
		if ((strcmp(entry->username.c_str(), name.c_str())) == 0){
			string tosend = "ERROR! Username already exists.";
			send_to_one(client_ID, tosend);
			return false;
		}
	}
	return true;
}

//Removes entry from the database
void db_remove(const int id){
	for (list<user_entry>::iterator entry = database.begin(); entry != database.end(); entry++){
		if (entry->client_ID == id){
			verbose("Removed " + entry->username + " from server DB (APP)");
			database.erase(entry);
			return;
		}
	}
}

//Get and send all users in database to requesting client
void send_users(const int client_ID){
	string to_users = "";
	//Iterate through the database
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if (to_users.compare("") == 0){
			to_users = "Users are:\n" + entry->username;
		}
		else to_users = to_users + "\n" + entry->username;
	}
	//If message needs to be split, else send to client
	if (to_users.size() > MAX_BUFF-1) split_send_one(client_ID, to_users);
	else send_to_one(client_ID, to_users+"\x89");
}

//Get and send user's information to requesting client
void send_info(const int client_ID, const string &userin){
	string to_users = "";
	//Iterate through the database
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if ((entry->username).compare(userin) == 0){
			string ega = convertInt(entry->age);
			to_users = "username:" + entry->username + " age:" + ega + " location:" + entry->location + " hobby:" + entry->hobby;

			//If message needs to be split, else send to client
			if (to_users.size() > MAX_BUFF-1) split_send_one(client_ID, to_users);
			else send_to_one(client_ID, to_users+"\x89");

			return;
		}
	}
	//If user does not exist in the server DB, return this to requesting client
	send_to_one(client_ID, "'what' request error: That user does not exist\x89");
}

//Find the username of the requesting client_ID
string return_username(const int ID){
	string name = "";
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if (entry->client_ID == ID)
			name = entry->username;
	}
	return name;
}

//Split up message over 256 bytes, and send to all clients
void split_send_all(string to_split){

	verbose("File being split, over 256 bytes (APP)");
	string tosend = "";
	int pieces=(int)ceil((double)to_split.size()/(double)MAX_BUFF);

	for(int i = 0; i < pieces - 1; i++){
		tosend.clear();
		tosend = to_split.substr(i*MAX_BUFF,(i+1)*MAX_BUFF - 1);
		send_to_all(tosend);
	}
		tosend.clear();
		tosend = to_split.substr((pieces-1)*MAX_BUFF, to_split.length());
		tosend = tosend + "\x89";
		send_to_all(tosend);
}

//Split up messages over 256 bytes, and send to one requesting client
void split_send_one(const int client_ID, string to_split){

	verbose("File being split, over 256 bytes (APP)");
	string tosend = "";
	int pieces=(int)ceil((double)to_split.size()/(double)MAX_BUFF);

	for(int i = 0; i < pieces - 1; i++){
		tosend.clear();
		tosend = to_split.substr(i*MAX_BUFF,(i+1)*MAX_BUFF - 1);
		send_to_one(client_ID, tosend);
	}
		tosend.clear();
		tosend = to_split.substr((pieces-1)*MAX_BUFF, to_split.length());
		tosend = tosend + "\x89";
		send_to_one(client_ID,tosend);
}

//Send string to all connected clients
void send_to_all(string tosend){
	//tosend = tosend + "\x89";
	int cnt = 0;
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		cnt = entry->client_ID;
		pthread_mutex_lock(&mutex_dl_send[cnt]);
		dl_send_q[cnt].push(tosend);
		pthread_mutex_unlock(&mutex_dl_send[cnt]);
	}
	verbose("PUSHED " + tosend + " to all (APP)");
}

//Send string to one requesting client
void send_to_one(const int client_ID, string tosend){
	//tosend = tosend + "\x89";
	pthread_mutex_lock(&mutex_dl_send[client_ID]);
	dl_send_q[client_ID].push(tosend);
	pthread_mutex_unlock(&mutex_dl_send[client_ID]);

	verbose("PUSHED " + tosend + " to one (APP)");
}

//Add chat history to file
void add_to_history(string message){
	ofstream history ("chat_history.txt", ios::app);
	if (!history) diewithError("Failed to open chat_history.txt, re-build server!");

	history << message + "\n";
	history.close();
}

//History file transfer
void send_history(const int client_ID){
	ofstream history ("chat_history.txt");

	if (!history) diewithError("Failed to open chat_history.txt, re-build server!");
	//Do shit with the file.
	//send_to_one(client_ID);

	cout << "THIS IS NOT WORKING YET" << endl;;

}

//Convert Integer to String
string convertInt(int number){
	stringstream ss;
	ss << number;
	return ss.str();
}

/*

//Receive File from user
void receive_file(const in client_ID){

	FILE * Output;
        char C;
        char Filename[20]="output.txt";
        string temp;

        strcpy(Filename, name.c_str());
        Output = fopen(Filename,"wb");

        while (writing||!file_recv_q.empty()) {

                pthread_mutex_lock(&mutex_file_recv);
                if (!file_recv_q.empty()){
                        temp=file_recv_q.front();
                        C=temp[0];
                        file_recv_q.pop();
                        fwrite(&C,1,1,Output);
                }
                pthread_mutex_unlock(&mutex_file_recv);

        }
        cout<<"Done receiving"<<endl;
        fclose(Output);


}

*/




