/**
 * @file lv_port_indev.c
 *
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 * INCLUDES
 *********************/
#include "lv_port_indev.h"
#include "touch_800x480.h" /* 引入你的触摸屏驱动头文件 */

/*********************
 * DEFINES
 *********************/

/**********************
 * TYPEDEFS
 **********************/

/**********************
 * STATIC PROTOTYPES
 **********************/

static void touchpad_init(void);
static void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data);
static bool touchpad_is_pressed(void);
static void touchpad_get_xy(int32_t * x, int32_t * y);

/**********************
 * STATIC VARIABLES
 **********************/
lv_indev_t * indev_touchpad;

/**********************
 * MACROS
 **********************/

/**********************
 * GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    /*------------------
     * Touchpad
     * -----------------*/

    /* 初始化触摸屏硬件 */
    touchpad_init();

    /* 注册触摸输入设备 */
    indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);

    /* ！！！新增代码：设置触摸防抖/滑动阈值 ！！！ */
    /* 默认值较小，手指微颤容易被误判为滑动从而取消点击。 */
    /* 这里设置为 20 像素，意味着手指由于抖动移动在 20px 以内都视为点击。 */
    lv_indev_set_scroll_limit(indev_touchpad, 20);
}

/**********************
 * STATIC FUNCTIONS
 **********************/

/*------------------
 * Touchpad
 * -----------------*/

/* 初始化你的触摸屏 */
static void touchpad_init(void)
{
    /* 调用底层驱动的初始化函数 */
    Touch_Init(); 
}

/* LVGL 库会定期调用此函数来读取触摸状态 */
static void touchpad_read(lv_indev_t * indev_drv, lv_indev_data_t * data)
{
    /* ！！！关键修改！！！ */
    /* 必须在此处调用底层的扫描函数，否则 touchInfo 不会更新 */
    Touch_Scan(); 
    /* */

    static int32_t last_x = 0;
    static int32_t last_y = 0;

    /* 保存按下时的坐标和状态 */
    if(touchpad_is_pressed()) {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    /* 设置最后的坐标 */
    data->point.x = last_x;
    data->point.y = last_y;
}

/* 返回 true 如果触摸屏被按下 */
static bool touchpad_is_pressed(void)
{
    /* 使用驱动中的全局变量 touchInfo 判断按下状态 */
    /* 注意：Touch_Scan() 会更新 touchInfo.flag */
    if(touchInfo.flag == 1) 
    {
        return true;
    } 
    else 
    {
        return false;
    }
}

/* 如果触摸屏被按下，获取 X 和 Y 坐标 */
static void touchpad_get_xy(int32_t * x, int32_t * y)
{
    /* 将硬件读取到的 uint16_t 坐标赋值给 LVGL 的 int32_t 坐标 */
    (*x) = (int32_t)touchInfo.x[0];
    (*y) = (int32_t)touchInfo.y[0];
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
