## Detail explain about socket

### What Does the Socket Do in the Machine?
Under the hood:
\
A socket uses the network stack of your operating system.
\
When you create a socket, you're basically asking the OS to give you access to its network system.
\
- TCP socket
  - The OS handles things like breaking your message into packets, ensuring delivery, reassembling data, retransmitting if needed, etc.
- UDP socket
  - faster, but no delivery guarantee), it just sends datagrams with no checks.
\
Each socket is identified by:
- IP address
- Port number
- Protocol (TCP/UDP)
\
This is often called a "socket pair" (IP + port on each side) to uniquely identify the connection.

### Common question: Why is creating a socket like creating an integer in C ?
When you call something like:
```c
int sockfd = socket(AF_INET, SOCK_STREAM, 0);
```
You're literally getting back an integer file descriptor.
\
That integer is just a handle or ID that the operating system gives you to represent the socket.

Here is the explaination:
```mark
In Unix-like systems (Linux, macOS, etc.), everything is treated as a file — sockets, regular files, pipes, etc.

When you create a socket, the kernel creates a socket object internally and returns an integer file descriptor (FD).

This integer (like 3, 4, etc.) is how your application refers to that socket from user space.

You’re not actually creating a raw integer — you’re asking the OS to create a socket, and the OS tells you, “Hey, that socket is FD #4. Use this number to refer to it.”

In such way, it become convenient and consistent because you can then use the same read(), write(), close() calls on sockets that you use on files.
```

## Common question: What’s the limit on how many sockets you can create?
**File Descriptor Limits (in term of each process):**
\
Each process has a limit on how many file descriptors it can open. Since a socket is a file descriptor, this directly limits how many sockets you can create.
- For current session
  - use ulimit
	```console
	# This peek on the maximum number of open files for the current shell session (e.g., 1024 by default on many systems)
	ulimit -n
	
	# You can change it (with caution) via:
	ulimit -n 10000
	```
- For permernant configuration
  - The configutation is in **/etc/security/limits.conf**

**System-Wide Limits:**
\
The kernel has overall limits too, such as:
- Maximum number of file descriptors system-wide
  - View with: cat **/proc/sys/fs/file-max**
- Maximum number of TCP connections
  - Based on available ports: ~64K per IP address (because TCP uses 16-bit port numbers: 0–65535)
  - But not all ports are usable (some are reserved)

**Memory Limits:**
\
Each socket uses memory (for buffers, states, queues, etc). If you try to open too many, you could exhaust available memory or hit kernel limits on socket buffers.

**Real-World Socket Limit Scenarios:**
- A simple desktop app might only need 1–10 sockets at a time.
- A high-performance server (like a load balancer or web server) may need tens of thousands.
- That’s where things like epoll (Linux), kqueue (BSD/macOS), or select/poll come into play — to efficiently manage many sockets.

## General concept about socket programming
Socket programming is a way of connecting two nodes on a network to communicate with each other.
\
One socket(node) listens on a particular port at an IP, while the other socket reaches out to the other to form a connection.
\
The server forms the listener socket while the client reaches out to the server.

### Basic scoket programming in C
Please refer to the link:
\
ref: https://www.geeksforgeeks.org/socket-programming-cc/

### Additionall supplement about scoket programming in C

### Set timout for socket
In socket programming with C, you can set timeouts for the send and recv functions. This is useful to prevent your program from hanging indefinitely if the network is slow or unresponsive.
```C
// Set timeout for send
timeout.tv_sec = 3;  // 3 seconds
timeout.tv_usec = 0; // 0 microseconds
if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
	perror("setsockopt send");
	exit(EXIT_FAILURE);
}

// Set timeout for recv
if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
	perror("setsockopt recv");
	exit(EXIT_FAILURE);
}
```

### TCP and UDP server using select
The Select function is used to select between TCP and UDP sockets. This function gives instructions to the kernel to wait for any of the multiple events to occur and awakens the process only after one or more events occur or a specified time passes.
\
ref: https://www.geeksforgeeks.org/tcp-and-udp-server-using-select/
\
```C
// Server program 
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#define PORT 5000 
#define MAXLINE 1024 
int max(int x, int y) 
{ 
	if (x > y) 
		return x; 
	else
		return y; 
} 
int main() 
{ 
	int listenfd, connfd, udpfd, nready, maxfdp1; 
	char buffer[MAXLINE]; 
	pid_t childpid; 
	fd_set rset; 
	ssize_t n; 
	socklen_t len; 
	const int on = 1; 
	struct sockaddr_in cliaddr, servaddr; 
	char* message = "Hello Client"; 
	void sig_chld(int); 

	/* create listening TCP socket */
	listenfd = socket(AF_INET, SOCK_STREAM, 0); 
	bzero(&servaddr, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// binding server addr structure to listenfd 
	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
	listen(listenfd, 10); 

	/* create UDP socket */
	udpfd = socket(AF_INET, SOCK_DGRAM, 0); 
	// binding server addr structure to udp sockfd 
	bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 

	// clear the descriptor set 
	FD_ZERO(&rset); 

	// get maxfd 
	maxfdp1 = max(listenfd, udpfd) + 1; 
	for (;;) { 

		// set listenfd and udpfd in readset 
		FD_SET(listenfd, &rset); 
		FD_SET(udpfd, &rset); 

		// select the ready descriptor 
		nready = select(maxfdp1, &rset, NULL, NULL, NULL); 

		// if tcp socket is readable then handle 
		// it by accepting the connection 
		if (FD_ISSET(listenfd, &rset)) { 
			len = sizeof(cliaddr); 
			connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); 
			if ((childpid = fork()) == 0) { 
				close(listenfd); 
				bzero(buffer, sizeof(buffer)); 
				printf("Message From TCP client: "); 
				read(connfd, buffer, sizeof(buffer)); 
				puts(buffer); 
				write(connfd, (const char*)message, sizeof(buffer)); 
				close(connfd); 
				exit(0); 
			} 
			close(connfd); 
		} 
		// if udp socket is readable receive the message. 
		if (FD_ISSET(udpfd, &rset)) { 
			len = sizeof(cliaddr); 
			bzero(buffer, sizeof(buffer)); 
			printf("\nMessage from UDP client: "); 
			n = recvfrom(udpfd, buffer, sizeof(buffer), 0, 
						(struct sockaddr*)&cliaddr, &len); 
			puts(buffer); 
			sendto(udpfd, (const char*)message, sizeof(buffer), 0, 
				(struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
		} 
	} 
} 
```
