#include "all.h"



#define BUFFER_SIZE 256
int PORT;
char* HOSTNAME;

void diewithError(string message) {
        cout<<message<<endl;
        exit(1);
}

void *phy_send_t(void* num);
void *phy_receive_t(void* num);
void *phy_layer_t(void* num);


//queues
queue<string> phy_send_q;
queue<string> phy_receive_q;

int alive=1;
int connected=0;//Socket connected=1

pthread_mutex_t mutex_phy_send = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_phy_receive = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_socket = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[]){

	//Args
	if (argc<2 )
		diewithError("Incorrect number of arguments");
	PORT=atoi(argv[1]);
	//HOSTNAME=argv[2];	

	pthread_t phy_send_thread, phy_receive_thread, phy_layer_thread;
	int iret1, iret2;
	//iret1 = pthread_create( &phy_send_thread, NULL, phy_send_t, (void*) 1);
	//iret2 = pthread_create( &phy_receive_thread, NULL, phy_receive_t, (void*) 1);
    iret2 = pthread_create( &phy_layer_thread, NULL, phy_layer_t, (void*) 1);

    
	//Thread status info, consider moving to own thread
    int count=0;
	while(alive){
		sleep(1);
        count++;
		cout<<"Phy Send Thread Size:"<<phy_send_q.size()<<endl;
		cout<<"Phy Receive Thread Size:"<<phy_receive_q.size()<<endl;

		if (!phy_receive_q.empty()){
			//Echo Back
			pthread_mutex_lock( &mutex_phy_send );
			phy_send_q.push(phy_receive_q.front());
			pthread_mutex_unlock( &mutex_phy_send );
			cout<<"Echo Message Back"<<endl;			
			pthread_mutex_lock( &mutex_phy_receive );
			phy_receive_q.pop();	
			pthread_mutex_unlock( &mutex_phy_receive );
	
		}
        if (count==5){
            pthread_mutex_lock( &mutex_phy_send );
			phy_send_q.push("TeST2");
			pthread_mutex_unlock( &mutex_phy_send );
            
        }
            
    }

	//Stop threads
	alive=0;
	//pthread_cancel(phy_send_thread);
	//pthread_cancel(phy_receive_thread);
    pthread_cancel(phy_layer_thread);

    
	return 0;

}





void *phy_layer_t(void* num){

    fd_set read_flags,write_flags; // you know what these are
    struct timeval waitd;          
    int thefd;             // The socket
    char outbuff[256];     // Buffer to hold outgoing data
    char inbuff[256];      // Buffer to read incoming data into
    int err;	       // holds return values
    
    memset(&outbuff,0,sizeof(outbuff)); // memset used for portability
    thefd=phy_setup_server1(PORT); // Connect to the finger port
    
    // Mark the socket as non-blocking, for safety.
    int x;
    x=fcntl(thefd,F_GETFL,0);
    fcntl(thefd,F_SETFL,x | O_NONBLOCK);
    
    if(thefd==-1) {
        printf("Could not connect to finger server\n");
        exit(0);
    }
    cout<<"SOCKET SETUP WOOTZ"<<endl;
    
    while(1) {
        FD_ZERO(&read_flags); // Zero the flags ready for using
        FD_ZERO(&write_flags);
        FD_SET(thefd, &read_flags);
        if(!phy_send_q.empty()) FD_SET(thefd, &write_flags);
        err=select(thefd+1, &read_flags,&write_flags,
                   (fd_set*)0,&waitd);
        if(err < 0) continue;
        
        //READ SOMETHING
        if(FD_ISSET(thefd, &read_flags)) { //Socket ready for reading
            FD_CLR(thefd, &read_flags);
            memset(&inbuff,0,sizeof(inbuff));
            cout<<"trying to read"<<endl;
            if (read(thefd, inbuff, sizeof(inbuff)-1) <= 0) {
                close(thefd);
                diewithError("Socket Bug socket closed");
                break;
            }
            else{
                printf("%s\n",inbuff);
                pthread_mutex_lock( &mutex_phy_receive );
                phy_receive_q.push(inbuff);
                pthread_mutex_unlock( &mutex_phy_receive );
            }
        }
        
        //WRITE SOMETHING
        if (!phy_send_q.empty()){
            cout<<"SOMETHING IN Q"<<endl;
            
            
            if(FD_ISSET(thefd, &write_flags)) { //Socket ready for writing
                FD_CLR(thefd, &write_flags);
                cout<<"SENDING"<<endl;
                strcpy(inbuff,"TEST22");
                write(thefd,inbuff,strlen(inbuff));
                memset(&inbuff,0,sizeof(inbuff));
                
                pthread_mutex_lock( &mutex_phy_send );
                phy_send_q.pop();
                pthread_mutex_unlock( &mutex_phy_send );
            }
        }
        // now the loop repeats over again
    }

    
}



