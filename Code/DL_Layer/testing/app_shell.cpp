#include "all.h"

int PORT;

int main(int argc, char *argv[]){

	if (argc<2){
		cout<<"Need Port"<<endl;
		exit(1);
	}
	PORT=atoi(argv[1]);

	//Start dl_layer
	//Initalize Physical Layer
        pthread_t dl_thread;
	int rc;
        rc = pthread_create(&dl_thread, NULL, dl_layer_server, (void *) 1);
        if (rc){
                cout<<"Data Link Layer Thread Failed to be created"<<endl;
                exit(1);
        }
	
	//DO something
	int count=0;
	while(1){
		sleep(1);
		cout<<"Doing something, I hope (APP)"<<endl;
		count++;
		if (count==12){
			cout<<"Sending Message (APP)"<<endl;
			dl_send_q.push("Message from APP_Layer");
		}

		if (count==50)
			break;		
	}

	pthread_cancel(dl_thread);
	return 0;
}
