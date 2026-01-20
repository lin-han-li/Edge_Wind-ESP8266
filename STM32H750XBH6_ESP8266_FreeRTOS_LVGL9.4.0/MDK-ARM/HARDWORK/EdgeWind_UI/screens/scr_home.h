/**
 * @file scr_home.h
 * @brief 主界面 - 完全参考 DONGHUA 的图标轮播实现
 * 
 * 功能特性：
 * - 14个应用图标（使用 lv_image）
 * - 左右滑动/按钮切换
 * - 中心图标放大(325)、两侧缩小(255)的轮播动画
 * - 底部显示图标名称
 * 
 * 动画参数（完全匹配 DONGHUA pw.c）：
 * - IMG_WIDTH = 100, IMG_HEIGHT = 100
 * - IMG_GAP = 50
 * - AniTime = 300ms
 * - ImgZoom = 325 (中心放大), 255 (正常)
 */

#ifndef SCR_HOME_H
#define SCR_HOME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"
#include <stdbool.h>
#include <stdint.h>

/*******************************************************************************
 * 常量定义 - 完全匹配 DONGHUA pw.c
 ******************************************************************************/

#define HOME_ICON_COUNT             14      /* 图标总数（匹配 DONGHUA IMG_COUNT） */

/* 轮播配置 - 完全匹配 DONGHUA */
#define HOME_CAROUSEL_CARD_W        600     /* 轮播卡片宽度（加大） */
#define HOME_CAROUSEL_CARD_H        260     /* 轮播卡片高度（加大） */
#define HOME_CAROUSEL_ICON_SIZE     100     /* 图标尺寸 IMG_WIDTH/IMG_HEIGHT */
#define HOME_CAROUSEL_ICON_GAP      50      /* 图标间距 IMG_GAP */
#define HOME_CAROUSEL_ANIM_TIME     300     /* 动画时长 AniTime (ms) */
#define HOME_CAROUSEL_ZOOM_FOCUS    325     /* 中心图标缩放 ImgZoom */
#define HOME_CAROUSEL_ZOOM_NORMAL   255     /* 非中心图标缩放 (256=100%) */
#define HOME_CAROUSEL_SWIPE_THRESHOLD 30    /* 滑动触发阈值(px) */
#define HOME_TAP_THRESHOLD          3       /* 轻点容差(px)：超出视为滑动 */

/* 颜色定义 - 与 DONGHUA 一致 */
#define HOME_COLOR_BG           lv_color_hex(0x050505)  /* 深黑背景 */
#define HOME_COLOR_BORDER       lv_color_hex(0x2195f6)  /* 蓝色边框 */
#define HOME_COLOR_CARD_BG      lv_color_hex(0xebc0c0)  /* 卡片背景（可换成图片） */
#define HOME_COLOR_LABEL        lv_color_hex(0xFFFF00)  /* 黄色标签文字 */

/*******************************************************************************
 * 主界面数据结构
 ******************************************************************************/

typedef struct {
    lv_obj_t *screen;

    /* 轮播容器 - 匹配 DONGHUA screen_1_cont_1 */
    lv_obj_t *carousel_card;
    
    /* 图标（lv_image 对象）- 匹配 DONGHUA imgs[] */
    lv_obj_t *icons[HOME_ICON_COUNT];

    /* 底部标签 - 匹配 DONGHUA screen_1_img_label */
    lv_obj_t *name_label;

    /* 状态 */
    int       current_index;        /* 当前中心图标索引 */
    bool      animating;
    lv_timer_t *anim_timer;
    lv_coord_t gesture_start_x;
    bool      gesture_active;
    bool      click_blocked;
    lv_coord_t icon_press_start_x;
    lv_coord_t icon_press_start_y;
} ew_home_t;

extern ew_home_t ew_home;

/*******************************************************************************
 * 函数声明
 ******************************************************************************/

/**
 * @brief 创建主界面
 */
void ew_home_create(void);

/**
 * @brief 加载主界面（带切换动画）
 */
void ew_home_load(void);

/**
 * @brief 更新图标位置和缩放（匹配 DONGHUA update_images）
 * @param dir 方向：0=左移，1=右移
 */
void ew_home_update_images(int dir);

#ifdef __cplusplus
}
#endif

#endif /* SCR_HOME_H */
