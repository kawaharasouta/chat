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
	//時間表示用
	time_t now;
	struct tm *pnow;
	//BOT用 タイマー
	time_t timer;
	//struct tm *ptimer;
	int bot_flug = 1;//ほかのアクションが起きたときはBOTが動かないようにする。
	int bot_num;//発言を選ぶよう
	//BOT用 言葉
	char bot_words[3][10] = {
		"Hello", "Hi", "Yeah"
	};
	//回数表示用
	int times[SOCK_MAX + 1];
	//ニックネーム用
	char name[SOCK_MAX + 1][10];
	//置き換え対象文字列
	char bad_words[3][10] = {
		"idiot", "suck", "stupid"
	};
	//ルーム用
	char room[SOCK_MAX + 1][10];
	
	timer = time(NULL);//現在時刻
	srand((int)timer);
	
	//ソケット生成 0番目のやつのみ
	if((s[0] = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(1);
	}
	
	printf("-- Server started!!\n", n);
	
	optval = 1;
	
	//全ソケットをUNUSED(-1)で初期化 回数を0に初期化
	for(i = 1;i < SOCK_MAX + 1; i++){
		s[i] = UNUSED;
		times[i] = 0;
	}
	
	//構造体のクリア
	bzero((char *)&saddr, sizeof(saddr));
	//構造体のクリア
	//memset((char *)&my_addr, 0, sizeof(my_addr));
	
	//各種設定
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(PORT);
	
	//0番目のやつのみ
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
		FD_ZERO(&readfds);//ディスクリプタの初期化
		
		for(i = 0;i < SOCK_MAX;i++){//使われてるソケットを監視対象に
			if(s[i] != UNUSED){
				FD_SET(s[i], &readfds);
			}
		}
		FD_SET(0, &readfds);//サーバ側の管理コマンドように標準入力を監視対象に
		
		if((n = select(FD_SETSIZE, &readfds, NULL, NULL, &timeout)) == -1){
			perror("select");
			exit(1);
		}
		printf("--select returns: %d\n", n);
		
		for(i = 1;i < SOCK_MAX;i++){
			if(s[i] != UNUSED){
				if(FD_ISSET(s[i], &readfds)){//監視対象に入力があるかみる
					printf("--s[%d] ready for reading\n", i);
					bzero(str, sizeof(str));
					if((msglen = read(s[i], str, sizeof(str))) == -1){//リードできなかったら除外
						perror("read");
						close(s[i]);
						s[i] = UNUSED;
						printf("Socket[%d]: %s", i, str);
					}
					else if(msglen != 0){//入力があったら
						now = time(NULL);//現在時刻
						pnow = localtime(&now);//を取得
						printf("%s(client[%d])(%d:%d:%d): %s", name[i], i, pnow->tm_hour,pnow->tm_min,pnow->tm_sec, str);
						bzero(buf, sizeof(buf));
						char *result;
						for(k=0;k<3;k++){//フィルタリング
							str_replace(str, bad_words[k], "**", &result);
							strcpy(str, result);
							//sprintf(str, "%s", result);
						}
						sprintf(buf, "%s(client[%d])(%d:%d:%d): %s", name[i], i, pnow->tm_hour,pnow->tm_min,pnow->tm_sec, str);
						free(result);
						for(j = 1;j< SOCK_MAX;j++){//他の参加者に通知
							if((s[j] != UNUSED) && (strcmp(room[i], room[j])) == 0){//ルーム名が同じやつのみに送信
								write(s[j], buf, strlen(buf));
							}
						}
						times[i]++;//回数に1足す
						if(times[i] >= 10){//回数制限 強制退場
							printf("--%s(client[%d]): Number of times limit.\n", name[i], i);
							close(s[i]);
							s[i] = UNUSED;
							times[i] = 0;
						}
					}
					else{//入力が空だったら
						printf("--%s(client[%d]): connection closed.\n", name[i], i);
						close(s[i]);
						s[i] = UNUSED;
						times[i] = 0;
					}
					
				}
			}
		}
		
		//新規参加者の受付
		if(FD_ISSET(s[0], &readfds) != 0){
			printf("Accept New one.\n");
			len = sizeof(caddr);
			for(i = 1;i < SOCK_MAX + 1;i++){
				if(s[i] == UNUSED){
					//未使用のソケット配列にacceptの結果(新ソケットFD)を設定する。
					s[i] = accept(s[0], (struct sockaddr *)&caddr, &len);
					printf("%d = accept()\n", s[i]);
					//ニックネームの設定
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
					
					//ルーム設定
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
					if(i == SOCK_MAX){//参加人数上限
						printf("refuse connection.\n");
						bzero(str, sizeof(str));
						strcpy(str, "Server is too busy.\n");
						write(s[i], str, strlen(str));
						close(s[i]);
						s[i] = UNUSED;
					}
					i = SOCK_MAX + 1;//新規が入ったらforを抜ける。
				}
			}
		}
		
		if(FD_ISSET(0, &readfds)){//標準入力を見る 管理コマンド 指定したユーザーを退場させる。(番号で指定)
			bzero(buf, sizeof(buf));
			fgets(buf, 1024, stdin);
			for(i = 1;i < SOCK_MAX + 1;i++){
				//if((strcmp(name[i], buf)) == 0){
				if(i == (*buf - '0')){//番号を見る
					printf("--%s(client[%d]): Forced exit.\n", name[i], i);
					close(s[i]);
					s[i] = UNUSED;
					times[i] = 0;
				}
				i = SOCK_MAX + 1;
			}
		}
		
		//ほかの動作のときにBOTが動かないように見張る
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
		 *タイムアウトするたびにBOTが発言するようにした
		 *タイムアウトは10秒に設定
		 *誰かがアクションを起こした場合はそこでタイムアウトのカウントが初期化されるので、沈黙が続かないと動かない
		 *本当は完全に時間で動かしたかったがうまくいかなかった
		 */
		//timer = time(NULL);//現在時刻
		//ptimer = localtime(&timer);//を取得
		if(bot_flug == 1){
			bot_num = (int)(rand()%3);
			bzero(str, sizeof(str));
			now = time(NULL);//現在時刻
			pnow = localtime(&now);//を取得
			sprintf(str, "BOT(%d:%d:%d): %s\n", pnow->tm_hour,pnow->tm_min,pnow->tm_sec, bot_words[bot_num]);
			for(j = 1;j< SOCK_MAX;j++){//他の参加者に
				if((s[j] != UNUSED)){
					write(s[j], str, strlen(str));
				}
			}
		}
		bot_flug = 1;
	}
	
	return 0;
}


//以下文字列置換用関数群
/**
 *  文字列を連結する
 * 
 *  引数で受け取る文字列は「配列型」を期待
 *  逆にポインタ型で定義するとエラーが発生するっぽい
 */
char* str_concat(char *str1, const char *str2) {
	// str1の最初のアドレスを保持しておく
	char *top = str1;

	// 文字列終端までポインタを進める
	while (*(str1++) != '\0');

	// `\0`分を消すため、ポインタをひとつ戻す（連結のため）
	str1 -= 1;

	// 終端直前から、str2の文字を追加していく
	do {
		*(str1++) = *str2;
	} while (*(str2++) != '\0');

	return str1;
}

/**
 *  mallocを使ってヒープに領域を確保するパターン
 */
char* str_concat2(const char *str1, const char *str2, char **result) {
	// 連結後の文字列分の領域を確保する（+1は終端文字（\0）分）
	size_t size = sizeof(char) * (strlen(str1) + strlen(str2) + 1);
	char *work = (char*)malloc(size);
	if (work == NULL) {
		printf("Cannot allocate memory.\n");
		return NULL;
	}

	// 確保した領域の最初のアドレスを保持しておく
	char *top = work;

	// str1をコピー
	strcpy(work, str1);

	// 追加した文字数分、ポインタを進める
	work += strlen(str1);

	// str2をコピー
	strcpy(work, str2);

	// 確保した先頭アドレスを結果に格納
	*result = top;

	return top;
}

/**
 *  文字列を置換する
 *
 *  @param src 評価する文字列
 *  @param target  置き換え対象となる文字列
 *  @param replace 置き換える文字列
 *  @param result  置換後の文字列を返すポインタ
 */
void str_replace(const char *src, const char *target, const char *replace, char **result) {
	char *temp = (char*)malloc(sizeof(char) * 1000);
	if (temp == NULL) {
		printf("Cannot allocate memory.\n");
		return;
	}

	char *top = temp;

	// 操作できるようにコピーする
	char *work = (char*)malloc(sizeof(char) * strlen(src));
	strcpy(work, src);

	char *p;
	while ((p = strstr(work, target)) != NULL) {
		// 検知した位置に文字列終端文字を挿入
		*p = '\0';

		// 検知した位置＋対象文字数分の位置にポインタを移動
		p += strlen(target);

		// 該当文字列以降をいったんtempに退避
		strcpy(temp, p);

		// 前半文字列と置き換え文字列を連結する
		str_concat(work, replace);
		str_concat(work, temp);
	}

	free(temp);

	*result = work;
}

