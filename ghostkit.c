#include "ghostkit.h"
#include <string.h>

// ============================================
// PLACEHOLDER MODULE VIEWS (GhostSignal / GhostDrop / GhostBLE)
//
// These modules are not implemented yet (see README roadmap). Rather than
// leave the menu items unwired (which would crash on selection), each gets a
// minimal "coming soon" screen so the app stays usable end-to-end.
// ============================================
static void ghostkit_stub_draw_signal(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GhostSignal");
    canvas_draw_line(canvas, 0, 13, 128, 13);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 30, "SubGHz scanner");
    canvas_draw_str(canvas, 2, 42, "Coming soon");
    canvas_draw_str(canvas, 2, 61, "[Back] Return to menu");
}

static void ghostkit_stub_draw_drop(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GhostDrop");
    canvas_draw_line(canvas, 0, 13, 128, 13);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 30, "BadUSB launcher");
    canvas_draw_str(canvas, 2, 42, "Coming soon");
    canvas_draw_str(canvas, 2, 61, "[Back] Return to menu");
}

static void ghostkit_stub_draw_ble(Canvas* canvas, void* model) {
    UNUSED(model);
    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GhostBLE");
    canvas_draw_line(canvas, 0, 13, 128, 13);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, 30, "Bluetooth recon");
    canvas_draw_str(canvas, 2, 42, "Coming soon");
    canvas_draw_str(canvas, 2, 61, "[Back] Return to menu");
}

static bool ghostkit_stub_input(InputEvent* event, void* ctx) {
    GhostKitApp* app = ctx;
    if(!app) return false;

    if(event->type == InputTypeShort && event->key == InputKeyBack) {
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewMainMenu);
    }
    return true;
}

static View* ghostkit_stub_alloc(GhostKitApp* app, ViewDrawCallback draw_callback) {
    furi_assert(app);
    View* view = view_alloc();
    view_set_draw_callback(view, draw_callback);
    view_set_input_callback(view, ghostkit_stub_input);
    view_set_context(view, app);
    view_set_previous_callback(view, ghostkit_back_to_menu);
    return view;
}

// ============================================
// ABOUT DIALOG
// ============================================
static void ghostkit_about_result_callback(DialogExResult result, void* context) {
    UNUSED(result);
    GhostKitApp* app = context;
    view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewMainMenu);
}

// ============================================
// NAVIGATION
// ============================================
uint32_t ghostkit_back_to_menu(void* context) {
    UNUSED(context);
    return GhostKitViewMainMenu;
}

static uint32_t ghostkit_exit_app(void* context) {
    UNUSED(context);
    return VIEW_NONE;
}

// ============================================
// MAIN MENU
// ============================================
static void ghostkit_menu_callback(void* context, uint32_t index) {
    GhostKitApp* app = context;

    switch(index) {
    case GhostKitMenuAccess:
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewAccess);
        break;
    case GhostKitMenuSignal:
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewSignal);
        break;
    case GhostKitMenuDrop:
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewDrop);
        break;
    case GhostKitMenuLog:
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewLog);
        break;
    case GhostKitMenuBLE:
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewBLE);
        break;
    case GhostKitMenuAbout:
        view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewAbout);
        break;
    default:
        break;
    }
}

// ============================================
// CUSTOM EVENTS (used to safely hand NFC worker-thread
// results back to the GUI thread)
// ============================================
static bool ghostkit_custom_event_callback(void* context, uint32_t event) {
    GhostKitApp* app = context;

    if(event == GhostKitCustomEventNfcDetected) {
        ghost_access_handle_detected(app);
        return true;
    }

    return false;
}

// ============================================
// APP LIFECYCLE
// ============================================
GhostKitApp* ghostkit_app_alloc() {
    GhostKitApp* app = malloc(sizeof(GhostKitApp));
    memset(app, 0, sizeof(GhostKitApp));

    app->storage = furi_record_open(RECORD_STORAGE);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->gui = furi_record_open(RECORD_GUI);

    // Make sure the findings directory exists before GhostLog ever tries to append to it
    storage_common_mkdir(app->storage, "/ext/ghostkit");

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, ghostkit_custom_event_callback);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);

    // Main menu
    app->main_menu = submenu_alloc();
    submenu_set_header(app->main_menu, "GhostKit");
    submenu_add_item(
        app->main_menu, "GhostAccess", GhostKitMenuAccess, ghostkit_menu_callback, app);
    submenu_add_item(
        app->main_menu, "GhostSignal", GhostKitMenuSignal, ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostDrop", GhostKitMenuDrop, ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostLog", GhostKitMenuLog, ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostBLE", GhostKitMenuBLE, ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "About", GhostKitMenuAbout, ghostkit_menu_callback, app);
    view_set_previous_callback(submenu_get_view(app->main_menu), ghostkit_exit_app);
    view_dispatcher_add_view(
        app->view_dispatcher, GhostKitViewMainMenu, submenu_get_view(app->main_menu));

    // Module views
    app->access_view = ghost_access_alloc(app);
    view_dispatcher_add_view(app->view_dispatcher, GhostKitViewAccess, app->access_view);

    app->signal_view = ghostkit_stub_alloc(app, ghostkit_stub_draw_signal);
    view_dispatcher_add_view(app->view_dispatcher, GhostKitViewSignal, app->signal_view);

    app->drop_view = ghostkit_stub_alloc(app, ghostkit_stub_draw_drop);
    view_dispatcher_add_view(app->view_dispatcher, GhostKitViewDrop, app->drop_view);

    app->log_view = ghost_log_alloc(app);
    view_dispatcher_add_view(app->view_dispatcher, GhostKitViewLog, app->log_view);

    app->ble_view = ghostkit_stub_alloc(app, ghostkit_stub_draw_ble);
    view_dispatcher_add_view(app->view_dispatcher, GhostKitViewBLE, app->ble_view);

    // About dialog
    app->about_dialog = dialog_ex_alloc();
    dialog_ex_set_header(app->about_dialog, "GhostKit v1.0", 64, 2, AlignCenter, AlignTop);
    dialog_ex_set_text(
        app->about_dialog,
        "Physical pentest toolkit\nby Chauncey\n\nMITRE-mapped findings\nexport to GhostLog CSV",
        64,
        16,
        AlignCenter,
        AlignTop);
    dialog_ex_set_center_button_text(app->about_dialog, "Back");
    dialog_ex_set_context(app->about_dialog, app);
    dialog_ex_set_result_callback(app->about_dialog, ghostkit_about_result_callback);
    view_set_previous_callback(dialog_ex_get_view(app->about_dialog), ghostkit_back_to_menu);
    view_dispatcher_add_view(
        app->view_dispatcher, GhostKitViewAbout, dialog_ex_get_view(app->about_dialog));

    view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewMainMenu);

    return app;
}

void ghostkit_app_free(GhostKitApp* app) {
    furi_assert(app);

    // Make sure no scan is left running
    if(app->nfc_scanner) {
        nfc_scanner_stop(app->nfc_scanner);
        nfc_scanner_free(app->nfc_scanner);
        app->nfc_scanner = NULL;
    }
    if(app->nfc) {
        nfc_free(app->nfc);
        app->nfc = NULL;
    }

    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewMainMenu);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewAccess);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewSignal);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewDrop);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewLog);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewBLE);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewAbout);

    submenu_free(app->main_menu);
    dialog_ex_free(app->about_dialog);

    view_free(app->access_view);
    view_free(app->signal_view);
    view_free(app->drop_view);
    view_free(app->log_view);
    view_free(app->ble_view);

    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_GUI);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_STORAGE);

    free(app);
}

// ============================================
// ENTRY POINT
// ============================================
int32_t ghostkit_main(void* p) {
    UNUSED(p);

    GhostKitApp* app = ghostkit_app_alloc();
    view_dispatcher_run(app->view_dispatcher);
    ghostkit_app_free(app);

    return 0;
}
