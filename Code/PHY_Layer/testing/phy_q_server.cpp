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

	//pthread_t phy_send_thread, phy_receive_thread, phy_layer_thread;
	//int iret1, iret2;
	//iret1 = pthread_create( &phy_send_thread, NULL, phy_send_t, (void*) 1);
	//iret2 = pthread_create( &phy_receive_thread, NULL, phy_receive_t, (void*) 1);

	//Setup Socket
        int sockfd, portno;
        socklen_t clilen;
        void *newsockfd;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                diewithError("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = PORT;

	//Basic Socket Parameters
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                diewithError("ERROR on binding");
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);

	//Threads
	int *socket[10];
	pthread_t phy_layer_thread[10];

	int client=0;
	int rc;

	try{
		while(1){
			//Wait for clients
			socket[client]=(int *) malloc(sizeof(int));
			*socket[client]=accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			cout<<"Socket Accepted"<<endl;
	    
			 // Mark the socket as non-blocking, for safety.
			int x;
			x=fcntl(*socket[client],F_GETFL,0);
			fcntl(*socket[client],F_SETFL,x | O_NONBLOCK);
			if(*socket[client]==-1) diewithError("Could not connect to client");
			cout<<"SOCKET SETUP FOR NONBLOCKING"<<endl;

			//Spawn Thread
			rc = pthread_create( &phy_layer_thread[client], NULL, phy_layer_t, (void*) socket[client]);
			if(rc)diewithError("ERROR; return code from pthread_create()");
			client++;

		}
	}
	catch(int e) {
		//Stop threads
		alive=0;
		for(int i=0;i<client;i++)
			pthread_cancel(phy_layer_thread[i]);
    	}
	return 0;

}





void *phy_layer_t(void* num){

    //Set socket num
    int n;
    int thefd;             // The socket
    int *id_ptr, socketfd;
    id_ptr = (int *) num;
    thefd = *id_ptr;

    //Other declarations
    fd_set read_flags,write_flags; // you know what these are
    struct timeval waitd;          
    char outbuff[256];     // Buffer to hold outgoing data
    char inbuff[256];      // Buffer to read incoming data into
    int err;	       // holds return values
    
    memset(&outbuff,0,sizeof(outbuff)); // memset used for portability
    
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
                cout<<"Socket Closed"<<endl;
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

	close(thefd);//close socket and leave
    
}



