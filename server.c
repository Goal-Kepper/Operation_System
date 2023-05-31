
# include <unistd.h>
# include <malloc.h>
# include <string.h>
# include "msglib/msglib.h"
# include "globals.h"

struct Users {
	long user_id;
	long communicate_channel;
};

int main() {

	char secret_key[32] = "secret";
	key_t secret_msq_key = ftok("secret", 0);

	// создание сервера
	int msqid = open_queue(secret_msq_key);

    // очистка предыдущей работы сервера
    {
        struct mymsgbuf qbuf;
        while ( read_message_no_wait(msqid, 0, &qbuf) != -1 );
    }

    // создание пользователей
    long CURRENT_USERS = 0;
    long MAX_USERS = 2;
    struct Users *users = (struct Users*) malloc (sizeof(struct Users) * MAX_USERS);

	// запуск сервера 
	printf("SERVER ON.\n");

	for (;;) {
		struct mymsgbuf request;

		// прослушивание сервера
	    int res = -1;       // -1 означает, что новых сообщений нет
    	int channel = -1;   // в случае, когда res != -1, его значение показывает канал, из которого пришло сообщение

    	while (res == -1) {
        	sleep(1);
        	printf("Waiting for requests...\n");
        	// прослушивание каналов авторизации и чата
        	for (int chan = SEND_MSG_TO_CLIENT_CHANNEL; chan <= AUTH_CHANNEL; ++chan)  {
                res = read_message_no_wait(msqid, chan, &request);
                printf("Listening channel %d...\n", chan);
                // после первого полученного сообщения начинается его обработка
                if (res != -1) {
            	    printf("Got new message! \n");
                    channel = chan;
                    break;
        	    }
            }

        	printf("Sleep for 1 sec...\n");
    	}

    	if (channel == AUTH_CHANNEL) {
            printf("New user was connected to the server\n");

    		// новый пользователь хочет добавиться в систему
            //
    		int channel_id = CHANNELS_OFFSET + CURRENT_USERS;

    		// сохранение информации о пользователе
    		users[CURRENT_USERS].user_id = channel_id;
    		users[CURRENT_USERS].communicate_channel = channel_id;

    		// подготовка ответа
    		struct mymsgbuf response;
    		response.mtype = AUTH_ANSWER_CHANNEL;
    		response.user_id = channel_id;
    		response.communicate_channel = channel_id;
            memset(response.key, 32, 0);
    		strcpy(response.key, secret_key);

    		//printf("mtype: %d", response.mtype);
    		send_message_no_wait(msqid, &response);

    		++CURRENT_USERS;

    		continue;
    	}

    	printf("=====\n");
    	printf("Got msg from user = %d to user = %d\n", request.from_user_id, request.to_user_id);
        printf("Encrypted msg is: '%s'\n", request.msgtext);
    	printf("=====\n");

    	// отправка зашифрованного сообщения получателю
    	struct mymsgbuf msgtouser;
    	msgtouser.mtype = request.to_user_id;
    	strcpy(msgtouser.msgtext, request.msgtext);
    	msgtouser.from_user_id = request.from_user_id;


    	send_message_no_wait(msqid, &msgtouser);


	}

	return 0;
}
