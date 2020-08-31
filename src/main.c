/*Simple Server for serving my static site */

/* How the program should work in my head */

/* Program takes 2 arguments, the location of the files to serve from,
   and the port number and serves requested files by searching for them in the
   given directory. */

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

void usage(void)
{
	fprintf(stderr,"Use -h for information about houw to use the program\n");

}

int create_socket(void)
{
	int server_fd;

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Cannont create socket");
		exit(EXIT_FAILURE);
	}

	return server_fd;
}

int main(int argc, char *argv[])
{
	int opt;

	if (argc == 1) {
		usage();
		exit(EXIT_SUCCESS);
	}

	while ((opt = getopt(argc, argv, "h")) != -1) {
		switch (opt) {
		case 'h':
			break;
		default:
			usage();
		}
	}
	exit(EXIT_SUCCESS);
}
