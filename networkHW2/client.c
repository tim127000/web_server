#include "network.h"

void rm_nextline(char *sendline){
	int length = strlen(sendline);
	if(sendline[length-1] == '\n') sendline[length-1] = '\0';
}

void strcli(FILE *fp , int sock_fd){
	char sendline[100000] , receiveline[100000];

	while(1){
		memset(receiveline , '\0' , sizeof(receiveline));	
		recv(sock_fd , receiveline , 100000 , 0);
		printf("%s" , receiveline);

		if(fgets(sendline , 100000 , fp) != EOF){
			rm_nextline(sendline);
			send(sock_fd , sendline , strlen(sendline) , 0);
		}
	}
}

void main(int argc , char *argv[]){
	int sock_fd;
	struct sockaddr_in serv_addr;
	
	sock_fd = socket(AF_INET , SOCK_STREAM , 0);

	bzero(&serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8080);
	inet_pton(AF_INET , argv[1] , &serv_addr.sin_addr);

	connect(sock_fd , (struct sockaddr*)&serv_addr , sizeof(serv_addr));

	strcli(stdin , sock_fd);
}
