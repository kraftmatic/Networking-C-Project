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
 	struct packet packet_reg; 

	/*  structore of the chat packet */
	struct chat_packet{
		short type;
		char uName[MAXNAME];
		char data[MAX_LINE];
	};
	struct chat_packet chat_pack;

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
		fprintf(stderr, "unkown host: %s\n", host);
		exit(1);
	}

	/* Constructing the registration packet at client */  
  	packet_reg.type = htons(121); 
	gethostname(hostbuffer, sizeof(hostbuffer));
	strcpy(packet_reg.mName, hostbuffer);
	strcpy(packet_reg.uName, u_name);

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
 	if(send(s,&packet_reg,sizeof(packet_reg),0) <0) { 
		printf("\n Send failed\n"); 
		exit(1); 
	} 

	// Pick up registration packet response
	if(recv(s,&packet_reg,sizeof(packet_reg),0) < 0) { 
		printf("\n Could not receive registration packet response \n"); 
		exit(1);
	}

	// Validate registration response packet type
	if(ntohs(packet_reg.type) != 221){
		printf("\n Invalid registration response packet format: %d\n", ntohs(packet_reg.type)); 
		exit(1);
	}

	/* Prep the chat packet with username */
	strcpy(chat_pack.uName, u_name);

	/* main loop: get and send lines of text */
	while(fgets(buf, sizeof(buf), stdin)){
		buf[MAX_LINE-1] = '\0';
		len = strlen(buf) + 1;

		/* load buffer into chat packet and set type to 131 */
		strcpy(chat_pack.data, buf);
		chat_pack.type = htons(131); 

		/* Send chat packet to server */
		if(send(s,&chat_pack,sizeof(chat_pack),0) <0) { 
			printf("\n Chat send failed\n"); 
			exit(1); 
		} else {

			/* Receive chat packet response from server */
			response = recv(s,&chat_pack,sizeof(chat_pack),0);
			if(response < 0) { 
				printf("\n Could not receive chat packet response \n"); 
				exit(1);
			} else {

				/* Validate registration response packet type */
				if(ntohs(chat_pack.type) != 231){
					printf("\n Invalid chat response packet format: %d\n", ntohs(chat_pack.type)); 
					exit(1);
				} 
				printf("%s: %s", chat_pack.uName, chat_pack.data);
			}
		}


	}
}
