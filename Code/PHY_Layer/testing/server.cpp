//Travis Collins
//ECE 506

#include "all.h"

#define BUFFER_SIZE 128

using namespace std;

//Function Prototypes
int handle_client(void *ptr);

int main(int argc, char *argv[])
{
     //Setup Main Socket
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        diewithError("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     //Basic Socket Parameters
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              diewithError("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
	
     //Wait for messages
     int processed=0;
     int thread_ID=0;
     int thread_status[10];
     pthread_t threads[10];
     while(1){
	     //Create new thread
     	     //pthread_t threads[thread_ID];
	     //Block until message received
	     newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);
	     //Spawn Thread
	     thread_status[thread_ID] = pthread_create( &threads[thread_ID], NULL, handle_client, (void*) newsockfd);
     	     thread_ID++;
     }

     close(sockfd);
     return 0; 
}




int handle_client(void *ptr){

     int n,socketfd;
     socketfd=(int) ptr;
     char buffer[BUFFER_SIZE];
     if (socketfd < 0) diewithError("ERROR on accept");
     bzero(buffer,BUFFER_SIZE);
     n = read(socketfd,buffer,BUFFER_SIZE-1);
     if (n < 0) diewithError("ERROR reading from socket");

     //LOOK AT MESSAGE
     printf(buffer);
     n = write(socketfd,buffer,strlen(buffer));//Send echoed message back
     if (n < 0) diewithError("ERROR writing to socket");
     close(socketfd);	
     return 0;
}
