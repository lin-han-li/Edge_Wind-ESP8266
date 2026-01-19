/**
 * @file edgewind_ui.c
 * @brief EdgeWind UI 主入口实现
 * 
 * 简化版：仅包含开机动画
 */

#include "edgewind_ui.h"

/*******************************************************************************
 * 内部变量
 ******************************************************************************/

static bool ui_initialized = false;

/*******************************************************************************
 * 公共函数实现
 ******************************************************************************/

void edgewind_ui_init(void)
{
    if (ui_initialized) return;
    
    /* 初始化主题和样式 */
    edgewind_theme_init();
    
    /* 创建并加载开机动画界面 */
    ew_boot_anim_create();
    lv_scr_load(ew_boot_anim.screen);
    
    ui_initialized = true;
}

void edgewind_ui_refresh(void)
{
    /* 开机动画阶段无需额外数据刷新 */
    /* 动画由 LVGL 内部 lv_anim 系统驱动 */
}

bool edgewind_ui_boot_finished(void)
{
    return ew_boot_anim_is_finished();
}
