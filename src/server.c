#include "bitter.h"
#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

Server *server_create(void) {
    struct Server *srv = malloc(sizeof(Server));
    struct wl_display *display = wl_display_create();
    struct wlr_backend *backend = wlr_backend_autocreate(display, NULL);
    struct wlr_renderer *renderer = wlr_backend_get_renderer(backend);
    struct wlr_output_layout *output_layout = wlr_output_layout_create();
    struct wlr_xdg_shell *xdg_shell = wlr_xdg_shell_create(display);
    struct wlr_cursor *cursor = wlr_cursor_create();
    struct wlr_xcursor_manager *xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    struct wlr_seat *seat = wlr_seat_create(display, "seat0");
    wlr_renderer_init_wl_display(renderer, display);
    wlr_cursor_attach_output_layout(cursor, output_layout);
    wlr_xcursor_manager_load(xcursor_manager, 1);
    wlr_compositor_create(display, renderer);
    wlr_data_device_manager_create(display);
    *srv = (Server) {
        .display = display,
        .backend = backend,
        .renderer = renderer,
        .output_layout = output_layout,
        .xdg_shell = xdg_shell,
        .cursor = cursor,
        .xcursor_manager = xcursor_manager,
        .seat = seat,
        .keyboards = {
            .prev = &srv->keyboards,
            .next = &srv->keyboards,
        },
        .outputs = {
            .prev = &srv->outputs,
            .next = &srv->outputs,
        },
        .on_new_input.notify = server_on_new_input,
        .on_new_output.notify = server_on_new_output,
        .on_new_xdg_surface.notify = server_on_new_xdg_surface,
        .on_cursor_motion.notify = server_on_cursor_motion,
        .on_cursor_button.notify = server_on_cursor_button,
    };
    wl_signal_add(&srv->backend->events.new_input, &srv->on_new_input);
    wl_signal_add(&srv->backend->events.new_output, &srv->on_new_output);
    wl_signal_add(&srv->xdg_shell->events.new_surface, &srv->on_new_xdg_surface);
    wl_signal_add(&srv->cursor->events.motion, &srv->on_cursor_motion);
    wl_signal_add(&srv->cursor->events.button, &srv->on_cursor_button);
    return srv;
}

bool server_run(Server *srv) {
    const char *socket = wl_display_add_socket_auto(srv->display);
    if (!socket)
        return false;
    if (!wlr_backend_start(srv->backend))
        return false;
    setenv("WAYLAND_DISPLAY", socket, true);
    wl_display_run(srv->display);
    return true;
}

void server_destroy(Server *srv) {
    wl_list_remove(&srv->on_new_input.link);
    wl_list_remove(&srv->on_new_output.link);
    wl_list_remove(&srv->on_new_xdg_surface.link);
    wl_list_remove(&srv->on_cursor_motion.link);
    wl_list_remove(&srv->on_cursor_button.link);
    wlr_xcursor_manager_destroy(srv->xcursor_manager);
    wlr_cursor_destroy(srv->cursor);
    wlr_output_layout_destroy(srv->output_layout);
    wl_display_destroy_clients(srv->display);
    wl_display_destroy(srv->display);
    free(srv);
}

void server_new_input(Server *srv, struct wlr_input_device *device) {
    switch (device->type) {
        case WLR_INPUT_DEVICE_KEYBOARD: {
            keyboard_create(srv, device);
        }
        case WLR_INPUT_DEVICE_POINTER: {
            wlr_cursor_attach_input_device(srv->cursor, device);
        }
        case WLR_INPUT_DEVICE_TOUCH: {
            // TODO
        }
        case WLR_INPUT_DEVICE_TABLET_TOOL: {
            // TODO
        }
        case WLR_INPUT_DEVICE_TABLET_PAD: {
            // TODO
        }
        case WLR_INPUT_DEVICE_SWITCH: {
            // TODO
        }
    }
    server_update_capabilities(srv);
}

void server_new_output(Server *srv, struct wlr_output *output) {
    Output *out = output_create(srv, output);
    srv->focused = out->root;
}

void server_new_xdg_surface(Server *srv, struct wlr_xdg_surface *surface) {
    XdgSurface *surf = xdg_surface_create(srv, surface);
    if (surf->surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
        view_set_tiled(&surf->base, true);
        node_insert(srv->focused, (View *)surf);
        server_reconfigure_outputs(srv);
    } else {
        assert(!"TODO: non-toplevel xdg surfaces");
    }
}

void server_cursor_motion(Server *srv, struct wlr_event_pointer_motion *event) {
    wlr_xcursor_manager_set_cursor_image(
        srv->xcursor_manager, "left_ptr", srv->cursor);
    wlr_cursor_move(srv->cursor, event->device, event->delta_x, event->delta_y);
    // TODO: send the event to the surface under the cursor
}

void server_cursor_button(Server *srv, struct wlr_event_pointer_button *event) {
    wlr_seat_pointer_notify_button(
        srv->seat, event->time_msec, event->button, event->state);
    // TODO: set pointer focus
}

void server_update_capabilities(Server *srv) {
    uint32_t capabilities = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&srv->keyboards))
        capabilities |= WL_SEAT_CAPABILITY_KEYBOARD;
    wlr_seat_set_capabilities(srv->seat, capabilities);
}

void server_reconfigure_outputs(Server *srv) {
    Output *out;
    wl_list_for_each (out, &srv->outputs, link) {
        output_configure(out);
    }
}
