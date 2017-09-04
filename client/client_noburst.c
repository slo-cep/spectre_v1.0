#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>

void error(const char *msg)
{
    perror(msg);
//    exit(0);
}

int main(int argc, char *argv[])
{
	unsigned int burst_i;
	unsigned long counter = 0;
	unsigned long eventCounter = 0;
	unsigned long time = 0;
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    unsigned int eventsPerSec;
	unsigned long sleeptimeNanosec;
	int slow = 0;
	char terminate = 0;
    char buffer[256];
    if (argc < 7) {
       fprintf(stderr,"usage %s hostname port filename numBursts BurstSize EventsPerSec\n", argv[0]);
       exit(0);
    }
	//sleep(1);
    portno = atoi(argv[2]);
	unsigned long numBursts = atoi(argv[4]);
	unsigned long burstSize = atoi(argv[5]);

    eventsPerSec = atoi(argv[6]);
	sleeptimeNanosec = 1000000000 / eventsPerSec;
	printf("sending %f events/sec\n", 1000000000.0 / sleeptimeNanosec);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
/*    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
*/    while (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
	{
        error("ERROR connecting");
		usleep(500000);
	}
	printf("connection established\n");
	struct timespec tsbeginSend={0,0}, tsendSend={0,0};
	{
		FILE * fp;
		char * line = NULL;
		size_t len = 0;
		ssize_t read;
		unsigned long endtime;

		fp = fopen(argv[3], "r");
		if (fp == NULL)
		    exit(EXIT_FAILURE);

		struct timespec tstart={0,0}, tend={0,0};

		clock_gettime(CLOCK_MONOTONIC, &tsbeginSend);
		clock_gettime(CLOCK_MONOTONIC, &tstart);
		long gesEvents = numBursts*burstSize;
		endtime = (tstart.tv_sec * 1000000000 + tstart.tv_nsec) + sleeptimeNanosec;
		tend.tv_sec = endtime / 1000000000;
		tend.tv_nsec = endtime % 1000000000;

		while (gesEvents > eventCounter && (read = getline(&line, &len, fp)) != -1) {
    		n = write(sockfd,line,strlen(line));
			eventCounter++;
			counter++;

			endtime = (tend.tv_sec * 1000000000 + tend.tv_nsec) + sleeptimeNanosec;
			tend.tv_sec = endtime / 1000000000;
			tend.tv_nsec = endtime % 1000000000;

			clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &tend, NULL);
		}
	    n = write(sockfd, &terminate, 1);

		fclose(fp);
		if (line)
		    free(line);
	}
	clock_gettime(CLOCK_MONOTONIC, &tsendSend);
	time = (tsendSend.tv_sec - tsbeginSend.tv_sec) * 1000000000 + tsendSend.tv_nsec - tsbeginSend.tv_nsec;
//	printf("slow %d\n", slow);
	printf("time: %f\n", ((double) time / 1000000000));
	printf("events: %lu\n", eventCounter);
	printf("average events/sec: %f\n", (double)eventCounter / ((double) time / 1000000000));
    close(sockfd);
    return 0;
}
