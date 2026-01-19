/**
 * @file edgewind_theme.h
 * @brief EdgeWind UI 主题配色与样式定义
 * 
 * 设计风格：Cyberpunk Industrial (赛博工业风)
 * 目标设备：STM32H750 / 800x480 RGB LCD
 */

#ifndef EDGEWIND_THEME_H
#define EDGEWIND_THEME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include "fonts/ew_fonts.h"

/*******************************************************************************
 * 配色方案 (Color Palette)
 ******************************************************************************/

/* 背景色系 */
#define EW_COLOR_BG         lv_color_hex(0x101014)  /* 极深蓝灰，接近纯黑 */
#define EW_COLOR_SURFACE    lv_color_hex(0x1E1E24)  /* 深灰，卡片/容器底色 */
#define EW_COLOR_DIVIDER    lv_color_hex(0x2C2C35)  /* 分割线颜色 */

/* 状态色系 */
#define EW_COLOR_PRIMARY    lv_color_hex(0x00D2FF)  /* 电光蓝 - 正常/待机 */
#define EW_COLOR_WARNING    lv_color_hex(0xFFAB00)  /* 琥珀黄 - 预警 */
#define EW_COLOR_DANGER     lv_color_hex(0xFF2E4D)  /* 霓虹红 - 报警 */
#define EW_COLOR_SUCCESS    lv_color_hex(0x00E676)  /* 荧光绿 - 恢复/正常 */

/* 文本色系 */
#define EW_COLOR_TEXT       lv_color_hex(0xFFFFFF)  /* 主文本 - 纯白 */
#define EW_COLOR_TEXT_SUB   lv_color_hex(0xA0A0A0)  /* 次级文本 - 浅灰 */

/*******************************************************************************
 * 尺寸常量 (Layout Constants)
 ******************************************************************************/

/* 屏幕尺寸 */
#define EW_SCREEN_WIDTH     800
#define EW_SCREEN_HEIGHT    480

/* 状态栏 */
#define EW_STATUSBAR_HEIGHT 40

/* 圆角 */
#define EW_RADIUS_LARGE     16  /* 大卡片 */
#define EW_RADIUS_SMALL     8   /* 按钮/小部件 */

/* 间距 */
#define EW_PAD_GLOBAL       20  /* 全局边距 */
#define EW_PAD_GAP          12  /* 控件间距 */

/*******************************************************************************
 * 全局样式 (Global Styles)
 ******************************************************************************/

/* 样式结构体声明 (使用 ew_ 前缀避免与 LVGL demo 冲突) */
extern lv_style_t ew_style_bg;           /* 背景样式 */
extern lv_style_t ew_style_card;         /* 卡片样式 */
extern lv_style_t ew_style_btn;          /* 按钮默认样式 */
extern lv_style_t ew_style_btn_pressed;  /* 按钮按下样式 */
extern lv_style_t ew_style_title;        /* 标题文本样式 */
extern lv_style_t ew_style_text;         /* 正文文本样式 */
extern lv_style_t ew_style_text_muted;   /* 次级文本样式 */
extern lv_style_t ew_style_value;        /* 大数值样式 */
extern lv_style_t ew_style_divider;      /* 分割线样式 */

/*******************************************************************************
 * 函数声明
 ******************************************************************************/

/**
 * @brief 初始化所有全局样式
 * @note  必须在 lv_init() 之后、创建任何 UI 之前调用
 */
void edgewind_theme_init(void);

/**
 * @brief 获取状态对应的颜色
 * @param status 0=正常, 1=警告, 2=故障
 * @return 对应的颜色值
 */
lv_color_t edgewind_get_status_color(uint8_t status);

#ifdef __cplusplus
}
#endif

#endif /* EDGEWIND_THEME_H */
