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
 
void run(int connfd) {
	FILE *fp;
	char cmd[100] = {0}, recv[MAX] = {0}, send[MAX] = {0}, name[100] = {0}, c;
	int n, size, now;
	while (read(connfd, recv, MAX) > 0) {
		/* parse the command from client */
		sscanf(recv, "%s", cmd);
		if (!strcmp(cmd, "download")) {
			/* parse the command to get the file name */
			sscanf(recv, "%*s%s", cmd);
			fp = fopen(cmd, "r");
			if (fp != NULL) {
				/* get the file size */
				fseek(fp, 0, SEEK_END);
				size = ftell(fp);
				rewind(fp);
				/* deal with the exception: downloading a directory */
				if (size == -1) {
					sprintf(send, "[e] Cannot download a directory.\n");
					write(connfd, send, strlen(send));
					continue;
				}
				/* give the client file size */
				sprintf(send, "[i] File '%s' is transfering, size: %d.\n", cmd, size);
				write(connfd, send, MAX);
				now = 0;
				/* transfer the file to the client */
				while (now < size) {
					n = fread(send, sizeof(char), MAX, fp);
					write(connfd, send, n);
					now += n;
				}
				fclose(fp);
			} else {
				/* deal with the exception: file not found */
				sprintf(send, "[e] File '%s' not found.\n", cmd);
				write(connfd, send, strlen(send));
			}
		} else if (!strcmp(cmd, "upload")) {
			/* respond to the request first */
			sprintf(send, "[i] Server is ready for uploading.\n");
			write(connfd, send, strlen(send));
			/* before getting the client status, retrive the name first */
			sscanf(recv, "%*s%s", name);
			/* get the status and file size from the client */
			read(connfd, recv, MAX);
			sscanf(recv, "%s", cmd);
			/* if the client is also ready for data transfering */
			if (!strcmp(cmd, "OK")) {
				sscanf(recv, "%*s%d", &size);
				/* open the file and start file uploading */
				fp = fopen(name, "w");
				now = 0;
				while (now < size) {
					n = read(connfd, recv, MAX);
					fwrite(recv, sizeof(char), n, fp);
					now += n;
				}
				fclose(fp);
			}
		} else if (!strcmp(cmd, "ls")) {
			fp = popen(recv, "r");
			int size = 0;
			while ((c = fgetc(fp)) != EOF)
				send[size++] = c;
			pclose(fp);
			if (size == 0) {
				sprintf(send, "[i] The directory is empty.\n");
				write(connfd, send, strlen(send));
				continue;
			}
			write(connfd, send, size);
		} else if (!strcmp(cmd, "cd")) {
			sscanf(recv, "%*s%s", cmd);
			if (chdir(cmd) < 0) sprintf(send, "[e] Failed to change directory.\n");
			else sprintf(send, "[i] Changed directory to '%s' successfully.\n", cmd);
			write(connfd, send, strlen(send));
		} else {
			sprintf(send, "[e] Command not found.\n");
			write(connfd, send, strlen(send));
		}
		memset(recv, 0, sizeof(recv));
		memset(send, 0, sizeof(send));
	}
}

int main(int argc, char **argv) {
	int listenfd, connfd;
	socklen_t clilen;
	struct sockaddr_in servaddr, cliaddr;
	
	if (argc < 2) {
		printf("Usage: a.out <ServerPort>\n");
		exit(1);
	}
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	
	bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
	listen(listenfd, 1024);
	
	mkdir("./Upload", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	printf("[i] Upload directory was created.\n");
	
	while (1) {
		printf("[i] Ready to accept.\n");
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		printf("[i] Connection from %s, port %d.\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
		if (fork() == 0) {
			close(listenfd);
			run(connfd);
			printf("[i] %s terminated!\n", inet_ntoa(cliaddr.sin_addr));
			exit(0);
		}
		close(connfd); 
	}
	return 0;
}
