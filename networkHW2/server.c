#include "network.h"

struct client_info{
	char name[100000];
	int client_fd;
	int login;
	int game;
	int connection;
};

struct client_info *clients;
int maxi , maxfd;
fd_set all_set , ready_set;

int create_socket(){

	int listen_fd;
	struct sockaddr_in serv_addr;

	listen_fd = socket(AF_INET , SOCK_STREAM , 0);

	bzero(&serv_addr , sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(8080);

	bind(listen_fd , (struct sockaddr*)&serv_addr , sizeof(serv_addr));

	listen(listen_fd , 1024);

	return listen_fd;
}

void create_fdset(int listen_fd){
	
	maxfd = listen_fd;
	maxi = -1;

	for(int i = 0 ; i<FD_SETSIZE ; i++){
		clients[i].client_fd = -1;
		clients[i].login = 0;
		clients[i].game = 0;
		clients[i].connection = -1;
	}

	FD_ZERO(&all_set);
	FD_SET(listen_fd , &all_set);
}

int login(char *receiveline , char *name){
	char *ptr , *ptr_end;
	char username[100000] , password[100000] , real_username[100000] , real_password[100000];
	FILE *fp;

	ptr = strstr(receiveline , "/") + 1;
	strcpy(password , ptr);

	ptr = receiveline;
	ptr_end = strstr(receiveline , "/");
	*ptr_end = '\0';
	strcpy(username , ptr);
	strcpy(name , ptr);

	fp = fopen("user.txt" , "r");

	while(1){
		if(fscanf(fp , "%s" , real_username) == EOF) break;
		fscanf(fp , "%s" , real_password);

		if(strcmp(real_username , username) == 0 && strcmp(real_password , password) == 0) return 1;
	}
	fclose(fp);
	return 0;
}

void start_game(char *receiveline , int i){
	
	char *ptr = receiveline + 1;
	char username[100000] , sendline[100000];

	while(isspace(*ptr)) ptr++;
	strcpy(username , ptr);

	for(int j = 0 ; j<=maxi ; j++){

		if(strcmp(clients[j].name , username) == 0 && clients[j].login == 1){

			sprintf(sendline , "%s invite you to have a game(Yes/No)\n" , clients[i].name);
			send(clients[j].client_fd , sendline , strlen(sendline) , 0);

			sprintf(sendline , "waiting for %s accept...\n" , clients[j].name);
			send(clients[i].client_fd , sendline , strlen(sendline) , 0);

			clients[i].game = 1;
			clients[j].game = 1;
			clients[i].connection = j;
			clients[j].connection = i;
		}
	}

}

void build_block(char *block , int *block_array){
	
	memset(block , '\0' , sizeof(block));
	
	if(block_array[9] == -1){

		for(int i = 0 ; i<3 ; i++){
			for(int j = 0 ; j<3 ; j++){
				sprintf(block , "%s|%d" , block , 3*i+j);
			}
			sprintf(block , "%s|\n" ,  block);
		}
	}
	else{
		for(int i = 0 ; i<3 ; i++){
			for(int j = 0 ; j<3 ; j++){
				if(block_array[3*i+j] == 1) sprintf(block , "%s|%c" , block , 'O');
				else if(block_array[3*i+j] == 2) sprintf(block , "%s|%c" , block , 'X');
				else if(block_array[3*i+j] == 0) sprintf(block , "%s|%c" , block , ' ');
			}
			sprintf(block , "%s|\n" , block);
		}
	}
}

int check_win(int *b){

	if(b[0] == 1 && b[1] == 1 && b[2] == 1) return 1;
	if(b[3] == 1 && b[4] == 1 && b[5] == 1) return 1;
	if(b[6] == 1 && b[7] == 1 && b[8] == 1) return 1;
	if(b[0] == 1 && b[3] == 1 && b[6] == 1) return 1;
	if(b[1] == 1 && b[4] == 1 && b[7] == 1) return 1;
	if(b[2] == 1 && b[5] == 1 && b[8] == 1) return 1;
	if(b[0] == 1 && b[4] == 1 && b[8] == 1) return 1;
	if(b[3] == 1 && b[4] == 1 && b[6] == 1) return 1;

	if(b[0] == 2 && b[1] == 2 && b[2] == 2) return 1;
	if(b[3] == 2 && b[4] == 2 && b[5] == 2) return 1;
	if(b[6] == 2 && b[7] == 2 && b[8] == 2) return 1;
	if(b[0] == 2 && b[3] == 2 && b[6] == 2) return 1;
	if(b[1] == 2 && b[4] == 2 && b[7] == 2) return 1;
	if(b[2] == 2 && b[5] == 2 && b[8] == 2) return 1;
	if(b[0] == 2 && b[4] == 2 && b[8] == 2) return 1;
	if(b[2] == 2 && b[4] == 2 && b[6] == 2) return 1;

	return 0;
}

int check_draw(int *b){

	for(int i = 0 ; i<9 ; i++){
		if(b[i] == 0) return 0;
	}

	return 1;
}

void start_server(){

	int listen_fd , conn_fd , sock_fd , nready , client_len , i , count , block_array[10] , turn = 0 , number;
	struct sockaddr_in cli_addr;
	char sendline[100000] , receiveline[100000] , name[100000] , block[100000];

	clients = (struct client_info*)malloc(FD_SETSIZE*sizeof(struct client_info));
	listen_fd = create_socket();

	create_fdset(listen_fd);

	while(1){
		ready_set = all_set;

		nready = select(maxfd+1 , &ready_set , NULL , NULL , NULL);

		if(FD_ISSET(listen_fd , &ready_set)){
			
			client_len = sizeof(cli_addr);
			conn_fd = accept(listen_fd , (struct sockaddr*)&cli_addr , &client_len);
			
			printf("new client: %s , port %d\n" , inet_ntop(AF_INET , &cli_addr.sin_addr , 4 , NULL) , ntohs(cli_addr.sin_port));

			sprintf(sendline , "Please enter your username/password\n");
			send(conn_fd , sendline , strlen(sendline) , 0);

			for(i = 0 ; i<FD_SETSIZE ; i++){
				if(clients[i].client_fd < 0){
					clients[i].client_fd = conn_fd;
					break;
				}
			}

			FD_SET(conn_fd , &all_set);

			if(conn_fd > maxfd) maxfd = conn_fd;
			if(i > maxi) maxi = i;
			if(--nready <= 0) continue;
		}

		for(i = 0 ; i<=maxi ; i++){
			
			if((sock_fd = clients[i].client_fd) < 0) continue;
			
			if(FD_ISSET(sock_fd , &ready_set)){
				
				if(clients[i].login == 0){
					
					memset(receiveline , '\0' , sizeof(receiveline));
					recv(sock_fd , receiveline , sizeof(receiveline) , 0);

					clients[i].login = login(receiveline , name);

					if(clients[i].login == 0){
						sprintf(sendline , "Wrong username or password\nPlease login again\n");
						send(sock_fd , sendline , strlen(sendline) , 0);
						continue;
					}
					else if(clients[i].login == 1){
						sprintf(sendline , "Login success\nL:list all users\nC:play game with the user\nQ:quit\n");
						send(sock_fd , sendline , strlen(sendline) , 0);
						strcpy(clients[i].name , name);
						continue;
					}
				}
				else if(clients[i].game == 0){
					
					memset(receiveline , '\0' , sizeof(receiveline));
					recv(sock_fd , receiveline , sizeof(receiveline) , 0);
					
					switch(receiveline[0]){
						
						case 'L':
							memset(sendline , '\0' , sizeof(sendline));
							count = 0;

							for(int j = 0 ; j<=maxi ; j++){
								
								if(clients[j].login == 1 && i != j){
									sprintf(sendline , "%s%s\n" , sendline , clients[j].name);
									count++;
								}
							}
							if(count == 0){
								sprintf(sendline , "No one else here\n");
								send(sock_fd , sendline , strlen(sendline) , 0);
							}
							else send(sock_fd , sendline , strlen(sendline) , 0);

							break;

						case 'C':
							start_game(receiveline , i);
							break;
						case 'Q':
							sprintf(sendline , "Goodbye!\n");
							send(sock_fd , sendline , strlen(sendline) , 0);

							fclose(sock_fd);
							FD_CLR(sock_fd , &all_set);
							clients[i].client_fd = -1;
							clients[i].login = 0;

							break;
						default:
							sprintf(sendline , "L:list all users\nC:play game with the user\nQ:quit\n");
							send(sock_fd , sendline , strlen(sendline) , 0);
							break;
					}
				}
				else if(clients[i].game == 1){
					
					memset(receiveline , '\0' , sizeof(receiveline));
					recv(sock_fd , receiveline , sizeof(receiveline) , 0);

					if(strcmp(receiveline , "Yes") == 0){
						
						block_array[9] = -1;
						build_block(block , block_array);

						sprintf(sendline , "Start game!\n%sChoose the number\n" , block);
						
						send(sock_fd , sendline , strlen(sendline) , 0);
						send(clients[clients[i].connection].client_fd , sendline , strlen(sendline) , 0);

						for(int j = 0 ; j<10 ; j++){
							block_array[j] = 0;
						}
					}
					else if(strcmp(receiveline , "No") == 0){
						
						sprintf(sendline , "%s say no\n" , clients[i].name);
						send(clients[clients[i].connection].client_fd , sendline , strlen(sendline) , 0);

						sprintf(sendline , "L:list all user\nC:play game with the user\nQ:quit\n");
						send(sock_fd , sendline , strlen(sendline) , 0);

						clients[clients[i].connection].game = 0;
						clients[clients[i].connection].connection = -1;
						clients[i].game = 0;
						clients[i].connection = -1;
					}
					else if(receiveline[0] <= '8' && receiveline[0] >= '0'){
						
						if(turn == 1){
							
							number = receiveline[0] - '0';
							block_array[number] = 1;
							build_block(block , block_array);

							turn = 0;
						}
						else if(turn == 0){
					
							number = receiveline[0] - '0';
							block_array[number] = 2;
							build_block(block , block_array);

							turn = 1;
						}

						if(check_win(block_array) == 1){

							sprintf(sendline , "%sYou win!" , block);
							send(sock_fd, sendline , strlen(sendline) , 0);
							
							sprintf(sendline , "%sYou loss!" , block);
							send(clients[clients[i].connection].client_fd , sendline , strlen(sendline) , 0);

							clients[clients[i].connection].game = 0;
							clients[clients[i].connection].connection = -1;
							clients[i].game = 0;
							clients[i].connection = -1;
						}
						else if(check_draw(block_array) == 1){

							sprintf(sendline , "%sDraw!\n" , block);
							send(sock_fd , sendline , strlen(sendline) , 0);

							send(clients[clients[i].connection].client_fd , sendline , strlen(sendline) , 0);
							
							clients[clients[i].connection].game = 0;
							clients[clients[i].connection].connection = -1;
							clients[i].game = 0;
							clients[i].connection = -1;
						}
						else{
							sprintf(sendline , "%s" , block);
							send(clients[clients[i].connection].client_fd , sendline , strlen(sendline) , 0);
						}
					}
				}	
			}
		}
	}
}

void main(){
	start_server();
}
