#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

int main() {
    const char *dir_name = "test_dir";
    const char *file_path = "test_dir/sample.txt";
    const char *write_data = "Hello, Linux File and Directory System Calls!\n";
    char read_buf[128] = {0};

    // 1. 디렉토리 생성 (mkdir)
    if (mkdir(dir_name, 0755) == -1) {
        perror("mkdir (이미 존재할 수 있음)");
    } else {
        printf("[1] 디렉토리 '%s' 생성 완료\n", dir_name);
    }

    // 2. 파일 열기 및 쓰기 (open, write, close)
    int fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open write 실패");
        exit(1);
    }
    write(fd, write_data, strlen(write_data));
    close(fd);
    printf("[2] 파일 '%s' 생성 및 데이터 쓰기 완료\n", file_path);

    // 3. 파일 읽기 (open, read, close)
    fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        perror("open read 실패");
        exit(1);
    }
    ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf) - 1);
    close(fd);
    printf("[3] 파일에서 읽은 내용 (%zd bytes): %s", bytes_read, read_buf);

    // 4. 파일 상태 정보 확인 (stat)
    struct stat file_stat;
    if (stat(file_path, &file_stat) == 0) {
        printf("[4] 파일 정보(stat) -> 크기: %ld bytes, 권한(8진수): %o\n",
               file_stat.st_size, file_stat.st_mode & 0777);
    }

    // 5. 디렉토리 내부 항목 조회 (opendir, readdir, closedir)
    DIR *dp = opendir(dir_name);
    if (dp != NULL) {
        struct dirent *entry;
        printf("[5] '%s' 디렉토리 내부 목록:\n", dir_name);
        while ((entry = readdir(dp)) != NULL) {
            printf("    - %s (inode: %lu)\n", entry->d_name, entry->d_ino);
        }
        closedir(dp);
    }

    return 0;
}
