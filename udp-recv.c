/**
 * UDP data grabber
 *
 * This file is distributed under the GPLv2 License. 
 */ 

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <time.h>

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define PORT 8
#define BUFSIZE 100*1024*4
#define SD_CARD_LOCATION "/media/ubuntu/disk/stack_radar"


/**************************************************************************
 * Public Types
 **************************************************************************/

/**************************************************************************
 * Global Variables
 **************************************************************************/

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

int main(int argc, char **argv)
{
	struct sockaddr_in myaddr;      /* our address */
	struct sockaddr_in remaddr;     /* remote address */
	socklen_t addrlen = sizeof(remaddr);            /* length of addresses */
	int recvlen;                    /* # bytes received */
	int fd;                         /* our socket */
	unsigned char buf[BUFSIZE];     /* receive buffer */
	char fname[80];
	int port = PORT;
	int packets = 0;                /* packet id */
	FILE *fp;                       /* file name */
	struct timeval tv;

	if (argc == 2) {
		port = atoi(argv[1]);
		fprintf(stderr, "port = %d\n", port);
	}
	
	/* create a UDP socket */
	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket\n");
		return 0;
	}

	/* bind the socket to any valid IP address and a specific port */
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		return 0;
	}

	/* now loop, receiving data and printing what we received */
	fprintf(stderr, "start main loop: waiting on port %d\n", PORT);  
	for (;;) {
		
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		printf( "received %d bytes\n", recvlen);

		if (recvlen > 0) {
			buf[recvlen] = 0;

			if (!strncmp(buf, "Ping", 4) || !strncmp(buf, "Pong", 4) ) {
				time_t rawtime;
				struct tm * timeinfo;
				/* session init */
				packets = 0;

				time (&rawtime);
				timeinfo = localtime (&rawtime);

				char end_text[80];
				strftime(end_text, 80, "%Y-%m-%d_%H-%M-%S", timeinfo);

				gettimeofday(&tv,NULL);
				int milli_sec = tv.tv_usec/1000;

				/*  SD card recording   */
				if(fname[0]!='\0')
					fclose(fp);
				sprintf(fname,"%s/%s_%03d.bin",SD_CARD_LOCATION,end_text,milli_sec);

				/* Tegra flash memory recording   */
				printf("\n the file name is %s",fname);
				fprintf(stderr, "received ping/pong. open %s\n", fname);
		  		
				fp = fopen(fname, "w+");
				if (!fp) {
					perror("open failed");
					exit(1);
				}
			} else {
				/* data packet */
				packets++;
				if(fp){
					int ret = fwrite(buf, 1, recvlen, fp);}
			}
				
		}
	}
	/* never exits */
}
