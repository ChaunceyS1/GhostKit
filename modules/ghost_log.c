#include "../ghostkit.h"

#define LOG_PATH "/ext/ghostkit/findings.csv"

// ============================================
// SCREEN DRAW
// ============================================
static void ghost_log_draw(Canvas* canvas, void* ctx) {
    GhostKitApp** model = ctx;
    if(!model || !*model) return;
    GhostKitApp* app = *model;

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GhostLog");
    canvas_draw_line(canvas, 0, 13, 128, 13);

    canvas_set_font(canvas, FontSecondary);
    char count_str[32];
    snprintf(count_str, sizeof(count_str),
        "Findings: %lu", app->finding_count);
    canvas_draw_str(canvas, 2, 26, count_str);
    canvas_draw_str(canvas, 2, 38, "[OK]   Log test finding");
    canvas_draw_str(canvas, 2, 50, "[Back] Return to menu");
    canvas_draw_line(canvas, 0, 54, 128, 54);
    canvas_draw_str(canvas, 2, 63, "/ext/ghostkit/findings.csv");
}

// ============================================
// BUTTON INPUT
// ============================================
static bool ghost_log_input(InputEvent* event, void* ctx) {
    GhostKitApp* app = ctx;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            ghost_log_write(app,
                "Test Finding",
                "HIGH",
                "T1078");
            notification_message(
                app->notifications,
                &sequence_single_vibro);
        }
        if(event->key == InputKeyBack) {
            view_dispatcher_switch_to_view(
                app->view_dispatcher,
                GhostKitViewMainMenu);
        }
    }
    return true;
}

// ============================================
// BACK NAVIGATION
// ============================================
static uint32_t ghost_log_back(void* context) {
    UNUSED(context);
    return GhostKitViewMainMenu;
}

// ============================================
// WRITE TO SD CARD
// ============================================
void ghost_log_write(GhostKitApp* app,
                     const char* title,
                     const char* severity,
                     const char* mitre) {

    File* file = storage_file_alloc(app->storage);

    if(storage_file_open(
        file, LOG_PATH,
        FSAM_WRITE, FSOM_OPEN_APPEND)) {

        FuriString* line = furi_string_alloc();
        furi_string_printf(
            line,
            "%lu,%s,%s,%s\n",
            app->finding_count++,
            title,
            severity,
            mitre
        );

        storage_file_write(
            file,
            furi_string_get_cstr(line),
            furi_string_size(line)
        );

        furi_string_free(line);
    }

    storage_file_close(file);
    storage_file_free(file);
}

// ============================================
// ALLOC — creates the view
// ============================================
View* ghost_log_alloc(GhostKitApp* app) {
    furi_assert(app);
    View* view = view_alloc();
    view_set_draw_callback(view, ghost_log_draw);
    view_set_input_callback(view, ghost_log_input);
    view_set_context(view, app);
    view_set_previous_callback(view, ghost_log_back);
    view_allocate_model(view, ViewModelTypeLockFree, sizeof(GhostKitApp*));
    GhostKitApp** model = view_get_model(view);
    *model = app;
    view_commit_model(view, false);
    return view;
}