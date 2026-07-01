#include "debug/debug_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "tusb.h"

void debug_log(const char* message) {
    if (!tud_cdc_connected()) {
        return;
    }
    tud_cdc_write_str(message);
    tud_cdc_write_str("\r\n");
    tud_cdc_write_flush();
}

void debug_logf(const char* format, ...) {
    if (!tud_cdc_connected()) {
        return;
    }

    char buffer[160];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    tud_cdc_write(buffer, strlen(buffer));
    tud_cdc_write_str("\r\n");
    tud_cdc_write_flush();
}
