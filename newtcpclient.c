#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

#define SERVER_PORT 9999
#define MAX_LINE 256
#define MAXNAME 200

int main(int argc, char* argv[])
{
	
	struct hostent *hp;
	struct sockaddr_in sin;

	/*  structure of the registration packet */ 
	struct packet{ 
		short type; 
		char uName[MAXNAME]; 
		char mName[MAXNAME]; 
		char data[MAX_LINE]; 
	}; 
 	struct packet packet; 

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
  	packet.type = htons(121); 
	gethostname(hostbuffer, sizeof(hostbuffer));
	strcpy(packet.mName, hostbuffer);
	strcpy(packet.uName, u_name);

	/* active open */
	if((s = socket(PF_INET, SOCK_STREAM, 0)) < 0){
		perror("tcpclient: socket");
		exit(1);
	} else {
		printf("Socket established\n");
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
	} else {
		printf("Connected to socket\n");
	}

	printf("Sending packet one.\n");
	/* Send the registration packet to the server  */ 
 	if(send(s,&packet,sizeof(packet),0) <0) { 
		printf("\nSend failed\n"); 
		exit(1); 
	} 

	printf("Sending packet two.\n");
	/* Send the registration packet two to the server  */ 
 	if(send(s,&packet,sizeof(packet),0) <0) { 
		printf("\nSend failed\n"); 
		exit(1); 
	} 

	printf("Sending packet three.\n");
	if(send(s,&packet,sizeof(packet),0) <0) { 
		printf("\nSend failed\n"); 
		exit(1); 
	} 

	// Pick up ack packet response
	if(recv(s,&packet,sizeof(packet),0) < 0) { 
		printf("\nCould not receive registration packet response \n"); 
		exit(1);
	} else {
		printf("ACK received successfully \n\n"); 
	}

	// Validate registration response packet type
	// if(ntohs(packet.type) != 221){
	// 	printf("\n Invalid registration response packet format: %d\n", ntohs(packet.type)); 
	// 	exit(1);
	// }

	/* Prep the chat packet with username */
	// strcpy(packet.uName, u_name);

	/* main loop: get and send lines of text */
	while(1){


		/* Continually read for multicaster messages */
		if(recv(s,&packet,sizeof(packet),0) < 0) { 
			printf("\nCould not receive data packet \n"); 
			exit(1);
		} else {
			printf("%s", packet.data);
		}

		// buf[MAX_LINE-1] = '\0';
		// len = strlen(buf) + 1;

		// /* load buffer into chat packet and set type to 131 */
		// strcpy(packet.data, buf);
		// packet.type = htons(131); 

		// /* Send chat packet to server */
		// if(send(s,&packet,sizeof(packet),0) <0) { 
		// 	printf("\n Chat send failed\n"); 
		// 	exit(1); 
		// } else {

		// 	/* Receive chat packet response from server */
		// 	response = recv(s,&packet,sizeof(packet),0);
		// 	if(response < 0) { 
		// 		printf("\n Could not receive chat packet response \n"); 
		// 		exit(1);
		// 	} else {

		// 		/* Validate registration response packet type */
		// 		if(ntohs(packet.type) != 231){
		// 			printf("\n Invalid chat response packet format: %d\n", ntohs(packet.type)); 
		// 			exit(1);
		// 		} 
		// 		printf("%s: %s", packet.uName, packet.data);
		// 	}
		// }
	}
}
