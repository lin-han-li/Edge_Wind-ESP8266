#include "changeGUI.h"


//对于GUI guider生成的文件要打开所以有图片的C源文件，修改头文件的引用
//例如，
//1.#include "lvgl/lvgl.h"需要修改为#include "lvgl.h"


//图表部件使用
//1.需要修改图表点数
//例如，打开图表所在屏幕的C源文件里的屏幕创建函数
//此处为setup_scr_screen_2，对其进行修改
//    lv_chart_set_point_count(ui->screen_2_chart_1, piont);
//需要将piont修改为显示的点数，否则会卡死
//对于图表的刷新需要在对应屏幕置标志位来实现
//图表数据的刷新方式如下
//A:直接操作式
//1.首先修改数据
//((&guider_ui)->screen_2_chart_1_1)->y_points[i] = 256;
//((&guider_ui)->screen_2_chart_1_0)->start_point = 0;
//即（对应图表的对应数据序列）所指向的Y值修改
//2.然后直接更新对应图表
//lv_chart_refresh((&guider_ui)->screen_2_chart_1);
//对于图表部件的使用需要在对应屏幕置下操作
//在其他屏幕不可对图表进行操作，否则会卡死

//文本部件的使用
//对于文本部件的显示内容的刷新方式如下
//1.首先将需要显示的内容取字符
//sprintf(text_2,"%.2fVPP",(adc_data_max-adc_data_min)/256.0/256*3.3);
//2.然后调用文本设置函数对相应的文本部件进行更新
//lv_label_set_text(ui->screen_2_label_1, (char *)text_2);
//对于文本部件的使用需要在对应屏幕置下操作


//滑块部件的使用
//滑块部件，人在按下进行滑动时触发的事件是pressing即LV_EVENT_PRESSING
//则对应需要在此事件下对数据进行修改
//而且滑块当前值的获取通过以下函数实现
//lv_slider_get_value((&guider_ui)->screen_2_slider_1);
//参数是对应部件号
//例如   
//case LV_EVENT_PRESSING:
//    {
//        //幅度控制正按下
//			PGA=lv_slider_get_value((&guider_ui)->screen_2_slider_1);
//        break;
//    }
//对于滑块部件的使用需要在对应屏幕置标志位来实现


//进度条部件的使用
//调用相应函数对进度条的当前值进行修改即可
//lv_bar_set_value(ui->screen_1_bar_1, 200, LV_ANIM_OFF);
//对于进度条部件的使用需要在对应屏幕置标志位来实现


//键盘部件的使用
//GUI guider没有键盘部件所以需要自己创建
//1.需要在GUI guider新建一个屏幕，用于刷新键盘

//2.在新建屏幕ui结构体后面添加键盘及文本区域作为全局ui的属性
//例如,打开GUIguider生成的头文件gui_guider.h，对结构体进行修改
//在需要刷新键盘的屏幕属性定义后面加上相关属性，
//此处刷新键盘的屏幕为screen_3
//则进行对代码以下操作，添加新的属性
//	bool screen_3_del;
//	lv_obj_t *screen_3_cont_1;
//	lv_obj_t *screen_3_keyboard1;
//	lv_obj_t *screen_3_textarea1;

//3.随后在对应的屏幕中，添加相应的LVGL代码以实现键盘部件的创建
//例如，打开键盘所在屏幕的C源文件里的屏幕创建函数
//此处为setup_scr_screen_3，对其进行修改
//添加以下代码实现键盘部件的创建
//    //The custom code of screen_3.
//   /* 定义并创建键盘 */
//   ui->screen_3_keyboard1= lv_keyboard_create(ui->screen_3);

//   /* 定义并创建文本框 */
//   ui->screen_3_textarea1 = lv_textarea_create(ui->screen_3);

//   /* 设置文本框宽度 */

//   lv_obj_set_width(ui->screen_3_textarea1, scr_act_width() - 10);

//   /* 设置文本框高度 */

//   lv_obj_set_height(ui->screen_3_textarea1, (scr_act_height() >> 1) - 10);
//   lv_obj_align(ui->screen_3_textarea1, LV_ALIGN_TOP_LEFT, 0, 0);

//   /* 为键盘指定一个文本区域 */
//   lv_keyboard_set_textarea(ui->screen_3_keyboard1, ui->screen_3_textarea1);

//   /* 开启按键弹窗 */
//   lv_keyboard_set_popovers(ui->screen_3_keyboard1, true); 

//4.除此之外还需要添加键盘输入事件用于实现键盘操作输入数据
//首先打开GUI guider生成的events_init.c源文件
//按照格式进行事件函数编写,以下为示例
// 全局变量
//extern float input_value;
//char te[20];

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
//        FRE = atof(txt);
//        printf("输入值: %d\n", FRE);  // 实际使用时替换为变量存储
//        AD9834_Set_Freq(FREQ_0, FRE);
//        B_AD9834_Set_Freq( B_FREQ_0,  FRE);
//        AD9833_Setup(AD9833_REG_FREQ0,FRE,AD9833_REG_PHASE1,1024,AD9833_OUT_SINUS);  	      
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

//编写完事件回调函数后将其放入对应屏幕的事件处理函数中
//此处位为screen_3
//修改如下
//void events_init_screen_3 (lv_ui *ui)
//{
//    lv_obj_add_event_cb(ui->screen_3, screen_3_event_handler, LV_EVENT_ALL, ui);
//	  lv_obj_add_event_cb(ui->screen_3_keyboard1, keyboard_event_handler, LV_EVENT_ALL, ui);
//}
//对应的是键盘的事件，所以第一个输入变量为ui->screen_3_keyboard1


//   //更新频率值
//  AD9834_Set_Freq(FREQ_0, lv_slider_get_value((&guider_ui)->screen_2_slider_2));
//  B_AD9834_Set_Freq( B_FREQ_0,  lv_slider_get_value((&guider_ui)->screen_2_slider_2));
//  AD9833_Setup(AD9833_REG_FREQ0,lv_slider_get_value((&guider_ui)->screen_2_slider_1),AD9833_REG_PHASE1,1024,AD9833_OUT_SINUS);	

//   //更新PGA的值			
//	writedataAD8370(lv_slider_get_value((&guider_ui)->screen_2_slider_1));	




//GUI guider的使用精华
//对于屏幕的标志位的实现
//应当在创建屏幕时，知晓哪些事件会触发更新这个屏幕
//在对应触发事件添加并行用户代码
//提前设计标志位变量
//置好标志位，加上注释
//生成文件时，先将外部变量引进，然后直接解开注释即可










