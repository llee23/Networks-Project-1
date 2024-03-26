# Semester Project 1

Name: Lauren Lee\
NDID: 902142174

## Server Design

The server first checks the validity of the given arguments.\
Once the checks are passed, I create the output directory.\
Then, I create a socket and set 2 options: 1 to allow the reuse of the socket and another to set the receive timeout.\
Once the socket is created, we bind the address to the socket and set it to listen to the given port.\
The socket is set to listen for new connections and accept them.\
When a new connection is accepted, the connection id is incremented and a new output file is created with the given id.\
We receive data from the connection with a buffer of size 512, and the contents of the buffer are written to the output file.\
If we get negative response from recv(), we check to see if it was a timeout error.\
If it is a timeout error, the file is closed and reopened to overwrite the contents with "ERROR"\
Otherwise, we assume there was an error and exit.\
If the response from recv() is 0, we close the connection and file and continue listening for new connections.

## Client Design

First the validity of the provided arguments is checked.\
If all checks are passed, we create a TCP socket using the provided IP address and port number.\
Then, a 10 second timer is set for connecting to the server. If the timer hits 0 before a successful connection, the program exits.\
If the connection is successful, we open the file we wish to send to the server.\
We read the file in chunks, keeping track of how many bytes were read and ensuring that the same number of bytes are sent before reading more from the file.\
Every time a send is successful, we reset the 10 second timer. If the time goes off, a send takes too much time and we assume a server issue and exit.\
Once the entire file is sent, we close the file and the connection and the program terminates.

## Problems and Solutions

One of the early problems I ran into was the "localhost" input for the client. I did not realize that this was causing problems when creating the socket. I went over the documentation for inet_pton() and realized that we needed the numerical hostname. I thus used getaddrinfo() to get the numerical ip address.

Another problem I ran into was implementing the 10 second timeout. I found information about the itimer in the sys/time.h library and based on the documentation thought that I had implemented it correctly. However, I was still not passing the test in the autograder. I went to office hours, and Daniel was able to help me narrow down the issue. I was not handling the signal correctly, so fixing this allowed me to pass the test.

## Additional libraries

### Client

- sys/stat.h
- netdb.h
- sys/time.h
- signal.h

### Server

- fstream
- sys/stat.h
- csignal

## Acknowledgement

I worked off of the provided base code from the project page.\

- https://man7.org/linux/man-pages/man3/getaddrinfo.3.html
- https://man7.org/linux/man-pages/man3/inet_pton.3.html
- https://linux.die.net/man/2/setitimer
- https://pubs.opengroup.org/onlinepubs/000095399/functions/setsockopt.html
- https://stackoverflow.com/questions/7738546/how-to-set-a-timeout-for-a-function-in-c
  - used to get an idea of what type of methods there are for timeouts
