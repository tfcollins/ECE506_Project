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
using namespace std;

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