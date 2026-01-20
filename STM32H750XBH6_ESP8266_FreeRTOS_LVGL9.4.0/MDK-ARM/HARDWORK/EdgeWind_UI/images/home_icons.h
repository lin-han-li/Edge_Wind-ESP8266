/**
 * @file home_icons.h
 * @brief 主界面图标声明
 * 
 * 图标映射：
 *   icon_1 - BiliBili
 *   icon_2 - 原神 (Yuansheng)
 *   icon_3 - 手表 (Wtch)
 *   icon_4 - QQ音乐 (QQMusic)
 * 
 * 替换图标方法：
 *   1. 使用 LVGL Image Converter 生成新的 .c 文件
 *   2. 将变量名改为 icon_1 / icon_2 / icon_3 / icon_4
 *   3. 替换对应的 icon_X.c 文件
 */

#ifndef HOME_ICONS_H
#define HOME_ICONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/*******************************************************************************
 * 图标声明
 ******************************************************************************/

LV_IMG_DECLARE(icon_1);  /* BiliBili */
LV_IMG_DECLARE(icon_2);  /* Yuansheng (原神) */
LV_IMG_DECLARE(icon_3);  /* Wtch (手表) */
LV_IMG_DECLARE(icon_4);  /* QQMusic (QQ音乐) */

/*******************************************************************************
 * 图标数组（供轮播索引使用）
 ******************************************************************************/

#define HOME_ICON_IMG_COUNT  4

/* 图标图片数组 */
extern const lv_image_dsc_t * const home_icon_images[HOME_ICON_IMG_COUNT];

/* 图标名称数组 */
extern const char * const home_icon_names[HOME_ICON_IMG_COUNT];

#ifdef __cplusplus
}
#endif

#endif /* HOME_ICONS_H */
