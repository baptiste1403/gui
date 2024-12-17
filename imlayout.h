#ifndef __IMLAYOUT__
#define __IMLAYOUT__

#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#define ARENA_IMPLEMENTATION
#include "arena.h"

const size_t FACTOR = 40;

typedef enum {
    LAYOUT_HORIZONTAL,
    LAYOUT_VERTICAL
} LayoutDirection;

typedef struct {
    LayoutDirection direction;
    Rectangle rect;
    size_t count;
    const size_t* grows;
    size_t num_element;
} Layout;

typedef struct {
    Layout* items;
    size_t count;
    size_t capacity;
} LayoutStack;

Layout make_layout(LayoutDirection direction, Rectangle rect, size_t count, size_t* grows);

#define layout_stack_push_ex(ctx, stack, direction, rect, count, grows) layout_stack_push_ex_((ctx), (stack), (direction), (rect), (count), (grows), __FILE__, __LINE__)
void layout_stack_push_ex_(arena_t* ctx, LayoutStack* stack, LayoutDirection direction, Rectangle rect, size_t count, const size_t* grows, const char* file, int line);
void layout_stack_push(arena_t* ctx, LayoutStack* stack, LayoutDirection direction, Rectangle rect, size_t count);
#define layout_stack_pop(stack) layout_stack_pop_(stack, __FILE__, __LINE__)
void layout_stack_pop_(LayoutStack* stack, const char* file, int line);
#define layout_slot(stack) layout_slot_((stack), __FILE__, __LINE__)
Rectangle layout_slot_(LayoutStack* stack, const char* file, int line);
#define layout_slot_with_padding(stack, size) layout_slot_with_padding_((stack), (size), __FILE__, __LINE__)
Rectangle layout_slot_with_padding_(LayoutStack* stack, size_t padding, const char* file, int line);
bool isHover(Rectangle rect);

#ifdef IMLAYOUT_IMPLEMENTATION

Layout make_layout(LayoutDirection direction, Rectangle rect, size_t count, size_t* grows) {
    Layout res = {direction, rect, count, grows, 0};
    return res;
}

#define layout_stack_push_ex(ctx, stack, direction, rect, count, grows) layout_stack_push_ex_((ctx), (stack), (direction), (rect), (count), (grows), __FILE__, __LINE__)

void layout_stack_push_ex_(arena_t* ctx, LayoutStack* stack, LayoutDirection direction, Rectangle rect, size_t count, const size_t* grows, const char* file, int line) {
    Layout l = make_layout(direction, rect, count, NULL);
    if (grows != NULL) {
        l.grows = grows;
        for(size_t i = 0; i < count; i++) {
            if(l.grows[i] == 0) {
                fprintf(stderr, "%s:%d : grow size must be at least 1\n", file, line);
                exit(-1);
            }
        }
    }
    arena_da_append(ctx, stack, l);
}

void layout_stack_push(arena_t* ctx, LayoutStack* stack, LayoutDirection direction, Rectangle rect, size_t count) {
    layout_stack_push_ex(ctx, stack, direction, rect, count, NULL);
}

#define layout_stack_pop(stack) layout_stack_pop_(stack, __FILE__, __LINE__)

void layout_stack_pop_(LayoutStack* stack, const char* file, int line) {
    if(stack->count == 0) {
        fprintf(stderr, "%s:%d : cannot pop layout, stack is empty\n", file, line);
        exit(-1);
    }
    stack->count--;
}

#define layout_slot(stack) layout_slot_((stack), __FILE__, __LINE__)

Rectangle layout_slot_(LayoutStack* stack, const char* file, int line) {
    if(stack->count == 0) {
        fprintf(stderr, "%s:%d : cannot get slot if no layout are available\n", file, line);
        exit(1);
    }
    Layout* layout = &stack->items[stack->count-1];

    if(layout->num_element >= layout->count) {
        fprintf(stderr, "%s:%d : cannot get slot if no slot left in layout\n", file, line);
        exit(1);
    }

    float slot_grow = 0, layout_grows = 0, before_grows = 0;

    if(layout->grows != NULL) {
        for(size_t i = 0; i < layout->count; i++) {
            layout_grows += (float)layout->grows[i];
            if(i < layout->num_element) {
                before_grows += (float)layout->grows[i];
            }
        }
        slot_grow = (float)layout->grows[layout->num_element];
    } else {
        layout_grows = (float)layout->count;
        before_grows = (float)layout->num_element;
        slot_grow = 1;
    }

    Rectangle r;
    float grow_unit;
    switch (layout->direction) {
        case LAYOUT_HORIZONTAL:
            grow_unit = layout->rect.width/layout_grows;

            r.width = grow_unit * slot_grow;
            r.height = layout->rect.height;
            r.x = layout->rect.x + grow_unit*before_grows;
            r.y = layout->rect.y;
            break;
        case LAYOUT_VERTICAL:
            grow_unit = layout->rect.height/layout_grows;

            r.width = layout->rect.width;
            r.height = grow_unit * slot_grow;
            r.x = layout->rect.x;
            r.y = layout->rect.y + grow_unit*before_grows;
            break;
    }

    layout->num_element++;
    return r;
}

#define layout_slot_with_padding(stack, size) layout_slot_with_padding_((stack), (size), __FILE__, __LINE__)

Rectangle layout_slot_with_padding_(LayoutStack* stack, size_t padding, const char* file, int line) {
    Rectangle res = layout_slot_(stack, file, line);
    res.x += (float)padding;
    res.y += (float)padding;
    res.width -= (float)padding*2;
    res.height -= (float)padding*2;
    return res;
}

bool isHover(Rectangle rect) {
    return CheckCollisionPointRec(GetMousePosition(), rect);
}

#endif // IMLAYOUT_IMPLEMENTATION
#endif // __IMLAYOUT__