#ifndef MSGLIB_H_
#define MSGLIB_H_ 

#include <time.h>
#include <sys/ipc.h>

#define MESSAGES_QUEUE_KEY "message_secret"

struct mymsgbuf {
    long mtype;                // номер канала в очереди сообщений

    // отправка сообщений
    char msgtext[64];          // сообщение пользователя
    long from_user_id;         // пользователь, которому нужно доставить сообщение
    long to_user_id;           // пользователь, отправивший сообщение

    // авторизация
    int communicate_channel;   // номер канала в очереди сообщений
    int user_id;               // ID, который сервер назначил пользователю
    char key[32];              // ключ шифрования
};

int open_queue(key_t keyval);
int send_message(int qid, struct mymsgbuf *qbuf);
int send_message_no_wait(int qid, struct mymsgbuf *qbuf);
int read_message(int qid, long type, struct mymsgbuf *qbuf);
int read_message_no_wait(int qid, long type, struct mymsgbuf *qbuf);
int remove_queue(int qid);

#endif
