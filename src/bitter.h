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

typedef struct Server Server;
typedef struct InputDevice InputDevice;
typedef struct Keyboard Keyboard;
typedef struct Pointer Pointer;
typedef struct Output Output;
typedef enum ViewKind ViewKind;
typedef struct View View;
typedef struct ViewImpl ViewImpl;
typedef struct XdgSurface XdgSurface;
typedef enum NodeKind NodeKind;
typedef struct Node Node;

struct Server {
    struct wl_display *display;
    struct wlr_backend *backend;
    struct wlr_renderer *renderer;
    struct wlr_output_layout *output_layout;
    struct wlr_xdg_shell *xdg_shell;
    struct wl_list input_devices;
    struct wl_list outputs;
    struct wl_listener on_new_input;
    struct wl_listener on_new_output;
    struct wl_listener on_new_xdg_surface;
    Node *focused;
    // TODO: make a linked list of last focused nodes
};

Server *server_create(void);
bool server_run(Server *);
void server_destroy(Server *);
void server_new_input(Server *, struct wlr_input_device *);
void server_new_output(Server *, struct wlr_output *);
void server_new_xdg_surface(Server *, struct wlr_xdg_surface *);

NOTIFY(Server, server, new_input)
NOTIFY(Server, server, new_output)
NOTIFY(Server, server, new_xdg_surface)

struct InputDevice {
    Server *srv;
    struct wlr_input_device *device;
    struct wl_list link;
};

InputDevice *input_device_create(Server *, struct wlr_input_device *);
void input_device_destroy(InputDevice *);

struct Keyboard {
    struct wlr_keyboard *keyboard;
};

Keyboard *keyboard_create(Server *, struct wlr_keyboard *);
void keyboard_destroy(Keyboard *);

struct Pointer {
    struct wlr_pointer *pointer;
};

Pointer *pointer_create(Server *, struct wlr_pointer *);
void pointer_destroy(Pointer *);

struct Output {
    Server *srv;
    Node *root;
    struct wlr_output *output;
    struct wl_listener on_frame;
    struct wl_list link;
};

Output *output_create(Server *, struct wlr_output *);
void output_destroy(Output *);
void output_frame(Output *, void *data);
void output_render(Output *, struct timespec *when, struct wlr_box *where);

NOTIFY(Output, output, frame)

enum ViewKind {
    ViewXdgSurface,
};

struct View {
    Server *srv;
    ViewKind kind;
    ViewImpl *impl;
    union {
        struct {
            struct wl_list link;
        } tiled;
    };
};

struct ViewImpl {
    uint32_t (*set_size)(View *, int width, int height);
    void (*for_each_surface)(View *, wlr_surface_iterator_func_t, void *data);
};

uint32_t view_set_size(View *, int width, int height);
void view_for_each_surface(View *, wlr_surface_iterator_func_t, void *data);

struct XdgSurface {
    View base;
    struct wlr_xdg_surface *surface;
    struct wl_listener on_destroy;
};

XdgSurface *xdg_surface_create(Server *, struct wlr_xdg_surface *);
XdgSurface *xdg_surface_from_view(View *);
void xdg_surface_destroy(XdgSurface *, void *);

NOTIFY(XdgSurface, xdg_surface, destroy)

enum NodeKind {
    NodeHorizontal,
    NodeVertical,
    NodeTerminalHorizontal,
    NodeTerminalVertical,
};

struct Node {
    NodeKind kind;
    union {
        struct {
            Node *left;
            Node *right;
        };
        struct wl_list children;
    };
};

Node *node_create(void);
void node_destroy(Node *);
Node *node_insert(Node *, View *);
void node_walk(Node *,
    void (*visit)(struct wl_list *link, struct wlr_box *box, void *data),
    struct wlr_box *box, void *data);
