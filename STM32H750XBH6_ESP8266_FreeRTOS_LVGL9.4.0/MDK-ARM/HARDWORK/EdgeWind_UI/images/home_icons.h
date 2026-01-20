/**
 * @file home_icons.h
 * @brief 主界面图标声明
 * 
 * 图标映射：
 *   01 实时监控 (rtmon)
 *   02 故障监测 (fault)
 *   03 数据分析 (analysis)
 *   04 历史记录 (history)
 *   05 日志查看 (log)
 *   06 报警设置 (alarm)
 *   07 参数设置 (param)
 *   08 网络配置 (net)
 *   09 服务器配置 (server)
 *   10 系统诊断 (diag)
 *   11 设备管理 (device)
 *   12 用户管理 (user)
 *   13 固件升级 (fwup)
 *   14 关于系统 (about)
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

LV_IMG_DECLARE(icon_01_rtmon);
LV_IMG_DECLARE(icon_02_fault);
LV_IMG_DECLARE(icon_03_analysis);
LV_IMG_DECLARE(icon_04_history);
LV_IMG_DECLARE(icon_05_log);
LV_IMG_DECLARE(icon_06_alarm);
LV_IMG_DECLARE(icon_07_param);
LV_IMG_DECLARE(icon_08_net);
LV_IMG_DECLARE(icon_09_server);
LV_IMG_DECLARE(icon_10_diag);
LV_IMG_DECLARE(icon_11_device);
LV_IMG_DECLARE(icon_12_user);
LV_IMG_DECLARE(icon_13_fwup);
LV_IMG_DECLARE(icon_14_about);

/*******************************************************************************
 * 图标数组（供轮播索引使用）
 ******************************************************************************/

#define HOME_ICON_IMG_COUNT  14

/* 图标图片数组 */
extern const lv_image_dsc_t * const home_icon_images[HOME_ICON_IMG_COUNT];

/* 图标名称数组 */
extern const char * const home_icon_names[HOME_ICON_IMG_COUNT];

#ifdef __cplusplus
}
#endif

#endif /* HOME_ICONS_H */
