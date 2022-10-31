#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include <fcntl.h>

#define SERVER_PORT 9999 
#define MAX_LINE 256
#define MAX_PENDING 5
#define MAXNAME 12

/* structure of the client packet */
struct clientPacket{ 
	short type; 
	char uName[MAXNAME]; 
	char data[MAX_LINE]; 
}; 

/* structure of server packet*/
struct serverPacket{ 
	short type;
	char data[MAX_LINE];
	char dataList[10][12];
}; 

/* structure of Registration Table */ 
struct registrationTable{ 
	int port; 
	int sockId; 
	char roomName[MAXNAME];
 	char uName[MAXNAME]; 
}; 

/* Declare global registration table */
struct registrationTable table[10];

/* Declare global mutex variable */
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Declare global row counter for registration table */
int table_index = 0;

/* Declare initial sequence number for text package tracking*/
int seqNumber = 0;

/* Declare socket variables to be accessible from main and join_handler */
int s, new_s, port;

void *client_handler(void* Data){

	printf("CLIENT_HANDLER: In joinhandler thread\n");

	int newsock = *(int*) Data;
	char roomName[12];
	struct clientPacket clientPacket;

	// Receive initial connection package and validate type
	if(recv(newsock, &clientPacket, sizeof(clientPacket), 0) >= 0){
		if (ntohs(clientPacket.type) != 121){
			printf("CLIENT_HANDLER: Bad connection request type %d, %s\n", 
				ntohs(clientPacket.type), clientPacket.uName);
			pthread_exit(NULL);
		}
		printf("CLIENT_HANDLER: %s validated\n", clientPacket.uName);
	} else {
		printf("CLIENT_HANDLER: Could not add user %s\n", clientPacket.uName);
		pthread_exit(NULL);
	}

	struct serverPacket serverPacket;
	serverPacket.type = htons(221);
	strcpy(serverPacket.dataList[0], "test");
	strcpy(serverPacket.dataList[1], "test2");

	// Send ACK message to client
	if(send(new_s,&serverPacket,sizeof(serverPacket),0) <0) { 
		printf("CLIENT_HANDLER: ACK send failed for socket %d\n", new_s); 
	}

	// Receive room request packet and validate type
	printf("CLIENT_HANDLER: Waiting for room request packet\n");
	if(recv(newsock, &clientPacket, sizeof(clientPacket), 0) >= 0){
		if (ntohs(clientPacket.type) != 131){
			printf("CLIENT_HANDLER: Bad connection request type %d, %s\n", 
				ntohs(clientPacket.type), clientPacket.uName);
			pthread_exit(NULL);
		}
		printf("CLIENT_HANDLER: %s requested to join %s\n", 
			clientPacket.uName, clientPacket.data);
		strcpy(roomName, clientPacket.data);
	} else {
		printf("CLIENT_HANDLER: Could not add user\n");
		pthread_exit(NULL);
	}

	// lock table
	pthread_mutex_lock(&my_mutex);
	
	// Update the reg table
	table[table_index].port = port; 
	table[table_index].sockId = new_s; 
	strcpy(table[table_index].uName, clientPacket.uName); 
	strcpy(table[table_index].roomName, roomName); 
	table_index++;

	//unlock table
	pthread_mutex_unlock(&my_mutex);

	printf("CLIENT_HANDLER: Added user %d to list at sequence %d\n", table_index, seqNumber);
	printf("CLIENT_HANDLER: Sending ACK to user %d\n", table_index);

	// Send ACK message to client with users
	serverPacket.type = htons(231);
	if(send(new_s,&serverPacket,sizeof(serverPacket),0) <0) { 
		printf("CLIENT_HANDLER: ACK send failed for socket %d\n", new_s); 
	}

	while(1){

		// Listen for incoming messages from the client
		if(recv(newsock, &clientPacket, sizeof(clientPacket), 0) >= 0){
			if (ntohs(clientPacket.type) != 141){
				printf("CLIENT_HANDLER: Bad connection request type %d, %s\n", 
					ntohs(clientPacket.type), clientPacket.uName);
				pthread_exit(NULL);
			}
		} else {
			printf("ERROR:  Lost connection to server...");
			pthread_exit(NULL);
		}

		// build message packet
		serverPacket.type = htons(241);
		strcpy(serverPacket.data, strcat(strcat(clientPacket.uName, ": "), clientPacket.data));

		// lock table during message sending
		pthread_mutex_lock(&my_mutex);
	
		// Scan for users in same room and send message
		for(int i = 0; i < table_index; i++){
			if (newsock != table[i].sockId){
				if (!strcmp(table[i].roomName, roomName)){
					if(send(table[i].sockId,&serverPacket,sizeof(serverPacket),0) <0) { 
						printf("CLIENT_HANDLER: Send failed for socket %d\n", table[i].sockId); 
					} else {
						printf("CLIENT_HANDLER: Send success to socket %d\n", table[i].sockId);
					}
				}
			}
		}

		//unlock table
		pthread_mutex_unlock(&my_mutex);
	}
}

int main()
{
	int currentThread = 0;

	/* Allocate variable memory */
	struct sockaddr_in clientAddr;
	struct clientPacket clientPacket;

	/* Reserve memory for an array of chat_handler threads */
	pthread_t threads[100];

	char buf[MAX_LINE];
	int len;
	struct sockaddr_in sin;

	/* setup passive open */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("tcpserver: socket");
		exit(1);
	} 

	/* build address data structure */
	bzero((char*)&sin, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;
	sin.sin_port = htons(SERVER_PORT);


	if(bind(s,(struct sockaddr *)&sin, sizeof(sin)) < 0){
		perror("tcpclient: bind");
		exit(1);
	}
	listen(s, MAX_PENDING);
	printf("MAIN: Server listening...\n");


	/* wait for connection, then receive and print text */
	while(1){
		ssize_t response;
		printf("MAIN: Waiting on client connection\n");
		if((new_s = accept(s, (struct sockaddr *)&clientAddr, &len)) < 0){
			perror("tcpserver: accept");
			exit(1);
		} else {
			printf("MAIN: Client connected on socket %d\n", new_s);
		}

		// Send socket to join_handler thread
		pthread_create(&threads[currentThread],NULL,client_handler,&new_s);
		currentThread++;

	}
}

