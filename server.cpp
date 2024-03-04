#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

#define maxStrLen 256


int main(int argc, const char * argv[]){
  // argument validation
  if (argc != 3) {
    std::cerr << "ERROR: Invalid number of arguments";
    exit(1);
  }
  // port number validation
  int portNum = std::stoi(argv[1]);
  if (portNum <= 1023 || portNum >= 49152){
    std::cerr << "ERROR: Port number out of range";
    exit(1);
  }
  int connectionID = 0; // keep track of connected id

  // create output directory
  std::string outputDirectory = "." + std::string(argv[2]);
  mkdir(outputDirectory.c_str(), 0777);

  //set output path
  std::string outputPath = outputDirectory + "/";

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // allow others to reuse the address
  int yes = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
    perror("setsockopt");
    return 1;
  }

  // bind address to socket
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(portNum); // the server will listen on input port 
  addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // open socket on localhost IP address for server
  memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

  if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return 2;
  }

  // set socket to listen status
  if (listen(sockfd, 1) == -1) {
    perror("listen");
    return 3;
  }

  // accept a new connection from a client
  struct sockaddr_in clientAddr;
  socklen_t clientAddrSize = sizeof(clientAddr);
  int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

  if (clientSockfd == -1) {
    perror("accept");
    return 4;
  }
  connectionID ++ ; // increase connection ID once connected accepted

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // open output file stream
  std::string filepath = outputPath + std::to_string(connectionID) + ".file";
  FILE* fp = fopen(filepath.c_str(), "wb");

  char chunk[512] = {0};
  int received = 0;

  while ((received = recv(clientSockfd, chunk, 512, 0)) > 0) {
    fwrite(chunk, sizeof(char), received, fp);
    memset(chunk, '\0', sizeof(chunk));
  }
    
  // Check if there was an error while receiving
  if (received < 0) {
    std::cerr << "Error receiving data." << std::endl;
    return 1;
  }

  // close socket
  close(clientSockfd);

  // close output file
  fclose(fp);

  return 0;
}
