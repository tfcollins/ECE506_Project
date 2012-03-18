//Travis Collins
//ECE 506

#include "all.h"

#define BUFFER_SIZE 128

void diewithError(string message) {
        cout<<message<<endl;
        exit(1);
}
//Function Prototypes
void *handle_client(void *socketFD);
int char_in_string(string word);

int main(int argc, char *argv[]) {
	//Setup Main Socket
	int sockfd, portno;
	socklen_t clilen;
	void *newsockfd;
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
		fprintf(stderr, "ERROR, no port provided\n");
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
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
		diewithError("ERROR on binding");
	listen(sockfd, 5);
	clilen = sizeof(cli_addr);

	//Wait for messages
	int processed = 0;
	int thread_ID = 0;
	int rc;
	int *socketID[10];
	pthread_t threads[10];

	while (1) {
		cout << "Sockets Created " << thread_ID << endl;
		//Block until message received
		socketID[thread_ID] = (int *) malloc(sizeof(int));
		*socketID[thread_ID] = accept(sockfd, (struct sockaddr *) &cli_addr,
				&clilen);
		//Spawn Thread
		rc = pthread_create(&threads[thread_ID], NULL, handle_client, (void *) socketID[thread_ID]);

		if (rc)
			diewithError("ERROR; return code from pthread_create()");
		thread_ID++;
	}

	close(sockfd);
	return 0;
}

//Accepts socket descriptor of new client
void *handle_client(void *socketFD) {

	int n;
	int *id_ptr, socketfd;
	id_ptr = (int *) socketFD;
	socketfd = *id_ptr;
	cout << "Socket FD: " << socketFD << endl;
	char buffer[BUFFER_SIZE];
	if (socketfd < 0)
		diewithError("ERROR on accept");
	//Overall receive loop
	while (1) {
		bzero(buffer, BUFFER_SIZE);
		//Wait for message, will run infinitely if socket closed on client
		n = read(socketfd, buffer, BUFFER_SIZE - 1);
		if (n < 0)
			diewithError("ERROR reading from socket");
		cout << "Buffer: " << buffer << endl;

		//Send Message Back
		cout << "Comparing strings" << endl;
		cout << strcmp(buffer, "done") << endl;
		if (strcmp(buffer, "done") == 0) {
			cout << "Sending response: " << buffer << endl;
			n = write(socketfd, buffer, strlen(buffer)); //Send echoed message back
			if (n < 0)
				diewithError("ERROR writing to socket");
			cout << "Quitting" << endl;
			break;
		} else
			n = write(socketfd, buffer, strlen(buffer)); //Send echoed message back

	}

	//LOOK AT MESSAGE
	//cout<<"Message: "<<buffer<<endl;
	//if (strcmp(buffer,"quit")==0)
	//break;
	close(socketfd);
	return 0;
}

int char_in_string(string word) {
	for (int i = 0; i < word.length(); i++)
		if (word[i] == 'q')
			return 1;

	return 0;

}

