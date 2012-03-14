//Generic APP_Layer testing
//JES

#include "all.h"

void diewithError(string message) {
        cout << message << endl;
        exit(1);
}

//Known port number to connect
#define PORTNO 8787

int main(int argc, char *argv[]) {
	int sockfd, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if (argc < 3) {
		fprintf(stderr, "usage: %s hostname command data\n", argv[0]);
		exit(0);
	}
	if (argc > 10) {
		printf("Error: Too many inputs\n");
		fprintf(stderr, "usage %s hostname command data\n", argv[0]);
		exit(0);
	}

	char buffer[256];

	//Check if input too long
	bzero(buffer, 256);
	strcpy(buffer, argv[3]);
	if (strlen(buffer) > 255) {
		printf("Error: Greater than one buffer!\n");
		exit(0);
	}

	server = gethostbyname(argv[1]);
	if (server == NULL) diewithError("Error, no such host!");

	sockfd = 100;//phy_setup(PORTNO, server);

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
