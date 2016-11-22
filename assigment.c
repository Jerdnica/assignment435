
#include <stdio.h>
#include <stdlib.h>

// Networking include files
#include <unistd.h> //read, write etc.
#include <string.h>    //strlen
#include <netdb.h>
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr

#define BUFSIZE 1024
#define PRINT_OPAQUE_STRUCT(p)  print_mem((p), sizeof(*(p)))
/* error() - a  wrapper for perror */
void error(char *msg) {
  perror(msg);
  exit(1);
}

char* nameToIpAdress(char name)
{
	FILE* file = fopen("./info/dns.xD", "r");
	char line[32];
	/* Maximum 15 characters (Think 255.255.255.255 as an example) + 1 for '\0' */
	char* ip = malloc(15);
	while (fgets(line, sizeof(line), file))
	{
		if(line[0] == name)
		{
			strcpy( ip, line+2 );
			break;
		}
	}
	fclose(file);
	return ip;

}
char* getReceiverIpAdress(char me, char messageTarget)
{
	char *fileName = malloc(21);

	strcpy(fileName, "./info/routes_");
	fileName[14] = me;
	strcat(fileName, ".txt");
	printf("%s\n",fileName );
    FILE* file = fopen(fileName, "r"); // file opened
	char receiver;
	char line[255];
    while (fgets(line, sizeof(line), file))
	{
		if(line[0] == messageTarget) // found target in routing table
		{
			receiver = line[2];
			break;
		}
	}
    fclose(file);
	if(receiver == me)
	{
		return "-";
	}
	else return nameToIpAdress(receiver);
}


int client()
{
	char me = 'A';

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


	hostname = nameToIpAdress('B'); // Sends to ip adress of B

	/* Remove trailing '\n' character from input and replace it with '\0' */
	char *pos;
	if ((pos=strchr(hostname, '\n')) != NULL)
	    *pos = '\0';

	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
	printf("%s\n",hostname );
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
	buf[0] = me;
	buf[1] = ',';
	buf[2] = 'E';
	buf[3] = '\n';
	 // adding SENDER,RECEIVER in first line
	printf("Please enter a message: ");
	fgets(buf+4, BUFSIZE, stdin);

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
server()
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
	   hostaddrp = getReceiverIpAdress('E','A');
	      if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
	   printf("server received datagram from %s (%s)\n",
		     hostp->h_name, hostaddrp);
	   printf("server received %d/%d bytes: \n%s\n", strlen(buf), n, buf);

	   char sender = buf[0];
	   char receiver = buf[1];

	   buf[0] = receiver;
	   buf[2] = sender; //swaps sender and receiver

	   /*
	    * sendto: echo the input back to the client
	    */
	   n = sendto(sockfd, buf, strlen(buf), 0,
			 (struct sockaddr *) &clientaddr, clientlen);
	   if (n < 0)
	   error("ERROR in sendto");
   	}
}
middle(char me)
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
	   hostaddrp = getReceiverIpAdress(me, buf[2]); //send it from me to ip in buf[2]
	 //  printf("Me is middle bruh!%s\n",inet_pton(clientaddr.sin_family ,clientaddr.sin_addr.s_addr) );
	      if (hostaddrp == NULL)
		error("ERROR on inet_ntoa\n");
	   printf("received datagram from %s (%s)\n",
		     hostp->h_name, hostaddrp);
	   printf("received %d/%d bytes: \n%s\n", strlen(buf), n, buf);
	   print_mem(clientaddr, clientlen);


	   /*
	    * sendto: echo the input back to the client
	    */
	   n = sendto(sockfd, buf, strlen(buf), 0,
			 (struct sockaddr *) &clientaddr, clientlen);
	   if (n < 0)
	   error("ERROR in sendto");
   	}
}
void print_mem(void const *vp, size_t n)
{
	printf("printing\n" );
    unsigned char const *p = vp;
    for (size_t i=0; i<n; i++)
        printf("%02x\n", p[i]);
    putchar('\n');
};
int main(int argc , char *argv[])
{
	if(argc != 2)
	{
		printf("You must enter parameter\n");	return 0;
	}
	if(argv[1][0] != '-') {
		printf("Wrong parameter\n"); return 0;
	}
	char mode = argv[1][1];
	if(mode == 'A')
	{
		client();
	}
	else if(mode == 'E')
	{
		server();
	}
	else if(mode == 'B' || mode == 'C' || mode == 'D')
	{
		middle(mode);
	}
	else
	{
		printf("Parameter must be CAPITAL letter [A-E]\n");
		return 0;
	}


	printf("%c\n",mode);


    return 0;
}
