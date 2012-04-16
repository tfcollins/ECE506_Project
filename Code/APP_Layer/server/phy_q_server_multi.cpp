#include "all.h"
#include <bitset>
#include <sys/signal.h>

//#define PORT 5001
#define BUFFER_SIZE 256

//Globals
char* HOSTNAME;
int clients=0;
pthread_t dl_thread[20]; 

//Function prototype
int get_crc(string str);

//Error Message Printing
void diewithError(string message) {
        cout<<message<<endl;
        exit(1);
}

//Verbose Message Printing
void verbose(string message){
	if (vb_mode) cout<<message<<endl;
}

//Socket Message Structure
typedef struct{
	int *socket;
	int client;
} info;

//TFC
//Main Physical Layer Thread (Accepts Connections)
void *phy_layer_server(void *num){

	verbose("Physical Active (PHY)");
	//Setup Socket
        int sockfd, portno;
        socklen_t clilen;
        void *newsockfd;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                diewithError("ERROR opening socket (PHY)");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = PORT;

	//Basic Socket Parameters
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                diewithError("ERROR on binding (PHY)");
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);

	//Threads
	int *socket[10];
	pthread_t phy_layer_thread[10];
	info client_info[20];
	int client=0;
	int rc;

	try{
		while(1){
			//Wait for clients
			socket[client]=(int *) malloc(sizeof(int));
			verbose("Waiting for clients (PHY)");
			*socket[client]=accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			verbose("Socket Accepted (PHY)");
	    
			 // Mark the socket as non-blocking, for safety.
			int x;
			x=fcntl(*socket[client],F_GETFL,0);
			fcntl(*socket[client],F_SETFL,x | O_NONBLOCK);
			if(*socket[client]==-1) diewithError("ERROR: Could not connect to client (PHY)");
			verbose("Socket Made non-blocking (PHY)");
			
			client_info[client].socket=socket[client];
			client_info[client].client=client;
			//Spawn Thread
			//cout<<&phy_layer_thread[client]<<endl;
			rc = pthread_create( &phy_layer_thread[client], NULL, phy_layer_t, &client_info[client]);
			if(rc)diewithError("ERROR; return code from pthread_create() (PHY)");
			pthread_detach(phy_layer_thread[client]);
			verbose("Thread spawned for client (PHY)");
			client++;
			clients=client;//Global
		}
	}
	//Something went wrong :(
	catch(int e) {
		//Stop threads
		for(int i=0;i<client;i++)
			pthread_cancel(phy_layer_thread[i]);
    	}
	return 0;

}



//TFC
//Physical Layer thread for individual user
void *phy_layer_t(void* num){

    //Recreate structure
    info temp_info = *((info*)(num));

    //Set socket num
    int thefd;             // The socket
    thefd = *temp_info.socket;

    //Set User
    int client;
    client = temp_info.client;

    //Other declarations
    fd_set read_flags,write_flags; // you know what these are
    struct timeval waitd;          
    char outbuff[256];     // Buffer to hold outgoing data
    char inbuff[256];      // Buffer to read incoming data into
    int err;	       // holds return values
    string previous;
	    
    memset(&outbuff,0,sizeof(outbuff)); // memset used for portability
    int crc;
    int total=0;

    //Start DL Layer for this client
    //pthread_t dl_thread;
    int rc;
    rc = pthread_create(&dl_thread[client], NULL, dl_layer_server, &client);
    if (rc){
    		diewithError("ERROR: Data Link Layer Thread Failed to be created (PHY)");
    }

    //Wait to send or receive messages
    while(1) {
	//cout<<"PHY LOOP\r";
	//Check if DL Layer thread is alive
	if(pthread_kill(dl_thread[client], 0))
		verbose("ERROR: DL Thread died for client (PHY)");		

        FD_ZERO(&read_flags); // Zero the flags ready for using
        FD_ZERO(&write_flags);
        FD_SET(thefd, &read_flags);

    	memset(&outbuff,0,256); // memset used for portability
    	memset(&inbuff,0,256); // memset used for portability
        
	//Something wants to be sent
	pthread_mutex_lock( &mutex_phy_send[client] );
	if(!phy_send_q[client].empty()){
		 FD_SET(thefd, &write_flags);
	}
	pthread_mutex_unlock( &mutex_phy_send[client] );

        err=select(thefd+1, &read_flags,&write_flags,(fd_set*)0,&waitd);
	if(err < 0) continue;//ERROR try next time around
        
        //READ SOMETHING
        if(FD_ISSET(thefd, &read_flags)) { //Socket ready for reading
            FD_CLR(thefd, &read_flags);
            memset(&inbuff,0,sizeof(inbuff));
            if (read(thefd, inbuff, sizeof(inbuff)-1) <= 0) {
                close(thefd);
                verbose("Socket Closed (PHY)");
                break;
            }
	    previous.append(string(inbuff));
	    if(previous[previous.length()-1]!='\b'){//Check if we have full message
                continue;
            }
		strcpy(inbuff,previous.c_str());
                previous="";//reset temp buffer
		verbose("FULL MESSAGE: "+string(inbuff)+"| (PHY)");    
		//count messages
		int messages=0;
		int saved=0;
		string temp_str="";
		//Remove multiple messages from stream
		for (int p=0;p<strlen(inbuff);p++){
			if (inbuff[p]=='\b'){
				char *pch;
				pch = new char [temp_str.size()+1];
				strcpy(pch,temp_str.c_str());
				char crc_c[2];
				crc_c[0]=pch[strlen(pch)-1];
				crc_c[1]='\0';
				crc=atoi(crc_c);
				//remove crc
				pch[strlen(pch)-1]= '\0';	
				//cout<<"Received Individual: "+string(pch)+" (PHY)"<<endl;

				//Check CRC
				if(get_crc(string(pch))==crc){				
					verbose("Correct CRC");
				}	
				else{//Drop Packet
					verbose("ERROR: CRC Checksum Failed (PHY)");
					continue;
				}
				pthread_mutex_lock( &mutex_phy_receive[client] );
				phy_receive_q[client].push(pch);
				pthread_mutex_unlock( &mutex_phy_receive[client] );
				temp_str.clear();
			}
			else//Accumulate Message
				temp_str=temp_str+inbuff[p];
			

		
            }
        }
        
        //WRITE SOMETHING
    	if(FD_ISSET(thefd, &write_flags)) { //Socket ready for writing
		FD_CLR(thefd, &write_flags);
		
		pthread_mutex_lock( &mutex_phy_send[client] );
		string temp=phy_send_q[client].front();
		pthread_mutex_unlock( &mutex_phy_send[client] );

		//CRC
		crc=get_crc(temp);
		char crc_s[5];
		sprintf(crc_s,"%d",crc);
		temp.append(crc_s);
		temp.append("\b");

		strcpy(outbuff,temp.c_str());

		//Send Message
		verbose("Sending (PHY): "+string(outbuff));
		pthread_mutex_lock( &mutex_phy_send[client] );
		phy_send_q[client].pop();
		pthread_mutex_unlock( &mutex_phy_send[client] );

		write(thefd,outbuff,strlen(outbuff));
		memset(&outbuff,0,sizeof(outbuff));
		
	}
	//}
        // now the loop repeats over again
    }

	close(thefd);//close socket and leave
    
}

//TFC
//Calculate checksum
int get_crc(string str){

	bitset<8> mybits=0;
        bitset<8> crc=0;
        for (int i=0;i<str.size();i++){

                mybits=bitset<8>(str[i]);
		crc=crc ^ mybits;
        }
        int g=(int) crc[0];
	return g;
}




