
#include <Arduino.h>
#include <LCDMenuLib2.h>

//TaskScheduler
void Task_Serial_Blink_Example();
void Task_input_check();
void Task_LCDMenuLib();

//TaskScheduler
void lcdml_menu_control(void);

//TaskScheduler
void mDyn_para(uint8_t);

//TaskScheduler
void lcdml_menu_clear();
void lcdml_menu_display();

//TaskScheduler
void mFunc_thread_start(uint8_t);
void mFunc_thread_stop(uint8_t);
void mFunc_information(uint8_t);
void mFunc_timer_info(uint8_t);
void mFunc_p2(uint8_t);
void mFunc_back(uint8_t);
void mFunc_screensaver(uint8_t);
void mFunc_goToRootMenu(uint8_t);
void mFunc_jumpTo_timer_info(uint8_t);
void mFunc_para(uint8_t);
