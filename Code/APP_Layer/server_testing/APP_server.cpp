//Server APP_Layer
//JES

#include "all.h"

#define DELIM " "
#define MAX_LEN 256

int PORT = 8787;		/* Known port number */
int vb_mode = 0;

//User Entry
struct user_entry{
	int client_ID;
	string username;
	string location;
	int age;
	string hobby;
};
//Database of Users
static list<user_entry> database;

//Function Prototypes
void handle_client(int ID);
bool add_entry(const int client_ID, const string &username, const string &location, const int age, const string &hobby);
void send_users(void);

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

    //Assume initial client count is 0

    while(1){
    	for(int ID = 0; ID < clients; ID++){
    		pthread_mutex_lock(&mutex_dl_receive[ID]);
    		if(!dl_receive_q[ID].empty()){
    			verbose("Handling client APP)");
    			handle_client(ID);
    		}
    		pthread_mutex_unlock(&mutex_dl_receive[ID]);
    	}
    }
}

void handle_client(int client_ID){

	string buff = dl_receive_q[client_ID].front();
	dl_receive_q[client_ID].pop();

	char recv_buff[MAX_LEN + 1] = {0};
	strcpy(recv_buff, buff.c_str());

	//Tokenize the string, first word is the command
	const char * command = strtok(recv_buff, DELIM);
	//If command is login, add to DB and send successful login to client
	if (strcmp(command, "login") == 0){
		char * user = strtok(NULL, DELIM);
		char * loc = strtok(NULL, DELIM);
		char * age = strtok(NULL, DELIM);
		char * hobby = strtok(NULL, DELIM);

		if(!user || !loc || !age || !hobby) diewithError("Login failed");
		if(!add_entry(client_ID, user, loc, atoi(age), hobby)) diewithError("Could not add Entry");
		cout << "Sending 'loggedin' to client" << endl;
		pthread_mutex_lock(&mutex_dl_send[client_ID]);
		dl_send_q[client_ID].push("loggedin");
		pthread_mutex_unlock(&mutex_dl_send[client_ID]);
	}
	if (strcmp(command,"who") == 0){
		verbose("Received 'who' from client (APP)");
		send_users();
	}


	verbose("Successfully Handled Client (APP)");
	return;
}

//Adding an entry to the database
bool add_entry(const int client_ID, const string &username, const string &location, const int age, const string &hobby){

    //Check if username exists
    //if (get_entry(id))
        //return false;

    //Create a database entry and add to DB
    user_entry entry = {client_ID, username, location, age, hobby};
    database.push_back(entry);

    return true;
}
void send_users(void){
	string to_users = "";
	for (list<user_entry>::const_iterator entry = database.begin(); entry != database.end(); entry++){
		if (to_users.compare("") == 0){
			to_users = "Users are:\n" + entry->username;
		}
		else to_users = to_users + "\n" + entry->username;
	}

	for(int i = 0; i < clients; i++){
		pthread_mutex_lock(&mutex_dl_send[i]);
		dl_send_q[i].push(to_users);
		pthread_mutex_unlock(&mutex_dl_send[i]);
	}
}
