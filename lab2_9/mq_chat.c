#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <sys/wait.h>

#define MQ_1_TO_2 "/chat_mq_1to2"
#define MQ_2_TO_1 "/chat_mq_2to1"
#define MAX_MSG_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0)) {
        printf("사용법: %s <1 또는 2>\n", argv[0]);
        exit(1);
    }

    int user_id = atoi(argv[1]);
    const char *send_q_name = (user_id == 1) ? MQ_1_TO_2 : MQ_2_TO_1;
    const char *recv_q_name = (user_id == 1) ? MQ_2_TO_1 : MQ_1_TO_2;

    // 메시지 큐 속성 설정 (기본 시스템 한도 내의 안전한 크기)
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    // 송신용 및 수신용 POSIX 메시지 큐 열기 (없으면 생성)
    mqd_t send_mq = mq_open(send_q_name, O_WRONLY | O_CREAT, 0666, &attr);
    mqd_t recv_mq = mq_open(recv_q_name, O_RDONLY | O_CREAT, 0666, &attr);

    if (send_mq == (mqd_t)-1 || recv_mq == (mqd_t)-1) {
        perror("mq_open 실패");
        exit(1);
    }

    printf("=== POSIX 메시지 큐 채팅 프로그램 (사용자 %d) ===\n", user_id);
    printf("메시지를 입력하세요. (종료하려면 /quit 입력)\n\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    } else if (pid == 0) {
        // [자식 프로세스] 상대방 메시지 수신 전담
        char recv_buf[MAX_MSG_SIZE + 1];
        while (1) {
            ssize_t bytes = mq_receive(recv_mq, recv_buf, MAX_MSG_SIZE, NULL);
            if (bytes >= 0) {
                recv_buf[bytes] = '\0';
                if (strncmp(recv_buf, "/quit", 5) == 0) {
                    printf("\n[알림] 상대방이 채팅을 종료했습니다.\n");
                    kill(getppid(), SIGTERM);
                    break;
                }
                printf("[상대방]: %s", recv_buf);
                fflush(stdout);
            }
        }
        mq_close(recv_mq);
        exit(0);
    } else {
        // [부모 프로세스] 키보드 입력 및 메시지 송신 전담
        char send_buf[MAX_MSG_SIZE];
        while (fgets(send_buf, MAX_MSG_SIZE, stdin) != NULL) {
            if (mq_send(send_mq, send_buf, strlen(send_buf) + 1, 0) == -1) {
                perror("mq_send 실패");
                break;
            }
            if (strncmp(send_buf, "/quit", 5) == 0) {
                printf("채팅을 종료합니다.\n");
                kill(pid, SIGTERM);
                break;
            }
        }
        wait(NULL);
        mq_close(send_mq);
        mq_close(recv_mq);
        mq_unlink(MQ_1_TO_2);
        mq_unlink(MQ_2_TO_1);
    }

    return 0;
}
