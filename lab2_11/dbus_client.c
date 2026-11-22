#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

int main(int argc, char *argv[]) {
    const char *title = (argc > 1) ? argv[1] : "시스템 알림";
    const char *body  = (argc > 2) ? argv[2] : "D-BUS를 통해 전달된 테스트 알림 메시지입니다.";

    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus_message *reply = NULL;
    sd_bus *bus = NULL;
    const char *response_msg = NULL;
    int r;

    // 1. 사용자 세션 버스에 연결
    r = sd_bus_open_user(&bus);
    if (r < 0) {
        fprintf(stderr, "세션 버스 연결 실패: %s\n", strerror(-r));
        goto finish;
    }

    // 2. 서버의 SendAlert 메소드 호출
    printf("[클라이언트] D-BUS 서버로 알림 전송 중...\n");
    r = sd_bus_call_method(bus,
                           "org.example.NotifyService", // 대상 서비스 이름
                           "/org/example/Notify",       // 대상 객체 경로
                           "org.example.Notify",        // 인터페이스 이름
                           "SendAlert",                 // 호출할 메소드 이름
                           &error,
                           &reply,
                           "ss",                        // 입력 인자 타입 (문자열 2개)
                           title, body);
    if (r < 0) {
        fprintf(stderr, "메소드 호출 실패: %s\n", error.message);
        goto finish;
    }

    // 3. 서버로부터 돌아온 응답 메시지 읽기
    r = sd_bus_message_read(reply, "s", &response_msg);
    if (r < 0) {
        fprintf(stderr, "응답 파싱 실패: %s\n", strerror(-r));
        goto finish;
    }

    printf("[클라이언트] 서버 응답: %s\n", response_msg);

finish:
    sd_bus_error_free(&error);
    sd_bus_message_unref(reply);
    sd_bus_unref(bus);
    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
