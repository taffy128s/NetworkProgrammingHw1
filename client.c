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

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in servaddr;
	if (argc < 3) {
		printf("Usage: a.out <ServerAddress> <ServerPort>\n");
		exit(1);
	}
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Socket error.\n");
		exit(2);
	}
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
		printf("Inet_pton error (%s).\n", argv[1]);
		exit(3);
	}
	if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
		printf("Connect error.\n");
		exit(4);
	}
	mkdir("./Download", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	printf("[i] Download directory was created.\n"); 
	
	return 0;
}