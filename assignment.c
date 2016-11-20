/**
 *
 * Prototype application to establish data communications between a client and a server. 
 * TODO: - calculate end-to-end delay and print it alongside the messages.
 *       - There is a problem when you send consecutive messages. The next message 
 *       is written over the previous message when you send it.
 *
 *       Example: when you first send 'abcdefgh' and then send 'ijkl', 'ijklefgh' is sent
 *       to the server.
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

// Networking include files 
#include <unistd.h> //read, write etc.
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr


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

    int socket_desc , client_sock , c , read_size;
    struct sockaddr_in server , client;
    char client_message[2000];
     
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 31313 );
     
    //Bind
    if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
    {
        //print the error message
        perror("bind failed. Error");
        return 1;
    }
    puts("bind done");
     
    //Listen
    listen(socket_desc , 3);
     
    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
     
    //accept connection from an incoming client
    client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    puts("Connection accepted");
     
    //Receive a message from client
    while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
    {
        //Send the message back to client
	printf("I received %s \n", client_message);
        write(client_sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
     
    return 0;
    }
    else if (clientmode == 1) // Do client stuff
    {
	int sock;
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
	{
	    printf("Could not create socket");
	}
	puts("Socket created");

	// Connect to the IP address specified by the user
	//
	char* ipAddress = malloc(16 * sizeof(char)); // 15 for max + 1 for '\0'
	printf("Enter IP address (Hint: its the first IP entry when you type 'sudo ifconfig' on the server)"\
		"\nConnect to: ");
	if (fgets(ipAddress, 16, stdin) == NULL)
	{
	    fprintf(stderr, "Failed to get IP address.");
	    return 1;
	}
	else
	{
	    printf("Setting socket ip address to: %s\n", ipAddress);
	}
	
	server.sin_addr.s_addr = inet_addr(ipAddress);
	server.sin_family = AF_INET;
	server.sin_port = htons( 31313 );

	//Connect to remote server
	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
	    perror("connect failed. Error");
	    return 1;
	}

	puts("Connected\n");

	//keep communicating with server
	while(1)
	{
	    printf("Enter message : ");
	    scanf("%s" , message);

	    //Send some data
	    if( send(sock , message , strlen(message) , 0) < 0)
	    {
		puts("Send failed");
		return 1;
	    }

	    //Receive a reply from the server
	    if( recv(sock , server_reply , 2000 , 0) < 0)
	    {
		puts("recv failed");
		break;
	    }

	    puts("Server reply :");
	    puts(server_reply);
	}

	close(sock);
	return 0;
    }
    else // Invalid argument
    {
	printf("No mode specified. Usage:\n./assignment -s to act as a server,"\
		"-c to act as a client\n");
	return 0;
    }
}

