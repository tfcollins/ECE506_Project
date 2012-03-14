//Travis Collins
//ECE 506
//Homework0
//EchoClient
//Based on examples from class textbook, "Computer Networks"

#include "all.h"
/*#include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <string.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h> 
 */

using namespace std;

int main(int argc, char *argv[]) {
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	cout << "Type done to quit" << endl;
	char buffer[256];
	if (argc < 4) {
		fprintf(stderr, "usage %s hostname port string\n", argv[0]);
		exit(0);
	}
	if (argc > 4) {
		printf("Error: Too many inputs\n");
		fprintf(stderr, "usage %s hostname port string\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);
	//Check if input too long
	bzero(buffer, 256);
	strcpy(buffer, argv[3]);
	if (strlen(buffer) > 32) {
		printf("Error: Input larger than 32 bytes\n");
		exit(0);
	}

	//Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		printf("ERROR opening socket\n");
		exit(0);
	}
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))
			< 0) {
		printf("ERROR connecting\n");
		exit(0);
	}

	//End of transmissions    
	strcpy(buffer, argv[3]);
	//Send data
	while (1) {
		bzero(buffer, 256);
		cout << "Enter input: ";
		cin >> buffer;
		cout << endl;
		n = write(sockfd, buffer, strlen(buffer));
		if (n < 0) {
			printf("ERROR writing to socket\n");
			exit(0);
		}
		if (strcmp(buffer, "done") == 0)
			break;
	}
	//Get echo back
	cout << "Waiting for response" << endl;
	//Get whatever is in the queue and then a message sent from server
	for (int j = 0; j < 2; j++) {
		bzero(buffer, 256);
		n = read(sockfd, buffer, 255);
		if (n < 0) {
			printf("ERROR reading from socket");
			exit(0);
		}
		//Print received message
		printf("Response: %s\n", buffer);
	}
	close(sockfd);

	return 0;
}
