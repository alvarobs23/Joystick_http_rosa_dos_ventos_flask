#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "example_http_client_util.h"

#define HOST         "192.168.18.8"
#define PORT         5000
#define INTERVALO_MS 200

#define JOY_X_PIN    26   // ADC0
#define JOY_Y_PIN    27   // ADC1

void adc_setup() {
    adc_init();
    adc_gpio_init(JOY_X_PIN);
    adc_gpio_init(JOY_Y_PIN);
}

const char* get_direction(int raw_x, int raw_y) {
    // Normaliza de 0–4095 para -1.0 a +1.0
    float x = (raw_x / 4095.0f) * 2.0f - 1.0f;
    float y = (raw_y / 4095.0f) * 2.0f - 1.0f;

    // Se joystick em zona morta, retorna NULL
    if (fabs(x) < 0.1f && fabs(y) < 0.1f) {
        return NULL;
    }

    // Ângulo em graus, com 0° apontando para Norte
    float ang = atan2f(x, y) * 180.0f / M_PI;  
    if (ang < 0) ang += 360.0f;

    // Divide o círculo em 8 setores de 45°
    if      (ang < 22.5f || ang >= 337.5f) return "Norte";
    else if (ang < 67.5f)                  return "Nordeste";
    else if (ang < 112.5f)                 return "Leste";
    else if (ang < 157.5f)                 return "Sudeste";
    else if (ang < 202.5f)                 return "Sul";
    else if (ang < 247.5f)                 return "Sudoeste";
    else if (ang < 292.5f)                 return "Oeste";
    else                                   return "Noroeste";
}

int main() {
    stdio_init_all();
    adc_setup();

    if (cyw43_arch_init()) return 1;
    cyw43_arch_enable_sta_mode();
    cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                       CYW43_AUTH_WPA2_AES_PSK, 30000);

    printf("Cliente iniciado, enviando direção...\n");

    while (1) {
        uint16_t raw_x, raw_y;
        adc_select_input(0);
        raw_x = adc_read();
        adc_select_input(1);
        raw_y = adc_read();

        const char *dir = get_direction(raw_x, raw_y);
        if (dir) {
            char path[64];
            snprintf(path, sizeof(path), "/compass?dir=%s", dir);

            EXAMPLE_HTTP_REQUEST_T req = {0};
            req.hostname   = HOST;
            req.url        = path;
            req.port       = PORT;
            req.headers_fn = http_client_header_print_fn;
            req.recv_fn    = http_client_receive_print_fn;

            printf("Enviando: %s\n", path);
            http_client_request_sync(cyw43_arch_async_context(), &req);
        }
        sleep_ms(INTERVALO_MS);
    }
    return 0;
}
