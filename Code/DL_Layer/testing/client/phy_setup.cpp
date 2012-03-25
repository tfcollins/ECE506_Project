/*
 * phy_setup.cpp
 *
 *  Created on: Mar 13, 2012
 *      Author: six
 *
 *      This function will setup sockets, and
 *      return the socket file descriptor
 *      
 */

#include "all.h"


//CLIENT
int phy_setup(int port, struct hostent *serv){

	int sock;
	struct sockaddr_in serv_addr;

	//Initiate socket, diewithError
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) diewithError("socket() failed");
	//Setup server address to connect
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *) serv->h_addr, (char *) &serv_addr.sin_addr.s_addr,serv->h_length);
	serv_addr.sin_port = htons(port);
	//Connect to socket 
	if (connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		diewithError("connect() failed");
	}
	return sock;
}

//SERVER1
int phy_setup_server1(int port){

	 //Setup Socket
        int sockfd, portno;
        socklen_t clilen;
        void *newsockfd;
        struct sockaddr_in serv_addr, cli_addr;
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
                diewithError("ERROR opening socket");
        bzero((char *) &serv_addr, sizeof(serv_addr));
        portno = port;

        //Basic Socket Parameters
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(portno);
        if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
                diewithError("ERROR on binding");
        listen(sockfd, 5);
        clilen = sizeof(cli_addr);

	int sock=accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	cout<<"Socket Accepted"<<endl;

	return sock;

}

