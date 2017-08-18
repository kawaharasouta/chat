#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 5000
#define SOCK_MAX 50
#define UNUSED (-1)

//org my_server.c

char* str_concat(char *str1, const char *str2);
char* str_concat2(const char *str1, const char *str2, char **result);
void str_replace(const char *src, const char *target, const char *replace, char **result);

int main(){
	int s[SOCK_MAX + 1], len, n = 0;
	fd_set readfds;
	int cllen;
	struct sockaddr_in saddr, caddr;
	char str[1024], buf[1024];
	int i, j, k;
	int msglen;
	int optval;
	//���ԕ\���p
	time_t now;
	struct tm *pnow;
	//BOT�p �^�C�}�[
	time_t timer;
	//struct tm *ptimer;
	int bot_flug = 1;//�ق��̃A�N�V�������N�����Ƃ���BOT�������Ȃ��悤�ɂ���B
	int bot_num;//������I�Ԃ悤
	//BOT�p ���t
	char bot_words[3][10] = {
		"Hello", "Hi", "Yeah"
	};
	//�񐔕\���p
	int times[SOCK_MAX + 1];
	//�j�b�N�l�[���p
	char name[SOCK_MAX + 1][10];
	//�u�������Ώە�����
	char bad_words[3][10] = {
		"idiot", "suck", "stupid"
	};
	//���[���p
	char room[SOCK_MAX + 1][10];
	
	timer = time(NULL);//���ݎ���
	srand((int)timer);
	
	//�\�P�b�g���� 0�Ԗڂ̂�̂�
	if((s[0] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(1);
	}
	
	printf("-- Server started!!\n", n);
	
	optval = 1;
	
	//�S�\�P�b�g��UNUSED(-1)�ŏ����� �񐔂�0�ɏ�����
	for(i = 1;i < SOCK_MAX + 1; i++){
		s[i] = UNUSED;
		times[i] = 0;
	}
	
	//�\���̂̃N���A
	bzero((char *)&saddr, sizeof(saddr));
	//�\���̂̃N���A
	//memset((char *)&my_addr, 0, sizeof(my_addr));
	
	//�e��ݒ�
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(PORT);
	
	//0�Ԗڂ̂�̂�
	if(bind(s[0], (struct sockaddr *)&saddr, sizeof(saddr)) == -1){
		//fprintf(stderr, "Cannot bind socket\n");
		perror("bind");
		exit(1);
	}
	if((listen(s[0], SOCK_MAX)) == -1){
		perror("listen");
		exit(1);
	}
	
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	
	while(1){
		FD_ZERO(&readfds);//�f�B�X�N���v�^�̏�����
		
		for(i = 0;i < SOCK_MAX;i++){//�g���Ă�\�P�b�g���Ď��Ώۂ�
			if(s[i] != UNUSED){
				FD_SET(s[i], &readfds);
			}
		}
		FD_SET(0, &readfds);//�T�[�o���̊Ǘ��R�}���h�悤�ɕW�����͂��Ď��Ώۂ�
		
		if((n = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout)) == -1){
			perror("select");
			exit(1);
		}
		printf("--select returns: %d\n", n);
		
		for(i = 1;i < SOCK_MAX;i++){
			if(s[i] != UNUSED){
				if(FD_ISSET(s[i], &readfds)){//�Ď��Ώۂɓ��͂����邩�݂�
					printf("--s[%d] ready for reading\n", i);
					bzero(str, sizeof(str));
					if((msglen = read(s[i], str, sizeof(str))) == -1){//���[�h�ł��Ȃ������珜�O
						perror("read");
						close(s[i]);
						s[i] = UNUSED;
						printf("Socket[%d]: %s", i, str);
					}
					else if(msglen != 0){//���͂���������
						now = time(NULL);//���ݎ���
						pnow = localtime(&now);//���擾
						printf("%s(client[%d])(%d:%d:%d): %s", name[i], i, pnow->tm_hour,pnow->tm_min,pnow->tm_sec, str);
						bzero(buf, sizeof(buf));
						char *result;
						for(k=0;k<3;k++){//�t�B���^�����O
							str_replace(str, bad_words[k], "**", &result);
							strcpy(str, result);
							//sprintf(str, "%s", result);
						}
						sprintf(buf, "%s(client[%d])(%d:%d:%d): %s", name[i], i, pnow->tm_hour,pnow->tm_min,pnow->tm_sec, str);
						free(result);
						for(j = 1;j< SOCK_MAX;j++){//���̎Q���҂ɒʒm
							if((s[j] != UNUSED) && (strcmp(room[i], room[j])) == 0){//���[������������݂̂ɑ��M
								write(s[j], buf, strlen(buf));
							}
						}
						times[i]++;//�񐔂�1����
						if(times[i] >= 10){//�񐔐��� �����ޏ�
							printf("--%s(client[%d]): Number of times limit.\n", name[i], i);
							close(s[i]);
							s[i] = UNUSED;
							times[i] = 0;
						}
					}
					else{//���͂��󂾂�����
						printf("--%s(client[%d]): connection closed.\n", name[i], i);
						close(s[i]);
						s[i] = UNUSED;
						times[i] = 0;
					}
					
				}
			}
		}
		
		//�V�K�Q���҂̎�t
		if(FD_ISSET(s[0], &readfds) != 0){
			printf("Accept New one.\n");
			len = sizeof(caddr);
			for(i = 1;i < SOCK_MAX + 1;i++){
				if(s[i] == UNUSED){
					//���g�p�̃\�P�b�g�z���accept�̌���(�V�\�P�b�gFD)��ݒ肷��B
					s[i] = accept(s[0], (struct sockaddr *)&caddr, &len);
					printf("%d = accept()\n", s[i]);
					//�j�b�N�l�[���̐ݒ�
					bzero(str, sizeof(str));
					//msglen = read(s[i], str, sizeof(str));
					if((n = recv(s[i], str, 1024, 0)) < 0){
						fprintf(stderr, "Cannot receive massage\n");
						close(s[i]);
						s[i] = UNUSED;
						exit(1);
					}
					sprintf(name[i],"%s", str);
					printf("%s login\n", name[i]);
					
					//���[���ݒ�
					bzero(str, sizeof(str));
					if((n = recv(s[i], str, 1024, 0)) < 0){
						fprintf(stderr, "Cannot receive massage\n");
						close(s[i]);
						s[i] = UNUSED;
						exit(1);
					}
					sprintf(room[i],"%s", str);
					
					
					if(s[i] == -1){
						perror(NULL);
						exit(1);
					}
					if(i == SOCK_MAX){//�Q���l�����
						printf("refuse connection.\n");
						bzero(str, sizeof(str));
						strcpy(str, "Server is too busy.\n");
						write(s[i], str, strlen(str));
						close(s[i]);
						s[i] = UNUSED;
					}
					i = SOCK_MAX + 1;//�V�K����������for�𔲂���B
				}
			}
		}
		
		if(FD_ISSET(0, &readfds)){//�W�����͂����� �Ǘ��R�}���h �w�肵�����[�U�[��ޏꂳ����B(�ԍ��Ŏw��)
			bzero(buf, sizeof(buf));
			fgets(buf, 1024, stdin);
			for(i = 1;i < SOCK_MAX + 1;i++){
				//if((strcmp(name[i], buf)) == 0){
				if(i == (*buf - '0')){//�ԍ�������
					printf("--%s(client[%d]): Forced exit.\n", name[i], i);
					close(s[i]);
					s[i] = UNUSED;
					times[i] = 0;
				}
				i = SOCK_MAX + 1;
			}
		}
		
		//�ق��̓���̂Ƃ���BOT�������Ȃ��悤�Ɍ�����
		for(i = 1;i < SOCK_MAX;i++){
			if(s[i] != UNUSED){
				if(FD_ISSET(s[i], &readfds)){
					bot_flug = 0;
					i = SOCK_MAX + 1;
				}
			}
		}
		if(FD_ISSET(s[0], &readfds)){
			bot_flug = 0;
			i = SOCK_MAX + 1;
		}
		else if(FD_ISSET(0, &readfds)){
			bot_flug = 0;
			i = SOCK_MAX + 1;
		}
		
		/**
		 *�^�C���A�E�g���邽�т�BOT����������悤�ɂ���
		 *�^�C���A�E�g��10�b�ɐݒ�
		 *�N�����A�N�V�������N�������ꍇ�͂����Ń^�C���A�E�g�̃J�E���g�������������̂ŁA���ق������Ȃ��Ɠ����Ȃ�
		 *�{���͊��S�Ɏ��Ԃœ������������������܂������Ȃ�����
		 */
		//timer = time(NULL);//���ݎ���
		//ptimer = localtime(&timer);//���擾
		if(bot_flug == 1){
			bot_num = (int)(rand()%3);
			bzero(str, sizeof(str));
			now = time(NULL);//���ݎ���
			pnow = localtime(&now);//���擾
			sprintf(str, "BOT(%d:%d:%d): %s\n", pnow->tm_hour,pnow->tm_min,pnow->tm_sec, bot_words[bot_num]);
			for(j = 1;j< SOCK_MAX;j++){//���̎Q���҂�
				if((s[j] != UNUSED)){
					write(s[j], str, strlen(str));
				}
			}
		}
		bot_flug = 1;
	}
	
	return 0;
}


//�ȉ�������u���p�֐��Q
/**
 *  �������A������
 * 
 *  �����Ŏ󂯎�镶����́u�z��^�v������
 *  �t�Ƀ|�C���^�^�Œ�`����ƃG���[������������ۂ�
 */
char* str_concat(char *str1, const char *str2) {
	// str1�̍ŏ��̃A�h���X��ێ����Ă���
	char *top = str1;

	// ������I�[�܂Ń|�C���^��i�߂�
	while (*(str1++) != '\0');

	// `\0`�����������߁A�|�C���^���ЂƂ߂��i�A���̂��߁j
	str1 -= 1;

	// �I�[���O����Astr2�̕�����ǉ����Ă���
	do {
		*(str1++) = *str2;
	} while (*(str2++) != '\0');

	return str1;
}

/**
 *  malloc���g���ăq�[�v�ɗ̈���m�ۂ���p�^�[��
 */
char* str_concat2(const char *str1, const char *str2, char **result) {
	// �A����̕����񕪂̗̈���m�ۂ���i+1�͏I�[�����i\0�j���j
	size_t size = sizeof(char) * (strlen(str1) + strlen(str2) + 1);
	char *work = (char*)malloc(size);
	if (work == NULL) {
		printf("Cannot allocate memory.\n");
		return NULL;
	}

	// �m�ۂ����̈�̍ŏ��̃A�h���X��ێ����Ă���
	char *top = work;

	// str1���R�s�[
	strcpy(work, str1);

	// �ǉ��������������A�|�C���^��i�߂�
	work += strlen(str1);

	// str2���R�s�[
	strcpy(work, str2);

	// �m�ۂ����擪�A�h���X�����ʂɊi�[
	*result = top;

	return top;
}

/**
 *  �������u������
 *
 *  @param src �]�����镶����
 *  @param target  �u�������ΏۂƂȂ镶����
 *  @param replace �u�������镶����
 *  @param result  �u����̕������Ԃ��|�C���^
 */
void str_replace(const char *src, const char *target, const char *replace, char **result) {
	char *temp = (char*)malloc(sizeof(char) * 1000);
	if (temp == NULL) {
		printf("Cannot allocate memory.\n");
		return;
	}

	char *top = temp;

	// ����ł���悤�ɃR�s�[����
	char *work = (char*)malloc(sizeof(char) * strlen(src));
	strcpy(work, src);

	char *p;
	while ((p = strstr(work, target)) != NULL) {
		// ���m�����ʒu�ɕ�����I�[������}��
		*p = '\0';

		// ���m�����ʒu�{�Ώە��������̈ʒu�Ƀ|�C���^���ړ�
		p += strlen(target);

		// �Y��������ȍ~����������temp�ɑޔ�
		strcpy(temp, p);

		// �O��������ƒu�������������A������
		str_concat(work, replace);
		str_concat(work, temp);
	}

	free(temp);

	*result = work;
}

