/*Simple Server for serving my static site */

/* How the program works in my head */

/* Program takes 2 arguments, the location of the files to serve from,
   and the port number and serves requested files by searching for them in the
   given directory. */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

struct {
	char *ext;
	char *filetype;
} extensions[] = {
	{ "gif", "image/gif" },	  { "jpg", "image/jpg" },
	{ "jpeg", "image/jpeg" }, { "png", "image/png" },
	{ "ico", "image/ico" },	  { "zip", "image/zip" },
	{ "gz", "image/gz" },	  { "tar", "image/tar" },
	{ "htm", "text/html" },	  { "html", "text/html" },
};

void usage(void)
{
	fprintf(stderr,
		"Use -h for information about how to use the program\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	/* used to process arguments */
	int i, opt, port = 0;
	char *location;

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
		fprintf(stderr, "Port not specified.\n");
		usage();
	} else if (port < 1 || port > 60000) {
		fprintf(stderr,
			"Port %d is out of the accepted range of 1-60000.\n",
			port);
		usage();
	}

	if (location == NULL) {
		fprintf(stderr, "Location not specified.\n");
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

	exit(EXIT_SUCCESS);
}
