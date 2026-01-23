/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h> /* atoi */
#include "lvgl.h"
#include "../../gui_assets.h"
#include "../../gui_ime_pinyin.h"
#include "../custom/scr_aurora.h"
#include "cmsis_os.h"
#include "main.h"
#include "../../../ESP8266/esp8266.h"
#include "fatfs.h"
#include "diskio.h"
#include "bsp_driver_sd.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif


static void Main_1_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch(dir) {
        case LV_DIR_LEFT:
        {
            lv_indev_wait_release(lv_indev_active());
            ui_load_scr_animation(&guider_ui, &guider_ui.Main_2, guider_ui.Main_2_del, &guider_ui.Main_1_del, setup_scr_Main_2, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
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

static void Main_1_dot_2_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->Main_2 == NULL) {
        setup_scr_Main_2(ui);
    }
    gui_assets_patch_images(ui);
    lv_screen_load_anim(ui->Main_2, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

static void Main_1_dot_3_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->Main_3 == NULL) {
        setup_scr_Main_3(ui);
    }
    gui_assets_patch_images(ui);
    lv_screen_load_anim(ui->Main_3, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void events_init_Main_1 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Main_1, Main_1_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Main_1_dot_2, Main_1_dot_2_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->Main_1_dot_3, Main_1_dot_3_event_handler, LV_EVENT_CLICKED, ui);
}

static void Main_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch(dir) {
        case LV_DIR_LEFT:
        {
            lv_indev_wait_release(lv_indev_active());
            ui_load_scr_animation(&guider_ui, &guider_ui.Main_3, guider_ui.Main_3_del, &guider_ui.Main_2_del, setup_scr_Main_3, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false, false);
            break;
        }
        case LV_DIR_RIGHT:
    {
            lv_indev_wait_release(lv_indev_active());
            ui_load_scr_animation(&guider_ui, &guider_ui.Main_1, guider_ui.Main_1_del, &guider_ui.Main_2_del, setup_scr_Main_1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false, false);
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

static void Main_2_tile_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_indev_wait_release(lv_indev_active());
        ui_load_scr_animation(&guider_ui, &guider_ui.ServerConfig, guider_ui.ServerConfig_del, &guider_ui.Main_2_del,
                              setup_scr_ServerConfig, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
        break;
    }
    default:
        break;
    }
}

static void Main_2_tile_2_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_indev_wait_release(lv_indev_active());
        ui_load_scr_animation(&guider_ui, &guider_ui.WifiConfig, guider_ui.WifiConfig_del, &guider_ui.Main_2_del,
                              setup_scr_WifiConfig, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
        break;
    }
    default:
        break;
    }
}

static void Main_2_tile_5_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_CLICKED:
    {
        lv_indev_wait_release(lv_indev_active());
        ui_load_scr_animation(&guider_ui, &guider_ui.DeviceConnect, guider_ui.DeviceConnect_del, &guider_ui.Main_2_del,
                              setup_scr_DeviceConnect, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
        break;
    }
    default:
        break;
    }
}

static void Main_2_dot_1_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->Main_1 == NULL) {
        setup_scr_Main_1(ui);
    }
    gui_assets_patch_images(ui);
    lv_screen_load_anim(ui->Main_1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
    }

static void Main_2_dot_3_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->Main_3 == NULL) {
        setup_scr_Main_3(ui);
    }
    gui_assets_patch_images(ui);
    lv_screen_load_anim(ui->Main_3, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void events_init_Main_2 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Main_2, Main_2_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Main_2_tile_2, Main_2_tile_2_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->Main_2_tile_3, Main_2_tile_3_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->Main_2_tile_5, Main_2_tile_5_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->Main_2_dot_1, Main_2_dot_1_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->Main_2_dot_3, Main_2_dot_3_event_handler, LV_EVENT_CLICKED, ui);
}

static void Main_3_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_GESTURE:
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        switch(dir) {
        case LV_DIR_RIGHT:
        {
            lv_indev_wait_release(lv_indev_active());
            ui_load_scr_animation(&guider_ui, &guider_ui.Main_2, guider_ui.Main_2_del, &guider_ui.Main_3_del, setup_scr_Main_2, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false, false);
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

static void Main_3_dot_1_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->Main_1 == NULL) {
        setup_scr_Main_1(ui);
    }
    gui_assets_patch_images(ui);
    lv_screen_load_anim(ui->Main_1, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
}

static void Main_3_dot_2_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->Main_2 == NULL) {
        setup_scr_Main_2(ui);
    }
    gui_assets_patch_images(ui);
    lv_screen_load_anim(ui->Main_2, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
}

static void WifiConfig_ta_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if (ui->WifiConfig_kb != NULL) {
            lv_textarea_clear_selection(ta);
            lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
            lv_keyboard_set_textarea(ui->WifiConfig_kb, ta);
#if LV_USE_IME_PINYIN
            (void)gui_ime_pinyin_attach(ui->WifiConfig_kb);
#endif
            lv_obj_remove_flag(ui->WifiConfig_kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void WifiConfig_kb_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(ui->WifiConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
}

/* ================= WifiConfig: 仅 UI 保存/读取（不改 ESP8266 配置） =================
 * 文件位置：SD(0:) -> 0:/config/ui_wifi.cfg
 * 格式（纯文本）：
 *   SSID=xxxx
 *   PWD=yyyy
 */
#define UI_WIFI_CFG_DIR  "0:/config"
#define UI_WIFI_CFG_FILE "0:/config/ui_wifi.cfg"

static void ui_wifi_cfg_set_status(lv_ui *ui, const char *text, uint32_t color_hex)
{
    if (!ui || !ui->WifiConfig_lbl_status || !lv_obj_is_valid(ui->WifiConfig_lbl_status))
        return;
    lv_label_set_text(ui->WifiConfig_lbl_status, text ? text : "");
    lv_obj_set_style_text_color(ui->WifiConfig_lbl_status, lv_color_hex(color_hex), LV_PART_MAIN);
}

static void ui_wifi_cfg_rstrip(char *s)
{
    if (!s)
        return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\r' || s[n - 1] == '\n' || s[n - 1] == ' ' || s[n - 1] == '\t'))
    {
        s[n - 1] = '\0';
        n--;
    }
}

/* UI 配置共用的 SD 挂载状态（简化版，直接在 LVGL 线程同步执行，彻底避免死锁） */
static volatile uint8_t g_ui_sd_mounted = 0;
static volatile FRESULT g_ui_sd_last_err = FR_OK;
static uint32_t g_ui_sd_last_err_tick = 0;

/* 统一 SD 错误码到 UI 状态文本的映射 */
static void ui_sd_result_to_status(lv_ui *ui, FRESULT res, 
                                    void (*set_status_fn)(lv_ui *, const char *, uint32_t),
                                    const char *success_text)
{
    switch (res) {
        case FR_OK:
            set_status_fn(ui, success_text, 0x3dfb00);
            break;
        case FR_NO_FILE:
            set_status_fn(ui, "未找到配置文件", 0xFFD000);
            break;
        case FR_NO_FILESYSTEM:
            set_status_fn(ui, "SD未格式化/格式化失败", 0xFF5555);
            break;
        case FR_NOT_READY:
            set_status_fn(ui, "SD未就绪", 0xFF5555);
            break;
        case FR_DISK_ERR:
            set_status_fn(ui, "SD读写错误(可重试)", 0xFF5555);
            break;
        case FR_TIMEOUT:
            set_status_fn(ui, "操作超时", 0xFF5555);
            break;
        default: {
            char buf[40];
            snprintf(buf, sizeof(buf), "SD错误: %d", (int)res);
            set_status_fn(ui, buf, 0xFF5555);
        break;
    }
}
}

/* 增强的 SD mount：支持自动 mkfs + 创建 config 目录 */
static FRESULT ui_sd_mount_with_mkfs(void)
{
    /* 检查是否正在 QSPI 同步（防止竞争） */
    extern volatile uint8_t g_qspi_sd_sync_in_progress;
    if (g_qspi_sd_sync_in_progress) {
        printf("[UI_SD] QSPI sync in progress, skip mount\r\n");
        return FR_DISK_ERR;
    }

    /* 即使已经 mount，也需要验证 SD 卡状态（可能在前一次写入后变为 BUSY/ERROR） */
    if (g_ui_sd_mounted) {
        uint8_t card_state = BSP_SD_GetCardState();
        DSTATUS disk_st = disk_status(0);
        printf("[UI_SD] recheck mounted: card_state=%u disk_st=0x%02X\r\n", 
               (unsigned)card_state, (unsigned)disk_st);
        
        /* 若状态正常，直接返回 */
        if (card_state == SD_TRANSFER_OK && !(disk_st & STA_NOINIT)) {
            return FR_OK;
        }
        
        /* 状态异常，重新初始化 */
        printf("[UI_SD] SD state abnormal, remount needed\r\n");
        g_ui_sd_mounted = 0;
        g_ui_sd_last_err = FR_NOT_READY;
    }

    /* 缓存 FR_NO_FILESYSTEM：5 秒内不再重复尝试（避免格式化失败后反复卡死） */
    if (g_ui_sd_last_err == FR_NO_FILESYSTEM) {
        if ((osKernelGetTickCount() - g_ui_sd_last_err_tick) < 5000U) {
            printf("[UI_SD] skip mount (cached NO_FILESYSTEM)\r\n");
            return FR_NO_FILESYSTEM;
        }
    }

    /* 确保驱动已链接 */
    if (SDPath[0] == '\0') {
        MX_FATFS_Init();
    }

    /* 先快速判定是否插卡 */
    if (BSP_SD_IsDetected() != SD_PRESENT)
    {
        g_ui_sd_mounted = 0;
        g_ui_sd_last_err = FR_NOT_READY;
        g_ui_sd_last_err_tick = osKernelGetTickCount();
        printf("[UI_SD] SD not present\r\n");
        return FR_NOT_READY;
    }

    /* 关键修复：UI 层不直接 BSP_SD_Init（避免与 diskio 内部初始化/状态机冲突）
     * 统一走 disk_initialize(0) + f_mount。
     */
    FRESULT res = FR_DISK_ERR;

    DSTATUS st_init = disk_initialize(0);
    DSTATUS st_stat = disk_status(0);
    printf("[UI_SD] disk_init=0x%02X status=0x%02X\r\n",
           (unsigned)st_init, (unsigned)st_stat);
    if (st_init & STA_NOINIT)
    {
        res = FR_NOT_READY;
        g_ui_sd_last_err = res;
        g_ui_sd_last_err_tick = osKernelGetTickCount();
        return res;
    }

    /* 等待卡进入 TRANSFER_OK（短超时） */
    uint32_t t0 = osKernelGetTickCount();
    while ((osKernelGetTickCount() - t0) < 200U)
    {
        if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
        break;
        osDelay(5);
    }
    printf("[UI_SD] state=%u (0=OK,1=BUSY)\r\n", (unsigned)BSP_SD_GetCardState());

    res = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
    printf("[UI_SD] mount %s -> %d\r\n", SDPath, (int)res);
    if (res == FR_OK)
    {
        g_ui_sd_mounted = 1;
        g_ui_sd_last_err = FR_OK;
        /* 确保 config 目录存在 */
        FRESULT mkdir_res = f_mkdir("0:/config");
        if (mkdir_res != FR_OK && mkdir_res != FR_EXIST) {
            printf("[UI_SD] mkdir 0:/config -> %d (non-fatal)\r\n", (int)mkdir_res);
        }
        return FR_OK;
    }

    /* 处理 FR_NO_FILESYSTEM：自动格式化 */
    if (res == FR_NO_FILESYSTEM)
{
        printf("[UI_SD] no filesystem, formatting (FAT32)...\r\n");
        static uint8_t mkfs_work[4096];
        FRESULT mkfs_res = f_mkfs((TCHAR const *)SDPath, FM_FAT32, 0, mkfs_work, sizeof(mkfs_work));
        printf("[UI_SD] mkfs -> %d\r\n", (int)mkfs_res);
        if (mkfs_res != FR_OK)
        {
            printf("[UI_SD] mkfs failed\r\n");
            g_ui_sd_last_err = FR_NO_FILESYSTEM;
            g_ui_sd_last_err_tick = osKernelGetTickCount();
            return FR_NO_FILESYSTEM;
        }
        /* 格式化成功，重新 mount */
        res = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
        printf("[UI_SD] remount after mkfs -> %d\r\n", (int)res);
        if (res == FR_OK)
        {
            g_ui_sd_mounted = 1;
            g_ui_sd_last_err = FR_OK;
            FRESULT mkdir_res = f_mkdir("0:/config");
            printf("[UI_SD] mkdir 0:/config -> %d\r\n", (int)mkdir_res);
            return FR_OK;
    }
    }

    g_ui_sd_mounted = 0;
    g_ui_sd_last_err = res;
    g_ui_sd_last_err_tick = osKernelGetTickCount();
    return res;
}

static FRESULT ui_wifi_cfg_ensure_dir(void)
{
    FILINFO fno;
    FRESULT res = f_stat(UI_WIFI_CFG_DIR, &fno);
    printf("[UI_CFG] stat dir %s -> %d\r\n", UI_WIFI_CFG_DIR, (int)res);
    if (res == FR_OK)
    {
        if (fno.fattrib & AM_DIR)
            return FR_OK;
        printf("[UI_CFG] path exists but not dir: %s\r\n", UI_WIFI_CFG_DIR);
        return FR_INVALID_NAME;
    }
    /* FR_NO_FILE(4) 或 FR_NO_PATH(5) 都表示目录不存在，需要创建 */
    if (res != FR_NO_FILE && res != FR_NO_PATH)
{
        printf("[UI_CFG] stat dir unexpected fail: %s res=%d\r\n", UI_WIFI_CFG_DIR, (int)res);
        return res;
    }
    printf("[UI_CFG] creating dir %s ...\r\n", UI_WIFI_CFG_DIR);
    res = f_mkdir(UI_WIFI_CFG_DIR);
    printf("[UI_CFG] mkdir %s -> %d\r\n", UI_WIFI_CFG_DIR, (int)res);
    /* FR_EXIST 也算成功（目录已存在） */
    if (res == FR_EXIST)
        return FR_OK;
    return res;
}

static FRESULT ui_wifi_cfg_read_file(char *ssid, size_t ssid_len, char *pwd, size_t pwd_len)
{
    if (!ssid || !pwd || ssid_len == 0 || pwd_len == 0)
        return FR_INVALID_OBJECT;
    ssid[0] = '\0';
    pwd[0] = '\0';

    FRESULT res = ui_sd_mount_with_mkfs();
    if (res != FR_OK)
    {
        printf("[WIFI_UI_CFG] load skipped: SD not ready (res=%d)\r\n", (int)res);
        return res;
    }

    FIL fil;
    for (int attempt = 1; attempt <= 2; ++attempt) {
        res = f_open(&fil, UI_WIFI_CFG_FILE, FA_READ);
        if (res == FR_OK) {
        break;
    }
        printf("[WIFI_UI_CFG] open for read fail: %s res=%d attempt=%d\r\n",
               UI_WIFI_CFG_FILE, (int)res, attempt);
        /* 磁盘错误：强制 remount 后重试一次 */
        if (res == FR_DISK_ERR && attempt == 1) {
            g_ui_sd_mounted = 0;
            (void)ui_sd_mount_with_mkfs();
            continue;
        }
        return res;
    }

    char line[160];
    while (f_gets(line, sizeof(line), &fil))
    {
        ui_wifi_cfg_rstrip(line);
        if (strncmp(line, "SSID=", 5) == 0)
        {
            strncpy(ssid, line + 5, ssid_len - 1);
            ssid[ssid_len - 1] = '\0';
        }
        else if (strncmp(line, "PWD=", 4) == 0)
        {
            strncpy(pwd, line + 4, pwd_len - 1);
            pwd[pwd_len - 1] = '\0';
        }
    }
    (void)f_close(&fil);

    printf("[WIFI_UI_CFG] loaded: ssid_len=%u pwd_len=%u\r\n", (unsigned)strlen(ssid), (unsigned)strlen(pwd));
    return FR_OK;
}

static void ui_wifi_cfg_apply_to_ui(lv_ui *ui, const char *ssid, const char *pwd)
{
    if (!ui)
        return;
    /* 允许覆盖为空串：用户清空后再次进入界面也能恢复（或显示空） */
    if (ui->WifiConfig_ta_ssid && lv_obj_is_valid(ui->WifiConfig_ta_ssid))
        lv_textarea_set_text(ui->WifiConfig_ta_ssid, ssid ? ssid : "");
    if (ui->WifiConfig_ta_pwd && lv_obj_is_valid(ui->WifiConfig_ta_pwd))
        lv_textarea_set_text(ui->WifiConfig_ta_pwd, pwd ? pwd : "");
}

static FRESULT ui_wifi_cfg_write_file(const char *ssid, const char *pwd)
{
    printf("[WIFI_UI_CFG] write_file: start\r\n");
    FRESULT res = ui_sd_mount_with_mkfs();
    if (res != FR_OK)
    {
        printf("[WIFI_UI_CFG] save failed: SD not ready (res=%d)\r\n", (int)res);
        return res;
    }
    printf("[WIFI_UI_CFG] write_file: SD mounted\r\n");

    res = ui_wifi_cfg_ensure_dir();
    printf("[WIFI_UI_CFG] write_file: ensure_dir res=%d\r\n", (int)res);
    if (res != FR_OK)
    {
        printf("[WIFI_UI_CFG] ensure dir fail: res=%d\r\n", (int)res);
        return res;
    }

    /* 原子写入：先写临时文件，再 rename 覆盖 */
    const char *tmp_path = "0:/config/.ui_wifi.cfg.tmp";

    printf("[WIFI_UI_CFG] write_file: opening tmp...\r\n");
    FIL fil;
    res = f_open(&fil, tmp_path, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        printf("[WIFI_UI_CFG] open for write fail: %s res=%d\r\n", tmp_path, (int)res);
        /* 若遇到磁盘错误，强制卸载/重挂载后再重试一次 */
        if (res == FR_DISK_ERR) {
            g_ui_sd_mounted = 0;
            (void)f_mount(NULL, (TCHAR const *)SDPath, 0);
            osDelay(10);
            (void)ui_sd_mount_with_mkfs();
            res = f_open(&fil, tmp_path, FA_CREATE_ALWAYS | FA_WRITE);
            printf("[WIFI_UI_CFG] retry open tmp -> %d\r\n", (int)res);
        }
        if (res != FR_OK) {
            g_ui_sd_last_err = res;
            g_ui_sd_last_err_tick = osKernelGetTickCount();
            return res;
        }
    }

    printf("[WIFI_UI_CFG] write_file: writing...\r\n");
    char buf[220];
    UINT bw = 0;
    int n = snprintf(buf, sizeof(buf), "SSID=%s\r\nPWD=%s\r\n", ssid ? ssid : "", pwd ? pwd : "");
    if (n < 0)
        n = 0;
    if ((size_t)n > sizeof(buf))
        n = (int)sizeof(buf);
    res = f_write(&fil, buf, (UINT)n, &bw);
    printf("[WIFI_UI_CFG] write_file: f_write res=%d bw=%u\r\n", (int)res, (unsigned)bw);
    
    /* 仅在写入成功时才 sync，且添加短延迟避免 SD 卡 BUSY 卡死 */
    if (res == FR_OK) {
        osDelay(5); /* 短延迟让 SD 卡缓冲稳定 */
        FRESULT sync_res = f_sync(&fil);
        printf("[WIFI_UI_CFG] write_file: f_sync res=%d\r\n", (int)sync_res);
        if (sync_res != FR_OK) {
            res = sync_res;
        }
    }
    
    FRESULT close_res = f_close(&fil);
    printf("[WIFI_UI_CFG] write_file: f_close res=%d\r\n", (int)close_res);
    if (close_res != FR_OK && res == FR_OK) {
        res = close_res;
    }
    
    /* 关闭后短暂等待 SD 进入就绪状态，避免立即切换界面时读取遇到 BUSY */
    if (res == FR_OK) {
        uint32_t t0 = osKernelGetTickCount();
        while ((osKernelGetTickCount() - t0) < 100U) {
            if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
        break;
            osDelay(5);
        }
        printf("[WIFI_UI_CFG] post-close wait: card_state=%u\r\n", (unsigned)BSP_SD_GetCardState());
    }

    /* 如果文件内容未成功落盘，清理临时文件并让下次强制 remount */
    if (res != FR_OK) {
        printf("[WIFI_UI_CFG] write_file failed, unlink tmp\r\n");
        (void)f_unlink(tmp_path);
        g_ui_sd_mounted = 0;
        g_ui_sd_last_err = res;
        g_ui_sd_last_err_tick = osKernelGetTickCount();
        return res;
    }

    /* 覆盖写入：先删旧文件，再 rename */
    (void)f_unlink(UI_WIFI_CFG_FILE);
    FRESULT rn = f_rename(tmp_path, UI_WIFI_CFG_FILE);
    printf("[WIFI_UI_CFG] rename tmp -> cfg res=%d\r\n", (int)rn);
    if (rn != FR_OK) {
        (void)f_unlink(tmp_path);
        g_ui_sd_mounted = 0;
        g_ui_sd_last_err = rn;
        g_ui_sd_last_err_tick = osKernelGetTickCount();
        return rn;
    }

    printf("[WIFI_UI_CFG] saved: final_res=%d bytes=%u ssid_len=%u pwd_len=%u\r\n",
           (int)res, (unsigned)bw, (unsigned)(ssid ? strlen(ssid) : 0U), (unsigned)(pwd ? strlen(pwd) : 0U));
    return FR_OK;
}

/* ============ WiFi 配置 SD 同步操作（直接在 LVGL 线程执行，彻底避免死锁） ============ */

static void ui_wifi_cfg_do_load_sync(lv_ui *ui)
{
    if (!ui) return;
    char ssid[64] = {0};
    char pwd[64]  = {0};
    FRESULT res = ui_wifi_cfg_read_file(ssid, sizeof(ssid), pwd, sizeof(pwd));
    if (res == FR_OK) {
        ui_wifi_cfg_apply_to_ui(ui, ssid, pwd);
    }
    ui_sd_result_to_status(ui, res, ui_wifi_cfg_set_status, "加载完成");
}

static void ui_wifi_cfg_do_save_sync(lv_ui *ui)
{
    if (!ui) return;
    const char *ssid = (ui->WifiConfig_ta_ssid) ? lv_textarea_get_text(ui->WifiConfig_ta_ssid) : "";
    const char *pwd  = (ui->WifiConfig_ta_pwd) ? lv_textarea_get_text(ui->WifiConfig_ta_pwd) : "";
    FRESULT res = ui_wifi_cfg_write_file(ssid ? ssid : "", pwd ? pwd : "");
    
    if (res == FR_OK) {
        /* 保存后立即读一次，回显验证 */
        char r_ssid[64] = {0}, r_pwd[64] = {0};
        if (ui_wifi_cfg_read_file(r_ssid, sizeof(r_ssid), r_pwd, sizeof(r_pwd)) == FR_OK) {
            ui_wifi_cfg_apply_to_ui(ui, r_ssid, r_pwd);
        }
    }
    
    ui_sd_result_to_status(ui, res, ui_wifi_cfg_set_status, "保存成功");
    
    if (ui->WifiConfig_btn_save && lv_obj_is_valid(ui->WifiConfig_btn_save))
        lv_obj_clear_state(ui->WifiConfig_btn_save, LV_STATE_DISABLED);
}

/* 进入 WifiConfig 界面后自动加载（延迟一帧执行，保证屏幕已渲染完成） */
static void WifiConfig_load_timer_cb(lv_timer_t *t)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(t);
    lv_timer_del(t);
    if (!ui) return;
    ui_wifi_cfg_set_status(ui, "正在加载...", 0xFFA500);
    /* 立即刷新一次，让用户看到"正在加载"提示 */
    lv_obj_update_layout(ui->WifiConfig);
    lv_refr_now(NULL);
    /* 同步执行 SD 读取（在 LVGL 线程内，不会死锁） */
    ui_wifi_cfg_do_load_sync(ui);
}

/* 每次屏幕被加载（切换进入）都触发一次自动加载 */
static void WifiConfig_screen_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCREEN_LOADED) {
        return;
    }
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    ui_wifi_cfg_set_status(ui, "正在加载...", 0xFFA500);
    lv_timer_create(WifiConfig_load_timer_cb, 100, ui);
}

static void WifiConfig_save_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    lv_indev_wait_release(lv_indev_active());
    if (ui->WifiConfig_kb) {
        lv_obj_add_flag(ui->WifiConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->WifiConfig_btn_save && lv_obj_is_valid(ui->WifiConfig_btn_save))
        lv_obj_add_state(ui->WifiConfig_btn_save, LV_STATE_DISABLED);
    ui_wifi_cfg_set_status(ui, "正在保存...", 0xFFA500);
    lv_obj_update_layout(ui->WifiConfig);
    lv_refr_now(NULL);
    /* 同步执行 SD 写入（在 LVGL 线程内，不会死锁） */
    ui_wifi_cfg_do_save_sync(ui);
}

static void WifiConfig_scan_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    lv_indev_wait_release(lv_indev_active());
    if (ui->WifiConfig_kb) {
        lv_obj_add_flag(ui->WifiConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
    ui_wifi_cfg_set_status(ui, "正在加载...", 0xFFA500);
    lv_obj_update_layout(ui->WifiConfig);
    lv_refr_now(NULL);
    /* 同步执行 SD 读取（在 LVGL 线程内，不会死锁） */
    ui_wifi_cfg_do_load_sync(ui);
}

static void WifiConfig_back_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (ui->WifiConfig_kb) {
        lv_obj_add_flag(ui->WifiConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
    ui_load_scr_animation(&guider_ui, &guider_ui.Main_1, guider_ui.Main_1_del, &guider_ui.WifiConfig_del,
                          setup_scr_Aurora, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
}

static void ServerConfig_ta_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *ta = lv_event_get_target(e);
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    if (code == LV_EVENT_CLICKED || code == LV_EVENT_FOCUSED) {
        if (ui->ServerConfig_kb != NULL) {
            lv_textarea_clear_selection(ta);
            lv_textarea_set_cursor_pos(ta, LV_TEXTAREA_CURSOR_LAST);
            lv_keyboard_set_textarea(ui->ServerConfig_kb, ta);
#if LV_USE_IME_PINYIN
            (void)gui_ime_pinyin_attach(ui->ServerConfig_kb);
#endif
            lv_obj_remove_flag(ui->ServerConfig_kb, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void ServerConfig_kb_event_handler(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    if (code == LV_EVENT_READY || code == LV_EVENT_CANCEL) {
        lv_obj_add_flag(ui->ServerConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
}

static void ServerConfig_back_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
            lv_indev_wait_release(lv_indev_active());
    if (ui->ServerConfig_kb) {
        lv_obj_add_flag(ui->ServerConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
    ui_load_scr_animation(&guider_ui, &guider_ui.Main_1, guider_ui.Main_1_del, &guider_ui.ServerConfig_del,
                          setup_scr_Aurora, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
}

/* ============ 服务器配置 SD 持久化（直接在 LVGL 线程同步执行） ============ */

#define UI_SERVER_CFG_FILE "0:/config/ui_server.cfg"

static void ui_server_cfg_set_status(lv_ui *ui, const char *text, uint32_t color_hex)
{
    if (!ui || !ui->ServerConfig_lbl_status || !lv_obj_is_valid(ui->ServerConfig_lbl_status))
        return;
    lv_label_set_text(ui->ServerConfig_lbl_status, text ? text : "");
    lv_obj_set_style_text_color(ui->ServerConfig_lbl_status, lv_color_hex(color_hex), LV_PART_MAIN);
        }

static void ui_server_cfg_apply_to_ui(lv_ui *ui, const char *ip, const char *port, const char *id, const char *loc)
{
    if (!ui) return;
    /* 允许覆盖为空串：用户清空后再次进入界面也能恢复（或显示空） */
    if (ui->ServerConfig_ta_ip && lv_obj_is_valid(ui->ServerConfig_ta_ip))
        lv_textarea_set_text(ui->ServerConfig_ta_ip, ip ? ip : "");
    if (ui->ServerConfig_ta_port && lv_obj_is_valid(ui->ServerConfig_ta_port))
        lv_textarea_set_text(ui->ServerConfig_ta_port, port ? port : "");
    if (ui->ServerConfig_ta_id && lv_obj_is_valid(ui->ServerConfig_ta_id))
        lv_textarea_set_text(ui->ServerConfig_ta_id, id ? id : "");
    if (ui->ServerConfig_ta_loc && lv_obj_is_valid(ui->ServerConfig_ta_loc))
        lv_textarea_set_text(ui->ServerConfig_ta_loc, loc ? loc : "");
}

static FRESULT ui_server_cfg_read_file(char *ip, size_t ip_len, char *port, size_t port_len,
                                       char *id, size_t id_len, char *loc, size_t loc_len)
{
    if (!ip || !port || !id || !loc) return FR_INVALID_OBJECT;
    ip[0] = port[0] = id[0] = loc[0] = '\0';

    FRESULT res = ui_sd_mount_with_mkfs();
    if (res != FR_OK) {
        printf("[SERVER_UI_CFG] load skipped: SD not ready (res=%d)\r\n", (int)res);
        return res;
    }

    FIL fil;
    for (int attempt = 1; attempt <= 2; ++attempt) {
        res = f_open(&fil, UI_SERVER_CFG_FILE, FA_READ);
        if (res == FR_OK) {
            break;
        }
        printf("[SERVER_UI_CFG] open for read fail: %s res=%d attempt=%d\r\n",
               UI_SERVER_CFG_FILE, (int)res, attempt);
        if (res == FR_DISK_ERR && attempt == 1) {
            g_ui_sd_mounted = 0;
            (void)ui_sd_mount_with_mkfs();
            continue;
        }
        return res;
    }

    char line[160];
    while (f_gets(line, sizeof(line), &fil)) {
        ui_wifi_cfg_rstrip(line);
        if (strncmp(line, "IP=", 3) == 0) {
            strncpy(ip, line + 3, ip_len - 1);
            ip[ip_len - 1] = '\0';
        } else if (strncmp(line, "PORT=", 5) == 0) {
            strncpy(port, line + 5, port_len - 1);
            port[port_len - 1] = '\0';
        } else if (strncmp(line, "ID=", 3) == 0) {
            strncpy(id, line + 3, id_len - 1);
            id[id_len - 1] = '\0';
        } else if (strncmp(line, "LOC=", 4) == 0) {
            strncpy(loc, line + 4, loc_len - 1);
            loc[loc_len - 1] = '\0';
        }
    }
    (void)f_close(&fil);

    printf("[SERVER_UI_CFG] loaded: ip=%s port=%s id=%s loc=%s\r\n", ip, port, id, loc);
    return FR_OK;
}

static FRESULT ui_server_cfg_write_file(const char *ip, const char *port, const char *id, const char *loc)
{
    printf("[SERVER_UI_CFG] write_file: start\r\n");
    FRESULT res = ui_sd_mount_with_mkfs();
    if (res != FR_OK) {
        printf("[SERVER_UI_CFG] save failed: SD not ready (res=%d)\r\n", (int)res);
        return res;
    }
    printf("[SERVER_UI_CFG] write_file: SD mounted\r\n");

    res = ui_wifi_cfg_ensure_dir();
    printf("[SERVER_UI_CFG] write_file: ensure_dir res=%d\r\n", (int)res);
    if (res != FR_OK) {
        printf("[SERVER_UI_CFG] ensure dir fail: res=%d\r\n", (int)res);
        return res;
    }

    /* 原子写入：先写临时文件，再 rename 覆盖 */
    const char *tmp_path = "0:/config/.ui_server.cfg.tmp";

    printf("[SERVER_UI_CFG] write_file: opening tmp...\r\n");
    FIL fil;
    res = f_open(&fil, tmp_path, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK) {
        printf("[SERVER_UI_CFG] open for write fail: %s res=%d\r\n", tmp_path, (int)res);
        /* 若遇到磁盘错误，强制卸载/重挂载后再重试一次 */
        if (res == FR_DISK_ERR) {
            g_ui_sd_mounted = 0;
            (void)f_mount(NULL, (TCHAR const *)SDPath, 0);
            osDelay(10);
            (void)ui_sd_mount_with_mkfs();
            res = f_open(&fil, tmp_path, FA_WRITE | FA_CREATE_ALWAYS);
            printf("[SERVER_UI_CFG] retry open tmp -> %d\r\n", (int)res);
        }
        if (res != FR_OK) {
            g_ui_sd_last_err = res;
            g_ui_sd_last_err_tick = osKernelGetTickCount();
            return res;
        }
    }

    printf("[SERVER_UI_CFG] write_file: writing...\r\n");
    char buf[320];
    int n = snprintf(buf, sizeof(buf), "IP=%s\nPORT=%s\nID=%s\nLOC=%s\n",
                     ip ? ip : "", port ? port : "", id ? id : "", loc ? loc : "");
    UINT bw = 0;
    res = f_write(&fil, buf, (UINT)n, &bw);
    printf("[SERVER_UI_CFG] write_file: f_write res=%d bw=%u\r\n", (int)res, (unsigned)bw);
    
    /* 仅在写入成功时才 sync，且添加短延迟避免 SD 卡 BUSY 卡死 */
    if (res == FR_OK) {
        osDelay(5); /* 短延迟让 SD 卡缓冲稳定 */
        FRESULT sync_res = f_sync(&fil);
        printf("[SERVER_UI_CFG] write_file: f_sync res=%d\r\n", (int)sync_res);
        if (sync_res != FR_OK) {
            res = sync_res;
        }
    }
    
    FRESULT close_res = f_close(&fil);
    printf("[SERVER_UI_CFG] write_file: f_close res=%d\r\n", (int)close_res);
    if (close_res != FR_OK && res == FR_OK) {
        res = close_res;
    }
    
    /* 关闭后短暂等待 SD 进入就绪状态，避免立即切换界面时读取遇到 BUSY */
    if (res == FR_OK) {
        uint32_t t0 = osKernelGetTickCount();
        while ((osKernelGetTickCount() - t0) < 100U) {
            if (BSP_SD_GetCardState() == SD_TRANSFER_OK)
        break;
            osDelay(5);
        }
        printf("[SERVER_UI_CFG] post-close wait: card_state=%u\r\n", (unsigned)BSP_SD_GetCardState());
    }

    if (res != FR_OK) {
        printf("[SERVER_UI_CFG] write_file failed, unlink tmp\r\n");
        (void)f_unlink(tmp_path);
        g_ui_sd_mounted = 0;
        g_ui_sd_last_err = res;
        g_ui_sd_last_err_tick = osKernelGetTickCount();
        return res;
    }

    (void)f_unlink(UI_SERVER_CFG_FILE);
    FRESULT rn = f_rename(tmp_path, UI_SERVER_CFG_FILE);
    printf("[SERVER_UI_CFG] rename tmp -> cfg res=%d\r\n", (int)rn);
    if (rn != FR_OK) {
        (void)f_unlink(tmp_path);
        g_ui_sd_mounted = 0;
        g_ui_sd_last_err = rn;
        g_ui_sd_last_err_tick = osKernelGetTickCount();
        return rn;
    }

    printf("[SERVER_UI_CFG] saved: final_res=%d bytes=%u\r\n", (int)res, (unsigned)bw);
    return FR_OK;
}

static void ui_server_cfg_do_load_sync(lv_ui *ui)
{
    if (!ui) return;
    char ip[64] = {0}, port[16] = {0}, id[80] = {0}, loc[80] = {0};
    FRESULT res = ui_server_cfg_read_file(ip, sizeof(ip), port, sizeof(port), id, sizeof(id), loc, sizeof(loc));
    if (res == FR_OK) {
        ui_server_cfg_apply_to_ui(ui, ip, port, id, loc);
    }
    ui_sd_result_to_status(ui, res, ui_server_cfg_set_status, "加载完成");
}

static void ui_server_cfg_do_save_sync(lv_ui *ui)
{
    if (!ui) return;
    const char *ip   = (ui->ServerConfig_ta_ip)   ? lv_textarea_get_text(ui->ServerConfig_ta_ip)   : "";
    const char *port = (ui->ServerConfig_ta_port) ? lv_textarea_get_text(ui->ServerConfig_ta_port) : "";
    const char *id   = (ui->ServerConfig_ta_id)   ? lv_textarea_get_text(ui->ServerConfig_ta_id)   : "";
    const char *loc  = (ui->ServerConfig_ta_loc)  ? lv_textarea_get_text(ui->ServerConfig_ta_loc)  : "";
    FRESULT res = ui_server_cfg_write_file(ip, port, id, loc);
    
    ui_sd_result_to_status(ui, res, ui_server_cfg_set_status, "保存成功");
    
    if (ui->ServerConfig_btn_save && lv_obj_is_valid(ui->ServerConfig_btn_save))
        lv_obj_clear_state(ui->ServerConfig_btn_save, LV_STATE_DISABLED);
}

static void ServerConfig_load_timer_cb(lv_timer_t *t)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(t);
    lv_timer_del(t);
    if (!ui) return;
    ui_server_cfg_set_status(ui, "正在加载...", 0xFFA500);
    lv_obj_update_layout(ui->ServerConfig);
    lv_refr_now(NULL);
    ui_server_cfg_do_load_sync(ui);
}

/* 每次屏幕被加载（切换进入）都触发一次自动加载 */
static void ServerConfig_screen_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCREEN_LOADED) {
        return;
    }
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    ui_server_cfg_set_status(ui, "正在加载...", 0xFFA500);
    lv_timer_create(ServerConfig_load_timer_cb, 100, ui);
}

static void ServerConfig_save_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    lv_indev_wait_release(lv_indev_active());
    if (ui->ServerConfig_kb) {
        lv_obj_add_flag(ui->ServerConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
    if (ui->ServerConfig_btn_save && lv_obj_is_valid(ui->ServerConfig_btn_save))
        lv_obj_add_state(ui->ServerConfig_btn_save, LV_STATE_DISABLED);
    ui_server_cfg_set_status(ui, "正在保存...", 0xFFA500);
    lv_obj_update_layout(ui->ServerConfig);
    lv_refr_now(NULL);
    ui_server_cfg_do_save_sync(ui);
}

static void ServerConfig_load_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    lv_indev_wait_release(lv_indev_active());
    if (ui->ServerConfig_kb) {
        lv_obj_add_flag(ui->ServerConfig_kb, LV_OBJ_FLAG_HIDDEN);
    }
    ui_server_cfg_set_status(ui, "正在加载...", 0xFFA500);
    lv_obj_update_layout(ui->ServerConfig);
    lv_refr_now(NULL);
    ui_server_cfg_do_load_sync(ui);
}

void events_init_Main_3 (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->Main_3, Main_3_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->Main_3_dot_1, Main_3_dot_1_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->Main_3_dot_2, Main_3_dot_2_event_handler, LV_EVENT_CLICKED, ui);
}

void events_init_WifiConfig(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->WifiConfig_ta_ssid, WifiConfig_ta_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->WifiConfig_ta_pwd, WifiConfig_ta_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->WifiConfig_kb, WifiConfig_kb_event_handler, LV_EVENT_READY, ui);
    lv_obj_add_event_cb(ui->WifiConfig_kb, WifiConfig_kb_event_handler, LV_EVENT_CANCEL, ui);
    lv_obj_add_event_cb(ui->WifiConfig_btn_back, WifiConfig_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->WifiConfig_btn_save, WifiConfig_save_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->WifiConfig_btn_scan, WifiConfig_scan_event_handler, LV_EVENT_CLICKED, ui);

    /* 每次切换进入屏幕都自动加载 */
    lv_obj_add_event_cb(ui->WifiConfig, WifiConfig_screen_event_handler, LV_EVENT_SCREEN_LOADED, ui);
}

void events_init_ServerConfig(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->ServerConfig_ta_ip, ServerConfig_ta_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->ServerConfig_ta_port, ServerConfig_ta_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->ServerConfig_ta_id, ServerConfig_ta_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->ServerConfig_ta_loc, ServerConfig_ta_event_handler, LV_EVENT_ALL, ui);
    lv_obj_add_event_cb(ui->ServerConfig_kb, ServerConfig_kb_event_handler, LV_EVENT_READY, ui);
    lv_obj_add_event_cb(ui->ServerConfig_kb, ServerConfig_kb_event_handler, LV_EVENT_CANCEL, ui);
    lv_obj_add_event_cb(ui->ServerConfig_btn_back, ServerConfig_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->ServerConfig_btn_save, ServerConfig_save_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->ServerConfig_btn_load, ServerConfig_load_event_handler, LV_EVENT_CLICKED, ui);

    /* 每次切换进入屏幕都自动加载 */
    lv_obj_add_event_cb(ui->ServerConfig, ServerConfig_screen_event_handler, LV_EVENT_SCREEN_LOADED, ui);
}

static void DeviceConnect_back_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    ui_load_scr_animation(&guider_ui, &guider_ui.Main_1, guider_ui.Main_1_del, &guider_ui.DeviceConnect_del,
                          setup_scr_Aurora, LV_SCR_LOAD_ANIM_FADE_ON, 200, 20, false, false);
}

/* ================= DeviceConnect: ESP8266 UI 交互（对齐 ESP8266操作.HTML） ================= */

typedef enum
{
    DC_MSG_LOG = 0,
    DC_MSG_STEP_DONE,
} dc_msg_type_t;

typedef struct
{
    dc_msg_type_t type;
    uint8_t step; /* esp_ui_cmd_t */
    uint8_t ok;
    char text[128];
} dc_msg_t;

static osMessageQueueId_t g_dc_q = NULL;
static lv_timer_t *g_dc_timer = NULL;
static lv_ui *g_dc_ui = NULL;
static uint8_t g_dc_auto_running = 0;
static uint32_t g_dc_report_stop_tick = 0;
static uint8_t g_dc_reg_dimmed = 0;
static lv_obj_t *g_dc_lbl_reg_countdown = NULL;
/* DeviceConnect 进入时是否已从 SD 加载并应用到 ESP 配置缓冲区 */
static uint8_t g_dc_cfg_loaded = 0;
/* 进入 DeviceConnect 时的配置加载 timer（去重，避免重复创建导致 UI 抖动/刷屏） */
static lv_timer_t *g_dc_cfg_timer = NULL;

typedef struct
{
    char ssid[64];
    char pwd[64];
    char ip[32];
    char port_s[16];
    char node_id[64];
    char node_loc[64];
} dc_cfg_buf_t;

/* 避免在 LVGL 线程上使用大量栈空间，改为静态缓冲区 */
static dc_cfg_buf_t g_dc_cfg_buf;

/* 停止上报后超过该时间，设备注册变暗并提示重新点击（单位 ms） */
#ifndef EW_DEVICECONNECT_REG_DIM_MS
#define EW_DEVICECONNECT_REG_DIM_MS 60000u
#endif

static void dc_set_buttons_enabled(lv_ui *ui, bool enabled)
{
    if (!ui)
        return;
    /* “开始上报”时，禁止反复重连操作；否则保持随时可点 */
    lv_obj_t *btns[] = {
        ui->DeviceConnect_btn_wifi,
        ui->DeviceConnect_btn_tcp,
        ui->DeviceConnect_btn_reg,
        ui->DeviceConnect_btn_auto,
    };
    for (size_t i = 0; i < sizeof(btns) / sizeof(btns[0]); i++)
    {
        if (btns[i] && lv_obj_is_valid(btns[i]))
        {
            if (enabled)
                lv_obj_clear_state(btns[i], LV_STATE_DISABLED);
            else
                lv_obj_add_state(btns[i], LV_STATE_DISABLED);
        }
    }
}

static void dc_set_reg_dim(lv_ui *ui, bool dim)
{
    if (!ui)
        return;
    if (ui->DeviceConnect_btn_reg && lv_obj_is_valid(ui->DeviceConnect_btn_reg))
    {
        if (dim)
        {
            lv_obj_set_style_bg_color(ui->DeviceConnect_btn_reg, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(ui->DeviceConnect_btn_reg, 160, LV_PART_MAIN);
        }
        else
        {
            lv_obj_set_style_bg_color(ui->DeviceConnect_btn_reg, lv_color_hex(0xFFA500), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(ui->DeviceConnect_btn_reg, 255, LV_PART_MAIN);
        }
    }
}

static void dc_reg_countdown_set_visible(bool visible)
{
    if (g_dc_lbl_reg_countdown && lv_obj_is_valid(g_dc_lbl_reg_countdown))
    {
        if (visible)
            lv_obj_clear_flag(g_dc_lbl_reg_countdown, LV_OBJ_FLAG_HIDDEN);
        else
            lv_obj_add_flag(g_dc_lbl_reg_countdown, LV_OBJ_FLAG_HIDDEN);
    }
}

static void dc_reg_countdown_update(lv_ui *ui)
{
    if (!ui)
        return;
    if (!g_dc_lbl_reg_countdown || !lv_obj_is_valid(g_dc_lbl_reg_countdown))
        return;

    /* 仅在“停止上报后已开始计时”且未过期时显示倒计时 */
    if (ESP_UI_IsReporting() || g_dc_report_stop_tick == 0 || g_dc_reg_dimmed)
    {
        dc_reg_countdown_set_visible(false);
        return;
    }

    uint32_t now = lv_tick_get();
    uint32_t elapsed = now - g_dc_report_stop_tick;
    if (elapsed >= EW_DEVICECONNECT_REG_DIM_MS)
    {
        dc_reg_countdown_set_visible(false);
        return;
    }

    uint32_t remain_ms = EW_DEVICECONNECT_REG_DIM_MS - elapsed;
    uint32_t remain_s = (remain_ms + 999u) / 1000u;
    char buf[16];
    (void)snprintf(buf, sizeof(buf), "%lus", (unsigned long)remain_s);
    lv_label_set_text(g_dc_lbl_reg_countdown, buf);
    dc_reg_countdown_set_visible(true);

    /* 跟随“注册状态文本”位置放在右侧（旁边） */
    if (ui->DeviceConnect_lbl_stat_reg && lv_obj_is_valid(ui->DeviceConnect_lbl_stat_reg))
    {
        int32_t x = lv_obj_get_x(ui->DeviceConnect_lbl_stat_reg) + lv_obj_get_width(ui->DeviceConnect_lbl_stat_reg) + 8;
        int32_t y = lv_obj_get_y(ui->DeviceConnect_lbl_stat_reg);
        lv_obj_set_pos(g_dc_lbl_reg_countdown, x, y);
    }
}

static void dc_post_log_from_esp(const char *line, void *ctx)
{
    (void)ctx;
    if (!g_dc_q || !line)
        return;
    /* 默认过滤掉“每秒刷屏”的调试统计，避免长期运行导致 LVGL 卡死 */
#ifndef EW_DEVICECONNECT_LOG_VERBOSE
#define EW_DEVICECONNECT_LOG_VERBOSE 0
#endif
#if (EW_DEVICECONNECT_LOG_VERBOSE == 0)
    if (strncmp(line, "[调试]", 6) == 0) {
        return;
    }
#endif
    dc_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = DC_MSG_LOG;
    /* 截断并去掉末尾 \r */
    size_t n = strlen(line);
    if (n >= sizeof(m.text))
        n = sizeof(m.text) - 1;
    memcpy(m.text, line, n);
    m.text[n] = 0;
    /* 将 \r\n 统一成 \n，避免 textarea 出现空行 */
    for (size_t i = 0; m.text[i]; ++i)
    {
        if (m.text[i] == '\r')
            m.text[i] = '\n';
    }
    (void)osMessageQueuePut(g_dc_q, &m, 0U, 0U);
}

static void dc_post_step_from_esp(esp_ui_cmd_t step, bool ok, void *ctx)
{
    (void)ctx;
    if (!g_dc_q)
        return;
    dc_msg_t m;
    memset(&m, 0, sizeof(m));
    m.type = DC_MSG_STEP_DONE;
    m.step = (uint8_t)step;
    m.ok = ok ? 1U : 0U;
    (void)osMessageQueuePut(g_dc_q, &m, 0U, 0U);
}

static void dc_console_append(lv_ui *ui, const char *line)
{
    if (!ui || !ui->DeviceConnect_ta_console || !line)
        return;
    if (!lv_obj_is_valid(ui->DeviceConnect_ta_console))
        return;
    if (line[0] == '\0')
        return;

    /* 控制最大长度，避免无限增长 */
    const uint32_t max_len = 2048;
    if (lv_textarea_get_text(ui->DeviceConnect_ta_console) &&
        strlen(lv_textarea_get_text(ui->DeviceConnect_ta_console)) > (max_len - 256))
    {
        lv_textarea_set_text(ui->DeviceConnect_ta_console, "> (log truncated)\n");
    }

    /* 模仿 HTML：每行前加 "> " */
    if (line[0] != '>' )
    {
        lv_textarea_add_text(ui->DeviceConnect_ta_console, "> ");
    }
    lv_textarea_add_text(ui->DeviceConnect_ta_console, line);
    size_t last = strlen(line);
    if (last == 0 || line[last - 1] != '\n')
        lv_textarea_add_text(ui->DeviceConnect_ta_console, "\n");

    lv_textarea_set_cursor_pos(ui->DeviceConnect_ta_console, LV_TEXTAREA_CURSOR_LAST);
}

static void dc_led_set_state(lv_obj_t *led, uint32_t color_hex, bool on)
{
    if (!led || !lv_obj_is_valid(led))
        return;
    lv_led_set_color(led, lv_color_hex(color_hex));
    if (on)
        lv_led_on(led);
    else
        lv_led_off(led);
}

static void dc_step_reset_ui(lv_obj_t *led, lv_obj_t *lbl_stat, lv_obj_t *btn, const char *stat_text)
{
    if (led && lv_obj_is_valid(led))
        dc_led_set_state(led, 0x999999, false);
    if (lbl_stat && lv_obj_is_valid(lbl_stat))
        lv_label_set_text(lbl_stat, stat_text ? stat_text : "未连接 (Idle)");
    /* 未上报时：按钮应该随时可点。这里仅清掉 disabled（若正在上报，外层会拦截点击） */
    if (btn && lv_obj_is_valid(btn))
        lv_obj_clear_state(btn, LV_STATE_DISABLED);
}

/* ============ DeviceConnect: 进入界面时从 SD 加载 WiFi/Server 配置，并写入 ESP 缓冲区 ============ */
static bool dc_apply_cfg_to_esp_from_files(lv_ui *ui)
{
    memset(&g_dc_cfg_buf, 0, sizeof(g_dc_cfg_buf));

    FRESULT r1 = ui_wifi_cfg_read_file(g_dc_cfg_buf.ssid, sizeof(g_dc_cfg_buf.ssid),
                                       g_dc_cfg_buf.pwd, sizeof(g_dc_cfg_buf.pwd));
    FRESULT r2 = ui_server_cfg_read_file(g_dc_cfg_buf.ip, sizeof(g_dc_cfg_buf.ip),
                                         g_dc_cfg_buf.port_s, sizeof(g_dc_cfg_buf.port_s),
                                         g_dc_cfg_buf.node_id, sizeof(g_dc_cfg_buf.node_id),
                                         g_dc_cfg_buf.node_loc, sizeof(g_dc_cfg_buf.node_loc));
    if (r1 != FR_OK || r2 != FR_OK)
    {
        /* 不在这里大量刷日志，避免进入界面时频繁重绘导致“乱飞”观感 */
        printf("[DC_CFG] load fail: wifi=%d server=%d\r\n", (int)r1, (int)r2);
        return false;
    }

    uint32_t port_u = 0;
    if (g_dc_cfg_buf.port_s[0])
        port_u = (uint32_t)atoi(g_dc_cfg_buf.port_s);
    if (port_u == 0 || port_u > 65535U)
        port_u = 0;

    SystemConfig_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    strncpy(cfg.wifi_ssid, g_dc_cfg_buf.ssid, sizeof(cfg.wifi_ssid) - 1);
    strncpy(cfg.wifi_password, g_dc_cfg_buf.pwd, sizeof(cfg.wifi_password) - 1);
    strncpy(cfg.server_ip, g_dc_cfg_buf.ip, sizeof(cfg.server_ip) - 1);
    cfg.server_port = (uint16_t)port_u;
    strncpy(cfg.node_id, g_dc_cfg_buf.node_id, sizeof(cfg.node_id) - 1);
    strncpy(cfg.node_location, g_dc_cfg_buf.node_loc, sizeof(cfg.node_location) - 1);

    ESP_Config_Apply(&cfg);
    printf("[DC_CFG] applied: ssid=%s ip=%s port=%u id=%s loc=%s\r\n",
           cfg.wifi_ssid, cfg.server_ip, (unsigned)cfg.server_port, cfg.node_id, cfg.node_location);
    return true;
}

static void DeviceConnect_cfg_load_timer_cb(lv_timer_t *t)
{
    lv_ui *ui = (lv_ui *)lv_timer_get_user_data(t);
    lv_timer_del(t);
    g_dc_cfg_timer = NULL;
    if (!ui) return;
    g_dc_cfg_loaded = dc_apply_cfg_to_esp_from_files(ui) ? 1U : 0U;
    if (!g_dc_cfg_loaded) {
        dc_console_append(ui, "Config not ready. Please enter WiFi/Server config and save.");
    }
}

static void DeviceConnect_screen_event_handler(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_SCREEN_LOADED)
        return;
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui) return;
    g_dc_cfg_loaded = 0;
    /* 不在这里刷控制台，避免进入界面时大量文本更新导致重绘异常观感 */
    /* 去重：若已存在 timer，先删除再创建 */
    if (g_dc_cfg_timer) {
        lv_timer_del(g_dc_cfg_timer);
        g_dc_cfg_timer = NULL;
    }
    /* 延迟一帧，避免首帧渲染被 SD IO 阻塞 */
    g_dc_cfg_timer = lv_timer_create(DeviceConnect_cfg_load_timer_cb, 120, ui);
}

static bool dc_require_cfg_loaded(lv_ui *ui)
{
    if (g_dc_cfg_loaded)
        return true;
    dc_console_append(ui, "Config not loaded. Go to WiFi/Server config and save first.");
    return false;
}

/* 规则：点击某一步时，其后的步骤需要全部“重新执行”——LED 灭掉、状态恢复 Idle、清掉相关就绪标志 */
static void dc_reset_following_steps(lv_ui *ui, esp_ui_cmd_t clicked_step)
{
    if (!ui)
        return;

    /* 任何“重新走流程”的动作，都应终止“注册过期倒计时/变暗”UI 状态 */
    g_dc_reg_dimmed = 0;
    g_dc_report_stop_tick = 0;
    dc_reg_countdown_set_visible(false);
    dc_set_reg_dim(ui, false);

    /* Report 步骤：恢复按钮文案/配色（避免残留“停止上报/红色”） */
    if (ui->DeviceConnect_lbl_btn_report && lv_obj_is_valid(ui->DeviceConnect_lbl_btn_report))
        lv_label_set_text(ui->DeviceConnect_lbl_btn_report, "开始上报");
    if (ui->DeviceConnect_btn_report && lv_obj_is_valid(ui->DeviceConnect_btn_report))
    {
        lv_obj_set_style_bg_color(ui->DeviceConnect_btn_report, lv_color_hex(0x3dfb00), LV_PART_MAIN);
    }
    if (ui->DeviceConnect_lbl_btn_report && lv_obj_is_valid(ui->DeviceConnect_lbl_btn_report))
    {
        lv_obj_set_style_text_color(ui->DeviceConnect_lbl_btn_report, lv_color_hex(0x2F35DA), LV_PART_MAIN);
    }

    switch (clicked_step)
    {
    case ESP_UI_CMD_WIFI:
        /* WiFi 之后：TCP/REG/REPORT 必须重做 */
        dc_step_reset_ui(ui->DeviceConnect_led_tcp, ui->DeviceConnect_lbl_stat_tcp, ui->DeviceConnect_btn_tcp, "未连接 (Idle)");
        dc_step_reset_ui(ui->DeviceConnect_led_reg, ui->DeviceConnect_lbl_stat_reg, ui->DeviceConnect_btn_reg, "未连接 (Idle)");
        dc_step_reset_ui(ui->DeviceConnect_led_report, ui->DeviceConnect_lbl_stat_report, ui->DeviceConnect_btn_report, "未连接 (Idle)");
        break;
    case ESP_UI_CMD_TCP:
        /* TCP 之后：REG/REPORT 必须重做 */
        dc_step_reset_ui(ui->DeviceConnect_led_reg, ui->DeviceConnect_lbl_stat_reg, ui->DeviceConnect_btn_reg, "未连接 (Idle)");
        dc_step_reset_ui(ui->DeviceConnect_led_report, ui->DeviceConnect_lbl_stat_report, ui->DeviceConnect_btn_report, "未连接 (Idle)");
        /* 清掉“可上报就绪”标志，强制重新注册 */
        ESP_UI_InvalidateReg();
        break;
    case ESP_UI_CMD_REG:
        /* REG 之后：REPORT 必须重做（但 REG 本身会在 ESP 侧重新设置就绪） */
        dc_step_reset_ui(ui->DeviceConnect_led_report, ui->DeviceConnect_lbl_stat_report, ui->DeviceConnect_btn_report, "未连接 (Idle)");
        break;
    case ESP_UI_CMD_AUTO_CONNECT:
        /* 一键连接：从头开始，清空所有步骤显示 */
        dc_step_reset_ui(ui->DeviceConnect_led_wifi, ui->DeviceConnect_lbl_stat_wifi, ui->DeviceConnect_btn_wifi, "未连接 (Idle)");
        dc_step_reset_ui(ui->DeviceConnect_led_tcp, ui->DeviceConnect_lbl_stat_tcp, ui->DeviceConnect_btn_tcp, "未连接 (Idle)");
        dc_step_reset_ui(ui->DeviceConnect_led_reg, ui->DeviceConnect_lbl_stat_reg, ui->DeviceConnect_btn_reg, "未连接 (Idle)");
        dc_step_reset_ui(ui->DeviceConnect_led_report, ui->DeviceConnect_lbl_stat_report, ui->DeviceConnect_btn_report, "未连接 (Idle)");
        /* 清掉“可上报就绪”标志，避免复用旧注册 */
        ESP_UI_InvalidateReg();
        break;
    default:
        break;
    }
}

static void dc_set_processing(lv_ui *ui, esp_ui_cmd_t step, const char *text)
{
    if (!ui)
        return;
    switch (step)
    {
    case ESP_UI_CMD_WIFI:
        dc_led_set_state(ui->DeviceConnect_led_wifi, 0xFFA500, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_wifi, text ? text : "Processing...");
        break;
    case ESP_UI_CMD_TCP:
        dc_led_set_state(ui->DeviceConnect_led_tcp, 0xFFA500, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_tcp, text ? text : "Processing...");
        break;
    case ESP_UI_CMD_REG:
        dc_led_set_state(ui->DeviceConnect_led_reg, 0xFFA500, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_reg, text ? text : "Processing...");
        break;
    case ESP_UI_CMD_REPORT_TOGGLE:
        dc_led_set_state(ui->DeviceConnect_led_report, 0xFFA500, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_report, text ? text : "Uploading...");
        break;
    default:
        break;
    }
}

static void dc_set_done(lv_ui *ui, esp_ui_cmd_t step, bool ok)
{
    if (!ui)
        return;

    uint32_t c_ok = 0x3dfb00;
    uint32_t c_err = 0xff4444;

    switch (step)
    {
    case ESP_UI_CMD_WIFI:
        dc_led_set_state(ui->DeviceConnect_led_wifi, ok ? c_ok : c_err, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_wifi, ok ? "WiFi Connected" : "WiFi Failed");
        /* WiFi 按钮永远可再次点击（除非已开始上报） */
        lv_obj_clear_state(ui->DeviceConnect_btn_wifi, LV_STATE_DISABLED);
        break;
    case ESP_UI_CMD_TCP:
        dc_led_set_state(ui->DeviceConnect_led_tcp, ok ? c_ok : c_err, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_tcp, ok ? "TCP Linked" : "TCP Failed");
        lv_obj_clear_state(ui->DeviceConnect_btn_tcp, LV_STATE_DISABLED);
        break;
    case ESP_UI_CMD_REG:
        dc_led_set_state(ui->DeviceConnect_led_reg, ok ? c_ok : c_err, true);
        lv_label_set_text(ui->DeviceConnect_lbl_stat_reg, ok ? "Registered" : "Register Failed");
        lv_obj_clear_state(ui->DeviceConnect_btn_reg, LV_STATE_DISABLED);
        if (ok)
        {
            /* 用户新需求：
             * 注册成功后即开始倒计时；若不点击“开始上报”，到期需重新注册。
             * 复用 g_dc_report_stop_tick 作为“注册有效期倒计时起点”（开始上报会清零；停止上报会重置为停止时刻）。
             */
            if (!ESP_UI_IsReporting())
            {
                g_dc_report_stop_tick = lv_tick_get();
                g_dc_reg_dimmed = 0;
                dc_set_reg_dim(ui, false);
                dc_reg_countdown_update(ui);
            }
        }
        break;
    case ESP_UI_CMD_REPORT_TOGGLE:
    {
        bool reporting = ESP_UI_IsReporting();
        if (!ok && !reporting)
        {
            dc_led_set_state(ui->DeviceConnect_led_report, c_err, true);
            lv_label_set_text(ui->DeviceConnect_lbl_stat_report, "Need REG first");
        }
        else
        {
            dc_led_set_state(ui->DeviceConnect_led_report, c_ok, reporting);
            lv_label_set_text(ui->DeviceConnect_lbl_stat_report, reporting ? "Uploading..." : "Paused");
        }

        if (reporting)
        {
            lv_label_set_text(ui->DeviceConnect_lbl_btn_report, "停止上报");
            lv_obj_set_style_bg_color(ui->DeviceConnect_btn_report, lv_color_hex(0xff4444), LV_PART_MAIN);
            lv_obj_set_style_text_color(ui->DeviceConnect_lbl_btn_report, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        }
        else
        {
            lv_label_set_text(ui->DeviceConnect_lbl_btn_report, "开始上报");
            lv_obj_set_style_bg_color(ui->DeviceConnect_btn_report, lv_color_hex(0x3dfb00), LV_PART_MAIN);
            lv_obj_set_style_text_color(ui->DeviceConnect_lbl_btn_report, lv_color_hex(0x2F35DA), LV_PART_MAIN);
        }

        lv_obj_clear_state(ui->DeviceConnect_btn_report, LV_STATE_DISABLED);

        /* 上报开始后，锁定连接按钮；停止上报后恢复 */
        dc_set_buttons_enabled(ui, !reporting);

        if (reporting)
        {
            /* 开始上报：清除“注册过期”状态 */
            g_dc_report_stop_tick = 0;
            g_dc_reg_dimmed = 0;
            dc_reg_countdown_set_visible(false);
        }
        else
        {
            /* 停止上报：启动计时 */
            g_dc_report_stop_tick = lv_tick_get();
            dc_reg_countdown_update(ui);
        }
    }
    break;
    default:
        break;
    }
}

static void dc_timer_cb(lv_timer_t *t)
{
    (void)t;
    if (!g_dc_q || !g_dc_ui)
        return;

    /* 停止上报后的倒计时：每次都刷新一次显示；到期后触发“需重新注册” */
    if (!ESP_UI_IsReporting() && g_dc_report_stop_tick != 0 && !g_dc_reg_dimmed)
    {
        dc_reg_countdown_update(g_dc_ui);

        uint32_t now = lv_tick_get();
        if ((now - g_dc_report_stop_tick) >= EW_DEVICECONNECT_REG_DIM_MS)
        {
            g_dc_reg_dimmed = 1;
            if (g_dc_ui->DeviceConnect_lbl_stat_reg && lv_obj_is_valid(g_dc_ui->DeviceConnect_lbl_stat_reg))
            {
                lv_label_set_text(g_dc_ui->DeviceConnect_lbl_stat_reg, "需重新注册");
            }
            /* 注册过期：LED 需要灭掉（变灰），提示必须重新注册；上报也显示停止 */
            dc_led_set_state(g_dc_ui->DeviceConnect_led_reg, 0x999999, false);
            dc_led_set_state(g_dc_ui->DeviceConnect_led_report, 0x999999, false);
            if (g_dc_ui->DeviceConnect_lbl_stat_report && lv_obj_is_valid(g_dc_ui->DeviceConnect_lbl_stat_report))
            {
                lv_label_set_text(g_dc_ui->DeviceConnect_lbl_stat_report, "Stopped");
            }
            if (g_dc_ui->DeviceConnect_lbl_btn_report && lv_obj_is_valid(g_dc_ui->DeviceConnect_lbl_btn_report))
            {
                lv_label_set_text(g_dc_ui->DeviceConnect_lbl_btn_report, "开始上报");
            }
            dc_console_append(g_dc_ui, "Registration expired, click REG again.");
            ESP_UI_InvalidateReg();
            dc_reg_countdown_set_visible(false);
        }
    }
    else
    {
        /* 非倒计时阶段：隐藏倒计时 */
        dc_reg_countdown_set_visible(false);
    }

    dc_msg_t m;
    /* 每次最多处理 N 条消息，避免某次日志爆发导致 LVGL 卡顿/假死 */
    uint32_t budget = 6;
    while (budget-- && osMessageQueueGet(g_dc_q, &m, NULL, 0U) == osOK)
    {
        if (m.type == DC_MSG_LOG)
        {
            dc_console_append(g_dc_ui, m.text);
        }
        else if (m.type == DC_MSG_STEP_DONE)
        {
            esp_ui_cmd_t step = (esp_ui_cmd_t)m.step;
            bool ok = (m.ok != 0U);
            dc_set_done(g_dc_ui, step, ok);

            /* 自动流程：成功后把下一步置为 processing（更像 HTML 的“串行执行”效果） */
            if (g_dc_auto_running)
            {
                if (!ok)
                {
                    g_dc_auto_running = 0;
                    /* 自动流程失败：恢复按钮（若未上报） */
                    dc_set_buttons_enabled(g_dc_ui, !ESP_UI_IsReporting());
                }
                else
                {
                    if (step == ESP_UI_CMD_WIFI)
                        dc_set_processing(g_dc_ui, ESP_UI_CMD_TCP, "Processing...");
                    else if (step == ESP_UI_CMD_TCP)
                        dc_set_processing(g_dc_ui, ESP_UI_CMD_REG, "Processing...");
                    else if (step == ESP_UI_CMD_REG)
                        dc_set_processing(g_dc_ui, ESP_UI_CMD_REPORT_TOGGLE, "Uploading...");
                    else if (step == ESP_UI_CMD_REPORT_TOGGLE)
                    {
                        g_dc_auto_running = 0;
                        dc_set_buttons_enabled(g_dc_ui, !ESP_UI_IsReporting());
                    }
                }
            }
        }
    }
}

static void DeviceConnect_auto_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui || !ui->DeviceConnect_ta_console) {
        return;
    }
    lv_indev_wait_release(lv_indev_active());
    if (!dc_require_cfg_loaded(ui)) {
        return;
    }
    if (ESP_UI_IsReporting()) {
        dc_console_append(ui, "Reporting active, stop reporting first.");
        return;
    }
    g_dc_auto_running = 1;

    /* 点击“一键连接”：后续步骤全部重新执行（从头开始） */
    dc_reset_following_steps(ui, ESP_UI_CMD_AUTO_CONNECT);

    /* 自动流程中仅临时禁用连接按钮，结束后恢复 */
    dc_set_buttons_enabled(ui, false);
    lv_obj_add_state(ui->DeviceConnect_btn_report, LV_STATE_DISABLED);

    dc_console_append(ui, "Auto sequence started...");
    dc_set_processing(ui, ESP_UI_CMD_WIFI, "Processing...");
    (void)ESP_UI_SendCmd(ESP_UI_CMD_AUTO_CONNECT);
}

static void DeviceConnect_wifi_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui)
        return;
    lv_indev_wait_release(lv_indev_active());
    if (!dc_require_cfg_loaded(ui)) {
        return;
    }
    if (ESP_UI_IsReporting()) {
        dc_console_append(ui, "Reporting active, stop reporting first.");
        return;
    }
    g_dc_auto_running = 0;

    /* 点击 WiFi：其后的 TCP/REG/REPORT 全部重置，强制重新执行 */
    dc_reset_following_steps(ui, ESP_UI_CMD_WIFI);

    lv_obj_add_state(ui->DeviceConnect_btn_wifi, LV_STATE_DISABLED);
    dc_set_processing(ui, ESP_UI_CMD_WIFI, "Processing...");
    dc_console_append(ui, "Executing WIFI...");
    (void)ESP_UI_SendCmd(ESP_UI_CMD_WIFI);
}

static void DeviceConnect_tcp_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui)
        return;
    lv_indev_wait_release(lv_indev_active());
    if (!dc_require_cfg_loaded(ui)) {
        return;
    }
    if (ESP_UI_IsReporting()) {
        dc_console_append(ui, "Reporting active, stop reporting first.");
        return;
    }
    g_dc_auto_running = 0;

    /* 点击 TCP：其后的 REG/REPORT 全部重置，强制重新执行 */
    dc_reset_following_steps(ui, ESP_UI_CMD_TCP);

    lv_obj_add_state(ui->DeviceConnect_btn_tcp, LV_STATE_DISABLED);
    dc_set_processing(ui, ESP_UI_CMD_TCP, "Processing...");
    dc_console_append(ui, "Executing TCP...");
    (void)ESP_UI_SendCmd(ESP_UI_CMD_TCP);
}

static void DeviceConnect_reg_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui)
        return;
    lv_indev_wait_release(lv_indev_active());
    if (!dc_require_cfg_loaded(ui)) {
        return;
    }
    if (ESP_UI_IsReporting()) {
        dc_console_append(ui, "Reporting active, stop reporting first.");
        return;
    }
    g_dc_auto_running = 0;
    if (g_dc_reg_dimmed) {
        g_dc_reg_dimmed = 0;
        g_dc_report_stop_tick = 0;
        dc_reg_countdown_set_visible(false);
    }

    /* 点击 REG：其后的 REPORT 全部重置，强制重新执行 */
    dc_reset_following_steps(ui, ESP_UI_CMD_REG);

    lv_obj_add_state(ui->DeviceConnect_btn_reg, LV_STATE_DISABLED);
    dc_set_processing(ui, ESP_UI_CMD_REG, "Processing...");
    dc_console_append(ui, "Executing REG...");
    (void)ESP_UI_SendCmd(ESP_UI_CMD_REG);
}

static void DeviceConnect_report_event_handler(lv_event_t *e)
{
    lv_ui *ui = (lv_ui *)lv_event_get_user_data(e);
    if (!ui)
        return;
    lv_indev_wait_release(lv_indev_active());
    if (!dc_require_cfg_loaded(ui)) {
        return;
    }
    lv_obj_add_state(ui->DeviceConnect_btn_report, LV_STATE_DISABLED);
    dc_set_processing(ui, ESP_UI_CMD_REPORT_TOGGLE, "Uploading...");
    (void)ESP_UI_SendCmd(ESP_UI_CMD_REPORT_TOGGLE);
}

void events_init_DeviceConnect(lv_ui *ui)
{
    lv_obj_add_event_cb(ui->DeviceConnect_btn_back, DeviceConnect_back_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->DeviceConnect_btn_auto, DeviceConnect_auto_event_handler, LV_EVENT_CLICKED, ui);

    lv_obj_add_event_cb(ui->DeviceConnect_btn_wifi, DeviceConnect_wifi_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->DeviceConnect_btn_tcp, DeviceConnect_tcp_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->DeviceConnect_btn_reg, DeviceConnect_reg_event_handler, LV_EVENT_CLICKED, ui);
    lv_obj_add_event_cb(ui->DeviceConnect_btn_report, DeviceConnect_report_event_handler, LV_EVENT_CLICKED, ui);

    /* 每次进入 DeviceConnect 界面都自动从 SD 加载配置并应用到 ESP 缓冲区 */
    lv_obj_add_event_cb(ui->DeviceConnect, DeviceConnect_screen_event_handler, LV_EVENT_SCREEN_LOADED, ui);

    /* 注册 hook：ESP 任务会把日志/步骤结果通过消息队列投递给 LVGL 线程 */
    g_dc_ui = ui;
    if (!g_dc_q)
        g_dc_q = osMessageQueueNew(24, sizeof(dc_msg_t), NULL);
    ESP_UI_SetHooks(dc_post_log_from_esp, NULL, dc_post_step_from_esp, NULL);

    if (!g_dc_timer)
        g_dc_timer = lv_timer_create(dc_timer_cb, 50, NULL);

    /* “重新注册倒计时”标签：显示在注册状态右侧 */
    if (!g_dc_lbl_reg_countdown && ui->DeviceConnect_cont_panel && lv_obj_is_valid(ui->DeviceConnect_cont_panel))
    {
        g_dc_lbl_reg_countdown = lv_label_create(ui->DeviceConnect_cont_panel);
        lv_label_set_text(g_dc_lbl_reg_countdown, "");
        lv_obj_set_style_text_font(g_dc_lbl_reg_countdown, &lv_font_montserrat_12, LV_PART_MAIN);
        lv_obj_set_style_text_color(g_dc_lbl_reg_countdown, lv_color_hex(0x666666), LV_PART_MAIN);
        lv_obj_add_flag(g_dc_lbl_reg_countdown, LV_OBJ_FLAG_HIDDEN);
        dc_reg_countdown_update(ui);
    }
}


void events_init(lv_ui *ui)
{

}
