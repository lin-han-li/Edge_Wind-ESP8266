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



void setup_scr_screen_2(lv_ui *ui)
{
    //Write codes screen_2
    ui->screen_2 = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_2, 800, 480);
    lv_obj_set_scrollbar_mode(ui->screen_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_cont_1
    ui->screen_2_cont_1 = lv_obj_create(ui->screen_2);
    lv_obj_set_pos(ui->screen_2_cont_1, 0, 0);
    lv_obj_set_size(ui->screen_2_cont_1, 800, 480);
    lv_obj_set_scrollbar_mode(ui->screen_2_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_2_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_2_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_2_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_2_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_2_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_2_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_2_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_2_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_2_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_2_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_chart_1
    ui->screen_2_chart_1 = lv_chart_create(ui->screen_2);
    lv_chart_set_type(ui->screen_2_chart_1, LV_CHART_TYPE_LINE);
    lv_chart_set_div_line_count(ui->screen_2_chart_1, 10, 10);
    lv_chart_set_point_count(ui->screen_2_chart_1, 24);
    lv_chart_set_range(ui->screen_2_chart_1, LV_CHART_AXIS_PRIMARY_Y, 0, 256);
    lv_chart_set_range(ui->screen_2_chart_1, LV_CHART_AXIS_SECONDARY_Y, 0, 256);
    lv_chart_set_axis_tick(ui->screen_2_chart_1, LV_CHART_AXIS_SECONDARY_Y, 16, 8, 5, 5, true, 40);
    lv_chart_set_axis_tick(ui->screen_2_chart_1, LV_CHART_AXIS_PRIMARY_X, 10, 5, 10, 5, true, 40);
    lv_chart_set_zoom_x(ui->screen_2_chart_1, 512);
    lv_chart_set_zoom_y(ui->screen_2_chart_1, 256);
    lv_obj_set_style_size(ui->screen_2_chart_1, 0, LV_PART_INDICATOR);
    ui->screen_2_chart_1_0 = lv_chart_add_series(ui->screen_2_chart_1, lv_color_hex(0x2F35DA), LV_CHART_AXIS_PRIMARY_Y);
#if LV_USE_FREEMASTER == 0
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 1);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 20);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 30);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 40);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 5);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 256);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 15);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 10);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 10);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 20);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_0, 0);
#endif
    ui->screen_2_chart_1_1 = lv_chart_add_series(ui->screen_2_chart_1, lv_color_hex(0x3dfb00), LV_CHART_AXIS_PRIMARY_Y);
#if LV_USE_FREEMASTER == 0
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 15);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 20);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 25);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 10);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
    lv_chart_set_next_value(ui->screen_2_chart_1, ui->screen_2_chart_1_1, 0);
#endif
    lv_obj_set_pos(ui->screen_2_chart_1, 0, 0);
    lv_obj_set_size(ui->screen_2_chart_1, 600, 450);
    lv_obj_set_scrollbar_mode(ui->screen_2_chart_1, LV_SCROLLBAR_MODE_ACTIVE);

    //Write style for screen_2_chart_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_chart_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_chart_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_chart_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_2_chart_1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->screen_2_chart_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->screen_2_chart_1, lv_color_hex(0xe8e8e8), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->screen_2_chart_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_chart_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui->screen_2_chart_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->screen_2_chart_1, lv_color_hex(0xe8e8e8), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->screen_2_chart_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_chart_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_2_chart_1, Part: LV_PART_TICKS, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->screen_2_chart_1, lv_color_hex(0xff6500), LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_chart_1, &lv_font_SourceHanSerifSC_Regular_12, LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_chart_1, 255, LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui->screen_2_chart_1, 2, LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->screen_2_chart_1, lv_color_hex(0x0016ff), LV_PART_TICKS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->screen_2_chart_1, 255, LV_PART_TICKS|LV_STATE_DEFAULT);

    //Write codes screen_2_btn_1
    ui->screen_2_btn_1 = lv_btn_create(ui->screen_2);
    ui->screen_2_btn_1_label = lv_label_create(ui->screen_2_btn_1);
    lv_label_set_text(ui->screen_2_btn_1_label, "返回");
    lv_label_set_long_mode(ui->screen_2_btn_1_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_2_btn_1_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_2_btn_1, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_2_btn_1_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_2_btn_1, 650, 4);
    lv_obj_set_size(ui->screen_2_btn_1, 100, 50);

    //Write style for screen_2_btn_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_btn_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_btn_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_2_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_btn_1, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_btn_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_btn_1, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_btn_1, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_btn_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_btn_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_btn_2
    ui->screen_2_btn_2 = lv_btn_create(ui->screen_2);
    ui->screen_2_btn_2_label = lv_label_create(ui->screen_2_btn_2);
    lv_label_set_text(ui->screen_2_btn_2_label, "频率键盘");
    lv_label_set_long_mode(ui->screen_2_btn_2_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_2_btn_2_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_2_btn_2, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_2_btn_2_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_2_btn_2, 641, 385);
    lv_obj_set_size(ui->screen_2_btn_2, 124, 50);

    //Write style for screen_2_btn_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_btn_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_btn_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_2_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_btn_2, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_btn_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_btn_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_btn_2, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_btn_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_btn_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_label_1
    ui->screen_2_label_1 = lv_label_create(ui->screen_2);
    lv_label_set_text(ui->screen_2_label_1, "2.00VPP");
    lv_label_set_long_mode(ui->screen_2_label_1, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_2_label_1, 462, 17);
    lv_obj_set_size(ui->screen_2_label_1, 130, 34);

    //Write style for screen_2_label_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_label_1, lv_color_hex(0x3dfb00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_label_1, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_label_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_label_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_label_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_slider_1
    ui->screen_2_slider_1 = lv_slider_create(ui->screen_2);
    lv_slider_set_range(ui->screen_2_slider_1, 127, 255);
    lv_slider_set_mode(ui->screen_2_slider_1, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_2_slider_1, 150, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_2_slider_1, 620, 120);
    lv_obj_set_size(ui->screen_2_slider_1, 170, 20);

    //Write style for screen_2_slider_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_slider_1, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_slider_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_slider_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_slider_1, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_2_slider_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_slider_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_2_slider_1, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_slider_1, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_slider_1, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_slider_1, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_slider_1, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for screen_2_slider_1, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_slider_1, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_slider_1, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_slider_1, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_slider_1, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes screen_2_slider_2
    ui->screen_2_slider_2 = lv_slider_create(ui->screen_2);
    lv_slider_set_range(ui->screen_2_slider_2, 0, 10000);
    lv_slider_set_mode(ui->screen_2_slider_2, LV_SLIDER_MODE_NORMAL);
    lv_slider_set_value(ui->screen_2_slider_2, 150, LV_ANIM_OFF);
    lv_obj_set_pos(ui->screen_2_slider_2, 620, 340);
    lv_obj_set_size(ui->screen_2_slider_2, 170, 20);

    //Write style for screen_2_slider_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_slider_2, 60, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_slider_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_slider_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_slider_2, 8, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_outline_width(ui->screen_2_slider_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_slider_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_2_slider_2, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_slider_2, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_slider_2, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_slider_2, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_slider_2, 8, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for screen_2_slider_2, Part: LV_PART_KNOB, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_slider_2, 255, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_slider_2, lv_color_hex(0x2195f6), LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_slider_2, LV_GRAD_DIR_NONE, LV_PART_KNOB|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_slider_2, 8, LV_PART_KNOB|LV_STATE_DEFAULT);

    //Write codes screen_2_label_2
    ui->screen_2_label_2 = lv_label_create(ui->screen_2);
    lv_label_set_text(ui->screen_2_label_2, "幅度控制");
    lv_label_set_long_mode(ui->screen_2_label_2, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_2_label_2, 646, 74);
    lv_obj_set_size(ui->screen_2_label_2, 100, 32);

    //Write style for screen_2_label_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_label_2, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_label_2, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_label_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_label_2, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_label_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_label_3
    ui->screen_2_label_3 = lv_label_create(ui->screen_2);
    lv_label_set_text(ui->screen_2_label_3, "频率控制");
    lv_label_set_long_mode(ui->screen_2_label_3, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_2_label_3, 650, 290);
    lv_obj_set_size(ui->screen_2_label_3, 100, 32);

    //Write style for screen_2_label_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_label_3, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_label_3, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_label_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_label_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_label_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_label_4
    ui->screen_2_label_4 = lv_label_create(ui->screen_2);
    lv_label_set_text(ui->screen_2_label_4, "PGA:150");
    lv_label_set_long_mode(ui->screen_2_label_4, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_2_label_4, 24, 408);
    lv_obj_set_size(ui->screen_2_label_4, 118, 32);

    //Write style for screen_2_label_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_label_4, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_label_4, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_label_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_label_4, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_label_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_label_5
    ui->screen_2_label_5 = lv_label_create(ui->screen_2);
    lv_label_set_text(ui->screen_2_label_5, "1000kHz");
    lv_label_set_long_mode(ui->screen_2_label_5, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(ui->screen_2_label_5, 506, 413);
    lv_obj_set_size(ui->screen_2_label_5, 143, 32);

    //Write style for screen_2_label_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_label_5, lv_color_hex(0xff6500), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_label_5, &lv_font_SourceHanSerifSC_Regular_22, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_label_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_label_5, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_label_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_2_btn_3
    ui->screen_2_btn_3 = lv_btn_create(ui->screen_2);
    ui->screen_2_btn_3_label = lv_label_create(ui->screen_2_btn_3);
    lv_label_set_text(ui->screen_2_btn_3_label, "幅度键盘");
    lv_label_set_long_mode(ui->screen_2_btn_3_label, LV_LABEL_LONG_WRAP);
    lv_obj_align(ui->screen_2_btn_3_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_all(ui->screen_2_btn_3, 0, LV_STATE_DEFAULT);
    lv_obj_set_width(ui->screen_2_btn_3_label, LV_PCT(100));
    lv_obj_set_pos(ui->screen_2_btn_3, 646, 166);
    lv_obj_set_size(ui->screen_2_btn_3, 124, 50);

    //Write style for screen_2_btn_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_2_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_2_btn_3, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_2_btn_3, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->screen_2_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_2_btn_3, 5, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_2_btn_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_2_btn_3, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_2_btn_3, &lv_font_SourceHanSerifSC_Regular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_2_btn_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_2_btn_3, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_2.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_2);

    //Init events for screen.
    events_init_screen_2(ui);
}
