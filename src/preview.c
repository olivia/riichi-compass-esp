#include "lvgl.h"
#include "../lib/ui/ui.h"

// initializes your UI (entry point of your application)
static void start_ui(void) {
    ui_init();
}


#ifdef LVGL_LIVE_PREVIEW
// gives the live preview tool a way to initialize your UI
void lvgl_live_preview_init(void) {
    start_ui();
}
#endif
