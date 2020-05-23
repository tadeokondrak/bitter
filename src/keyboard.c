#include "bitter.h"
#include <stdlib.h>
#include <stdint.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

Keyboard *keyboard_create(Server *srv, struct wlr_input_device *device) {
    Keyboard *kb = malloc(sizeof(Keyboard));

    struct xkb_rule_names rules = {
        .layout = "us",
        .variant = "colemak",
    };
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(device->keyboard, 50, 180);

    *kb = (Keyboard) {
        .srv = srv,
        .device = device,
        .on_key.notify = keyboard_on_key,
        .on_modifiers.notify = keyboard_on_modifiers,
        .on_destroy.notify = keyboard_on_destroy,
    };
    wl_list_insert(&srv->keyboards, &kb->link);
    wl_signal_add(&kb->device->keyboard->events.key, &kb->on_key);
    wl_signal_add(&kb->device->keyboard->events.modifiers, &kb->on_modifiers);
    wl_signal_add(&kb->device->keyboard->events.destroy, &kb->on_destroy);
    return kb;
}

void keyboard_key(Keyboard *kb, struct wlr_event_keyboard_key *event) {
    uint32_t keycode = event->keycode + 8;
    wlr_seat_set_keyboard(kb->srv->seat, kb->device);
    wlr_seat_keyboard_notify_key(kb->srv->seat, event->time_msec,
        event->keycode, event->state);
}

void keyboard_modifiers(Keyboard *kb, void *data) {
    wlr_seat_set_keyboard(kb->srv->seat, kb->device);
    wlr_seat_keyboard_notify_modifiers(kb->srv->seat,
        &kb->device->keyboard->modifiers);
}

void keyboard_destroy(Keyboard *kb, void *data) {
    wl_list_remove(&kb->on_key.link);
    wl_list_remove(&kb->on_modifiers.link);
    wl_list_remove(&kb->on_destroy.link);
    wl_list_remove(&kb->link);
    free(kb);
}
