#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

void list_dir_recursive(const char *base_path) {
    DIR *dp = opendir(base_path);
    if (dp == NULL) {
        perror(base_path);
        return;
    }

    struct dirent *entry;
    struct stat statbuf;
    char path[1024];

    // 1. 현재 디렉토리 경로 출력 (ls -R 형식)
    printf("%s:\n", base_path);

    // 2. 현재 디렉토리 내의 모든 항목 먼저 출력
    while ((entry = readdir(dp)) != NULL) {
        // 숨김 파일(. 및 .. 포함) 제외 여부: ls -R 기본 동작과 동일하게 '.'으로 시작하는 항목 건너뜀
        if (entry->d_name[0] == '.') {
            continue;
        }
        printf("%s  ", entry->d_name);
    }
    printf("\n\n");

    // 3. 디렉토리 스트림을 처음으로 되감아 하위 디렉토리 재귀 방문
    rewinddir(dp);
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') {
            continue;
        }

        snprintf(path, sizeof(path), "%s/%s", base_path, entry->d_name);

        // 심볼릭 링크 무한 루프 방지를 위해 lstat 사용
        if (lstat(path, &statbuf) == 0 && S_ISDIR(statbuf.st_mode)) {
            list_dir_recursive(path);
        }
    }

    closedir(dp);
}

int main(int argc, char *argv[]) {
    // 인자가 주어지지 않으면 현재 디렉토리(".")를 기준으로 탐색
    const char *target_dir = (argc > 1) ? argv[1] : ".";
    list_dir_recursive(target_dir);
    return 0;
}
