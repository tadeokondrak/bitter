#include "bitter.h"
#include <stdlib.h>

Node *node_create(void) {
    Node *n = malloc(sizeof(Node));
    *n = (Node) {
        .kind = NodeTerminalVertical,
    };
    wl_list_init(&n->children);
    return n;
}

void node_destroy(Node *n) {
    free(n);
}

Node *node_insert(Node *n, View *view) {
    switch (n->kind) {
        case NodeHorizontal:
        case NodeVertical: {
            return node_insert(n->right, view);
        }
        case NodeTerminalHorizontal:
        case NodeTerminalVertical: {
            wl_list_insert(&n->children, &view->tiled.link); // TODO
            return n;
        }
    }
}

void node_walk(Node *n,
    void (*visit)(struct wl_list *link, struct wlr_box *box, void *data),
    struct wlr_box *box, void *data)
{
    switch (n->kind) {
        case NodeHorizontal: {
            struct wlr_box child_box = {
                .x = box->x,
                .y = box->y,
                .width = box->width / 2,
                .height = box->height,
            };
            node_walk(n->left, visit, &child_box, data);
            child_box.x += child_box.width;
            node_walk(n->right, visit, &child_box, data);
            break;
        }
        case NodeVertical: {
            struct wlr_box child_box = {
                .x = box->x,
                .y = box->y,
                .width = box->width,
                .height = box->height / 2,
            };
            node_walk(n->left, visit, &child_box, data);
            child_box.y += child_box.height;
            node_walk(n->left, visit, &child_box, data);
            break;
        }
        case NodeTerminalHorizontal: {
            int len = wl_list_length(&n->children);
            if (len == 0)
                return;
            struct wlr_box child_box = {
                .x = box->x,
                .y = box->y,
                .width = box->width / len,
                .height = box->height,
            };
            for (struct wl_list *pos = n->children.next;
                pos != &n->children; pos = pos->next)
            {
                visit(pos, &child_box, data);
                child_box.x += child_box.width;
            }
            break;
        }
        case NodeTerminalVertical: {
            int len = wl_list_length(&n->children);
            if (len == 0)
                return;
            struct wlr_box child_box = {
                .x = box->x,
                .y = box->y,
                .width = box->width,
                .height = box->height / len,
            };
            for (struct wl_list *pos = n->children.next;
                pos != &n->children; pos = pos->next)
            {
                visit(pos, &child_box, data);
                child_box.y += child_box.height;
            }
            break;
        }
    }
}
