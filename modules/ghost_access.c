#include "ghostkit.h"

// ============================================
// MENU CALLBACK
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
// BACK NAVIGATION
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
// APP ALLOC
// ============================================
GhostKitApp* ghostkit_app_alloc() {
    GhostKitApp* app = malloc(sizeof(GhostKitApp));

    app->storage = furi_record_open(RECORD_STORAGE);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->finding_count = 0;

    storage_simply_mkdir(app->storage, "/ext/ghostkit");

    app->view_dispatcher = view_dispatcher_alloc();

    // Main menu
    app->main_menu = submenu_alloc();
    submenu_add_item(app->main_menu, "GhostAccess", GhostKitMenuAccess, ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostSignal",  GhostKitMenuSignal,  ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostDrop",    GhostKitMenuDrop,    ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostLog",     GhostKitMenuLog,     ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "GhostBLE",     GhostKitMenuBLE,     ghostkit_menu_callback, app);
    submenu_add_item(app->main_menu, "About",        GhostKitMenuAbout,   ghostkit_menu_callback, app);

    view_set_previous_callback(
        submenu_get_view(app->main_menu), ghostkit_exit_app);

    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewMainMenu,
        submenu_get_view(app->main_menu));

    // About dialog
    app->about_dialog = dialog_ex_alloc();
    dialog_ex_set_header(app->about_dialog, "GhostKit v1.0", 64, 0, AlignCenter, AlignTop);
    dialog_ex_set_text(
        app->about_dialog,
        "Physical Pentest Toolkit\nby Chauncey\n\ngithub.com/[yourusername]\n/GhostKit",
        64, 32, AlignCenter, AlignCenter);
    dialog_ex_set_center_button_text(app->about_dialog, "Back");

    view_set_previous_callback(
        dialog_ex_get_view(app->about_dialog), ghostkit_back_to_menu);

    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewAbout,
        dialog_ex_get_view(app->about_dialog));

    // GhostLog
    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewLog,
        ghost_log_alloc(app));

    // GhostAccess
    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewAccess,
        ghost_access_alloc(app));

    // GhostSignal stub
    View* signal_view = view_alloc();
    view_set_previous_callback(signal_view, ghostkit_back_to_menu);
    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewSignal,
        signal_view);

    // GhostDrop stub
    View* drop_view = view_alloc();
    view_set_previous_callback(drop_view, ghostkit_back_to_menu);
    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewDrop,
        drop_view);

    // GhostBLE stub
    View* ble_view = view_alloc();
    view_set_previous_callback(ble_view, ghostkit_back_to_menu);
    view_dispatcher_add_view(
        app->view_dispatcher,
        GhostKitViewBLE,
        ble_view);

    return app;
}

// ============================================
// APP FREE
// ============================================
void ghostkit_app_free(GhostKitApp* app) {
    furi_assert(app);

    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewMainMenu);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewAbout);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewLog);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewAccess);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewSignal);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewDrop);
    view_dispatcher_remove_view(app->view_dispatcher, GhostKitViewBLE);

    submenu_free(app->main_menu);
    dialog_ex_free(app->about_dialog);
    view_dispatcher_free(app->view_dispatcher);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);

    free(app);
}

// ============================================
// ENTRY POINT
// ============================================
int32_t ghostkit_main(void* p) {
    UNUSED(p);

    GhostKitApp* app = ghostkit_app_alloc();

    Gui* gui = furi_record_open(RECORD_GUI);
    view_dispatcher_attach_to_gui(
        app->view_dispatcher, gui, ViewDispatcherTypeFullscreen);

    view_dispatcher_switch_to_view(
        app->view_dispatcher, GhostKitViewMainMenu);

    view_dispatcher_run(app->view_dispatcher);

    ghostkit_app_free(app);
    furi_record_close(RECORD_GUI);

    return 0;
}