//Server APP_Layer
//JES

#include "all.h"

#define DELIM " "
#define MAX_LEN 256

int PORT = 8787;		/* Known port number */
int vb_mode = 0;

//User Entry
struct user_entry{
	string username;
	string location;
	int age;
	string hobby;
};
//Database of Users
static list<user_entry> database;

//Function Prototypes
void handle_client(int ID);

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
    			verbose("Handling client (APP)");
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
	if (strcmp(command, "login") == 0){
		char * user = strtok(NULL, DELIM);
		char * loc = strtok(NULL, DELIM);
		char * age = strtok(NULL, DELIM);
		char * hobby = strtok(NULL, DELIM);

		if(!user || !loc || !age || !hobby) diewithError("Login failed");
		cout << "Sending 'loggedin' to client" << endl;
		pthread_mutex_lock(&mutex_dl_send[client_ID]);
		dl_send_q[client_ID].push("loggedin");
		pthread_mutex_unlock(&mutex_dl_send[client_ID]);
		cout << "YUPPPPPP";
	}
}
