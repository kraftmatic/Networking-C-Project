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
#define MAX_PENDING 5
#define MAXNAME 10

int main()
{

	struct sockaddr_in sin;
	struct sockaddr_in clientAddr;

	struct packet{ 
		short type; 
		char uName[MAXNAME]; 
		char mName[MAXNAME]; 
		char data[MAXNAME]; 
	}; 
	struct packet packet_reg;

	/* structure of Registration Table */ 
	struct registrationTable{ 
		int port; 
		int sockid; 
		char mName[MAXNAME]; 
		char uName[MAXNAME]; 
	}; 
	struct registrationTable table[10];

	char buf[MAX_LINE];
	int s, new_s;
	int len;
	int table_index = 0;


	

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
	printf("\nServer listening...\n");


	/* wait for connection, then receive and print text */
	while(1){
		if((new_s = accept(s, (struct sockaddr *)&clientAddr, &len)) < 0){
			perror("tcpserver: accept");
			exit(1);
		}

		// Pick up registration packet
		if(recv(new_s,&packet_reg,sizeof(packet_reg),0) < 0) { 
			printf("\n Could not receive first registration packet \n"); 
			exit(1);
		}

		if(ntohs(packet_reg.type) != 121){
			printf("\n Invalid registration packet format: %d\n", ntohs(packet_reg.type)); 
			exit(1);
		}

		// Add user to chat list
		table[table_index].port = clientAddr.sin_port; 
		table[table_index].sockid = new_s; 
		strcpy(table[table_index].uName, packet_reg.uName); 
		strcpy(table[table_index].mName, packet_reg.mName); 

		// Clear screen and print chat list
        system("clear");
		for(int i = 0; i <= table_index; i++){
			printf("%s %s\n", table[i].mName, table[i].uName);
		}

		while(len = recv(new_s, buf, sizeof(buf), 0)){
			fputs(buf, stdout);
		}

		close(new_s);
	}
}
