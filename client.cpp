#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <sys/stat.h>
#include <netdb.h>


#define maxStrLen 256

int main(int argc, const char * argv[]) {
  // argument validation
  if (argc != 4) {
    std::cerr << "ERROR: Invalid number of arguments";
    exit(1);
  }
  // port number validation
  int portNum = std::stoi(argv[2]);
  if (portNum <= 1023 || portNum >= 49152){
    std::cerr << "ERROR: Port number out of range";
    exit(1);
  }
  // hostname/IP validation
  char hostname[maxStrLen];
  char ip[maxStrLen];
  strcpy(hostname, argv[1]);

  struct addrinfo hints, *res;
  memset(&hints, '\0', sizeof(hints));
  int error = getaddrinfo( hostname, argv[2], &hints, &res);
  if (error ==0){
    inet_ntop(AF_INET, &res->ai_addr->sa_data[2], ip, sizeof(ip));
  }

  struct sockaddr_in sa;

  if (inet_pton(AF_INET, ip, &(sa.sin_addr))==0){
    std::cerr << "ERROR: Invalid hostname";
    exit(1);
  }

  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0){
    std::cerr << "ERROR: opening socket";
    exit(1);
  }

  struct sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(portNum);  // open a socket on input port # of the server
  serverAddr.sin_addr.s_addr = inet_addr(ip); // use input ip as the IP address of the server to set up the socket
  memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));

  // connect to the server
  if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
    perror("connect");
    return 2;
  }

  struct sockaddr_in clientAddr;
  socklen_t clientAddrLen = sizeof(clientAddr);
  if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
    perror("getsockname");
    return 3;
  }

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  std::cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << std::endl;


  // open file to send and get file size
  char filename[maxStrLen];
  struct stat file_stat;
  int fd;
  strcpy(filename, argv[3]);
  FILE* fp = fopen(filename, "rb");
  fd = fileno(fp);
  fstat(fd, &file_stat);

  std::string fileSize = std::to_string(file_stat.st_size);

  // send file size to server
  // if (send(sockfd, &fileSize, sizeof(fileSize), 0) < 0) {
  //   std::cerr << "ERROR: sending file size";
  //   std::cout << "attempting to send file size again";
  //   bool success = false;

  //   while (!success){
  //     success = (send(sockfd, &fileSize, sizeof(fileSize), 0) < 0) ? false : true;
  //   }
  // }
    
  // send file to server in chunks
  char chunk[512] = {0};
  int sent = 0;
  int nbytes = 0;
  while ( (nbytes = fread(chunk, sizeof(char), 512, fp)) > 0) {
    int offset = 0;
    while ((sent = send(sockfd, chunk + offset, nbytes, 0)) > 0) {
            offset += sent;
            nbytes -= sent;
    }
  }
  //close file
  fclose(fp);
  // close socket
  close(sockfd);

  return 0;
}
