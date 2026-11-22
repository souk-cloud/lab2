#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>

// 터미널을 한 글자씩 즉시 입력받는(비정규) 모드로 전환하는 함수
int getch_custom(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr(STDIN_FILENO, &oldattr);
    newattr = oldattr;
    newattr.c_lflag &= ~(ICANON | ECHO); // 버퍼링(엔터 대기) 및 자동 에코 끄기
    tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
    return ch;
}

int main() {
    const char *sentences[] = {
        "Linux is a free and open-source operating system.",
        "System programming requires understanding processes and files.",
        "Practice makes perfect in C and Rust programming."
    };
    int num_sentences = sizeof(sentences) / sizeof(sentences[0]);

    int total_errors = 0;
    int total_chars = 0;
    struct timeval start_time, end_time;

    printf("=== 리눅스 터미널 타자 연습 프로그램 ===\n");
    printf("제시되는 %d개의 문장을 정확히 입력하세요. (백스페이스 불가, 오타 시 다음 글자로 진행)\n\n", num_sentences);

    // 전체 시작 시간 기록
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < num_sentences; i++) {
        const char *target = sentences[i];
        int len = strlen(target);
        total_chars += len;

        printf("[문장 %d/%d]\n", i + 1, num_sentences);
        printf("제시문: %s\n", target);
        printf("입력란: ");
        fflush(stdout);

        for (int j = 0; j < len; j++) {
            char ch = (char)getch_custom();
            // 엔터(\n)가 실수로 눌린 경우 무시
            if (ch == '\n' || ch == '\r') {
                j--;
                continue;
            }

            if (ch == target[j]) {
                printf("%c", ch); // 맞으면 그대로 출력
            } else {
                printf("\033[1;31m%c\033[0m", ch); // 틀리면 빨간색으로 표시
                total_errors++;
            }
            fflush(stdout);
        }
        printf("\n\n");
    }

    // 전체 종료 시간 기록
    gettimeofday(&end_time, NULL);

    double elapsed_sec = (end_time.tv_sec - start_time.tv_sec) +
                         (end_time.tv_usec - start_time.tv_usec) / 1000000.0;
    double elapsed_min = elapsed_sec / 60.0;

    // 분당 타자수(CPM: Characters Per Minute) 및 분당 단어수(WPM: 5타=1단어 기준) 계산
    double cpm = (elapsed_min > 0) ? (total_chars / elapsed_min) : 0.0;
    double wpm = cpm / 5.0;

    printf("================ [측정 결과] ================\n");
    printf("총 입력 글자 수 : %d 자\n", total_chars);
    printf("잘못 타이핑 횟수: %d 회\n", total_errors);
    printf("총 소요 시간    : %.2f 초\n", elapsed_sec);
    printf("평균 분당 타자수: %.1f 타/분 (CPM) / %.1f WPM\n", cpm, wpm);
    printf("=============================================\n");

    return 0;
}
