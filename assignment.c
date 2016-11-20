#include <stdio.h>
#include <stdlib.h>

// Networking include files 
#include <unistd.h> //read, write etc.
#include <string.h>    //strlen
#include <netdb.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

#define BUFSIZE 1024

/* error() - a  wrapper for perror */
void error(char *msg) {
  perror(msg);
  exit(1);
}



int main(int argc, char** argv)
{
    int servermode, clientmode, ch;
    servermode = clientmode = 0;
    
    // Get the intended action whether it is acting as a server or a client
    while (--argc > 0 && (*++argv)[0] == '-')
    {
        while (ch = *++argv[0]) // First argument of the -argument
            switch (ch) {
                case 's':
		    servermode = 1;
                    break;
                case 'c':
		    clientmode = 1;
                    break;
                default:
                    printf("Illegal option as argument: %c\n",
                            ch);
                    argc = 0;
            }
    }
    // If the host wants to act like a server and a client at the same time.
    if (servermode == 1 && clientmode == 1)
    {
	fprintf(stderr, "You can't act as both a client and a server!\n");
	return 0;
    }
    else if (servermode == 1) // Does server stuff instead.
    {
	int sockfd; /* socket file descriptor*/
	int portno = 8888; /* port to listen on, by default 8888 */
	int clientlen; /* byte size of client's address */
	struct sockaddr_in serveraddr; /* server's addr */
	struct sockaddr_in clientaddr; /* client addr */
	struct hostent *hostp; /* client host info */
	char buf[BUFSIZE]; /* message buf */
	char *hostaddrp; /* dotted decimal host addr string */
	int optval; /* flag value for setsockopt */
	int n; /* message byte size */

	/* socket: create the parent socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
	  error("ERROR opening socket");

	/* setsockopt: Handy debugging trick that lets 
	 * us rerun the server immediately after we kill it; 
	 * otherwise we have to wait about 20 secs. 
	 * Eliminates "ERROR on binding: Address already in use" error. 
	 */
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
		   (const void *)&optval , sizeof(int));

	/* * build the server's Internet address */
	
	bzero((char *) &serveraddr, sizeof(serveraddr));

	/* Set server IP address properties */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	/* * bind: associate the parent socket with a port */
	if (bind(sockfd, (struct sockaddr *) &serveraddr, 
		 sizeof(serveraddr)) < 0) 
	  error("ERROR on binding");

	/* 
	 * main loop: wait for a datagram, then echo it back
	 */
	clientlen = sizeof(clientaddr);
	while (1) {
	   /*
	    * recvfrom: receive a UDP datagram from a client
	    * Note: clientaddr is of type struct sockaddr_in, which 
	    * can be safely typecasted into a struct sockaddr.
	    */
	   bzero(buf, BUFSIZE);
	   n = recvfrom(sockfd, buf, BUFSIZE, 0,
			 (struct sockaddr *) &clientaddr, &clientlen);
	   if (n < 0)
	       error("ERROR in recvfrom");

	   /* * gethostbyaddr helps to  determine who sent the datagram */
	   hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
				  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
	   if (hostp == NULL)
		error("ERROR on gethostbyaddr");
	   hostaddrp = inet_ntoa(clientaddr.sin_addr);
	      if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
	   printf("server received datagram from %s (%s)\n", 
		     hostp->h_name, hostaddrp);
	   printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf);
	      
	   /* 
	    * sendto: echo the input back to the client 
	    */
	   n = sendto(sockfd, buf, strlen(buf), 0, 
			 (struct sockaddr *) &clientaddr, clientlen);
	   if (n < 0) 
		error("ERROR in sendto");
       }
    }
    else if (clientmode == 1) // Do client stuff
    {
	int sockfd, n;
	int serverlen;
	struct sockaddr_in serveraddr;
	struct hostent *server;
	char *hostname;
	char buf[BUFSIZE];
	int portno = 8888; /* Set the default port to 8888 for testing */


	/* socket: create the socket */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
	    error("ERROR opening socket");


	/* Fill the hostname with the IP address supplied 
	 * by the user of the program */

	/* Maximum 15 characters (Think 255.255.255.255 as an example) + 1 for '\0' */
	hostname = calloc(16, sizeof(char));
	printf("Enter the IP address to send a message: ");
	fgets(hostname, 16, stdin);

	/* Remove trailing '\n' character from input and replace it with '\0' */
	char *pos;
	if ((pos=strchr(hostname, '\n')) != NULL)
	    *pos = '\0';

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	if (server == NULL) {
	    fprintf(stderr,"ERROR, no such host as %s\n", hostname);
	    exit(0);
	}

	/* build the server's Internet address */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
	      (char *)&serveraddr.sin_addr.s_addr, server->h_length);
	serveraddr.sin_port = htons(portno);

	/* get a message from the user */
	bzero(buf, BUFSIZE);
	printf("Please enter a message: ");
	fgets(buf, BUFSIZE, stdin);

	/* send the message to the server */
	serverlen = sizeof(serveraddr);
	n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
	if (n < 0) 
	  error("ERROR in sendto");
	
	/* print the server's reply */
	n = recvfrom(sockfd, buf, strlen(buf), 0, &serveraddr, &serverlen);

	if (n < 0) 
	  error("ERROR in recvfrom");

	printf("Echo from server: %s", buf);
	return 0;
    }
    else // Invalid argument
    {
	printf("No mode specified. Usage:\n./assignment -s to act as a server,"\
		"-c to act as a client\n");
	return 0;
    }
}

