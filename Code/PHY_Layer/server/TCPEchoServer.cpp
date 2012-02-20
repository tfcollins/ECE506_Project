
#include "Socket.h"  // For Socket, ServerSocket, and SocketException
#include <iostream>           // For cerr and cout
#include <cstdlib>            // For atoi()

using namespace std;

const unsigned int RCVBUFSIZE = 32;    // Size of receive buffer

void HandleTCPClient(TCPSocket *sock); // TCP client handling function

int main(int argc, char *argv[]) {
  if (argc != 2) {                     // Test for correct number of arguments
    cerr << "Usage: " << argv[0] << " <Server Port>" << endl;
    exit(1);
  }

  unsigned short echoServPort = atoi(argv[1]);  // First arg: local port

  try {
    TCPServerSocket servSock(echoServPort);     // Server Socket object
  
    for (;;) {   // Run forever
      HandleTCPClient(servSock.accept());       // Wait for a client to connect
    }
  } catch (SocketException &e) {
    cerr << e.what() << endl;
    exit(1);
  }
  // NOT REACHED

  return 0;
}

// TCP client handling function
void HandleTCPClient(TCPSocket *sock) {
  cout << "Handling client ";
  try {
    cout << sock->getForeignAddress() << ":";
  } catch (SocketException e) {
    cerr << "Unable to get foreign address" << endl;
  }
  try {
    cout << sock->getForeignPort();
  } catch (SocketException e) {
    cerr << "Unable to get foreign port" << endl;
  }
  cout << endl;

  // Send received string and receive again until the end of transmission
  char echoBuffer[RCVBUFSIZE];
  int recvMsgSize;
  while ((recvMsgSize = sock->recv(echoBuffer, RCVBUFSIZE)) > 0) { // Zero means
                                                         // end of transmission
    // Echo message back to client
    sock->send(echoBuffer, recvMsgSize);
  }
  delete sock;
}
