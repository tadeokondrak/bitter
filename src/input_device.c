#include "bitter.h"
#include <stdlib.h>
#include <wlr/interfaces/wlr_input_device.h>

InputDevice *input_device_create(Server *srv, struct wlr_input_device *device) {
    InputDevice *dev = malloc(sizeof(InputDevice));
    *dev = (InputDevice) {
        .srv = srv,
        .device = device,
    };
    wl_list_insert(&srv->input_devices, &dev->link);
    return dev;
}

void input_device_destroy(InputDevice *dev) {
    free(dev);
}
