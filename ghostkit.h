#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

// ============================================
// Views
// ============================================
typedef enum {
    GhostKitViewMainMenu,
    GhostKitViewAccess,
    GhostKitViewSignal,
    GhostKitViewDrop,
    GhostKitViewLog,
    GhostKitViewBLE,
    GhostKitViewAbout,
} GhostKitView;

// ============================================
// Menu items
// ============================================
typedef enum {
    GhostKitMenuAccess = 0,
    GhostKitMenuSignal = 1,
    GhostKitMenuDrop   = 2,
    GhostKitMenuLog    = 3,
    GhostKitMenuBLE    = 4,
    GhostKitMenuAbout  = 5,
} GhostKitMenuItem;

// ============================================
// App state
// ============================================
typedef struct {
    ViewDispatcher*  view_dispatcher;
    Submenu*         main_menu;
    DialogEx*        about_dialog;
    Storage*         storage;
    NotificationApp* notifications;
    uint32_t         finding_count;
} GhostKitApp;

// ============================================
// Function declarations
// ============================================

// App lifecycle
GhostKitApp* ghostkit_app_alloc();
void         ghostkit_app_free(GhostKitApp* app);

// Navigation
uint32_t ghostkit_back_to_menu(void* context);

// Module allocators
View* ghost_log_alloc(GhostKitApp* app);
View* ghost_access_alloc(GhostKitApp* app);

// GhostLog write
void ghost_log_write(
    GhostKitApp* app,
    const char* title,
    const char* severity,
    const char* mitre);