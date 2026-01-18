/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void screen_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_1, guider_ui.screen_1_del, &guider_ui.screen_del, setup_scr_screen_1, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
        //进入全局滑块，非示波器，非键盘
        //extern int shiboqi_flag,slider_flag;
        //Fre_keyboard_flag=0;
        //shiboqi_flag=0;
        //slider_flag=1;
        break;
    }
    default:
        break;
    }
}

static void screen_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_4, guider_ui.screen_4_del, &guider_ui.screen_del, setup_scr_screen_4, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
        //进入幅度键盘,非示波器，非全局滑块
        //extern int PGA_keyboard_flag,shiboqi_flag,slider_flag;
        //PGA_keyboard_flag=1;
        //shiboqi_flag=0;
        //slider_flag=0;
        break;
    }
    default:
        break;
    }
}

static void screen_btn_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_3, guider_ui.screen_3_del, &guider_ui.screen_del, setup_scr_screen_3, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
        //进入频率键盘,非示波器，非全局滑块
        //Fre_keyboard_flag=1;
        //shiboqi_flag=0;
        //slider_flag=0;
        break;
    }
    default:
        break;
    }
}

void events_init_screen (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_btn_1, screen_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_btn_2, screen_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_btn_3, screen_btn_3_event_handler, LV_EVENT_ALL, ui);
}

static void screen_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_1_del, setup_scr_screen, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
            //非键盘,非示波器，非全局滑块
            //Fre_keyboard_flag=0;
            //shiboqi_flag=0;
            //slider_flag=0;

            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void screen_1_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_1_del, setup_scr_screen, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
        //非键盘,非示波器，非全局滑块
        //Fre_keyboard_flag=0;
        //shiboqi_flag=0;
        //slider_flag=0;
        break;
    }
    default:
        break;
    }
}

static void screen_1_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_2, guider_ui.screen_2_del, &guider_ui.screen_1_del, setup_scr_screen_2, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
        //非键盘,进入示波器，非全局滑块
        //Fre_keyboard_flag=0;
        //shiboqi_flag=1;
        //slider_flag=0;

        break;
    }
    default:
        break;
    }
}

static void screen_1_slider_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_PRESSING:
    {
        //全局滑块按下
        break;
    }
    default:
        break;
    }
}

void events_init_screen_1 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_1, screen_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_1_btn_1, screen_1_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_1_btn_2, screen_1_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_1_slider_1, screen_1_slider_1_event_handler, LV_EVENT_ALL, ui);
}

static void screen_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen_1, guider_ui.screen_1_del, &guider_ui.screen_2_del, setup_scr_screen_1, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
            //非键盘,非示波器，进入全局滑块
            //shiboqi_flag=0;
            //slider_flag=1;
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

static void screen_2_btn_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_1, guider_ui.screen_1_del, &guider_ui.screen_2_del, setup_scr_screen_1, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
        //非键盘,非示波器，进入全局滑块
        //Fre_keyboard_flag=0;
        //shiboqi_flag=0;
        //slider_flag=1;
        break;
    }
    default:
        break;
    }
}

static void screen_2_btn_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_3, guider_ui.screen_3_del, &guider_ui.screen_2_del, setup_scr_screen_3, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
        //进入频率键盘,非示波器，非全局滑块
        //Fre_keyboard_flag=1;
        //shiboqi_flag=0;
        //slider_flag=0;
        break;
    }
    default:
        break;
    }
}

static void screen_2_slider_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_PRESSING:
    {
        //幅度控制正按下
        break;
    }
    default:
        break;
    }
}

static void screen_2_slider_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_PRESSING:
    {
        //频率控制正按下
        break;
    }
    default:
        break;
    }
}

static void screen_2_btn_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        ui_load_scr_animation(&guider_ui, &guider_ui.screen_4, guider_ui.screen_4_del, &guider_ui.screen_2_del, setup_scr_screen_4, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
        //进入幅度键盘,非示波器，非全局滑块
        //PGA_keyboard_flag=1;
        //shiboqi_flag=0;
        //slider_flag=0;
        break;
    }
    default:
        break;
    }
}

void events_init_screen_2 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_2, screen_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_2_btn_1, screen_2_btn_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_2_btn_2, screen_2_btn_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_2_slider_1, screen_2_slider_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_2_slider_2, screen_2_slider_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->screen_2_btn_3, screen_2_btn_3_event_handler, LV_EVENT_ALL, ui);
}

static void screen_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen_2, guider_ui.screen_2_del, &guider_ui.screen_3_del, setup_scr_screen_2, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
            //非键盘,进入示波器，非全局滑块
            //Fre_keyboard_flag=0;
            //shiboqi_flag=1;
            //slider_flag=0;
            break;
        }
        case LV_DIR_TOP:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_3_del, setup_scr_screen, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
            //返回主界面，非键盘,非示波器，非全局滑块
            //Fre_keyboard_flag=1;
            //shiboqi_flag=0;
            //slider_flag=0;
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_screen_3 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_3, screen_3_event_handler, LV_EVENT_ALL, ui);
}

static void screen_4_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
        switch(dir) {
        case LV_DIR_BOTTOM:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen_2, guider_ui.screen_2_del, &guider_ui.screen_4_del, setup_scr_screen_2, LV_SCR_LOAD_ANIM_OVER_BOTTOM, 200, 200, false, true);
            //非键盘,进入示波器，非全局滑块
            //Fre_keyboard_flag=0;
            //shiboqi_flag=1;
            //slider_flag=0;
            break;
        }
        case LV_DIR_TOP:
        {
            lv_indev_wait_release(lv_indev_get_act());
            ui_load_scr_animation(&guider_ui, &guider_ui.screen, guider_ui.screen_del, &guider_ui.screen_4_del, setup_scr_screen, LV_SCR_LOAD_ANIM_OVER_TOP, 200, 200, false, true);
            //返回主界面，非键盘,非示波器，非全局滑块
            //Fre_keyboard_flag=1;
            //shiboqi_flag=0;
            //slider_flag=0;
            break;
        }
        default:
            break;
        }
        break;
    }
    default:
        break;
    }
}

void events_init_screen_4 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_4, screen_4_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
