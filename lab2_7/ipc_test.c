#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    char buffer[128] = {0};

    printf("=== [Part 1] 파이프(pipe)를 이용한 부모-자식 간 메시지 전달 ===\n");
    if (pipe(fd) == -1) {
        perror("pipe 생성 실패");
        exit(1);
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // 자식 프로세스: 쓰기용 디스크립터(fd[1]) 닫고 읽기(fd[0]) 수행
        close(fd[1]);
        read(fd[0], buffer, sizeof(buffer) - 1);
        printf("  [자식 PID:%d] 파이프로 수신한 메시지: \"%s\"\n", getpid(), buffer);
        close(fd[0]);
        exit(0);
    } else {
        // 부모 프로세스: 읽기용 디스크립터(fd[0]) 닫고 쓰기(fd[1]) 수행
        close(fd[0]);
        const char *msg = "Hello IPC via Linux Pipe!";
        printf("[부모 PID:%d] 파이프에 데이터 쓰기: \"%s\"\n", getpid(), msg);
        write(fd[1], msg, strlen(msg));
        close(fd[1]);
        wait(NULL);
    }

    printf("\n=== [Part 2] fcntl()을 이용한 파일 레코드 잠금(Record Locking) ===\n");
    int file_fd = open("lock_test.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
    if (file_fd == -1) {
        perror("open 실패");
        exit(1);
    }

    pid_t pid2 = fork();
    if (pid2 > 0) {
        // 부모 프로세스: 먼저 배제적 쓰기 잠금(F_WRLCK) 설정
        struct flock lock;
        memset(&lock, 0, sizeof(lock));
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0; // 파일 전체 잠금

        fcntl(file_fd, F_SETLKW, &lock);
        printf("[부모 PID:%d] 파일 배제 잠금(F_WRLCK) 획득! 2초간 작업 후 해제합니다...\n", getpid());
        write(file_fd, "Parent wrote first.\n", 20);
        sleep(2);

        lock.l_type = F_UNLCK;
        fcntl(file_fd, F_SETLK, &lock);
        printf("[부모 PID:%d] 파일 잠금 해제(F_UNLCK) 완료.\n", getpid());
        wait(NULL);
        close(file_fd);
    } else if (pid2 == 0) {
        // 자식 프로세스: 부모가 잠금을 먼저 잡도록 0.5초 대기 후 잠금 시도
        usleep(500000);
        struct flock lock;
        memset(&lock, 0, sizeof(lock));
        lock.l_type = F_WRLCK;
        lock.l_whence = SEEK_SET;
        lock.l_start = 0;
        lock.l_len = 0;

        printf("  [자식 PID:%d] 파일 쓰기 잠금 요청 (부모 해제 시까지 대기 중)...\n", getpid());
        fcntl(file_fd, F_SETLKW, &lock); // 부모가 해제할 때까지 블로킹 대기
        printf("  [자식 PID:%d] 잠금 획득 성공! 데이터 기록 후 종료합니다.\n", getpid());
        write(file_fd, "Child wrote second.\n", 20);

        lock.l_type = F_UNLCK;
        fcntl(file_fd, F_SETLK, &lock);
        close(file_fd);
        exit(0);
    }

    return 0;
}
