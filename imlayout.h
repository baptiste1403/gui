#ifndef __IMLAYOUT__
#define __IMLAYOUT__

#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#define ARENA_IMPLEMENTATION

const size_t FACTOR = 40;

typedef enum {
    HORIZONTAL_LAYOUT = 1 << 0,
    VERTICAL_LAYOUT = 1 << 1,
    FIXED_LAST = 1 << 2,
    FIXED_FIRST = 1 << 3
} LayoutOption;

typedef enum {
    LINEAR_LAYOUT,
    FIXED_LAYOUT
} LayoutKind;

typedef union {
    const size_t* grows;
    size_t fixed_size;
} LayoutParameter;

typedef struct {
    LayoutKind kind;
    int option;
    Rectangle rect;
    size_t count;
    LayoutParameter parameter;
    size_t num_element;
} Layout;

typedef struct {
    Layout* items;
    size_t count;
    size_t capacity;
    long unsigned int focused;
} LayoutStack;

void begin_ui();
void end_ui();
void close_ui();

Layout make_layout(LayoutKind kind, int option, Rectangle rect, size_t count);

#define linear_layout(option, rect, count, grows) linear_layout_((option), (rect), (count), (grows), __FILE__, __LINE__)
void linear_layout_(int option, Rectangle rect, size_t count, size_t* grows, const char* file, int line);

#define fixed_layout(option, rect, fixed_size) fixed_layout_((option), (rect), (fixed_size), __FILE__, __LINE__)
void fixed_layout_(int option, Rectangle rect, size_t fixed_size, const char* file, int line);

void layout_stack_push_(LayoutKind kind, int option, Rectangle rect, size_t count, LayoutParameter parameter, const char* file, int line);
void layout_stack_pop_(const char* file, int line);
Rectangle layout_slot_(const char* file, int line);
Rectangle layout_slot_with_padding_(size_t padding, const char* file, int line);
bool is_hover(Rectangle rect);

#ifdef IMLAYOUT_IMPLEMENTATION

static LayoutStack stack = {0};

Layout make_layout(LayoutKind kind, int option, Rectangle rect, size_t count) {
    Layout res = {0};
    res.kind = kind;
    res.option = option;
    res.rect = rect;
    res.count = count;
    res.num_element = 0;
    return res;
}

void linear_layout_(int option, Rectangle rect, size_t count, size_t* grows, const char* file, int line) {
    LayoutParameter parameter;
    parameter.grows = grows;
    layout_stack_push_(LINEAR_LAYOUT, option, rect, count, parameter, file, line);
}

void fixed_layout_(int option, Rectangle rect, size_t fixed_size, const char* file, int line) {
    LayoutParameter parameter;
    parameter.fixed_size = fixed_size;
    layout_stack_push_(FIXED_LAYOUT, option, rect, 2, parameter, file, line);
}

void begin_ui() { }

void end_ui() {
    assert(stack.count == 0);
}

void close_ui() {
    free(stack.items);
}
#define DA_INIT_CAPACITY 10
#define da_append(array, item) do { \
    if((array)->count >= (array)->capacity) { \
        if((array)->capacity == 0) { \
            (array)->items = malloc(DA_INIT_CAPACITY*sizeof((array)->items[0])); \
            (array)->capacity = DA_INIT_CAPACITY; \
        } else {\
            (array)->items = realloc((array)->items, (array)->capacity*2*sizeof((array)->items[0])); \
            (array)->capacity *= 2; \
        } \
    } \
    (array)->items[(array)->count] = (item); \
    (array)->count++; \
} while(0) \

void layout_stack_push_(LayoutKind kind, int option, Rectangle rect, size_t count, LayoutParameter parameter, const char* file, int line) {
    Layout l = make_layout(kind, option, rect, count);
    switch(l.kind) {
        case LINEAR_LAYOUT:
            if (parameter.grows != NULL) {
                l.parameter.grows = parameter.grows;
                for(size_t i = 0; i < count; i++) {
                    if(l.parameter.grows[i] == 0) {
                        fprintf(stderr, "%s:%d : grow size must be at least 1\n", file, line);
                        exit(-1);
                    }
                }
            }
            break;
        case FIXED_LAYOUT:
            if(parameter.fixed_size > 0 && count == 2) {
                if(l.option & HORIZONTAL_LAYOUT) {
                        l.parameter.fixed_size = (parameter.fixed_size < rect.width) ? parameter.fixed_size : (size_t)floorf(rect.width);
                } else if(l.option & VERTICAL_LAYOUT) {
                        l.parameter.fixed_size = (parameter.fixed_size < rect.height) ? parameter.fixed_size : (size_t)floorf(rect.height);
                } else {
                    fprintf(stderr, "%s:%d : FIXED_LAYOUT should have at least option VERTICAL_LAYOUT or HORIZONTAL_LAYOUT\n", file, line);
                    exit(-1);
                }
                if(!(l.option & FIXED_FIRST) && !(l.option & FIXED_LAST)) l.option |= FIXED_FIRST;
            } else {
                fprintf(stderr, "%s:%d : FIXED_LAYOUT should have exactly 2 elements and a fixed sized greater than 0\n", file, line);
                exit(-1);
            }
    }
    da_append(&stack, l);
}

#define layout_stack_pop() layout_stack_pop_(__FILE__, __LINE__)

void layout_stack_pop_(const char* file, int line) {
    if(stack.count == 0) {
        fprintf(stderr, "%s:%d : cannot pop layout, stack is empty\n", file, line);
        exit(-1);
    }
    stack.count--;
}

#define layout_slot() layout_slot_(__FILE__, __LINE__)

Rectangle layout_slot_(const char* file, int line) {
    if(stack.count == 0) {
        fprintf(stderr, "%s:%d : cannot get slot if no layout are available\n", file, line);
        exit(1);
    }
    Layout* layout = &stack.items[stack.count-1];

    if(layout->num_element >= layout->count) {
        fprintf(stderr, "%s:%d : cannot get slot if no slot left in layout\n", file, line);
        exit(1);
    }

    float slot_grow = 0, layout_grows = 0, before_grows = 0; // LINEAR LAYOUT

    Rectangle r = {0};

    switch(layout->kind) {
        case LINEAR_LAYOUT:
            if(layout->parameter.grows != NULL) {
                for(size_t i = 0; i < layout->count; i++) {
                    layout_grows += (float)layout->parameter.grows[i];
                    if(i < layout->num_element) {
                        before_grows += (float)layout->parameter.grows[i];
                    }
                }
                slot_grow = (float)layout->parameter.grows[layout->num_element];
            } else {
                layout_grows = (float)layout->count;
                before_grows = (float)layout->num_element;
                slot_grow = 1;
            }

            
            float grow_unit;
            if(layout->option & HORIZONTAL_LAYOUT) {
                grow_unit = layout->rect.width/layout_grows;

                r.width = grow_unit * slot_grow;
                r.height = layout->rect.height;
                r.x = layout->rect.x + grow_unit*before_grows;
                r.y = layout->rect.y;
            } else if(layout->option & VERTICAL_LAYOUT) {
                grow_unit = layout->rect.height/layout_grows;

                r.width = layout->rect.width;
                r.height = grow_unit * slot_grow;
                r.x = layout->rect.x;
                r.y = layout->rect.y + grow_unit*before_grows;
            }
            break;
        case FIXED_LAYOUT:
            if(layout->option & HORIZONTAL_LAYOUT) {
                if(layout->num_element == 0 && (layout->option & FIXED_FIRST)) {
                    r.width = (float)layout->parameter.fixed_size;
                    r.x = layout->rect.x;
                } else if(layout->num_element == 1 && (layout->option & FIXED_FIRST)) {
                    r.width = layout->rect.width - (float)layout->parameter.fixed_size;
                    r.x = layout->rect.x + (float)layout->parameter.fixed_size;
                } else if(layout->num_element == 1 && (layout->option & FIXED_LAST)) {
                    r.width = (float)layout->parameter.fixed_size;
                    r.x = layout->rect.width - (float)layout->parameter.fixed_size;
                } else if(layout->num_element == 0 && (layout->option & FIXED_LAST)) {
                    r.width = layout->rect.width - (float)layout->parameter.fixed_size;
                    r.x = layout->rect.x;
                }
                r.height = layout->rect.height;
                r.y = layout->rect.y;
            } else if(layout->option & VERTICAL_LAYOUT) {
                if(layout->num_element == 0 && (layout->option & FIXED_FIRST)) {
                    r.height = (float)layout->parameter.fixed_size;
                    r.y = layout->rect.y;
                } else if(layout->num_element == 1 && (layout->option & FIXED_FIRST)) {
                    r.height = layout->rect.height - (float)layout->parameter.fixed_size;
                    r.y = layout->rect.y + (float)layout->parameter.fixed_size;
                } else if(layout->num_element == 1 && (layout->option & FIXED_LAST)) {
                    r.height = (float)layout->parameter.fixed_size;
                    r.y = layout->rect.height - (float)layout->parameter.fixed_size;
                } else if(layout->num_element == 0 && (layout->option & FIXED_LAST)) {
                    r.height = layout->rect.height - (float)layout->parameter.fixed_size;
                    r.y = layout->rect.y;
                }
                r.width = layout->rect.width;
                r.x = layout->rect.x;
            }
        break;
    }
    

    layout->num_element++;
    return r;
}

#define layout_slot_with_padding(size) layout_slot_with_padding_((size), __FILE__, __LINE__)

Rectangle layout_slot_with_padding_(size_t padding, const char* file, int line) {
    Rectangle res = layout_slot_(file, line);
    res.x += (float)padding;
    res.y += (float)padding;
    res.width -= (float)padding*2;
    res.height -= (float)padding*2;
    return res;
}

bool is_hover(Rectangle rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

unsigned long hash(const char *str) {
    unsigned long hash = 5381;
    unsigned int c;

    while((c = (unsigned int)*str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

void set_focused(const char* id) {
    stack.focused = hash(id);
}

bool is_focused(const char* id) {
    return (stack.focused == hash(id));
}

#endif // IMLAYOUT_IMPLEMENTATION
#endif // __IMLAYOUT__