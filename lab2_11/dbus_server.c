#include <stdio.h>
#include <stdlib.h>
#include <systemd/sd-bus.h>

// 클라이언트가 SendAlert 메소드를 호출할 때 실행되는 콜백 함수
static int method_send_alert(sd_bus_message *m, void *userdata, sd_bus_error *ret_error) {
    const char *title = NULL;
    const char *body = NULL;

    // 클라이언트로부터 두 개의 문자열(제목, 내용) 읽기
    int r = sd_bus_message_read(m, "ss", &title, &body);
    if (r < 0) {
        fprintf(stderr, "메시지 파싱 실패: %s\n", strerror(-r));
        return r;
    }

    printf("\n========================================\n");
    printf("[D-BUS 알림 수신] 제목: %s\n", title);
    printf("[D-BUS 알림 내용] %s\n", body);
    printf("========================================\n");
    fflush(stdout);

    // 클라이언트에게 응답 문자열 반환
    return sd_bus_reply_method_return(m, "s", "알림이 서버에 정상 전달되었습니다.");
}

// D-BUS 인터페이스 vtable 정의
static const sd_bus_vtable notify_vtable[] = {
    SD_BUS_VTABLE_START(0),
    SD_BUS_METHOD("SendAlert", "ss", "s", method_send_alert, SD_BUS_VTABLE_UNPRIVILEGED),
    SD_BUS_VTABLE_END
};

int main(void) {
    sd_bus_slot *slot = NULL;
    sd_bus *bus = NULL;
    int r;

    // 1. 사용자 세션 버스에 연결
    r = sd_bus_open_user(&bus);
    if (r < 0) {
        fprintf(stderr, "세션 버스 연결 실패: %s\n", strerror(-r));
        goto finish;
    }

    // 2. 객체 경로 및 인터페이스 등록
    r = sd_bus_add_object_vtable(bus, &slot,
                                 "/org/example/Notify",
                                 "org.example.Notify",
                                 notify_vtable,
                                 NULL);
    if (r < 0) {
        fprintf(stderr, "인터페이스 등록 실패: %s\n", strerror(-r));
        goto finish;
    }

    // 3. 잘 알려진(Well-known) 서비스 이름 요청
    r = sd_bus_request_name(bus, "org.example.NotifyService", 0);
    if (r < 0) {
        fprintf(stderr, "서비스 이름 획득 실패: %s\n", strerror(-r));
        goto finish;
    }

    printf("=== D-BUS 알림 서버 실행 중 (org.example.NotifyService) ===\n");
    printf("클라이언트의 알림 메시지를 대기하고 있습니다...\n");

    // 4. 버스 이벤트 루프
    while (1) {
        r = sd_bus_process(bus, NULL);
        if (r < 0) {
            fprintf(stderr, "버스 처리 오류: %s\n", strerror(-r));
            goto finish;
        }
        if (r > 0) continue;

        r = sd_bus_wait(bus, (uint64_t)-1);
        if (r < 0) {
            fprintf(stderr, "버스 대기 오류: %s\n", strerror(-r));
            goto finish;
        }
    }

finish:
    sd_bus_slot_unref(slot);
    sd_bus_unref(bus);
    return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}
