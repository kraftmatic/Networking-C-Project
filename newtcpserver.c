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
#define MAXNAME 200

/* structure of the registration packet */
struct packet{ 
	short type; 
	char uName[MAXNAME]; 
	char mName[MAXNAME]; 
	char data[MAX_LINE]; 
}; 

/* structure of Registration Table */ 
struct registrationTable{ 
	int port; 
	int sockId; 
	char mName[MAXNAME]; 
	char uName[MAXNAME]; 
}; 

/* Declare global registration table */
struct registrationTable table[10];

/* Declare global mutex variable */
pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Declare global row counter for registration table */
int table_index = 0;

/* Declare socket variables to be accessible from main and join_handler */
int s, new_s, port;

void *join_handler(void* Data){

	printf("JOIN_HANDLER: In joinhandler thread\n");

	int newsock = *(int*) Data;
	struct packet packet_reg;

	if(recv(newsock, &packet_reg, sizeof(packet_reg), 0) >= 0){
		printf("JOIN_HANDLER: Adding user\n");
	} else {
		printf("JOIN_HANDLER: Could not add user\n");
		pthread_exit(NULL);
	}
	// Wait for registration packet and sent ack

	// lock table
	pthread_mutex_lock(&my_mutex);
	
	// Update the reg table
	table[table_index].port = port; 
	table[table_index].sockId = new_s; 
	strcpy(table[table_index].uName, packet_reg.uName); 
	strcpy(table[table_index].mName, packet_reg.mName); 
	table_index++;

	printf("JOIN_HANDLER: Added user %d to list\n", table_index);
	printf("JOIN_HANDLER: Sending ACK to user %d\n", table_index);
	if(send(new_s,&packet_reg,sizeof(packet_reg),0) <0) { 
		printf("JOIN_HANDLER: ACK send failed for socket %d\n", new_s); 
	}

	//unlock table
	pthread_mutex_unlock(&my_mutex);

	//Leave the thread
	printf("JOIN_HANDLER: Exiting join handler thread\n");
	pthread_exit(NULL);
}

void *chat_multicaster(){

	printf("CHAT_MULTICASTER: multicaster thread created\n");

	char *filename;
	char text[256];
	int fd, nread;

	filename = "input.txt";
	fd = open(filename, O_RDONLY,0);

	while(1){

		

		printf("CHAT_MULTICASTER: %d users detected\n", table_index);
		if (table_index > 0){
			read(fd, text, 256);

			struct packet data_packet;
			strcpy(data_packet.data, text);

			// Lock table during reads
			pthread_mutex_lock(&my_mutex);

			for(int i = 0; i < table_index; i++){
				printf("CHAT_MULTICASTER: Sending to client on socket %d\n", table[i].sockId);
				if(send(table[i].sockId,&data_packet,sizeof(data_packet),0) <0) { 
					printf("Send failed for socket %d\n", table[i].sockId); 
				}
			}

			//Unlock table
			pthread_mutex_unlock(&my_mutex);

		}

		//Sleep for 5 seconds
		sleep(5);

	}
}

int main()
{

	/* Allocate variable memory */
	struct sockaddr_in clientAddr;
	struct packet packet;

	/* Reserve memory for the two threads for Join Handler and Multicaster */
	pthread_t threads[2];

	/* Create join handler and multicaster threads */
	pthread_create(&threads[1],NULL,chat_multicaster,NULL);

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

		// Pick up registration packet
		printf("MAIN: Waiting on registration packet one\n");
		if(response = recv(new_s,&packet,sizeof(packet),0) < 0) { 
			printf("MAIN: Could not receive first registration packet \n"); 
			exit(1);
		} else {
			printf("Receieved first reg packet: %zd\n", response);
		}

		// Pick up registration packet
		printf("MAIN: Waiting on registration packet two\n");
		if(response = recv(new_s,&packet,sizeof(packet),0) < 0) { 
			printf("\n Could not receive first registration packet \n"); 
			exit(1);
		} else {
			printf("Receieved second reg packet: %zd\n", response);
		}

		// Send socket to join_handler thread
		pthread_create(&threads[0],NULL,join_handler,&new_s);

	}
}

