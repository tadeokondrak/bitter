#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <wayland-server-core.h>
#include <wlr/interfaces/wlr_input_device.h>
#include <wlr/interfaces/wlr_output.h>
#include <wlr/types/wlr_xdg_shell.h>

#define NOTIFY(type, prefix, name)                                         \
    __attribute__((__flatten__)) __attribute__((__unused__)) static inline \
    void prefix##_on_##name(struct wl_listener *listener, void *data) {    \
        type *ptr = wl_container_of(listener, ptr, on_##name);             \
        prefix##_##name(ptr, data);                                        \
    }

typedef struct Server {
    struct wl_display *display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_output_layout *output_layout;
    struct wlr_xdg_shell *xdg_shell;
    struct wlr_cursor *cursor;
    struct wlr_seat *seat;
    struct wl_list keyboards;
    struct wl_list outputs;
    struct wl_listener on_new_input;
    struct wl_listener on_new_output;
    struct wl_listener on_new_xdg_surface;
    struct Node *focused;
    // TODO: make a linked list of last focused nodes
} Server;

Server *server_create(void);
bool server_run(Server *);
void server_destroy(Server *);
void server_new_input(Server *, struct wlr_input_device *);
void server_new_output(Server *, struct wlr_output *);
void server_new_xdg_surface(Server *, struct wlr_xdg_surface *);
void server_update_capabilities(Server *);

NOTIFY(Server, server, new_input)
NOTIFY(Server, server, new_output)
NOTIFY(Server, server, new_xdg_surface)

typedef struct Keyboard {
    Server *srv;
    struct wlr_input_device *device;
    struct wl_listener on_key;
    struct wl_listener on_destroy;
    struct wl_listener on_modifiers;
    struct wl_list link;
} Keyboard;

Keyboard *keyboard_create(Server *, struct wlr_input_device *);
void keyboard_key(Keyboard *, struct wlr_event_keyboard_key *);
void keyboard_destroy(Keyboard *, void *);
void keyboard_modifiers(Keyboard *, void *);

NOTIFY(Keyboard, keyboard, destroy)
NOTIFY(Keyboard, keyboard, key)
NOTIFY(Keyboard, keyboard, modifiers)

typedef struct Output {
    Server *srv;
    struct Node *root;
    struct wlr_output *output;
    struct wl_listener on_frame;
    struct wl_list link;
} Output;

Output *output_create(Server *, struct wlr_output *);
void output_destroy(Output *);
void output_frame(Output *, void *data);
void output_render(Output *, struct timespec *when, struct wlr_box *where);

NOTIFY(Output, output, frame)

typedef enum ViewKind {
    ViewXdgSurface,
} ViewKind;

typedef struct View {
    Server *srv;
    ViewKind kind;
    struct ViewImpl *impl;
    union {
        struct {
            struct wl_list link;
        } tiled;
    };
} View;

typedef struct ViewImpl {
    uint32_t (*set_size)(View *, int width, int height);
    void (*for_each_surface)(View *, wlr_surface_iterator_func_t, void *data);
} ViewImpl;

uint32_t view_set_size(View *, int width, int height);
void view_for_each_surface(View *, wlr_surface_iterator_func_t, void *data);

typedef struct XdgSurface {
    View base;
    struct wlr_xdg_surface *surface;
    struct wl_listener on_destroy;
} XdgSurface;

XdgSurface *xdg_surface_create(Server *, struct wlr_xdg_surface *);
XdgSurface *xdg_surface_from_view(View *);
void xdg_surface_destroy(XdgSurface *, void *);

NOTIFY(XdgSurface, xdg_surface, destroy)

typedef enum NodeKind {
    NodeHorizontal,
    NodeVertical,
    NodeTerminalHorizontal,
    NodeTerminalVertical,
} NodeKind;

typedef struct Node {
    NodeKind kind;
    union {
        struct {
            struct Node *left;
            struct Node *right;
        };
        struct wl_list children;
    };
} Node;

Node *node_create(void);
void node_destroy(Node *);
Node *node_insert(Node *, View *);
void node_walk(Node *,
    void (*visit)(struct wl_list *link, struct wlr_box *box, void *data),
    struct wlr_box *box, void *data);
