#include "bitter.h"
#include <stdlib.h>
#include <time.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_matrix.h>

static void render_view(struct wl_list *link, struct wlr_box *box, void *data);
static void render_surface(struct wlr_surface *surface, int sx, int sy, void *data);

Output *output_create(Server *srv, struct wlr_output *output) {
    Output *out = malloc(sizeof(Output));
    *out = (Output) {
        .srv = srv,
        .root = node_create(),
        .output = output,
        .on_frame.notify = output_on_frame,
    };
    wl_signal_add(&out->output->events.frame, &out->on_frame);
    wl_list_insert(&srv->outputs, &out->link);
    wlr_output_layout_add_auto(srv->output_layout, out->output);
    return out;
}

void output_destroy(Output *out) {
    node_destroy(out->root);
    wlr_output_layout_remove(out->srv->output_layout, out->output);
    wl_list_remove(&out->link);
    free(out);
}

typedef struct ViewRenderData ViewRenderData;
struct ViewRenderData {
    Output *out;
    struct timespec *when;
};

void output_frame(Output *out, void *data) {
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    wlr_output_attach_render(out->output, NULL);
    int width, height;
    wlr_output_effective_resolution(out->output, &width, &height);
    wlr_renderer_begin(out->srv->renderer, width, height);
    wlr_renderer_clear(out->srv->renderer, (float[4]){0.0f, 0.5f, 0.5f, 1.0f});
    ViewRenderData vdata = {
        .out = out,
        .when = &now,
    };
    struct wlr_box there = {
        .x = 0,
        .y = 0,
        .width = out->output->width,
        .height = out->output->height,
    };
    node_walk(out->root, render_view, &there, &vdata);
    wlr_output_render_software_cursors(out->output, NULL);
    wlr_renderer_end(out->srv->renderer);
    wlr_output_commit(out->output);
}

typedef struct SurfaceRenderData SurfaceRenderData;
struct SurfaceRenderData {
    View *view;
    Output *out;
    struct wlr_box *where;
    struct timespec *when;
};

static int int_min(int a, int b) {
    return a < b ? a : b;
}

static void render_view(struct wl_list *link, struct wlr_box *box, void *data) {
    View *view = wl_container_of(link, view, tiled.link); // TODO
    XdgSurface *xdg_surface = (XdgSurface *)view;
    ViewRenderData *vdata = data;
    SurfaceRenderData sdata = {
        .view = view,
        .out = vdata->out,
        .where = &(struct wlr_box) {
            // TODO: this seems like a hack, but it makes alacritty's csd work in tiling.
            // hopefully it actually works everywhere.
            .x = box->x - int_min(0, xdg_surface->surface->geometry.x),
            .y = box->y - int_min(0, xdg_surface->surface->geometry.y),
            .width = box->width,
            .height = box->height,
        },
        .when = vdata->when,
    };
    view_for_each_surface(view, render_surface, &sdata);
}

static void render_surface(
    struct wlr_surface *surface, int sx, int sy, void *data)
{
    SurfaceRenderData *sdata = data;
    struct wlr_texture *texture = wlr_surface_get_texture(surface);
    if (!texture)
        return;

    double ox = 0, oy = 0;
    wlr_output_layout_output_coords(sdata->view->srv->output_layout,
        sdata->out->output, &ox, &oy);

    ox += sdata->where->x + sx;
    oy += sdata->where->y + sy;

    struct wlr_box box = {
        .x = ox,
        .y = oy,
        .width = surface->current.width,
        .height = surface->current.height,
    };

    float matrix[9];
    enum wl_output_transform transform =
        wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(matrix, &box, transform, 0,
        sdata->out->output->transform_matrix);
    wlr_render_texture_with_matrix(
        sdata->out->srv->renderer, texture, matrix, 1);

    wlr_surface_send_frame_done(surface, sdata->when);
}

void output_configure(Output *out) {
    struct wlr_box *output_box = wlr_output_layout_get_box(out->srv->output_layout, out->output);
    node_configure(out->root, output_box);
}
