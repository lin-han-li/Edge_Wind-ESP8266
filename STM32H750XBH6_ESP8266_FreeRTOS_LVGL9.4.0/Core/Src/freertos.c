/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "led.h"
#include "touch_800x480.h"
#include "tim.h"
#include "stm32h7xx_hal.h"
#include "SD.h"
#include "fatfs.h"
#include "tim.h"

#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "demos/lv_demos.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// 任务·4
extern int AD9248_flag;
extern float AD9248_OrgSig[4096];
float Value, Value2;
uint16_t mask = 1 << 13;
uint16_t Bit15[4096];
uint16_t INA[4096];
uint16_t INB[4096];

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for ledTask */
osThreadId_t ledTaskHandle;
const osThreadAttr_t ledTask_attributes = {
  .name = "ledTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for uartTask */
osThreadId_t uartTaskHandle;
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 30000 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for ESP8266_Task */
osThreadId_t ESP8266_TaskHandle;
const osThreadAttr_t ESP8266_Task_attributes = {
  .name = "ESP8266_Task",
  .stack_size = 5128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartTask02(void *argument);
void StartTask03(void *argument);
void StartTask04(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void vApplicationTickHook(void);

/* USER CODE BEGIN 3 */
void vApplicationTickHook(void)
{
  /* This function will be called by each tick interrupt if
  configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. User code can be
  added here, but the tick hook is called from an interrupt context, so
  code must not attempt to block, and only the interrupt safe FreeRTOS API
  functions can be used (those that end in FromISR()). */

  lv_tick_inc(1);
}
/* USER CODE END 3 */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of ledTask */
  ledTaskHandle = osThreadNew(StartTask02, NULL, &ledTask_attributes);

  /* creation of uartTask */
  uartTaskHandle = osThreadNew(StartTask03, NULL, &uartTask_attributes);

  /* creation of ESP8266_Task */
  ESP8266_TaskHandle = osThreadNew(StartTask04, NULL, &ESP8266_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  uint32_t time;
  //  // LVGL图形库初始化三件套
  lv_init();            // 初始化LVGL核心库（内存管理、内部变量等）
  lv_port_disp_init();  // 初始化显示驱动接口（配置帧缓冲区、注册刷新回调）
  lv_port_indev_init(); // 初始化输入设备接口（注册触摸屏/编码器驱动）
  /* 启动 Demo */
  lv_demo_widgets();
  //  // 用户界面初始化
  //  setup_ui(&guider_ui);    // 创建UI组件（按钮/标签/列表等，通常由GUI设计工具生成）
  //  events_init(&guider_ui); // 绑定事件处理器（如按钮点击、滑块拖动等回调函数）
  /* Infinite loop */
  for (;;)
  {

    //    // === 临界区开始（保护LVGL操作）===
    osMutexAcquire(mutex_id, osWaitForever); // 获取互斥锁（防止多任务冲突）

    //    /* LVGL核心处理（必须在所有任务中周期性调用）*/
    lv_task_handler(); // 执行重绘/动画/事件分发（耗时操作，建议5-30ms调用一次）
                       //                       // 注意：任何LVGL API调用（如lv_label_set_text）都需要类似的锁保护

    //    /* 触摸输入采集 */
    //    Touch_Scan(); // 读取触摸控制器数据（如I2C通讯）
    //                  // 实际触摸事件通过lv_port_indev_init注册的回调注入LVGL

    osMutexRelease(mutex_id); // 释放互斥锁（允许其他任务访问共享资源）
                              //    // === 临界区结束 ===

    //    /* 周期延时（关键性能参数）*/
    osDelay(LV_DEF_REFR_PERIOD + 1); // 保持屏幕刷新率稳定（典型值30ms≈33FPS）

    // 注意：
    // 1. 延时过短：导致刷新不全（屏幕闪烁）
    // 2. 延时过长：界面响应迟滞
    // 3. LV_DISP_DEF_REFR_PERIOD应与屏显参数匹配（在lv_conf.h中配置）

    // 调试语句（使用时需注意串口输出可能影响实时性）
    // printf("GUI task heartbeat\r\n");
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartTask02 */
/**
 * @brief Function implementing the ledTask02 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask02 */
void StartTask02(void *argument)
{
  /* USER CODE BEGIN StartTask02 */
  /* Infinite loop */
  for (;;)
  {
    LED1_Toggle;
    osDelay(500);
  }
  /* USER CODE END StartTask02 */
}

/* USER CODE BEGIN Header_StartTask03 */
/**
 * @brief Function implementing the uartTask03 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask03 */
void StartTask03(void *argument)
{
  /* USER CODE BEGIN StartTask03 */

  /* Infinite loop */
  for (;;)
  {

    osDelay(2);
  }
  /* USER CODE END StartTask03 */
}

/* USER CODE BEGIN Header_StartTask04 */
/**
 * @brief Function implementing the AUTO_Task04 thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTask04 */
void StartTask04(void *argument)
{
  /* USER CODE BEGIN StartTask04 */
  int i;
  /* Infinite loop */
  for (;;)
  {

    osDelay(10);
  }
  /* USER CODE END StartTask04 */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

