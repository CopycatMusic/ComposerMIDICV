#include "app/app.h"

#include "pico/stdlib.h"

int main(void) {
    stdio_init_all();
    app_init();
    app_run();
    return 0;
}
