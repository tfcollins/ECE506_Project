#include "all.h"

int PORT;
char* HOSTNAME;
int vb_mode=0;

int main(int argc, char *argv[]){

	if (argc<3){
		cout<<"Need Port and Hostname"<<endl;
		exit(1);
	}
	if (argc==4)
		vb_mode=atoi(argv[3]);

	PORT=atoi(argv[1]);
	HOSTNAME=argv[2];

	//Start dl_layer
	//Initalize Physical Layer
        pthread_t dl_thread;
	int rc;
        rc = pthread_create(&dl_thread, NULL, dl_layer_client, (void *) 1);
        if (rc){
                cout<<"Data Link Layer Thread Failed to be created"<<endl;
                exit(1);
        }
	
	//DO something
	int count=0;
	while(1){
		sleep(1);
		//cout<<"Doing something, I hope (APP)"<<endl;
		count++;
		if (count==30){
			cout<<"Sending Message (APP)"<<endl;
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message1");
			pthread_mutex_unlock(&mutex_app_send);
			//sleep(1);
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message2");
			pthread_mutex_unlock(&mutex_app_send);
			//sleep(1);
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message3");
			pthread_mutex_unlock(&mutex_app_send);
			//sleep(1);
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message4");
			pthread_mutex_unlock(&mutex_app_send);
			//sleep(1);
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message5");
			pthread_mutex_unlock(&mutex_app_send);
			//sleep(1);
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message6");
			pthread_mutex_unlock(&mutex_app_send);
			
		}
		if (count==5){
			cout<<"Sending Message (APP)"<<endl;
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("Message00");
			pthread_mutex_unlock(&mutex_app_send);
		}
		if (count==400){
			cout<<"Sending Message (APP)"<<endl;
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("THISDASDLASDLSAKJGFDGJLKDFLGKFKLDGKLFDGLFDKLGFDLKGKLFDGLKFDLKGJKLFDGLKDFLKGKLDFGLKGLKDLFKGLKDFSISIIFDSFSDFASDASDDKLFFdsfkldsfjslkdfldskflsdf");
			pthread_mutex_unlock(&mutex_app_send);
			cout<<"Sending Message (APP)"<<endl;
			pthread_mutex_lock(&mutex_app_send);
			dl_send_q.push("THISDASDLASDLSAKJGFDGJLKDFLGKFKLDGKLFDGLFDKLGFDLKGKLFDGLKFDLKGJKLFDGLKDFLKGKLDFGLKGLKDLFKGLKDFSISIIFDSFSDFASDASDDKLFFdsfkldsfjslkdfldskflsdf2");
			pthread_mutex_unlock(&mutex_app_send);


		}

		if (count==40)
			break;

		if (!dl_receive_q.empty()){
			pthread_mutex_lock(&mutex_dl_receive);
			cout<<"Message Received: "<<dl_receive_q.front()<<endl;
			dl_receive_q.pop();
			pthread_mutex_unlock(&mutex_dl_receive);
						
		}
	}

	pthread_cancel(dl_thread);
	return 0;
}
