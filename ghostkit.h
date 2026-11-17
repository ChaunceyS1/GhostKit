#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/dialog_ex.h>
#include <storage/storage.h>
#include <notification/notification_messages.h>

#include <nfc/nfc.h>
#include <nfc/nfc_scanner.h>
#include <nfc/nfc_device.h>

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
// Custom view-dispatcher events
// ============================================
typedef enum {
    GhostKitCustomEventNfcDetected,
} GhostKitCustomEvent;

// Max number of simultaneously detected NFC protocols we keep around
#define GHOSTKIT_NFC_MAX_PROTOCOLS 8

// ============================================
// App state
// ============================================
typedef struct {
    ViewDispatcher*  view_dispatcher;
    Gui*             gui;
    Submenu*         main_menu;
    DialogEx*        about_dialog;
    Storage*         storage;
    NotificationApp* notifications;
    uint32_t         finding_count;

    // Raw custom views we own directly (freed with view_free)
    View* access_view;
    View* signal_view;
    View* drop_view;
    View* ble_view;
    View* log_view;

    // GhostAccess NFC scan state
    Nfc*        nfc;
    NfcScanner* nfc_scanner;
    NfcProtocol detected_protocols[GHOSTKIT_NFC_MAX_PROTOCOLS];
    size_t      detected_protocol_count;
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

// GhostAccess: called on the GUI thread once NfcScanner reports a detection
void ghost_access_handle_detected(GhostKitApp* app);
