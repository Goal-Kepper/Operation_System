
# include <unistd.h>
# include <stdio.h>
# include <malloc.h>
# include <string.h>
# include "msglib/msglib.h"
# include "globals.h"

struct User {
	long user_id;
	long communicate_channel;
	char key[32];
};

void init_mymsgbuf(struct mymsgbuf *msg) {
    // отправка сообщений
    memset(msg->msgtext, 64, 0);
    msg->from_user_id = -1;
    msg->to_user_id = -1;

    // авторизация
    msg->communicate_channel = -1;
    msg->user_id = -1;
    memset(msg->key, 32, 0);

};

int hash_key(char *key, int keylen) {
    /* Хеширование ключа для представления его числом */

    int hashed = 0;
    for (int i = 0; i < keylen; ++i) {
        hashed += key[i];
    }

    return hashed;
}

void encrypt(char *message, int msglen, struct User *myinfo) {
    /* Функция потокового шифрования  */

    int key = hash_key(myinfo->key, strlen(myinfo->key));
    for (int i = 0; i < msglen; ++i) {
        message[i] = message[i] ^ key; 
    }

}

void decrypt(char *message, int msglen, struct User *myinfo) {
    /* Функция потокового расшифрования */

    int key = hash_key(myinfo->key, strlen(myinfo->key));
    for (int i = 0; i < msglen; ++i) {
        message[i] = message[i] ^ key; 
    }
}

int main() {
    // получение адреса сервера
    key_t secret_msq_key = ftok("secret", 0);

    // подключение к серверу
    int msqid = open_queue(secret_msq_key);

    // информация о текущем клиенте 
   	struct User myinfo;
    // инициализация начальными значениями
    myinfo.user_id = -1;
    myinfo.communicate_channel = -1;
    memset(myinfo.key, 32, 0); 

    /* отправка запроса на авторизацию */
    struct mymsgbuf qbuf;
    init_mymsgbuf(&qbuf); 
    qbuf.mtype = AUTH_CHANNEL;
    send_message_no_wait(msqid, &qbuf);

   	/* получение информации об авторизации на сервере */
   	struct mymsgbuf response;
    init_mymsgbuf(&response); 
   	read_message(msqid, AUTH_ANSWER_CHANNEL, &response);

    // заполнение информации, полученной от сервера, о текущем клиенте
   	myinfo.user_id = response.user_id;
   	myinfo.communicate_channel = response.communicate_channel;
   	strcpy(myinfo.key,response.key);

    // вывод полезной для клиента информации
    printf("\n===\n");
    printf("Your user ID is: %d\n", myinfo.user_id);
    printf("Your channel to receive messages from server is: %d\n", myinfo.communicate_channel);
    printf("Your secret key is: '%s'\n", myinfo.key);
    printf("\n===\n");

    // основной функционал клиента: чтение сообщений от другого пользователя, отправка сообщений
   	for (;;) {

   		struct qbuf; init_mymsgbuf(&qbuf);
   		// прочитать сообщения от пользователей
        int res = read_message_no_wait(msqid, myinfo.communicate_channel, &qbuf);
        if (res != -1) {

            // вывод информации о получении нового сообщения
            printf("Got new message from user with ID=%d\n", qbuf.from_user_id);
            printf("Msg was decrypted from: '%s'\n", qbuf.msgtext);
        	decrypt(qbuf.msgtext, strlen(qbuf.msgtext), &myinfo);
            printf("User with ID=%d sent: '%s'\n", qbuf.from_user_id, qbuf.msgtext);

            // очистка структуры для чтения следующего сообщения
            init_mymsgbuf(&qbuf);
            while ( res =  read_message_no_wait(msqid, myinfo.communicate_channel, &qbuf) != -1 ) {
                decrypt(qbuf.msgtext, strlen(qbuf.msgtext), &myinfo);
                printf("Got new message from user with ID=%d\n", qbuf.from_user_id);
                printf("%d sent: %s\n", qbuf.from_user_id, qbuf.msgtext);

                // очистка структуры для чтения следующего сообщения
                init_mymsgbuf(&qbuf);
            }
        
        }

        // пользователь вводит сообщение для отправки
        struct mymsgbuf msgtouser;
        init_mymsgbuf(&msgtouser);

        // отправка сообщения на сервер в канал для отправки сообщения другим клиентам
        msgtouser.mtype = SEND_MSG_TO_CLIENT_CHANNEL;
        printf("Type message to send: ");
        fgets(msgtouser.msgtext, 64, stdin);
        msgtouser.msgtext[strcspn(msgtouser.msgtext, "\n")] = 0;
        encrypt(msgtouser.msgtext, strlen(msgtouser.msgtext), &myinfo);
        printf("Your msg was encrypted to: '%s'\n", msgtouser.msgtext);

        // установка пользователя, кому будет отправлено сообщение
        msgtouser.from_user_id = myinfo.user_id;
        int to_user_id;
        if (myinfo.user_id == 4) {
        	to_user_id = 5;
        } else to_user_id = 4;
        msgtouser.to_user_id = to_user_id;

        // отправка собщения
        send_message_no_wait(msqid, &msgtouser);

   	}

    return 0;
}
