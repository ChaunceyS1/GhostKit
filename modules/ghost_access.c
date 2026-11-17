#include "../ghostkit.h"
#include <string.h>

// ============================================
// GhostAccess — RFID/NFC access control recon
//
// Uses the Momentum NFC stack's NfcScanner to passively detect which
// contactless protocol(s) a nearby card speaks (ISO14443-3A, MIFARE
// Classic/Ultralight, etc). Detection happens on an NFC worker thread, so the
// result is handed back to the GUI thread via a ViewDispatcher custom event
// (see ghostkit_custom_event_callback / ghost_access_handle_detected).
//
// A detected card is logged as a GhostLog finding (MITRE T1078 — Valid
// Accounts via a clonable/weak access credential).
// ============================================

#define GHOST_ACCESS_PROTOCOL_NAME_LEN 48
#define GHOST_ACCESS_SEVERITY_LEN 8

typedef struct {
    bool scanning;
    bool has_result;
    char last_protocol[GHOST_ACCESS_PROTOCOL_NAME_LEN];
    char last_severity[GHOST_ACCESS_SEVERITY_LEN];
} GhostAccessModel;

// ============================================
// Per-protocol vulnerability assessment.
//
// Not every contactless protocol is equally weak: MIFARE Classic's Crypto1
// cipher has been broken for over a decade (trivially clonable), while
// DESFire uses AES and is not. Severity reflects that instead of a flat
// "MEDIUM" for every card.
// ============================================
static int ghost_access_severity_rank(NfcProtocol protocol) {
    switch(protocol) {
    case NfcProtocolMfClassic:
    case NfcProtocolMfUltralight:
        return 3; // HIGH
    case NfcProtocolIso15693_3:
    case NfcProtocolMfPlus:
    case NfcProtocolSlix:
    case NfcProtocolSt25tb:
        return 2; // MEDIUM
    default:
        return 1; // LOW
    }
}

static const char* ghost_access_severity_label(int rank) {
    switch(rank) {
    case 3:
        return "HIGH";
    case 2:
        return "MEDIUM";
    default:
        return "LOW";
    }
}

static const char* ghost_access_protocol_note(NfcProtocol protocol) {
    switch(protocol) {
    case NfcProtocolMfClassic:
        return "MIFARE Classic - Crypto1 cipher is broken, card is clonable";
    case NfcProtocolMfUltralight:
        return "MIFARE Ultralight - typically unauthenticated, clonable";
    case NfcProtocolMfPlus:
        return "MIFARE Plus - may fall back to weaker Classic-compatible mode";
    case NfcProtocolMfDesfire:
        return "MIFARE DESFire - AES-128 secured, not trivially clonable";
    case NfcProtocolIso15693_3:
        return "ISO15693 - common access/asset tag, often unauthenticated";
    case NfcProtocolIso14443_3a:
    case NfcProtocolIso14443_3b:
        return "Bare ISO14443-3 UID only, no higher-layer auth confirmed";
    default:
        return "Protocol detected, no specific vulnerability assessment yet";
    }
}

// ============================================
// NFC SCANNER CALLBACK — runs on the NFC worker thread
// ============================================
static void ghost_access_scanner_callback(NfcScannerEvent event, void* context) {
    GhostKitApp* app = context;
    if(!app) return;

    if(event.type == NfcScannerEventTypeDetected) {
        size_t count = event.data.protocol_num;
        if(count > GHOSTKIT_NFC_MAX_PROTOCOLS) {
            count = GHOSTKIT_NFC_MAX_PROTOCOLS;
        }
        for(size_t i = 0; i < count; i++) {
            app->detected_protocols[i] = event.data.protocols[i];
        }
        app->detected_protocol_count = count;

        // Hand off to the GUI thread — never touch views/NFC teardown from here
        view_dispatcher_send_custom_event(app->view_dispatcher, GhostKitCustomEventNfcDetected);
    }
}

// ============================================
// Called on the GUI thread once a detection event has been posted
// ============================================
void ghost_access_handle_detected(GhostKitApp* app) {
    furi_assert(app);

    if(app->nfc_scanner) {
        nfc_scanner_stop(app->nfc_scanner);
        nfc_scanner_free(app->nfc_scanner);
        app->nfc_scanner = NULL;
    }
    if(app->nfc) {
        nfc_free(app->nfc);
        app->nfc = NULL;
    }

    FuriString* names = furi_string_alloc();
    for(size_t i = 0; i < app->detected_protocol_count; i++) {
        if(i > 0) {
            furi_string_cat_str(names, ", ");
        }
        furi_string_cat_str(names, nfc_device_get_protocol_name(app->detected_protocols[i]));
    }
    if(app->detected_protocol_count == 0) {
        furi_string_cat_str(names, "Unknown protocol");
    }

    // Report on whichever detected protocol is the most severe, not just the first one
    NfcProtocol primary = NfcProtocolInvalid;
    int best_rank = 0;
    for(size_t i = 0; i < app->detected_protocol_count; i++) {
        int rank = ghost_access_severity_rank(app->detected_protocols[i]);
        if(rank > best_rank) {
            best_rank = rank;
            primary = app->detected_protocols[i];
        }
    }
    const char* severity = ghost_access_severity_label(best_rank);
    const char* note = app->detected_protocol_count > 0 ? ghost_access_protocol_note(primary) :
                                                            "No card responded";

    FuriString* title = furi_string_alloc();
    furi_string_printf(
        title, "RFID/NFC Card Detected: %s - %s", furi_string_get_cstr(names), note);
    ghost_log_write(app, furi_string_get_cstr(title), severity, "T1078");

    notification_message(app->notifications, &sequence_single_vibro);

    if(app->access_view) {
        GhostAccessModel* model = view_get_model(app->access_view);
        model->scanning = false;
        model->has_result = true;
        strncpy(model->last_protocol, furi_string_get_cstr(names), GHOST_ACCESS_PROTOCOL_NAME_LEN - 1);
        model->last_protocol[GHOST_ACCESS_PROTOCOL_NAME_LEN - 1] = '\0';
        strncpy(model->last_severity, severity, GHOST_ACCESS_SEVERITY_LEN - 1);
        model->last_severity[GHOST_ACCESS_SEVERITY_LEN - 1] = '\0';
        view_commit_model(app->access_view, true);
    }

    furi_string_free(title);
    furi_string_free(names);
}

// ============================================
// SCREEN DRAW
// ============================================
static void ghost_access_draw(Canvas* canvas, void* model_ptr) {
    GhostAccessModel* model = model_ptr;
    if(!model) return;

    canvas_clear(canvas);

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "GhostAccess");
    canvas_draw_line(canvas, 0, 13, 128, 13);
    canvas_set_font(canvas, FontSecondary);

    if(model->has_result) {
        char severity_line[56];
        snprintf(severity_line, sizeof(severity_line), "Severity: %s", model->last_severity);

        canvas_draw_str(canvas, 2, 25, "Card detected:");
        canvas_draw_str(canvas, 2, 37, model->last_protocol);
        canvas_draw_str(canvas, 2, 49, severity_line);
        canvas_draw_str(canvas, 2, 61, "[Back] Return to menu");
    } else if(model->scanning) {
        canvas_draw_str(canvas, 2, 25, "Scanning...");
        canvas_draw_str(canvas, 2, 37, "Hold card to Flipper's back");
        canvas_draw_str(canvas, 2, 61, "[Back] Cancel scan");
    } else {
        canvas_draw_str(canvas, 2, 25, "Hold card to Flipper");
        canvas_draw_str(canvas, 2, 37, "to scan NFC/RFID");
        canvas_draw_str(canvas, 2, 49, "[OK]   Start scan");
        canvas_draw_str(canvas, 2, 61, "[Back] Return to menu");
    }
}

// ============================================
// BUTTON INPUT
// ============================================
static bool ghost_access_input(InputEvent* event, void* ctx) {
    GhostKitApp* app = ctx;
    if(!app) return false;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyOk) {
            GhostAccessModel* model = view_get_model(app->access_view);
            bool start_scan = !model->scanning;
            if(start_scan) {
                model->scanning = true;
                model->has_result = false;
            }
            view_commit_model(app->access_view, true);

            if(start_scan) {
                app->detected_protocol_count = 0;
                app->nfc = nfc_alloc();
                app->nfc_scanner = nfc_scanner_alloc(app->nfc);
                nfc_scanner_start(app->nfc_scanner, ghost_access_scanner_callback, app);
            }
        }
        if(event->key == InputKeyBack) {
            if(app->nfc_scanner) {
                nfc_scanner_stop(app->nfc_scanner);
                nfc_scanner_free(app->nfc_scanner);
                app->nfc_scanner = NULL;
            }
            if(app->nfc) {
                nfc_free(app->nfc);
                app->nfc = NULL;
            }

            GhostAccessModel* model = view_get_model(app->access_view);
            model->scanning = false;
            model->has_result = false;
            view_commit_model(app->access_view, false);

            view_dispatcher_switch_to_view(app->view_dispatcher, GhostKitViewMainMenu);
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

    view_allocate_model(view, ViewModelTypeLockFree, sizeof(GhostAccessModel));
    GhostAccessModel* model = view_get_model(view);
    model->scanning = false;
    model->has_result = false;
    model->last_protocol[0] = '\0';
    model->last_severity[0] = '\0';
    view_commit_model(view, false);

    app->access_view = view;
    return view;
}
