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

void showMenu() {
	printf("-------------------------\n");
	printf("|1. cd <Directory>\n");
	printf("|2. ls <Command>\n");
	printf("|3. upload <LocalFile>\n");
	printf("|4. download <RemoteFile>\n");
	printf("|5. exit\n");
	printf("-------------------------\n");
}

int main(int argc, char **argv) {
	FILE *fp;
	int sockfd, size, n, now;
	struct sockaddr_in servaddr;
	char send[MAX] = {0}, recv[MAX] = {0}, cmd[100] = {0}, path[100] = {0};
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
	while (1) {
		printf("[i] You can enter the following commands:\n");
		showMenu();
		printf("[!] Your input:\n");
		if (fgets(send, MAX, stdin) == NULL || !strcmp(send, "exit\n")) break;
		/* write command first */
		write(sockfd, send, strlen(send));
		/* receive response */
		read(sockfd, recv, MAX);
		/* print server reply */
		printf("[!] Server reply:\n");
		printf("%s", recv);
		/* parse command */
		sscanf(send, "%s", cmd);
		if (!strcmp(cmd, "upload")) {
			/* parse command */
			sscanf(send, "%*s%s", cmd);
			/* open the file for reading */
			fp = fopen(cmd, "r");
			if (fp != NULL) {
				/* calculate the size of the file */
				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				rewind(fp);
				/* deal with exception: directory */
				if (size == -1) {
					printf("[e] Cannot upload a directory.\n");
					sprintf(send, "terminate\n");
					write(sockfd, send, strlen(send));
					continue;
				}
				printf("[i] File '%s' is uploading, size: %d.\n", cmd, size);
				sprintf(send, "OK %d\n", size);
				/* send the file size */
				write(sockfd, send, MAX);
				now = 0;
				/* upload the file */
				while (now < size) {
					n = fread(send, sizeof(char), MAX, fp);
					write(sockfd, send, n);
					now += n;
				}
				printf("[i] Data transfer completed.\n");
				fclose(fp);
			} else {
				/* deal with the exception: file not found */
				printf("[e] File '%s' not found.\n", cmd);
				sprintf(send, "terminate\n");
				write(sockfd, send, strlen(send));
			}
		} else if (!strcmp(cmd, "download")) {
			/* parse command */
			sscanf(recv, "%s", cmd);
			if (!strcmp(cmd, "[i]")) {
				/* get the file size and the file name */
				sscanf(recv, "%*s%*s%*s%*s%*s%*s%d", &size);
				sscanf(send, "%*s%s", cmd);
				strcpy(path, "Download/");
				strcat(path, cmd);
				fp = fopen(path, "w");
				now = 0;
				/* download the file */
				while (now < size) {
					n = read(sockfd, recv, MAX);
					fwrite(recv, sizeof(char), n, fp);
					now += n;
				}
				fclose(fp);
				printf("[i] Data transfer completed.\n");
			}
		}
		/* always do this in the end */
		memset(send, 0, sizeof(send));
		memset(recv, 0, sizeof(recv));
	}
	return 0;
}
