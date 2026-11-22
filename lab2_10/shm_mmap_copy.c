#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHM_NAME "/posix_file_copy_shm"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "사용법: %s <원본_파일> <대상_파일>\n", argv[0]);
        exit(1);
    }

    const char *src_path = argv[1];
    const char *dst_path = argv[2];

    // 1. 원본 파일 열기 및 크기 확인
    int src_fd = open(src_path, O_RDONLY);
    if (src_fd == -1) {
        perror("원본 파일 open 실패");
        exit(1);
    }

    struct stat st;
    if (fstat(src_fd, &st) == -1) {
        perror("fstat 실패");
        close(src_fd);
        exit(1);
    }

    size_t file_size = st.st_size;
    if (file_size == 0) {
        fprintf(stderr, "원본 파일이 비어 있습니다.\n");
        close(src_fd);
        exit(1);
    }

    // 2. POSIX 공유 메모리 객체 생성 및 크기 설정 (shm_open, ftruncate)
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open 실패");
        close(src_fd);
        exit(1);
    }
    if (ftruncate(shm_fd, file_size) == -1) {
        perror("ftruncate 실패");
        exit(1);
    }

    // 3. 원본 파일과 POSIX 공유 메모리를 각각 mmap으로 매핑
    void *src_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, src_fd, 0);
    void *shm_map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (src_map == MAP_FAILED || shm_map == MAP_FAILED) {
        perror("mmap 실패");
        exit(1);
    }

    // 부모 프로세스: 원본 파일 내용을 POSIX 공유 메모리에 복사
    memcpy(shm_map, src_map, file_size);
    printf("[부모 PID:%d] 원본 '%s' (%zu bytes) -> POSIX 공유 메모리 적재 완료\n",
           getpid(), src_path, file_size);

    munmap(src_map, file_size);
    close(src_fd);

    // 4. 자식 프로세스 생성 후 공유 메모리 내용을 대상 파일(mmap)로 복사
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 실패");
        exit(1);
    } else if (pid == 0) {
        int dst_fd = open(dst_path, O_CREAT | O_RDWR | O_TRUNC, 0644);
        if (dst_fd == -1) {
            perror("대상 파일 open 실패");
            exit(1);
        }
        if (ftruncate(dst_fd, file_size) == -1) {
            perror("대상 파일 ftruncate 실패");
            exit(1);
        }

        void *dst_map = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, dst_fd, 0);
        if (dst_map == MAP_FAILED) {
            perror("대상 파일 mmap 실패");
            exit(1);
        }

        // 공유 메모리 -> 대상 파일 매핑 영역으로 메모리 복사
        memcpy(dst_map, shm_map, file_size);
        msync(dst_map, file_size, MS_SYNC); // 디스크 동기화

        printf("  [자식 PID:%d] POSIX 공유 메모리 -> 대상 '%s' 파일 복사 완료\n",
               getpid(), dst_path);

        munmap(dst_map, file_size);
        munmap(shm_map, file_size);
        close(dst_fd);
        close(shm_fd);
        exit(0);
    } else {
        wait(NULL);
        munmap(shm_map, file_size);
        close(shm_fd);
        shm_unlink(SHM_NAME); // 공유 메모리 객체 제거
        printf("[부모 PID:%d] 공유 메모리 해제 및 파일 복사 작업 종료\n", getpid());
    }

    return 0;
}
