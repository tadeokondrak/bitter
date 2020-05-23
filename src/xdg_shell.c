#include "bitter.h"
#include <stdlib.h>
#include <assert.h>
#include <wlr/types/wlr_seat.h>

static ViewImpl xdg_surface_impl;

XdgSurface *xdg_surface_create(Server *srv, struct wlr_xdg_surface *surface) {
    XdgSurface *surf = malloc(sizeof(XdgSurface));
    *surf = (XdgSurface) {
        .base = (View) {
            .srv = srv,
            .kind = ViewXdgSurface,
            .impl = &xdg_surface_impl,
        },
        .surface = surface,
        .on_destroy.notify = xdg_surface_on_destroy,
    };
    wl_signal_add(&surf->surface->events.destroy, &surf->on_destroy);
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(srv->seat);
    wlr_seat_keyboard_notify_enter(srv->seat, surface->surface, keyboard->keycodes,
        keyboard->num_keycodes, &keyboard->modifiers);
    return surf;
}

XdgSurface *xdg_surface_from_view(View *view) {
    assert(view->kind == ViewXdgSurface);
    return (XdgSurface *)view;
}

void xdg_surface_destroy(XdgSurface *surf, void *data) {
    wl_list_remove(&surf->base.tiled.link);
    free(surf);
}

static uint32_t set_size_impl(View *view, int width, int height) {
    return wlr_xdg_toplevel_set_size(
        xdg_surface_from_view(view)->surface, width, height);
}

static uint32_t set_tiled_impl(View *view, bool tiled) {
    return wlr_xdg_toplevel_set_tiled(
        xdg_surface_from_view(view)->surface, tiled);
}
    
static void for_each_surface_impl(View *view,
    wlr_surface_iterator_func_t iter, void *data)
{
    wlr_xdg_surface_for_each_surface(
        xdg_surface_from_view(view)->surface, iter, data);
}

static ViewImpl xdg_surface_impl = {
    .set_size = set_size_impl,
    .set_tiled = set_tiled_impl,
    .for_each_surface = for_each_surface_impl,
};
