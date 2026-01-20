/**
 * @file edgewind_ui.c
 * @brief EdgeWind UI 主入口实现
 * 
 * 包含开机动画 -> 主界面的切换逻辑
 */

#include "edgewind_ui.h"

/*******************************************************************************
 * 调试开关：设为 1 时跳过开机动画，直接测试图标显示
 ******************************************************************************/
#define DEBUG_SIMPLE_IMAGE_TEST  0

/*******************************************************************************
 * 内部变量
 ******************************************************************************/

static bool ui_initialized = false;
static bool home_loaded = false;
static bool image_test_mode = false;

/* 导入图标图片（用于桌面/调试自检） */
LV_IMG_DECLARE(icon_1);
LV_IMG_DECLARE(icon_2);
LV_IMG_DECLARE(icon_3);
LV_IMG_DECLARE(icon_4);

static bool ew_image_decoder_ok(void)
{
    /* 不依赖外部资源：1x1 RGB565 “变量图片”仅用于验证 decoder 是否已注册并可解析 header */
    static const uint16_t px_1x1_rgb565[1] = {0xF800}; /* red */
    static const lv_image_dsc_t img_1x1 = {
        .header.magic = LV_IMAGE_HEADER_MAGIC,
        .header.cf = LV_COLOR_FORMAT_RGB565,
        .header.stride = 2,
        .header.w = 1,
        .header.h = 1,
        .data_size = 2,
        .data = (const uint8_t *)px_1x1_rgb565,
    };

    lv_image_header_t h;
    return lv_image_decoder_get_info(&img_1x1, &h) == LV_RESULT_OK;
}

static void ew_show_image_test_screen(void)
{
    /*==========================================================================
     * 简单测试模式：
     * - 用 LV_SYMBOL 测试字体图标
     * - 用 Canvas 手动绘制像素
     * - 用外部 icon_1 测试图片渲染
     * - 顶部显示 decoder 自检结果
     *========================================================================*/

    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_set_size(scr, 800, 480);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0x000020), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    /* ========== 关键自检：image decoder 是否工作 ========== */
    lv_image_header_t icon_h;
    lv_result_t icon_res = lv_image_decoder_get_info(&icon_1, &icon_h);
    char dec_buf[160];
    if(icon_res == LV_RESULT_OK) {
        lv_snprintf(dec_buf, sizeof(dec_buf),
                    "Decoder OK: cf=%d w=%d h=%d stride=%d",
                    (int)icon_h.cf, (int)icon_h.w, (int)icon_h.h, (int)icon_h.stride);
    }
    else {
        lv_snprintf(dec_buf, sizeof(dec_buf),
                    "Decoder FAIL: get_info(icon_1)=%d",
                    (int)icon_res);
    }
    lv_obj_t *dec_label = lv_label_create(scr);
    lv_label_set_text(dec_label, dec_buf);
    lv_obj_set_style_text_color(dec_label,
                                icon_res == LV_RESULT_OK ? lv_color_hex(0x00FFFF) : lv_color_hex(0xFF4040), 0);
    lv_obj_set_style_text_font(dec_label, &lv_font_montserrat_14, 0);
    lv_obj_align(dec_label, LV_ALIGN_TOP_LEFT, 20, 70);

    /* ========== 测试1：LVGL 内置符号图标 ========== */
    lv_obj_t *title1 = lv_label_create(scr);
    lv_label_set_text(title1, "TEST1: LVGL Symbols (font icons)");
    lv_obj_set_style_text_color(title1, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(title1, &lv_font_montserrat_16, 0);
    lv_obj_align(title1, LV_ALIGN_TOP_LEFT, 20, 10);

    lv_obj_t *symbol_label = lv_label_create(scr);
    lv_label_set_text(symbol_label, LV_SYMBOL_OK " " LV_SYMBOL_CLOSE " " LV_SYMBOL_HOME " " LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_color(symbol_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(symbol_label, &lv_font_montserrat_28, 0);
    lv_obj_align(symbol_label, LV_ALIGN_TOP_LEFT, 20, 40);

    /* ========== 测试2：彩色矩形 ========== */
    lv_obj_t *title2 = lv_label_create(scr);
    lv_label_set_text(title2, "TEST2: Color boxes");
    lv_obj_set_style_text_color(title2, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(title2, &lv_font_montserrat_16, 0);
    lv_obj_align(title2, LV_ALIGN_TOP_LEFT, 20, 90);

    uint32_t colors[] = {0xFF0000, 0x00FF00, 0x0000FF};
    for(int i = 0; i < 3; i++) {
        lv_obj_t *box = lv_obj_create(scr);
        lv_obj_set_size(box, 50, 50);
        lv_obj_set_style_bg_color(box, lv_color_hex(colors[i]), 0);
        lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
        lv_obj_set_style_radius(box, 0, 0);
        lv_obj_set_style_border_width(box, 0, 0);
        lv_obj_set_pos(box, 20 + i * 60, 120);
    }

    /* ========== 测试3：Canvas 手动绘制 ========== */
    lv_obj_t *title3 = lv_label_create(scr);
    lv_label_set_text(title3, "TEST3: Canvas draw (if red square shows = draw OK)");
    lv_obj_set_style_text_color(title3, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(title3, &lv_font_montserrat_16, 0);
    lv_obj_align(title3, LV_ALIGN_TOP_LEFT, 20, 190);

    #define CANVAS_W 50
    #define CANVAS_H 50
    void *canvas_buf = lv_malloc(CANVAS_W * CANVAS_H * 2);  /* RGB565 = 2 bytes/pixel */
    if(canvas_buf) {
        uint16_t *buf16 = (uint16_t *)canvas_buf;
        uint16_t red_565 = 0xF800;  /* RGB565 red */
        for(int i = 0; i < CANVAS_W * CANVAS_H; i++) {
            buf16[i] = red_565;
        }

        lv_obj_t *canvas = lv_canvas_create(scr);
        lv_canvas_set_buffer(canvas, canvas_buf, CANVAS_W, CANVAS_H, LV_COLOR_FORMAT_RGB565);
        lv_obj_set_pos(canvas, 20, 220);

        for(int y = 10; y < 40; y++) {
            for(int x = 10; x < 40; x++) {
                lv_canvas_set_px(canvas, x, y, lv_color_hex(0x0000FF), LV_OPA_COVER);
            }
        }
    }
    else {
        lv_obj_t *err = lv_label_create(scr);
        lv_label_set_text(err, "Canvas malloc failed!");
        lv_obj_set_style_text_color(err, lv_color_hex(0xFF0000), 0);
        lv_obj_set_pos(err, 20, 220);
    }

    /* ========== 测试4：外部图标（只测试 icon_1）========== */
    lv_obj_t *title4 = lv_label_create(scr);
    lv_label_set_text(title4, "TEST4: External icon_1");
    lv_obj_set_style_text_color(title4, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_text_font(title4, &lv_font_montserrat_16, 0);
    lv_obj_align(title4, LV_ALIGN_TOP_RIGHT, -20, 10);

    lv_obj_t *bg1 = lv_obj_create(scr);
    lv_obj_set_size(bg1, 110, 110);
    lv_obj_set_style_bg_color(bg1, lv_color_hex(0x404040), 0);
    lv_obj_set_style_bg_opa(bg1, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bg1, 2, 0);
    lv_obj_set_style_border_color(bg1, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_radius(bg1, 0, 0);
    lv_obj_set_style_pad_all(bg1, 0, 0);
    lv_obj_align(bg1, LV_ALIGN_TOP_RIGHT, -20, 40);

    lv_obj_t *img1 = lv_image_create(scr);
    lv_image_set_src(img1, &icon_1);
    lv_obj_align(img1, LV_ALIGN_TOP_RIGHT, -25, 45);

    lv_obj_t *hint = lv_label_create(scr);
    lv_label_set_text(hint,
                      "Results:\n"
                      "- Symbols show: LVGL font OK\n"
                      "- Color boxes show: basic draw OK\n"
                      "- Canvas red: canvas draw OK\n"
                      "- Icon in gray box: external image OK");
    lv_obj_set_style_text_color(hint, lv_color_hex(0xFFFF00), 0);
    lv_obj_set_style_text_font(hint, &lv_font_montserrat_14, 0);
    lv_obj_align(hint, LV_ALIGN_BOTTOM_LEFT, 20, -20);

    lv_scr_load(scr);
}

/*******************************************************************************
 * 公共函数实现
 ******************************************************************************/

void edgewind_ui_init(void)
{
    if (ui_initialized) return;

    const bool decoder_ok = ew_image_decoder_ok();
    if(DEBUG_SIMPLE_IMAGE_TEST || !decoder_ok) {
        image_test_mode = true;
        ew_show_image_test_screen();
        home_loaded = true;
        ui_initialized = true;
        return;
    }

    /* 初始化主题和样式 */
    edgewind_theme_init();
    
    /* 创建并加载开机动画界面 */
    ew_boot_anim_create();
    lv_scr_load(ew_boot_anim.screen);

    ui_initialized = true;
}

void edgewind_ui_refresh(void)
{
    if(image_test_mode) return;

    /* 检查开机动画是否完成，完成后切换到主界面 */
    if (!home_loaded && ew_boot_anim_is_finished()) {
        /* 立即设置标志位，防止重复进入 */
        home_loaded = true;
        
        /* 创建主界面 */
        ew_home_create();
        
        /* 带动画切换到主界面 */
        ew_home_load();
    }
}

bool edgewind_ui_boot_finished(void)
{
    if(image_test_mode) return true;
    return ew_boot_anim_is_finished();
}
