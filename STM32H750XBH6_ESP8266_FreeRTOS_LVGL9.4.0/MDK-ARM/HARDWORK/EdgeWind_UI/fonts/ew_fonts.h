/**
 * @file ew_fonts.h
 * @brief EdgeWind UI 字库声明
 * 
 * 使用 GUI Guider 生成的思源宋体 (Source Han Serif SC) 字库
 * 包含项目所需的完整中文字符集
 */

#ifndef EW_FONTS_H
#define EW_FONTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/*******************************************************************************
 * GUI Guider 生成的思源宋体字库声明
 * 
 * lv_font_SourceHanSerifSC_Regular_14 - 14px 思源宋体
 * lv_font_SourceHanSerifSC_Regular_16 - 16px 思源宋体
 * lv_font_SourceHanSerifSC_Regular_20 - 20px 思源宋体
 * 
 * 包含字符: 实时波形、频谱分析、历史记录、系统设置、无线网络、
 *          采样率、上传频率、自动频谱、恢复默认、返回正常、
 *          警告、故障、离线、运行、暂停、主页、示波器、设置、
 *          伏特、通道、增益 等
 ******************************************************************************/

LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_14);
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_16);
LV_FONT_DECLARE(lv_font_SourceHanSerifSC_Regular_20);

/*******************************************************************************
 * 字体别名（方便使用）
 * 
 * 中文使用 GUI Guider 生成的思源宋体，英文数字使用 Montserrat
 ******************************************************************************/

/* 中文字体 */
/* 注意: 14px/16px/20px 字库均包含完整中文字符 */
#define EW_FONT_CN_SMALL    (&lv_font_SourceHanSerifSC_Regular_14)  /* 14px 小字/次级文本 */
#define EW_FONT_CN_NORMAL   (&lv_font_SourceHanSerifSC_Regular_14)  /* 14px 正常/按钮标签 */
#define EW_FONT_CN_TITLE    (&lv_font_SourceHanSerifSC_Regular_16)  /* 16px 标题 */
#define EW_FONT_CN_LARGE    (&lv_font_SourceHanSerifSC_Regular_14)  /* 14px 大字 */
#define EW_FONT_CN_HOME_LABEL (&lv_font_SourceHanSerifSC_Regular_20) /* 20px 主界面底部标签 */

/* 英文/数字字体（使用 Montserrat） */
#define EW_FONT_EN_SMALL    (&lv_font_montserrat_14)                /* 14px 英文小字 */
#define EW_FONT_EN_NORMAL   (&lv_font_montserrat_16)                /* 16px 英文正常 */
#define EW_FONT_EN_TITLE    (&lv_font_montserrat_20)                /* 20px 英文标题 */
#define EW_FONT_EN_LARGE    (&lv_font_montserrat_28)                /* 28px 英文大数值 */

#ifdef __cplusplus
}
#endif

#endif /* EW_FONTS_H */
