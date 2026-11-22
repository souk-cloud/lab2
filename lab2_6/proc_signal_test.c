#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

// 자식 프로세스용 사용자 정의 시그널(SIGUSR1) 핸들러
void sigusr1_handler(int signo) {
    printf("  [자식 PID:%d] 부모로부터 SIGUSR1(번호: %d) 시그널을 수신했습니다!\n", getpid(), signo);
}

// 부모 프로세스용 알람 시그널(SIGALRM) 핸들러
void sigalrm_handler(int signo) {
    printf("[부모 PID:%d] 2초 타이머 만료! SIGALRM(번호: %d) 시그널을 수신했습니다.\n", getpid(), signo);
}

int main() {
    pid_t pid;
    int status;

    printf("[부모 PID:%d] 프로세스 및 시그널 실습을 시작합니다.\n", getpid());

    // 1. fork()를 이용한 자식 프로세스 생성
    pid = fork();

    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    } else if (pid == 0) {
        // --- 자식 프로세스 영역 ---
        signal(SIGUSR1, sigusr1_handler); // SIGUSR1 핸들러 등록
        printf("  [자식 PID:%d, 부모 PPID:%d] 생성 완료. 부모의 시그널을 대기합니다...\n", getpid(), getppid());

        pause(); // 시그널이 도착할 때까지 대기

        printf("  [자식 PID:%d] 작업을 마치고 종료 코드 42로 종료합니다.\n", getpid());
        exit(42);
    } else {
        // --- 부모 프로세스 영역 ---
        sleep(1); // 자식이 핸들러를 등록하고 pause()에 들어갈 시간을 확보
        printf("[부모 PID:%d] 자식(PID:%d)에게 kill()로 SIGUSR1 시그널을 전송합니다.\n", getpid(), pid);
        kill(pid, SIGUSR1);

        // 2. wait()를 이용한 자식 프로세스 종료 대기 및 상태 회수
        pid_t terminated_pid = wait(&status);
        if (WIFEXITED(status)) {
            printf("[부모 PID:%d] 자식(PID:%d) 정상 종료 확인 (종료 코드: %d)\n",
                   getpid(), terminated_pid, WEXITSTATUS(status));
        }

        // 3. alarm()을 이용한 타이머 시그널 실습
        signal(SIGALRM, sigalrm_handler);
        printf("[부모 PID:%d] alarm(2) 설정 후 2초간 대기합니다...\n", getpid());
        alarm(2);
        pause();
    }

    printf("[부모 PID:%d] 모든 실습을 정상 완료했습니다.\n", getpid());
    return 0;
}
