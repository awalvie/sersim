/*Simple Server for serving my static site */

/* How the program works in my head */

/* Program takes 2 arguments, the location of the files to serve from,
   and the port number and serves requested files by searching for them in the
   given directory. */

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#define VERSION 1
#define CHARACTER_BUFFER 8096

struct {
	char *ext;
	char *filetype;
} extensions[] = {
	{ "gif", "image/gif" },	  { "jpg", "image/jpg" },
	{ "jpeg", "image/jpeg" }, { "png", "image/png" },
	{ "ico", "image/ico" },	  { "zip", "image/zip" },
	{ "gz", "image/gz" },	  { "tar", "image/tar" },
	{ "htm", "text/html" },	  { "html", "text/html" },
	{ "ico", "image/ico"},
};

void usage(void)
{
	fprintf(stderr,
		"Use -h for information about how to use the program\n");
	exit(EXIT_FAILURE);
}

void web(int peer_fd)
{
	int i, j, fd;
	long file_len, ret, buflen, extlen;
	char *ftype;

	char buffer[CHARACTER_BUFFER] = { 0 };
	ret = read(peer_fd, buffer, sizeof(buffer) / sizeof(char));

	/* check various return values  */
	if (ret == 0 || ret == -1) {
		fprintf(stderr, "ERROR: Failed to read browser request.\n");
		exit(EXIT_FAILURE);
	}

	if (ret > 0 || ret < CHARACTER_BUFFER) {
		buffer[ret] = 0;
	} else {
		buffer[0] = 0;
	}

	/* remove carriage returns and newline characters */
	for (i = 0; i < ret; i++) {
		if (buffer[i] == '\r' || buffer[i] == '\n') {
			buffer[i] = '*';
		}
	}

	/* grab the request type and the requested page */
	/* assigning int 0 to a character buffer lets us put and escape  */
	/* char there  */
	for (i = 4; i < ret; i++) {
		if (buffer[i] == ' ') {
			buffer[i] = 0;
			break;
		}
	}

	/* check if parent file is being requested in the path */
	for (j = 0; j < i - 1; j++) {
		if (buffer[j] == '.' && buffer[j + 1] == '.') {
			fprintf(stderr,
				"ERROR: Requesting parent directory in the path is not allowed\n");
		}
	}

	/* request index.html by default */
	if (!strncmp(&buffer[0], "GET /\0", 6)) {
		strcpy(buffer, "GET /index.html");
	}

	buflen = strlen(buffer);
	for (i = 0; extensions[i].ext != 0; i++) {
		extlen = strlen(extensions[i].ext);
		if (!strncmp(&buffer[buflen - extlen], extensions[i].ext,
			     extlen)) {
			ftype = extensions[i].filetype;
			break;
		}
	}

	if (!ftype) {
		fprintf(stderr, "ERROR: The filetype is not allowed.\n");
	}

	if ((fd = open(&buffer[5], O_RDONLY)) == -1) {
		fprintf(stderr, "ERROR: %s: File does not exist.\n",
			&buffer[5]);
	}

	/* find total length of the file */
	file_len = lseek(fd, 0, SEEK_END);
	/* set the cursor back to the start of the file */
	lseek(fd, 0, SEEK_SET);
	/* print header */
	sprintf(buffer,
		"HTTP/1.1 200 OK\nServer: simper/%d.0\nContent-Length: %ld\nConnection: close\nContent-Type: %s\n\n",
		VERSION, file_len, ftype);
	write(peer_fd, buffer, strlen(buffer));

	while ((ret = read(fd, buffer, CHARACTER_BUFFER)) > 0) {
		write(peer_fd, buffer, ret);
	}
	sleep(1);
	close(peer_fd);
}

int main(int argc, char *argv[])
{
	/* Argument Parsing */
	int i, opt, server_fd, peer_fd;
	int port = 0;
	char *location;
	socklen_t length;

	static struct sockaddr_in server_addr;
	static struct sockaddr_in peer_addr;

	if (argc == 1) {
		usage();
	}

	while ((opt = getopt(argc, argv, "hp:l:")) != -1) {
		switch (opt) {
		case 'h':
			fprintf(stdout,
				"Usage: simper -l LOCATION -p PORT\n"
				"Simper is a simple server writtern in ANSI C.\n\n"
				"  -l\tNeeds to be the absolute location of the directory.\n"
				"  -p\tBetween 1-60000\n"
				"\nUnsupported direcries: /, /tmp, /bin, /lib, /usr, /sbin, /dev, /etc\n"
				"Supported File Types: ");
			for (i = 0; extensions[i].ext != 0; i++) {
				fprintf(stdout, "%s, ", extensions[i].ext);
			}
			fprintf(stdout, "\n\nWritten By awalvie\n\n");
			exit(EXIT_SUCCESS);
		case 'p':
			port = atoi(optarg);
			break;
		case 'l':
			location = optarg;
			break;
		default:
			usage();
		}
	}

	if (port == 0) {
		fprintf(stderr, "ERROR: Port not specified.\n");
		usage();
	} else if (port < 1 || port > 60000) {
		fprintf(stderr,
			"ERROR: Port %d is out of the accepted range of 1-60000.\n",
			port);
		usage();
	}

	if (location == NULL) {
		fprintf(stderr, "ERROR: Location not specified.\n");
		usage();
	} else {
		if (!strncmp(location, "/", 2) ||
		    !strncmp(location, "/etc", 5) ||
		    !strncmp(location, "/bin", 5) ||
		    !strncmp(location, "/lib", 5) ||
		    !strncmp(location, "/tmp", 5) ||
		    !strncmp(location, "/usr", 5) ||
		    !strncmp(location, "/dev", 5) ||
		    !strncmp(location, "/sbin", 6)) {
			fprintf(stderr, "ERROR: Bad top directory %s\n",
				location);
			usage();
			exit(EXIT_FAILURE);
		}
	}

	if (chdir(location) == -1) {
		fprintf(stderr,
			"ERROR: Cannot change to the directory.\n"
			"Please ensure the directory exists and ins't unsupported.\n");
		usage();
	}

	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		fprintf(stderr, "ERROR: Could not start socket\n");
		exit(EXIT_FAILURE);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(port);

	if ((bind(server_fd, (struct sockaddr *)&server_addr,
		  sizeof(server_addr))) < 0) {
		fprintf(stderr, "ERROR: Could not bind port\n");
		exit(EXIT_FAILURE);
	}

	if ((listen(server_fd, 64)) < 0) {
		fprintf(stderr, "ERROR: Could not listen on given port.\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		fprintf(stdout, "\nWaiting for connection\n");
		length = sizeof(peer_addr);
		if ((peer_fd = accept(server_fd, (struct sockaddr *)&peer_addr,
				      &length)) < 0) {
			fprintf(stderr,
				"ERROR: Could not accept connection.\n");
			exit(EXIT_FAILURE);
		}

		web(peer_fd);
	}

	exit(EXIT_SUCCESS);
}
