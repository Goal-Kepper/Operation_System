#include "msglib.h"
#include <sys/ipc.h>
#include <sys/msg.h>

int open_queue(key_t keyval) {
    // открытие или создание очереди сообщений
    int qid;
    if ((qid = msgget( keyval, IPC_CREAT | 0660 )) == -1) return (-1);
    return qid;
}

int send_message(int qid, struct mymsgbuf *qbuf) {
    // отправка сообщения в очередь
    int result, length;
    length = sizeof(struct mymsgbuf) - sizeof(long);
    if ((result = msgsnd( qid, qbuf, length, 0)) == -1) return (-1);
    return result;
}

int send_message_no_wait(int qid, struct mymsgbuf *qbuf) {
    // отправка сообщения в очередь
    int result, length;
    length = sizeof(struct mymsgbuf) - sizeof(long);
    if ((result = msgsnd( qid, qbuf, length, IPC_NOWAIT)) == -1) return (-1);
    return result;
}


int read_message(int qid, long type, struct mymsgbuf *qbuf) {
    // чтение сообщения из очереди
    int result, length;
    length = sizeof(struct mymsgbuf) - sizeof(long);
    if ((result = msgrcv(qid, qbuf, length, type, 0)) == -1) return (-1);
    return result;
}

int read_message_no_wait(int qid, long type, struct mymsgbuf *qbuf) {
    // чтение сообщения из очереди
    int result, length;
    length = sizeof(struct mymsgbuf) - sizeof(long);
    if ((result = msgrcv(qid, qbuf, length, type, IPC_NOWAIT)) == -1) return (-1);
    return result;
}

int remove_queue(int qid) {
    if (msgctl(qid, IPC_RMID, 0) == -1) return (-1);
    return 0;
}
