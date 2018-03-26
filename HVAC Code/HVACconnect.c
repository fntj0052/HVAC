#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wiringPi.h>
#include <pcf8591.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>

#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define BUFFER_SIZE 1024
#define PCF       120

char request[256];
int fd;
char buffer[BUFFER_SIZE];

int socket_connect(char *host, in_port_t port){
	struct hostent *hp;
	struct sockaddr_in addr;
	int on = 1, sock;     
      printf("%s, \n", host);

	if((hp = gethostbyname(host)) == NULL){
		herror("gethostbyname");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}
	
	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);

	}
	return sock;
}

int sendData(char *email, float temp, float moisture, short sound){

      //char request[] = "GET /hvacWS/sensors/read?from=now&to=now\r\n";
      //char request[] = "GET /hvac/read.php HTTP/1.1\r\nHost: www.justlikerav.com:80\r\n\r\n";

      char requestformat[] = "GET /hvac/write.php?email=%s&temp=%f&moisture=%f&sound=%d HTTP/1.1\r\nHost: www.justlikerav.com:80\r\n\r\n";
      sprintf(request, requestformat, email, temp, moisture, sound);
	write(fd, request, strlen(request)); // write(fd, char[]*, len);  
	bzero(buffer, BUFFER_SIZE);
	while(read(fd, buffer, BUFFER_SIZE - 1) != 0){
		fprintf(stdout, "%s", buffer);
		bzero(buffer, BUFFER_SIZE);
	}
      //todo error analysation
      return 0;
}
int main (int argc, char *argv[])
{
      char hvacStatus[128] = "";
	int padPin = 4;
	int alreadyPressed = 0;
	int value1, value2;
	int data1, data2;
	int counter = 0;
	int step = 1; //1
	int i = 0;
	int offset = 85;  //85
	int deviation = 10; //10
	int noiseCounter = 0;
	int noiseDuration = 5; //20
	int noiseOffCounter = 0;
	int noiseOffDuration = 1000; //1000
	int noise = 0;
      int padPressed;
      char email[50] = "person@gmail.com";
      float temp;
      float moisture;
      short sound;
      int datacounter;
      int result;

	if(argc < 3){
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		exit(1); 
	}

	if (wiringPiSetup () == -1) {
		printf("Error at wiringPiSetup()");
    		return 1 ;
	}
	pinMode(padPin, INPUT);
	pinMode (0, OUTPUT) ;         // aka BCM_GPIO pin 17
	pinMode (1, OUTPUT) ;	// aka BCM_GPIO pin 18
 

	// Setup pcf8591 on base pin 120, and address 0x48
//	printf("%d\n", ++step);

	if (pcf8591Setup (PCF, 0x48) == -1) {
		printf("Error at pcf8591Setup()");
    		return 1 ;
	}
	
       printf("Lowest noise level \t\t\t%d\n", offset);
       printf("Data has been generalized with weight value of %d\n", step);
       
	fd = socket_connect(argv[1], atoi(argv[2])); 
	printf("\n Debug: %d\n   ", fd);

	while(1)
      {
                  datacounter++;
			padPressed = digitalRead(padPin);
			if(padPressed)
			{
				if(alreadyPressed){}
				else
				{
                              strcpy(hvacStatus, "Pressed");
					alreadyPressed = 1;
			      }
			}
			else
			{
				alreadyPressed = 0;
                        strcpy(hvacStatus, "Unpressed");
			}
		data1 += analogRead  (PCF + 0);	//white - local
		data2 += analogRead  (PCF + 1);	//yellow - remote
		counter++;
		if(noise) {
			noiseOffCounter++;
		}
            temp = 36.6; //todo temperature sensor
            moisture = 30; //todo moisture sensor
            sound = noise;
            if(datacounter%1000)
            {
            result = sendData(email, temp, moisture, sound);
            if (result == 0)
            return 0;
            }
		if(counter == step ) {
			counter = 0;
			value1 = round((double)data1 / step);
			value2 = round((double)data2 / step);
			data1 = 0;
			data2 = 0;

			if ((value1 < offset) || (value2 < offset)) {

				if( abs (value1 - value2) < deviation)  {
					if (++noiseCounter > noiseDuration) {
						noiseCounter = 0;
						noiseOffCounter = 0;
						noise = 1;
					}
				}
				else  {
					if(noise) {
						if(noiseOffCounter > noiseOffDuration) {
							noiseOffCounter = 0;
							noise = 0;						
						}
					}
					else	if( --noiseCounter < 0)  {
							noiseCounter = 0;
					}
				}

				if(noise) {
					printf("\n\t%s\tNOISE !!!!  #1:%3d #2:%3d", hvacStatus, value1, value2);
					//both microphones are sensing noise - red 
					digitalWrite (0, LOW);
					digitalWrite (1, HIGH);
				}
				else {
					printf("\n\t%s\tVoice In!!  #1:%3d #2:%3d", hvacStatus, value1, value2);
					// Voice in at any of microphones - green
					digitalWrite (0, HIGH);
					digitalWrite (1, LOW);
				}

				for(i = offset; (i > offset - value1) && (i > 0); i--) {
					printf(" ");
				}
				for(i = offset; i > value1; i--) {
					printf("-");
				}
				printf("|");
				for(i = offset; i > value2; i--) {
					printf("+");
				}
			}
			else  {
				if(!noise) {
					digitalWrite (0, LOW);
					digitalWrite (1, LOW);
				}
			}
		}
	}
	shutdown(fd, SHUT_RDWR); 
	close(fd); 
	return 0;
}

