#include <raylib.h>

#define IMLAYOUT_IMPLEMENTATION
#include "imlayout.h"

void widget_debug(Rectangle rect) {

    if(isHover(rect)) {
        char buffer[50];
        sprintf(buffer, "(x:%.0f, y:%.0f, w:%.0f, h:%.0f)", roundf(rect.x), roundf(rect.y), roundf(rect.width), roundf(rect.height));
    
        Vector2 font_size = MeasureTextEx(GetFontDefault(), buffer, 10, 2);
        Vector2 padding = {5, 5};

        DrawRectangle((int)(rect.x + (rect.width - font_size.x)/2 - padding.x), (int)(rect.y + (rect.height - font_size.y)/2 - padding.y), (int)(font_size.x + padding.x*2), (int)(font_size.y + padding.y*2), RED);
        DrawTextEx(GetFontDefault(), buffer, (Vector2){rect.x + (rect.width - font_size.x)/2 , rect.y + (rect.height - font_size.y) / 2}, 10, 2, WHITE);

        DrawRectangleLinesEx(rect, 2, RED);
    }
}

void widget(Rectangle rect, Color color) {
    DrawRectangle((int)ceilf(rect.x), (int)ceilf(rect.y), (int)ceilf(rect.width), (int)ceilf(rect.height), color);
    if(isHover(rect)) {
        DrawRectangleLinesEx((Rectangle){(int)ceilf(rect.x), (int)ceilf(rect.y), (int)ceilf(rect.width), (int)ceilf(rect.height)}, 1, BLACK);
    }
    widget_debug(rect);
}

void widget_hover(Rectangle rect) {
    if(CheckCollisionPointRec(GetMousePosition(), rect)) {
        widget(rect, BLACK);
    } else {
        widget(rect, WHITE);
    }
}

bool button(Rectangle rect, Color color) {
    
    if(CheckCollisionPointRec(GetMousePosition(), rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        widget(rect, ColorBrightness(color, 0.75));
        return true;
    }

    widget(rect, color);
    return false;
}

void widget_fps(Rectangle rect, Color color) {
    widget(rect, color);
    char buffer[20];
    sprintf(buffer, "FPS : %d", GetFPS());
    DrawText(buffer, (int)(rect.x + (rect.width - (float)MeasureText(buffer, 10))/2.0f) , (int)(rect.y + (rect.height - 10.0f)) / 2, 10, BLACK);
}

int main(void) {

    arena_t arena_ctx = {0};

    int width = 800;
    int height = 800;

    SetTraceLogLevel(LOG_ERROR);

    InitWindow(width, height, "UI");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    LayoutStack stack = {0};

    while(!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground(BLACK);

            layout_stack_push_ex(&arena_ctx, 
                               &stack, 
                               LAYOUT_VERTICAL, 
                               ((Rectangle){0, 0, (float)GetRenderWidth(), (float)GetRenderHeight()}), 
                               2,
                               ((size_t[]){10, 90}));
                widget(layout_slot(&stack), BLUE);
                layout_stack_push(&arena_ctx, &stack, LAYOUT_HORIZONTAL, layout_slot(&stack), 2);
                    widget(layout_slot(&stack), GREEN);
                    layout_stack_push(&arena_ctx, &stack, LAYOUT_VERTICAL, layout_slot(&stack), 2);
                        widget(layout_slot(&stack), GRAY);
                        layout_stack_push(&arena_ctx, &stack, LAYOUT_HORIZONTAL, layout_slot(&stack), 2);
                            widget(layout_slot(&stack), YELLOW);
                            widget(layout_slot(&stack), VIOLET);
                            layout_stack_pop(&stack);
                    layout_stack_pop(&stack);
                layout_stack_pop(&stack);
            layout_stack_pop(&stack);

        EndDrawing();

        assert(stack.count == 0);
    }

    CloseWindow();
    arena_free(&arena_ctx);

    return 0;
}