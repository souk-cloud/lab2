#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/select.h>

#define FIFO_1_TO_2 "/tmp/chat_fifo_1to2"
#define FIFO_2_TO_1 "/tmp/chat_fifo_2to1"
#define BUF_SIZE 256

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "1") != 0 && strcmp(argv[1], "2") != 0)) {
        printf("사용법: %s <1 또는 2>\n", argv[0]);
        exit(1);
    }

    int user_id = atoi(argv[1]);

    // 두 방향의 이름 있는 파이프(FIFO) 생성 (이미 존재하면 무시)
    mkfifo(FIFO_1_TO_2, 0666);
    mkfifo(FIFO_2_TO_1, 0666);

    int read_fd, write_fd;
    printf("=== 파이프 채팅 프로그램 (사용자 %d) ===\n", user_id);
    printf("상대방 접속 대기 중...\n");

    // 교착 상태(Deadlock) 방지를 위해 사용자 1과 2의 open 순서를 반대로 설정
    if (user_id == 1) {
        write_fd = open(FIFO_1_TO_2, O_WRONLY);
        read_fd  = open(FIFO_2_TO_1, O_RDONLY);
    } else {
        read_fd  = open(FIFO_1_TO_2, O_RDONLY);
        write_fd = open(FIFO_2_TO_1, O_WRONLY);
    }

    printf("연결 완료! 메시지를 입력하세요. (종료하려면 /quit 입력)\n\n");

    fd_set read_fds;
    char buf[BUF_SIZE];
    int max_fd = (read_fd > STDIN_FILENO ? read_fd : STDIN_FILENO) + 1;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds); // 표준 입력(키보드) 감시
        FD_SET(read_fd, &read_fds);      // 수신 파이프 감시

        if (select(max_fd, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select 오류");
            break;
        }

        // 1. 내 키보드로 메시지를 입력한 경우
        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            if (fgets(buf, BUF_SIZE, stdin) != NULL) {
                write(write_fd, buf, strlen(buf) + 1);
                if (strncmp(buf, "/quit", 5) == 0) {
                    printf("채팅을 종료합니다.\n");
                    break;
                }
            }
        }

        // 2. 상대방 파이프로부터 메시지가 도착한 경우
        if (FD_ISSET(read_fd, &read_fds)) {
            ssize_t n = read(read_fd, buf, BUF_SIZE);
            if (n <= 0 || strncmp(buf, "/quit", 5) == 0) {
                printf("\n[알림] 상대방이 채팅을 종료했습니다.\n");
                break;
            }
            printf("[상대방]: %s", buf);
            fflush(stdout);
        }
    }

    close(read_fd);
    close(write_fd);
    unlink(FIFO_1_TO_2);
    unlink(FIFO_2_TO_1);
    return 0;
}
