#include "bitter.h"
#include <stdarg.h>
#include <stdio.h>
#include <wlr/util/log.h>

static void log_callback(enum wlr_log_importance verbosity, const char *fmt,
    va_list args)
{
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

int main(void) {
    wlr_log_init(WLR_DEBUG, log_callback);
    Server *server = server_create();
    server_run(server);
    server_destroy(server);
}
