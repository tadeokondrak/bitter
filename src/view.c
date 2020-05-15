#include "bitter.h"

uint32_t view_set_size(View *view, int width, int height) {
    return view->impl->set_size(view, width, height);
}

void view_for_each_surface(View *view, wlr_surface_iterator_func_t iter,
    void *data)
{
    view->impl->for_each_surface(view, iter, data);
}
