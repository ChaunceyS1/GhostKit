#include "ghostkit.h"

// ============================================
// SCREEN DRAW
// ============================================
static void ghost_access_draw(Canvas* canvas, void* ctx) {
    UNUSED(ctx);

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GhostAccess");
    canvas_draw_line(canvas, 0, 13, 128, 13);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 25, "Hold card to Flipper");
    canvas_draw_str(canvas, 2, 37, "to scan NFC/RFID");
    canvas_draw_str(canvas, 2, 49, "[OK]   Start scan");
    canvas_draw_str(canvas, 2, 61, "[Back] Return to menu");
}

// ============================================
// BUTTON INPUT
// ============================================
static bool ghost_access_input(InputEvent* event, void* ctx) {
    GhostKitApp* app = ctx;
    if(!app) return false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(
                app->view_dispatcher,
                GhostKitViewMainMenu);
        }
    }
    return true;
}

// ============================================
// ALLOC
// ============================================
View* ghost_access_alloc(GhostKitApp* app) {
    furi_assert(app);
    View* view = view_alloc();
    view_set_draw_callback(view, ghost_access_draw);
    view_set_input_callback(view, ghost_access_input);
    view_set_context(view, app);
    view_set_previous_callback(view, ghostkit_back_to_menu);
    return view;
}