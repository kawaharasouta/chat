#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

#include<sock_connect.h>

int* sock_connect(char *hostname, int port){
  int fd;
  struct sockaddr_in addr;
  struct hostent *hp;
  //<83>\<83>P<83>b<83>g<90>¶<90>
  if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    fprintf(stderr, "Cannot create socket\n");
    exit(1);
  }

  //<8d>\<91>￠<91>I<82>I<83>N<83><8a><83>
  bzero((char *)&addr, sizeof(addr));
  //memset((char *)&server_addr, 0, sizeof(server_addr));

  //<8a>e<8e>i<90>Y<92>e92
  if((hp = gethostbyname(hostname)) == NULL){
    perror("No such host");
    exit(1);
  }
  bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);

  //<83>T<81>[<83>o<82>E<90>U<91>±
  if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
    fprintf(stderr, "Cannot connect\n");
    exit(1);
  }
  printf("Connected.\n");

  return fd;
}
