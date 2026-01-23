/**
 * @file guider_entry.c
 * @brief GUI Guider 入口：进入系统后加载 Main_1
 */

#include "../EdgeWind_UI/edgewind_ui.h"
#include "gui_assets.h"
#include "src/generated/gui_guider.h"

lv_ui guider_ui;

static bool guider_initialized = false;

void edgewind_ui_on_enter_system(void)
{
    if (!guider_initialized) {
        gui_assets_init();
        setup_ui(&guider_ui);
        gui_assets_patch_images(&guider_ui);
        guider_initialized = true;
        return;
    }

    if (guider_ui.Main_1) {
        lv_screen_load(guider_ui.Main_1);
    }
}
