#include "bitter.h"
#include <assert.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

Server *server_create(void) {
    struct Server *srv = malloc(sizeof(Server));
    struct wl_display *display = wl_display_create();
    struct wlr_backend *backend = wlr_backend_autocreate(display, NULL);
    struct wlr_renderer *renderer = wlr_backend_get_renderer(backend);
    struct wlr_output_layout *output_layout = wlr_output_layout_create();
    struct wlr_xdg_shell *xdg_shell = wlr_xdg_shell_create(display);
    wlr_renderer_init_wl_display(renderer, display);
    wlr_compositor_create(display, renderer);
    wlr_data_device_manager_create(display);
    *srv = (Server) {
        .display = display,
        .backend = backend,
        .renderer = renderer,
        .output_layout = output_layout,
        .xdg_shell = xdg_shell,
        .input_devices = {
            .prev = &srv->input_devices,
            .next = &srv->input_devices,
        },
        .outputs = {
            .prev = &srv->outputs,
            .next = &srv->outputs,
        },
        .on_new_input.notify = server_on_new_input,
        .on_new_output.notify = server_on_new_output,
        .on_new_xdg_surface.notify = server_on_new_xdg_surface,
    };
    wl_signal_add(&srv->backend->events.new_input, &srv->on_new_input);
    wl_signal_add(&srv->backend->events.new_output, &srv->on_new_output);
    wl_signal_add(&srv->xdg_shell->events.new_surface, &srv->on_new_xdg_surface);
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
    wlr_output_layout_destroy(srv->output_layout);
    wl_display_destroy_clients(srv->display);
    wl_display_destroy(srv->display);
    free(srv);
}

void server_new_input(Server *srv, struct wlr_input_device *device) {
    input_device_create(srv, device);
}

void server_new_output(Server *srv, struct wlr_output *output) {
    Output *out = output_create(srv, output);
    srv->focused = out->root;
}

void server_new_xdg_surface(Server *srv, struct wlr_xdg_surface *surface) {
    XdgSurface *surf = xdg_surface_create(srv, surface);
    if (surf->surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL)
        node_insert(srv->focused, (View *)surf);
    else
        assert(!"TODO: non-toplevel xdg surfaces");
}
