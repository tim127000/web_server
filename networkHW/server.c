#include "network.h"

int create_socket(){
	int listenfd;
	struct sockaddr_in serv_addr;

	listenfd = socket(AF_INET , SOCK_STREAM , 0);
	
	bzero(&serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(8080);

	bind(listenfd , (struct sockaddr*)&serv_addr , sizeof(serv_addr));

	listen(listenfd , 1024);

	return listenfd;

}

fd_set wait_clients(fd_set allset , int listenfd , int *client , int *maxfd , int *maxi){

	*maxfd = listenfd;
	*maxi = -1;
	for(int i = 0 ; i<FD_SETSIZE ; i++){
		client[i] = -1;
	}
	FD_ZERO(&allset);
	FD_SET(listenfd , &allset);

	return allset;
}

char *get_route(char *request){
	char *ptr , *ptr_end;
	ptr = request + 4;
	ptr_end = strstr(ptr , " ");
	*ptr_end = '\0';
	return ptr;
}

void response(char *path , int sockfd){
	char buffer[1024] , full_path[128];
	if(strcmp(path , "/") == 0) path = "./page/index.html";
	if(strcmp(path , "/computer.jpg") == 0) path = "./page/computer.jpg";
	if(strcmp(path , "/upload.html") == 0) path = "./page/upload.html";
	FILE *fp = fopen(path , "rb");

	sprintf(buffer , "HTTP/1.1 200 OK\r\n\r\n");
	send(sockfd , buffer , strlen(buffer) , 0);

	int r = fread(buffer , 1 , 1024 , fp);
	while(r){
		send(sockfd , buffer , r , 0);
		r = fread(buffer , 1 , 1024 , fp);
	}
	fclose(fp);
}

char *get_body(char *request){
	char *ptr , *ptr_end;
	ptr = strstr(request , "\r\n\r\n");
	ptr+=4;
	ptr = strstr(ptr , "\r\n\r\n");
	ptr+=4;
	ptr_end = strstr(ptr , "\n");
	*ptr_end = '\0';
	return ptr;
}

char *get_name(char *request){
	char *ptr , *ptr_end;
	ptr = strstr(request , "\r\n\r\n");
	ptr = strstr(ptr , "filename=");
	ptr+=10;
	ptr_end = strstr(ptr , "\"");
	*ptr_end = '\0';
	return ptr;
}

void response_post(char *name , char *body , int sockfd){
	FILE *fp;
	char buffer[1024];
	fp = fopen(name , "wb");
	fwrite(body , 1 , strlen(body) , fp);
	fclose(fp);
	sprintf(buffer , "HTTP/1.1 200 OK\r\n\r\n");
	send(sockfd , buffer , 1024 , 0);
}

void start_server(){
	int listenfd , connfd , sockfd;
	int maxfd , maxi;
	int client[FD_SETSIZE] , cli_len;
	int r , i , nready;
	fd_set rset , allset;
	struct sockaddr_in cliaddr;
	char request[1000000];
	char *path , *body , *name;

	listenfd = create_socket();

	allset = wait_clients(allset , listenfd , client , &maxfd , &maxi);

	while(1){
		rset = allset;

		nready = select(maxfd+1 , &rset , NULL , NULL , NULL);

		//new client connection
		if(FD_ISSET(listenfd , &rset)){
			cli_len = sizeof(cliaddr);
			connfd = accept(listenfd , (struct sockaddr*)&cliaddr , &cli_len);
			printf("new client: %s , port %d\n" , inet_ntop(AF_INET , &cliaddr.sin_addr , 4 , NULL) , ntohs(cliaddr.sin_port));

			for(i = 0 ; i<FD_SETSIZE ; i++){
				if(client[i] < 0){
					client[i] = connfd;
					break;
				}
			}

			FD_SET(connfd , &allset);

			if(connfd > maxfd) maxfd = connfd;
			if(i > maxi) maxi = i;
			if(--nready <= 0) continue;
		}

		for(i = 0 ; i<=maxi ; i++){
			if((sockfd = client[i]) < 0) continue;
			if(FD_ISSET(sockfd , &rset)){
				r = recv(sockfd , request , sizeof(request) , 0);
				printf("%s\n" , request);
			
				//GET
				if(strstr(request , "GET") != NULL){
					path = get_route(request);
					printf("%s\n" , path);
					response(path , sockfd);
				}

				//POST
				if(strstr(request , "POST") != NULL){
					body = get_body(request);
					name = get_name(request);
					response_post(name , body , sockfd);
				}

				close(sockfd);
				FD_CLR(sockfd , &allset);
				client[i] = -1;
			}
		}
	}


}

void main(){
	start_server();
}
