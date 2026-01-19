/**
 * @file edgewind_theme.c
 * @brief EdgeWind UI 主题样式实现
 */

#include "edgewind_theme.h"

/*******************************************************************************
 * 全局样式定义
 ******************************************************************************/

lv_style_t ew_style_bg;
lv_style_t ew_style_card;
lv_style_t ew_style_btn;
lv_style_t ew_style_btn_pressed;
lv_style_t ew_style_title;
lv_style_t ew_style_text;
lv_style_t ew_style_text_muted;
lv_style_t ew_style_value;
lv_style_t ew_style_divider;

/*******************************************************************************
 * 函数实现
 ******************************************************************************/

void edgewind_theme_init(void)
{
    /* 1) 全局背景：深邃工业黑 + 微弱渐变（避免“死黑”） */
    lv_style_init(&ew_style_bg);
    lv_style_set_bg_color(&ew_style_bg, lv_color_hex(0x101014));
    lv_style_set_bg_grad_color(&ew_style_bg, lv_color_hex(0x181820));
    lv_style_set_bg_grad_dir(&ew_style_bg, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&ew_style_bg, LV_OPA_COVER);
    lv_style_set_border_width(&ew_style_bg, 0);
    lv_style_set_pad_all(&ew_style_bg, 0);
    
    /* 2) 卡片样式：轻量磨砂玻璃（Glassmorphism Lite） */
    lv_style_init(&ew_style_card);
    lv_style_set_bg_color(&ew_style_card, lv_color_hex(0x1E1E24));
    lv_style_set_bg_opa(&ew_style_card, LV_OPA_90);
    lv_style_set_radius(&ew_style_card, 16);
    lv_style_set_pad_all(&ew_style_card, EW_PAD_GAP);
    /* 轻描边（低不透明度） */
    lv_style_set_border_width(&ew_style_card, 1);
    lv_style_set_border_color(&ew_style_card, lv_color_hex(0x3A3A45));
    lv_style_set_border_opa(&ew_style_card, LV_OPA_50);
    /* 深色投影（体现悬浮层次） */
    lv_style_set_shadow_width(&ew_style_card, 20);
    lv_style_set_shadow_color(&ew_style_card, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&ew_style_card, LV_OPA_40);
    
    /* 3) 按钮默认样式：去边框，用底部高光条暗示立体感 */
    lv_style_init(&ew_style_btn);
    lv_style_set_bg_color(&ew_style_btn, lv_color_hex(0x25252C));
    lv_style_set_bg_grad_color(&ew_style_btn, lv_color_hex(0x2A2A32));
    lv_style_set_bg_grad_dir(&ew_style_btn, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&ew_style_btn, LV_OPA_COVER);
    lv_style_set_radius(&ew_style_btn, 12);
    lv_style_set_pad_all(&ew_style_btn, EW_PAD_GAP);
    /* 仅保留底部边：高光条 */
    lv_style_set_border_side(&ew_style_btn, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_width(&ew_style_btn, 2);
    lv_style_set_border_color(&ew_style_btn, lv_color_hex(0x00D2FF));
    lv_style_set_border_opa(&ew_style_btn, LV_OPA_30);
    lv_style_set_text_color(&ew_style_btn, EW_COLOR_TEXT);
    lv_style_set_shadow_width(&ew_style_btn, 0);
    
    /* 4) 按钮按下：轻微发光 + 微缩（保持通透） */
    lv_style_init(&ew_style_btn_pressed);
    lv_style_set_bg_color(&ew_style_btn_pressed, lv_color_hex(0x00D2FF));
    lv_style_set_bg_opa(&ew_style_btn_pressed, LV_OPA_20);
    lv_style_set_transform_scale(&ew_style_btn_pressed, 240); /* 微缩 */
    lv_style_set_border_color(&ew_style_btn_pressed, lv_color_hex(0x00D2FF));
    lv_style_set_border_opa(&ew_style_btn_pressed, LV_OPA_COVER);
    lv_style_set_shadow_width(&ew_style_btn_pressed, 15);
    lv_style_set_shadow_color(&ew_style_btn_pressed, lv_color_hex(0x00D2FF));
    lv_style_set_shadow_opa(&ew_style_btn_pressed, LV_OPA_60);
    
    /* 标题文本样式 - 使用思源宋体 28px */
    lv_style_init(&ew_style_title);
    lv_style_set_text_color(&ew_style_title, EW_COLOR_TEXT);
    lv_style_set_text_font(&ew_style_title, EW_FONT_CN_TITLE);
    
    /* 正文文本样式 - 使用思源宋体 22px */
    lv_style_init(&ew_style_text);
    lv_style_set_text_color(&ew_style_text, EW_COLOR_TEXT);
    lv_style_set_text_font(&ew_style_text, EW_FONT_CN_NORMAL);
    
    /* 次级文本样式 - 使用思源宋体 12px */
    lv_style_init(&ew_style_text_muted);
    lv_style_set_text_color(&ew_style_text_muted, EW_COLOR_TEXT_SUB);
    lv_style_set_text_font(&ew_style_text_muted, EW_FONT_CN_SMALL);
    
    /* 大数值样式 - 使用思源宋体 30px */
    lv_style_init(&ew_style_value);
    lv_style_set_text_color(&ew_style_value, EW_COLOR_PRIMARY);
    lv_style_set_text_font(&ew_style_value, EW_FONT_CN_LARGE);
    
    /* 分割线样式 */
    lv_style_init(&ew_style_divider);
    lv_style_set_bg_color(&ew_style_divider, EW_COLOR_DIVIDER);
    lv_style_set_bg_opa(&ew_style_divider, LV_OPA_COVER);
}

lv_color_t edgewind_get_status_color(uint8_t status)
{
    switch (status) {
        case 0:  return EW_COLOR_SUCCESS;  /* 正常 - 绿色 */
        case 1:  return EW_COLOR_WARNING;  /* 警告 - 黄色 */
        case 2:  return EW_COLOR_DANGER;   /* 故障 - 红色 */
        default: return EW_COLOR_PRIMARY;  /* 默认 - 蓝色 */
    }
}
