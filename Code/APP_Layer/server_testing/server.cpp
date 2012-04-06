//Server APP_Layer
//JES

//Server
#include "all.h"
#define DELIM " "
#define PORT 8787		/* Known port number */

using namespace std;
int client_CNT;

int main(int argc, char *argv[]){

	if (argc!=1){
		fprintf(stderr, "usage:\n %s", argv[0]);
		exit(0);
	}

	//Start dl_layer
	//Initialize Physical Layer
    pthread_t dl_thread;
	int rc;
    rc = pthread_create(&dl_thread, NULL, dl_layer_server, (void *) 1);
    if (rc){
    	cout<<"Data Link Layer Thread Failed to be created"<<endl;
        exit(1);
    }

    //Assume initial client count is 0
    client_CNT = 0;

    while(1){
    	for(int ID = 0; ID <= client_CNT; ID++){
    		if(!dl_receive_q[ID].empty()){
    			handle_client(ID);
    			client_CNT++;
    		}
    	}

    }

}
