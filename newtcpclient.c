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
#define MAXNAME 10

int main(int argc, char* argv[])
{
	
	struct hostent *hp;
	struct sockaddr_in sin;

	/*  structure of the packet */ 
	struct packet{ 
		short type; 
		char uName[MAXNAME]; 
		char mName[MAXNAME]; 
		char data[MAXNAME]; 
	}; 
 	struct packet packet_reg; 

	char *host;
	char *first_name;
	char *last_name;
	char buf[MAX_LINE];
	int s;
	int len;

	if(argc == 4){
		host = argv[1];
		first_name = argv[2];
		last_name = argv[3];
	}
	else{
		fprintf(stderr, "usage:client server first_name last_name\n");
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
	strcpy(packet_reg.mName, first_name);
	strcpy(packet_reg.uName, last_name);

 	// Call gethostname() method to store the client’s machine name in an 
	//array called clientname 
	char hostname[1024];
    gethostname(hostname, 1024);

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

	/* main loop: get and send lines of text */
	while(fgets(buf, sizeof(buf), stdin)){
		buf[MAX_LINE-1] = '\0';
		len = strlen(buf) + 1;
		send(s, buf, len, 0);
	}
}
