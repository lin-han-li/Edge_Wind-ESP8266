/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen_3(lv_ui *ui)
{
    //Write codes screen_3
    ui->screen_3 = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_3, 800, 480);
    lv_obj_set_scrollbar_mode(ui->screen_3, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_3_cont_1
    ui->screen_3_cont_1 = lv_obj_create(ui->screen_3);
    lv_obj_set_pos(ui->screen_3_cont_1, 0, 0);
    lv_obj_set_size(ui->screen_3_cont_1, 800, 480);
    lv_obj_set_scrollbar_mode(ui->screen_3_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_3_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_3_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_3_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_3_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_3_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_3_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_3_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_3_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_3_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_3_label_1
    ui->screen_3_label_1 = lv_label_create(ui->screen_3);
    lv_label_set_text(ui->screen_3_label_1, "频率值:1000kHz");
    lv_label_set_long_mode(ui->screen_3_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_3_label_1, 450, 100);
    lv_obj_set_size(ui->screen_3_label_1, 300, 30);

    //Write style for screen_3_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_3_label_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_3_label_1, &lv_font_SourceHanSerifSC_Regular_28, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_3_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_3_label_1, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_3_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_3_label_1, lv_color_hex(0x2FDAAE), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_3_label_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_3_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_3.
    //   /* 定义并创建键盘 */
//   ui->screen_3_Fre_keyboard= lv_keyboard_create(ui->screen_3);

//   /* 定义并创建文本框 */
//   ui->screen_3_Fre_textarea = lv_textarea_create(ui->screen_3);

//   /* 设置文本框宽度 */

//   lv_obj_set_width(ui->screen_3_Fre_textarea, scr_act_width() - 10);

//   /* 设置文本框高度 */

//   lv_obj_set_height(ui->screen_3_Fre_textarea, (scr_act_height() >> 1) - 10);
//   lv_obj_align(ui->screen_3_Fre_textarea, LV_ALIGN_TOP_LEFT, 0, 0);

//   /* 为键盘指定一个文本区域 */
//   lv_keyboard_set_textarea(ui->screen_3_Fre_keyboard, ui->screen_3_Fre_textarea);

//   /* 开启按键弹窗 */
//   lv_keyboard_set_popovers(ui->screen_3_Fre_keyboard, true);

    //Update current screen layout.
    lv_obj_update_layout(ui->screen_3);

    //Init events for screen.
    events_init_screen_3(ui);
}
