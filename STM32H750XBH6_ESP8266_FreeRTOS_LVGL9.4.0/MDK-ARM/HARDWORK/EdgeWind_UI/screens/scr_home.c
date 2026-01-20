/**
 * @file scr_home.c
 * @brief 主界面实现 - 完全参考 DONGHUA 的 setup_scr_screen_1.c 和 pw.c
 * 
 * 动画效果：
 * - 4个图标横向排列
 * - 中心图标放大到 325 (约127%)
 * - 非中心图标缩放为 255 (约99.6%)
 * - 左右滑动/按钮切换，带平滑动画
 * - 底部显示当前图标名称
 */

#include "scr_home.h"
#include "../edgewind_theme.h"
#include "../images/home_icons.h"
#include "../fonts/ew_fonts.h"

/*******************************************************************************
 * 全局变量
 ******************************************************************************/

ew_home_t ew_home = {0};

/*******************************************************************************
 * 图标数据 - 定义图标数组
 ******************************************************************************/

/* 图标图片数组 */
const lv_image_dsc_t * const home_icon_images[HOME_ICON_COUNT] = {
    &icon_1,    /* BiliBili */
    &icon_2,    /* Yuansheng (原神) */
    &icon_3,    /* Wtch (手表) */
    &icon_4,    /* QQMusic (QQ音乐) */
};

/* 图标名称数组 */
const char * const home_icon_names[HOME_ICON_COUNT] = {
    "BiliBili",
    "Yuansheng",
    "Watch",
    "QQMusic",
};

/* 图标中文名称数组（用于底部当前项标签） */
static const char * const home_icon_names_cn[HOME_ICON_COUNT] = {
    "设备管理",
    "网络配置",
    "故障监测",
    "服务器配置",
};

/*******************************************************************************
 * 内部函数声明
 ******************************************************************************/

static void create_carousel(void);
static void carousel_gesture_cb(lv_event_t *e);
static void icon_click_cb(lv_event_t *e);
static void anim_timer_cb(lv_timer_t *timer);
static void home_label_show_for_index(int idx, bool animate);

/* 单一底部标签：记录上次显示的索引，避免重复动画造成闪烁 */
static int g_label_last_index = -1;

/*******************************************************************************
 * LVGL 9.x 缩放函数适配
 * 
 * DONGHUA 使用 lv_img_set_zoom() (LVGL 8.x)
 * LVGL 9.x 中改为 lv_image_set_scale()
 ******************************************************************************/

static void anim_set_zoom(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (!obj) return;
    lv_image_set_scale(obj, (uint32_t)v);
}

static int32_t get_img_zoom(lv_obj_t *obj)
{
    if (!obj) return 256;
    return (int32_t)lv_image_get_scale(obj);
}

static void anim_set_label_opa(void *var, int32_t v)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    if (!obj) return;
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    lv_obj_set_style_opa(obj, (lv_opa_t)v, 0);
}

static void home_label_show_for_index(int idx, bool animate)
{
    if (idx < 0 || idx >= HOME_ICON_COUNT) return;
    lv_obj_t *lab = ew_home.name_label;
    if (!lab) return;

    const bool same_index = (g_label_last_index == idx);
    if (!same_index) {
        lv_label_set_text(lab, home_icon_names_cn[idx]);
        g_label_last_index = idx;
    }

    /* 标签位置：固定在卡片底部居中（像 DONGHUA 一样）。
     * 说明：图标 x 在动画过程中会变化，若此时用 icon 的实时 x 来定位，会导致标签“跑偏”。
     * 固定居中能保证永远稳定，并且与“选中才显示对应标签”的体验一致。
     */
    lv_obj_clear_flag(lab, LV_OBJ_FLAG_HIDDEN);
    lv_obj_move_foreground(lab);
    lv_obj_align(lab, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_coord_t y_to = lv_obj_get_y(lab);

    lv_obj_set_style_text_opa(lab, LV_OPA_COVER, 0);
    lv_obj_set_style_opa(lab, LV_OPA_COVER, 0);

    /* 已经显示且未切换索引：不重复动画，避免闪烁 */
    if (!animate || same_index) return;

    /* “上划 + 淡入”魔法：y: y+10 -> y，opa: 0 -> 255 */
    lv_obj_set_style_opa(lab, LV_OPA_0, 0);
    lv_obj_set_y(lab, y_to + 10);

    lv_anim_delete(lab, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_t a_y;
    lv_anim_init(&a_y);
    lv_anim_set_var(&a_y, lab);
    lv_anim_set_values(&a_y, y_to + 10, y_to);
    lv_anim_set_time(&a_y, HOME_CAROUSEL_ANIM_TIME);
    lv_anim_set_path_cb(&a_y, lv_anim_path_linear);
    lv_anim_set_exec_cb(&a_y, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_start(&a_y);

    lv_anim_delete(lab, (lv_anim_exec_xcb_t)anim_set_label_opa);
    lv_anim_t a_opa;
    lv_anim_init(&a_opa);
    lv_anim_set_var(&a_opa, lab);
    lv_anim_set_values(&a_opa, 0, 255);
    lv_anim_set_time(&a_opa, HOME_CAROUSEL_ANIM_TIME);
    lv_anim_set_path_cb(&a_opa, lv_anim_path_linear);
    lv_anim_set_exec_cb(&a_opa, (lv_anim_exec_xcb_t)anim_set_label_opa);
    lv_anim_start(&a_opa);
}

/*******************************************************************************
 * 公共函数实现
 ******************************************************************************/

void ew_home_create(void)
{
    /* 创建屏幕 */
    ew_home.screen = lv_obj_create(NULL);
    lv_obj_set_size(ew_home.screen, EW_SCREEN_WIDTH, EW_SCREEN_HEIGHT);
    /* 主界面全屏背景，遮住上一界面 */
    lv_obj_set_style_bg_color(ew_home.screen, HOME_COLOR_BG, 0);
    lv_obj_set_style_bg_opa(ew_home.screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ew_home.screen, LV_OBJ_FLAG_SCROLLABLE);
    
    /* 初始化状态 - 匹配 DONGHUA: current_index = IMG_COUNT/2 = 2 */
    ew_home.current_index = HOME_ICON_COUNT / 2;
    ew_home.animating = false;
    ew_home.anim_timer = NULL;
    ew_home.gesture_active = false;
    ew_home.gesture_start_x = 0;
    g_label_last_index = -1;
    
    /* 创建轮播容器和图标 */
    create_carousel();
}

void ew_home_load(void)
{
    /* 带淡入动画切换到主界面 */
    lv_scr_load_anim(ew_home.screen, LV_SCR_LOAD_ANIM_FADE_IN, 500, 0, true);
}

/**
 * @brief 更新图标位置和缩放 - 完全匹配 DONGHUA pw.c 的 update_images()
 * @param dir 方向：0=左移(current_index--)，1=右移(current_index++)
 */
void ew_home_update_images(int dir)
{
    /* 更新 current_index */
    if (dir) {
        /* 右移 */
        if (ew_home.current_index < HOME_ICON_COUNT - 1) {
            ew_home.current_index++;
        } else {
            ew_home.current_index = 0;  /* 循环 */
        }
    } else {
        /* 左移 */
        if (ew_home.current_index > 0) {
            ew_home.current_index--;
        } else {
            ew_home.current_index = HOME_ICON_COUNT - 1;  /* 循环 */
        }
    }
    
    /* 对每个图标应用动画 */
    for (int i = 0; i < HOME_ICON_COUNT; i++) {
        int offset = i - ew_home.current_index;
        
        /* 缩放动画 - 匹配 DONGHUA */
        int target_zoom = (i == ew_home.current_index) ? 
                          HOME_CAROUSEL_ZOOM_FOCUS : HOME_CAROUSEL_ZOOM_NORMAL;
        
        lv_anim_t zoom_anim;
        lv_anim_init(&zoom_anim);
        lv_anim_set_var(&zoom_anim, ew_home.icons[i]);
        lv_anim_set_values(&zoom_anim, get_img_zoom(ew_home.icons[i]), target_zoom);
        lv_anim_set_time(&zoom_anim, HOME_CAROUSEL_ANIM_TIME);
        lv_anim_set_path_cb(&zoom_anim, lv_anim_path_linear);
        lv_anim_set_exec_cb(&zoom_anim, anim_set_zoom);
        lv_anim_start(&zoom_anim);
        
        /* 位置动画 - 完全匹配 DONGHUA 公式 */
        lv_coord_t target_x = (HOME_CAROUSEL_CARD_W / 2) + 
                              offset * (HOME_CAROUSEL_ICON_SIZE + HOME_CAROUSEL_ICON_GAP) - 
                              (HOME_CAROUSEL_ICON_SIZE / 2);
        
        lv_anim_t move_anim;
        lv_anim_init(&move_anim);
        lv_anim_set_var(&move_anim, ew_home.icons[i]);
        lv_anim_set_values(&move_anim, lv_obj_get_x(ew_home.icons[i]), target_x);
        lv_anim_set_time(&move_anim, HOME_CAROUSEL_ANIM_TIME);
        lv_anim_set_path_cb(&move_anim, lv_anim_path_linear);
        lv_anim_set_exec_cb(&move_anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
        lv_anim_start(&move_anim);
    }
    
    /* 更新选中图标的中文标签（只显示一个） */
    home_label_show_for_index(ew_home.current_index, true);
    
    /* 设置动画锁定 */
    ew_home.animating = true;
    if (ew_home.anim_timer) {
        lv_timer_del(ew_home.anim_timer);
    }
    ew_home.anim_timer = lv_timer_create(anim_timer_cb, HOME_CAROUSEL_ANIM_TIME + 50, NULL);
    lv_timer_set_repeat_count(ew_home.anim_timer, 1);
}

/*******************************************************************************
 * 内部函数实现
 ******************************************************************************/

/**
 * @brief 创建轮播容器和图标 - 匹配 DONGHUA setup_scr_screen_1.c + create_gui()
 */
static void create_carousel(void)
{
    /* 轮播容器 - 匹配 DONGHUA screen_1_cont_1 */
    ew_home.carousel_card = lv_obj_create(ew_home.screen);
    lv_obj_set_pos(ew_home.carousel_card, 
                   (EW_SCREEN_WIDTH - HOME_CAROUSEL_CARD_W) / 2,
                   (EW_SCREEN_HEIGHT - HOME_CAROUSEL_CARD_H) / 2 - 40);
    lv_obj_set_size(ew_home.carousel_card, HOME_CAROUSEL_CARD_W, HOME_CAROUSEL_CARD_H);
    lv_obj_set_scrollbar_mode(ew_home.carousel_card, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(ew_home.carousel_card, LV_OBJ_FLAG_SCROLLABLE);
    
    /* 容器样式 - 匹配 DONGHUA */
    lv_obj_set_style_border_width(ew_home.carousel_card, 2, 0);
    lv_obj_set_style_border_opa(ew_home.carousel_card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(ew_home.carousel_card, HOME_COLOR_BORDER, 0);
    lv_obj_set_style_radius(ew_home.carousel_card, 0, 0);
    lv_obj_set_style_bg_opa(ew_home.carousel_card, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(ew_home.carousel_card, lv_color_hex(0x121218), 0);
    lv_obj_set_style_pad_all(ew_home.carousel_card, 0, 0);
    lv_obj_set_style_shadow_width(ew_home.carousel_card, 0, 0);
    
    /* 手势事件 */
    lv_obj_add_flag(ew_home.carousel_card, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ew_home.carousel_card, carousel_gesture_cb, LV_EVENT_ALL, NULL);
    
    /* 创建 4 个图标 - 匹配 DONGHUA screen_1_img_1 ~ screen_1_img_4 */
    for (int i = 0; i < HOME_ICON_COUNT; i++) {
        ew_home.icons[i] = lv_image_create(ew_home.carousel_card);
        lv_obj_add_flag(ew_home.icons[i], LV_OBJ_FLAG_CLICKABLE);
        lv_image_set_src(ew_home.icons[i], home_icon_images[i]);
        lv_image_set_pivot(ew_home.icons[i], HOME_CAROUSEL_ICON_SIZE / 2, 
                                              HOME_CAROUSEL_ICON_SIZE / 2);
        lv_obj_set_size(ew_home.icons[i], HOME_CAROUSEL_ICON_SIZE, HOME_CAROUSEL_ICON_SIZE);
        
        /* 图标样式 - 匹配 DONGHUA */
        lv_obj_set_style_image_recolor_opa(ew_home.icons[i], 0, 0);
        lv_obj_set_style_image_opa(ew_home.icons[i], LV_OPA_COVER, 0);
        lv_obj_set_style_radius(ew_home.icons[i], 0, 0);
        lv_obj_set_style_clip_corner(ew_home.icons[i], true, 0);
        
        /* 点击事件 */
        lv_obj_set_user_data(ew_home.icons[i], (void *)(intptr_t)i);
        lv_obj_add_event_cb(ew_home.icons[i], icon_click_cb, LV_EVENT_CLICKED, NULL);
        
        /* 事件冒泡 - 让手势能传递到父容器 */
        lv_obj_add_flag(ew_home.icons[i], LV_OBJ_FLAG_EVENT_BUBBLE);
    }

    /* 单一底部标签 - 匹配 DONGHUA screen_1_img_label */
    ew_home.name_label = lv_label_create(ew_home.carousel_card);
    lv_label_set_text(ew_home.name_label, "");
    lv_label_set_long_mode(ew_home.name_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_size(ew_home.name_label, 200, 32);

    lv_obj_set_style_border_width(ew_home.name_label, 0, 0);
    lv_obj_set_style_radius(ew_home.name_label, 0, 0);
    lv_obj_set_style_text_color(ew_home.name_label, HOME_COLOR_LABEL, 0);
    /* 20px 字库已生成，底部标签使用更大字体 */
    lv_obj_set_style_text_font(ew_home.name_label, EW_FONT_CN_HOME_LABEL, 0);
    lv_obj_set_style_text_opa(ew_home.name_label, LV_OPA_COVER, 0);
    lv_obj_set_style_text_align(ew_home.name_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(ew_home.name_label, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(ew_home.name_label, 0, 0);
    lv_obj_set_style_shadow_width(ew_home.name_label, 0, 0);
    lv_obj_align(ew_home.name_label, LV_ALIGN_BOTTOM_MID, 0, -20);
    
    /* 初始化图标位置和缩放 - 匹配 DONGHUA create_gui() */
    const int icon_y = HOME_CAROUSEL_CARD_H / 4;
    
    for (int i = 0; i < HOME_ICON_COUNT; i++) {
        int offset = i - ew_home.current_index;
        
        /* 计算初始位置 - 匹配 DONGHUA 公式 */
        lv_coord_t pos_x = (HOME_CAROUSEL_CARD_W / 2) + 
                           offset * (HOME_CAROUSEL_ICON_SIZE + HOME_CAROUSEL_ICON_GAP) - 
                           (HOME_CAROUSEL_ICON_SIZE / 2);
        
        lv_obj_set_pos(ew_home.icons[i], pos_x, icon_y);
        
        /* 设置初始缩放 - 匹配 DONGHUA */
        int initial_zoom = (i == ew_home.current_index) ? 
                           HOME_CAROUSEL_ZOOM_FOCUS : HOME_CAROUSEL_ZOOM_NORMAL;
        lv_image_set_scale(ew_home.icons[i], initial_zoom);
    }

    /* 初次进入：显示当前选中图标的标签（带上划+淡入） */
    home_label_show_for_index(ew_home.current_index, true);
}

/**
 * @brief 手势事件回调 - 处理滑动
 */
static void carousel_gesture_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    
    /* 获取输入设备 */
    lv_indev_t *indev = lv_event_get_indev(e);
    if (!indev) {
        indev = lv_indev_get_act();
    }
    if (!indev) return;
    
    /* 手动滑动检测 */
    if (code == LV_EVENT_PRESSED) {
        lv_point_t p;
        lv_indev_get_point(indev, &p);
        ew_home.gesture_start_x = p.x;
        ew_home.gesture_active = true;
        return;
    }

    /* 轻滑时不隐藏标签：保证“当前图标标签始终可见” */
    if (code == LV_EVENT_PRESSING) {
        return;
    }
    
    if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        bool did_switch = false;
        if (ew_home.gesture_active && !ew_home.animating) {
            lv_point_t p;
            lv_indev_get_point(indev, &p);
            lv_coord_t delta = p.x - ew_home.gesture_start_x;
            
            if (delta > HOME_CAROUSEL_SWIPE_THRESHOLD) {
                /* 右滑 - 向左移动（current_index--） */
                ew_home_update_images(0);
                did_switch = true;
            } else if (delta < -HOME_CAROUSEL_SWIPE_THRESHOLD) {
                /* 左滑 - 向右移动（current_index++） */
                ew_home_update_images(1);
                did_switch = true;
            } else {
                /* no-op */
            }
        }
        ew_home.gesture_active = false;

        /* 无论是否切换，只要松手就保证当前选中项的标签可见 */
        if(!did_switch) home_label_show_for_index(ew_home.current_index, true);
        return;
    }
    
    /* LVGL 手势识别（如果启用） */
    if (code == LV_EVENT_GESTURE && !ew_home.animating) {
        lv_dir_t dir = lv_indev_get_gesture_dir(indev);
        if (dir == LV_DIR_LEFT) {
            ew_home_update_images(1);  /* 左滑 - 向右移动 */
        } else if (dir == LV_DIR_RIGHT) {
            ew_home_update_images(0);  /* 右滑 - 向左移动 */
        } else {
            /* 非水平手势/未识别方向：强制恢复当前标签 */
            home_label_show_for_index(ew_home.current_index, true);
        }
    }
}

/**
 * @brief 图标点击事件回调
 */
static void icon_click_cb(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_target(e);
    int icon_index = (int)(intptr_t)lv_obj_get_user_data(obj);
    
    LV_LOG_USER("Icon %d clicked: %s", icon_index, home_icon_names[icon_index]);
    
    /* TODO: 在这里添加界面跳转逻辑 */
    /* 例如：
     * switch(icon_index) {
     *     case 0: load_bilibili_screen(); break;
     *     case 1: load_yuansheng_screen(); break;
     *     case 2: load_watch_screen(); break;
     *     case 3: load_qqmusic_screen(); break;
     * }
     */
}

/**
 * @brief 动画完成定时器回调
 */
static void anim_timer_cb(lv_timer_t *timer)
{
    LV_UNUSED(timer);
    ew_home.animating = false;
    if (ew_home.anim_timer) {
        lv_timer_del(ew_home.anim_timer);
        ew_home.anim_timer = NULL;
    }

    /* 动画结束最终兜底：确保当前标签可见（必须上划淡入；show_for_index 内部去重） */
    home_label_show_for_index(ew_home.current_index, true);
}
