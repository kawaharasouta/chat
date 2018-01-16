#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>

#include<sock_connect.h>

#define PORT 5000

/*int* sock_connect(char *hostname, int port){
	int fd;
	struct sockaddr_in addr;
	struct hostent *hp;
	//�\�P�b�g����
	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	
	//�\���̂̃N���A
	bzero((char *)&addr, sizeof(addr));
	//memset((char *)&server_addr, 0, sizeof(server_addr));
	
	//�e��ݒ�
	if((hp = gethostbyname(hostname)) == NULL){
		perror("No such host");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	
	//�T�[�o�ɐڑ�
	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		fprintf(stderr, "Cannot connect\n");
		exit(1);
	}
	printf("Connected.\n");
	
	return fd;
}*/

int main(int argc, char *argv[]){
	int fd, len, n, ret;
	struct sockaddr_in addr;
	struct hostent *hp;
	char buf[1024];
	fd_set readfds, writefds;
	
	if(argc != 4){//����������Ȃ������葽�������肵����G���[
		printf("Usage: ./chatclient [IP_adress] [your_name] [room_number]\n");
		exit(1);
	}
	
	//�\�P�b�g����
/*	if((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr, "Cannot create socket\n");
		exit(1);
	}
	
	//�\���̂̃N���A
	bzero((char *)&addr, sizeof(addr));
	//memset((char *)&server_addr, 0, sizeof(server_addr));
	
	//�e��ݒ�
	if((hp = gethostbyname(argv[1])) == NULL){
		perror("No such host");
		exit(1);
	}
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	
	//�T�[�o�ɐڑ�
	if(connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0){
		fprintf(stderr, "Cannot connect\n");
		exit(1);
	}
	printf("Connected.\n");*/
	fd = sock_connect(argv[1], (int)PORT);
	
	//�j�b�N�l�[���ݒ� ��2����
	bzero(buf, sizeof(buf));
	sprintf(buf, "%s", argv[2]);
	printf("your name :%s\n", buf);
	if(send(fd, buf, 1024, 0) < 0){
		fprintf(stderr, "Cannot send message\n");
		close(fd);
		exit(1);
	}
	
	//���[���ݒ� ��3����
	bzero(buf, sizeof(buf));
	sprintf(buf, "%s", argv[3]);
	printf("your room :%s\n", buf);
	if(send(fd, buf, 1024, 0) < 0){
		fprintf(stderr, "Cannot send message\n");
		close(fd);
		exit(1);
	}
	
	for(;;){//�������[�v
		FD_ZERO(&readfds);
		//FD_ZERO(&writefds);
		FD_SET(fd, &readfds);
		FD_SET(0, &readfds);
		
		if( (n = select(FD_SETSIZE, &readfds, NULL, NULL, NULL)) == -1){
			perror("select");
			fprintf(stderr, "select() error\n");
			exit(1);
		}
		
		//printf("--Return value of select() = %d [%d]\n", n, __LINE__);
		
		if(FD_ISSET(fd, &readfds)){//�\�P�b�g�̓��͂��m�F
			//printf("--Data coming from SOCKET[%d]\n",__LINE__);
			bzero(buf, sizeof(buf));
			//ret = read(s, str_buf, 1024);//���͂�ǂݍ���(recv)
			//printf("%s",str_buf);
			if((n = recv(fd, buf, 1024, 0)) < 0){
				fprintf(stderr, "Cannot receive massage\n");
				close(fd);
				exit(1);
			}
			else if(n == 0){//�T�[�o����R�l�N�V�������؂�ꂽ�ꍇ
				printf("\n\n--close received\n\n");
				printf("%s", buf);
				fflush(stdout);
				break;
			}
			else{
				printf("%s", buf);
				fflush(stdout);
			}
		}
		if(FD_ISSET(0, &readfds)){//�W�����͂�����
			//printf("--Data coming from KEYBOAD[%d]\n",__LINE__);
			fgets(buf, 1024, stdin);
			//write(s, str_buf, 1024);
			if(send(fd, buf, 1024, 0) < 0){
				fprintf(stderr, "Cannot send message\n");
				close(fd);
				exit(1);
			}
		}
	}
	
	close(fd);
	return(0);
}

