#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#define SERVER_PORT 9999
#define MAX_LINE 256
#define MAXNAME 12

/*  structure of the client packet */ 
struct clientPacket{ 
	short type;
	char uName[MAXNAME]; 
	char data[MAX_LINE]; 
}; 
struct clientPacket clientPacket; 

/* structure of server packet*/
struct serverPacket{ 
	short type;
	char data[MAX_LINE];
	char dataList[10][12];
}; 
struct serverPacket serverPacket;

void *chat_receiver(void* Data){

	int newsock = *(int*) Data;

	// Receive initial connection package and validate type
	printf("Receiving on socket %d\n", newsock);
	if(recv(newsock, &serverPacket, sizeof(serverPacket), 0) >= 0){
		if (ntohs(serverPacket.type) != 231){
			printf("JOIN_HANDLER: Bad connection request type %d\n", 
				ntohs(serverPacket.type));
			pthread_exit("Session established.\n");
		}
	} else {
		printf("ERROR:  Lost connection to server...");
		pthread_exit(NULL);
	}

	while(1){
		if(recv(newsock, &serverPacket, sizeof(serverPacket), 0) >= 0){
			if (ntohs(serverPacket.type) != 241){
				printf("JOIN_HANDLER: Bad connection request type %d\n", 
					ntohs(serverPacket.type));
				pthread_exit("Session established.\n");
			}
			printf("%s", serverPacket.data);
		} else {
			printf("ERROR:  Lost connection to server...");
			pthread_exit(NULL);
	}
	}
	
}

int main(int argc, char* argv[])
{
	
	struct hostent *hp;
	struct sockaddr_in sin;

	/* Reserve memory for the receiver thread */
	pthread_t threads[2];



	char *host;
	char *u_name;
	char buf[MAX_LINE];
	char hostbuffer[256];
	int s;
	int len;
	int response;

	/* Validate that the correct number of arguments are added */
	if(argc == 3){
		host = argv[1];
		u_name = argv[2];
	}else{
		fprintf(stderr, "usage:client server user_name \n");
		exit(1);
	}

	/* translate host name into peer's IP address */
	hp = gethostbyname(host);
	if(!hp){
		fprintf(stderr, "unknown host: %s\n", host);
		exit(1);
	}

	/* Constructing the registration packet at client */  
  	clientPacket.type = htons(121); 
	gethostname(hostbuffer, sizeof(hostbuffer));
	strcpy(clientPacket.uName, u_name);

	/* active open */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("tcpclient: socket");
		exit(1);
	}

	/* build address data structure */
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	bcopy(hp->h_addr, (char *)&sin.sin_addr, hp->h_length);
	sin.sin_port = htons(SERVER_PORT);

	if(connect(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("tcpclient: connect");
		close(s);
		exit(1);
	}

	/* Send the registration packet to the server  */ 
 	if(send(s,&clientPacket,sizeof(clientPacket),0) <0) { 
		printf("\nSend failed\n"); 
		exit(1); 
	} 

	// Pick up server ack response with open rooms
	if(recv(s,&serverPacket,sizeof(serverPacket),0) < 0) { 
		printf("\nCould not receive registration packet response \n"); 
		exit(1);
	} else {
		if(ntohs(serverPacket.type) != 221){
			printf("\nInvalid response from server \n"); 
			exit(1);
		} 
	}

	// Print out a list of available rooms
	printf("\nRoom List: \n");
	for(int i = 0; i < 10; i++){
		if (strcmp(serverPacket.dataList[i], "")){
			printf("%s\n", serverPacket.dataList[i]);
		}
	}

	// Prompt user for room to enter
	char roomName[12];
    printf("Enter Room Name: ");
    fgets(roomName, sizeof(roomName), stdin);  // read string

	clientPacket.type = htons(131);
	strcpy(clientPacket.data, roomName);

	/* Send the registration packet to the server  */ 
 	if(send(s,&clientPacket,sizeof(clientPacket),0) <0) { 
		printf("\nSend failed\n"); 
		exit(1); 
	} 
	pthread_create(&threads[0],NULL,chat_receiver,&s);

	sleep(1);
	/* main loop: send lines of text */
	while(fgets(buf, sizeof(buf), stdin)){
		buf[MAX_LINE-1] = '\0';
		len = strlen(buf) + 1;

		/* load buffer into chat packet and set type to 131 */
		strcpy(clientPacket.data, buf);
		clientPacket.type = htons(141); 

		/* Send chat packet to server */
		if(send(s,&clientPacket,sizeof(clientPacket),0) <0) { 
			printf("\n Chat send failed\n"); 
			exit(1); 
		} 

		sleep(1);

	}
}
