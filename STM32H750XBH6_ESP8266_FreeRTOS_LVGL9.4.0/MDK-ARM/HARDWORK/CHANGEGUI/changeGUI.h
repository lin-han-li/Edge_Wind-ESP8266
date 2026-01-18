#ifndef _CHANGE_GUI_H_
#define _CHANGE_GUI_H_




//	bool screen_3_del;
//	lv_obj_t *screen_3_cont_1;
//	lv_obj_t *screen_3_keyboard1;
//	lv_obj_t *screen_3_textarea1;




//// 全局变量
//float input_value;


//static void keyboard_event_handler(lv_event_t *e) 
//{
//    lv_event_code_t code = lv_event_get_code(e);
//    
//    switch (code) {
//    case LV_EVENT_READY:  // 按下键盘确认键
//    {
//        lv_obj_t * kb = lv_event_get_target(e);
//        lv_obj_t * ta = lv_keyboard_get_textarea(kb);  // 获取关联的文本框
//        
//        // 1. 获取输入内容并存储到变量
//        const char * txt = lv_textarea_get_text(ta);
//        input_value = atof(txt);
//        printf("输入值: %f\n", input_value);  // 实际使用时替换为变量存储
//        AD9833_Setup(AD9833_REG_FREQ0,input_value,AD9833_REG_PHASE1,1024,AD9833_OUT_SINUS);
//        // 2. 清除文本区域
//        lv_textarea_set_text(ta, "");  // 重置为空字符串
//        
//        // 3. 隐藏键盘
////        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//        break;
//    }
//    case LV_EVENT_CANCEL:  // 取消输入
//    {
//        lv_obj_t * kb = lv_event_get_target(e);
//        lv_obj_t * ta = lv_keyboard_get_textarea(kb);
//        
//        // 清除文本区域但不保存值
//        lv_textarea_set_text(ta, "");
////		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//        break;
//    }
//    default:
//        break;
//    }
//}


//void events_init_screen_3 (lv_ui *ui)
//{
//    lv_obj_add_event_cb(ui->screen_3, screen_3_event_handler, LV_EVENT_ALL, ui);
//	  lv_obj_add_event_cb(ui->screen_3_keyboard1, keyboard_event_handler, LV_EVENT_ALL, ui);
//}















//   //The custom code of VCO1_SET.

//    //   /* 定义并创建键盘 */
//       ui->VCO1_SET_keyboard1= lv_keyboard_create(ui->VCO1_SET);

//    //   /* 定义并创建文本框 */
//      ui->VCO1_SET_textarea1 = lv_textarea_create(ui->VCO1_SET);

//    //   /* 设置文本框宽度 */

//       lv_obj_set_width(ui->VCO1_SET_textarea1, scr_act_width() - 400);

//    //   /* 设置文本框高度 */

//       lv_obj_set_height(ui->VCO1_SET_textarea1, (scr_act_height() >> 1) - 10);
//       lv_obj_align(ui->VCO1_SET_textarea1, LV_ALIGN_TOP_LEFT, 0, 0);

//    //   /* 为键盘指定一个文本区域 */
//       lv_keyboard_set_textarea(ui->VCO1_SET_keyboard1, ui->VCO1_SET_textarea1);

//    //   /* 开启按键弹窗 */
//       lv_keyboard_set_popovers(ui->VCO1_SET_keyboard1, true);



//    //The custom code of PIVD_SET.
//    //   /* 定义并创建键盘 */
//       ui->PIVD_SET_keyboard1= lv_keyboard_create(ui->PIVD_SET);

//    //   /* 定义并创建文本框 */
//      ui->PIVD_SET_textarea1 = lv_textarea_create(ui->PIVD_SET);

//    //   /* 设置文本框宽度 */

//       lv_obj_set_width(ui->PIVD_SET_textarea1, scr_act_width() - 400);

//    //   /* 设置文本框高度 */

//       lv_obj_set_height(ui->PIVD_SET_textarea1, (scr_act_height() >> 1) - 10);
//       lv_obj_align(ui->PIVD_SET_textarea1, LV_ALIGN_TOP_LEFT, 0, 0);

//    //   /* 为键盘指定一个文本区域 */
//       lv_keyboard_set_textarea(ui->PIVD_SET_keyboard1, ui->PIVD_SET_textarea1);

//    //   /* 开启按键弹窗 */
//       lv_keyboard_set_popovers(ui->PIVD_SET_keyboard1, true);






//	lv_obj_t *VCO1_SET;
//	bool VCO1_SET_del;
//	lv_obj_t *VCO1_SET_label_1;
//	lv_obj_t *VCO1_SET_cont_1;
//	lv_obj_t *VCO1_SET_keyboard1;
//	lv_obj_t *VCO1_SET_textarea1;


//	lv_obj_t *PIVD_SET;
//	bool PIVD_SET_del;
//	lv_obj_t *PIVD_SET_label_1;
//	lv_obj_t *PIVD_SET_cont_1;
//	lv_obj_t *PIVD_SET_keyboard1;
//	lv_obj_t *PIVD_SET_textarea1;





//示波器
//static void VCO1_SET_event_handler (lv_event_t *e)
//{
//    lv_event_code_t code = lv_event_get_code(e);
//    switch (code) {
//    case LV_EVENT_SCREEN_LOADED:
//    {
//        VCO1_keyboard_Flag=1;//进入VCO1调节界面
//        break;
//    }
//    case LV_EVENT_SCREEN_UNLOADED:
//    {
//        VCO1_keyboard_Flag=0;//退出VCO1调节界面
//        break;
//    }
//    case LV_EVENT_GESTURE:
//    {
//        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
//        switch(dir) {
//        case LV_DIR_BOTTOM:
//        {
//            lv_indev_wait_release(lv_indev_get_act());
//            ui_load_scr_animation(&guider_ui, &guider_ui.shiboqi, guider_ui.shiboqi_del, &guider_ui.VCO1_SET_del, setup_scr_shiboqi, LV_SCR_LOAD_ANIM_NONE, 20, 20, false, true);
//            break;
//        }
//        default:
//            break;
//        }
//        break;
//    }
//    default:
//        break;
//    }
//}

////添加VCO1事件函数
//static void VCO1_keyboard_event_handler(lv_event_t *e) 
//{
//    lv_event_code_t code = lv_event_get_code(e);
//    float u;
//    switch (code) {
//    case LV_EVENT_READY:  // 按下键盘确认键
//    {
//        lv_obj_t * kb = lv_event_get_target(e);
//        lv_obj_t * ta = lv_keyboard_get_textarea(kb);  // 获取关联的文本框
//        
//        // 1. 获取输入内容并存储到变量
//        const char * txt = lv_textarea_get_text(ta);
//        u = atof(txt);
//        printf("VCO1输入值: %f\n", u);  // 实际使用时替换为变量存储
//			  VC1=u;
//        R_SET_FS(VC1,P1, P3,Period);			
//        // 2. 清除文本区域
//        lv_textarea_set_text(ta, "");  // 重置为空字符串
//        
//        // 3. 隐藏键盘
////        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//        break;
//    }
//    case LV_EVENT_CANCEL:  // 取消输入
//    {
//        lv_obj_t * kb = lv_event_get_target(e);
//        lv_obj_t * ta = lv_keyboard_get_textarea(kb);
//        
//        // 清除文本区域但不保存值
//        lv_textarea_set_text(ta, "");
////		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//        break;
//    }
//    default:
//        break;
//    }
//}



//void events_init_VCO1_SET (lv_ui *ui)
//{
//    lv_obj_add_event_cb(ui->VCO1_SET, VCO1_SET_event_handler, LV_EVENT_ALL, ui);
//    lv_obj_add_event_cb(ui->VCO1_SET_keyboard1, VCO1_keyboard_event_handler, LV_EVENT_ALL, ui);	
//}

//static void PIVD_SET_event_handler (lv_event_t *e)
//{
//    lv_event_code_t code = lv_event_get_code(e);
//    switch (code) {
//    case LV_EVENT_SCREEN_LOADED:
//    {
//        PDIV1_keyboard_Flag=1;//进入PDIV1调节界面
//        break;
//    }
//    case LV_EVENT_SCREEN_UNLOADED:
//    {
//        PDIV1_keyboard_Flag=0;//退出PDIV1调节界面
//        break;
//    }
//    case LV_EVENT_GESTURE:
//    {
//        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
//        switch(dir) {
//        case LV_DIR_BOTTOM:
//        {
//            lv_indev_wait_release(lv_indev_get_act());
//            ui_load_scr_animation(&guider_ui, &guider_ui.shiboqi, guider_ui.shiboqi_del, &guider_ui.PIVD_SET_del, setup_scr_shiboqi, LV_SCR_LOAD_ANIM_NONE, 10, 10, false, true);
//            break;
//        }
//        default:
//            break;
//        }
//        break;
//    }
//    default:
//        break;
//    }
//}

////添加PDIV1事件函数
//static void PDIV1_keyboard_event_handler(lv_event_t *e) 
//{
//    lv_event_code_t code = lv_event_get_code(e);
//    float u;
//    switch (code) {
//    case LV_EVENT_READY:  // 按下键盘确认键
//    {
//        lv_obj_t * kb = lv_event_get_target(e);
//        lv_obj_t * ta = lv_keyboard_get_textarea(kb);  // 获取关联的文本框
//        
//        // 1. 获取输入内容并存储到变量
//        const char * txt = lv_textarea_get_text(ta);
//        u = atof(txt);
//        printf("PDIV1输入值: %f\n", u);  // 实际使用时替换为变量存储
////			  P1=u;
////			  P3=P1/2*Period;
////        R_SET_FS(VC1, P1, P3,Period);	
//          Auto_Key_Process(u);


//			
//        // 2. 清除文本区域
//        lv_textarea_set_text(ta, "");  // 重置为空字符串
//        
//        // 3. 隐藏键盘
////        lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//        break;
//    }
//    case LV_EVENT_CANCEL:  // 取消输入
//    {
//        lv_obj_t * kb = lv_event_get_target(e);
//        lv_obj_t * ta = lv_keyboard_get_textarea(kb);
//        
//        // 清除文本区域但不保存值
//        lv_textarea_set_text(ta, "");
////		lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
//        break;
//    }
//    default:
//        break;
//    }
//}


//void events_init_PIVD_SET (lv_ui *ui)
//{
//    lv_obj_add_event_cb(ui->PIVD_SET, PIVD_SET_event_handler, LV_EVENT_ALL, ui);
//    lv_obj_add_event_cb(ui->PIVD_SET_keyboard1, PDIV1_keyboard_event_handler, LV_EVENT_ALL, ui);
//}


//void events_init(lv_ui *ui)
//{

//}








#endif













