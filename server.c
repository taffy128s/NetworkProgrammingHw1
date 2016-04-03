#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#define MAX 2048
#define PORT 6666

void run() {
	
}

int main() {
	int listenfd, connfd;
	socklen_t clilen;
	struct sockaddr_in servaddr, cliaddr;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);
	
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listenfd, 1024);
	
	mkdir("./Upload", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	printf("[i] Upload directory was created.\n");
	
	while (1) {
		printf("[i] Ready to accept.\n");
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		printf("[i] Connection from %s, port %d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
		if (fork() == 0) {
			close(listenfd);
			run();
			printf("[i] %s terminated!\n", inet_ntoa(cliaddr.sin_addr));
			exit(0);
		}
		close(connfd);
	}
	return 0;
}