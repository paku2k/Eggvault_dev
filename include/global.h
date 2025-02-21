#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <ESP32AnalogRead.h>
#include <DS3231.h>
//#include <StateMachine.h>
//#include "RTClib.h"
#include <SparkFun_RV8803.h>
#include <TimeLib.h>
#include <Wire.h>
//#include "arduino-esp32\arduino-esp32-master\tools\sdk\include\driver\driver\rtc_io.h"
#include <Preferences.h>


//#include <menu.h>
//#include <menuIO/softKeyIn.h>
//#include <menuIO/liquidCrystalOut.h>
//#include <menuIO/chainStream.h>
//#include <menuIO/serialIn.h>

//using namespace Menu;
void writeValuesToFlash();
void makeLocalTime();


#define MAX_DEPTH 4

//macros
#define LCD_ON pinMode(LCD_EN, OUTPUT);\
               pinMode(LCD_BL_EN, OUTPUT);\
               digitalWrite(LCD_EN, LOW);\
               digitalWrite(LCD_BL_EN, LOW);

#define LCD_OFF pinMode(LCD_EN, INPUT); \
                pinMode(LCD_BL_EN, INPUT);

#define PREP_FOR_DEEP_SLEEP for (int i=12; i<32; i++) { \
                        if((i!=21)&&(i!=24)&&(i!=29)&&(i!=34)&&(i!=39)&&(i!=35)&&(i!=36)&&(i!=3)&&(i!=20)&&(i!=28)&&(i!=29)&&(i!=30)&&(i!=31)){\
                        gpio_set_pull_mode((gpio_num_t)i, GPIO_FLOATING);\
         }\
       }\
       if(digitalRead(LED) == 1){\
        gpio_hold_en((gpio_num_t)LED);\
       }\
       writeValuesToFlash();\
       LCD_OFF;\
       delay(200);\
       esp_sleep_enable_ext1_wakeup(WAKEUP_PIN_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);\
       esp_deep_sleep_start();\


//#define WAKEUP_PINMASK 0xF08000000
//#define WAKEUP_PINMASK 0x008000000
//pins
#define LED 2
#define LCD_RS 4
#define LCD_D5 5
#define END_LOW 39 //12
#define M_FWD 2 //13
#define M_BACK 2 //14
#define LCD_BL_EN 2 // 15
#define LCD_E 16
#define LCD_D4 17
#define LCD_D6 18
#define LCD_D7 19
#define I2C_SDA 21
#define I2C_SCL 22
#define LCD_EN 23
#define LDR_EN 25
#define END_UP 26
#define CLK_INT 27
#define SW_EXIT 32
#define SW_SELECT 33
#define SW_BACK 34
#define SW_FWD 35
#define LDR_VAL 36
#define V_BATT 2 //39

#define WAKEUP_PIN_BITMASK 0xF08000000


#define S_TO_uS 1000000
#define LDR_SLEEP 600 //Seconds Delay between Measurement
#define LDR_MEAS_SLEEP 60 //Seconds Delay between Measurement, when threshold was broken
#define CLOSE_LIGHT_BUFFER 60 //Minutes Delay before Closing by sensor takes place
#define LONG_PRESS_STARTUP_DURATION 2000 //ms for a long press at startup 

#define BATT_CRIT 5.3 //Critical Volts for Battery
#define BATT_MIN  5.0 //Minimum Volts for Battery
#define THRESHOLD_COUNT_MAX 2 //How often has the LDR to be over threshold

#define LCD_OFF_TIME 2000000 //Zeit bis das LCD dunkel wird nach dem letzten Tastendruck
#define NOX_SLEEP_TIME 4000000 //Zeit bis die Klappe einschläft nach dem letzten Tastendruck

#define BUTTON_DEBOUNCE_TIME_NORMAL 400 //Debounce time für normalen tastendruck
#define BUTTON_LONG_PRESS_TIME 1500 //Zeit, nachdem ein long button press erkannt wurde
#define BUTTON_DEBOUNCE_TIME_FAST 200 // Zeit die nach einem long button press für das inkrementieren gebraucht wird

#define MINLUX 0
#define MAXLUX 9

enum ERROR_CODE {NO_ERROR, TIMER_BLOCKED_ELAPSED, TIMER_OPEN_ELAPSED, TIMER_CLOSE_ELAPSED, WRONG_LDR_FLAG,WRONG_ALARM_FLAG,POS_BLOCKED_CANNOT_BE_SET,WRONG_ALARM_MODE,UNDEFINED};


enum KLAPPENPOSITION {POS_UP, POS_DOWN, POS_DRIVING, POS_BLOCKED};
enum KLAPPENRICHTUNG{MOVING_DOWN, FINISHING_DOWN, MOVING_UP, STANDING};
enum openingMode {LICHT, ZEIT, LICHT_ZEIT, NICHT};
enum TimerLogic {NEXT_OPEN, NEXT_CLOSE, NO_TIMER};
enum TimerReturnVal {GO_DOWN, GO_UP, SLEEP_LONG, SLEEP_SHORT};

enum OpenClose{OPEN, CLOSE};
enum TimerVar {BLOCKED_TIMER, MOVING_TIMER};

enum Language {DEUTSCH, ENGLISCH, FRANZ};
enum BTNAction {SELECT, EXIT, LEFT, RIGHT,NOTHING};
enum Setmode {SETMINUTE, SETHOUR, SETNOTHING, SETLUX, SETDAY, SETMONTH, SETYEAR, SETCONFIRM};
enum Daymode {WEEK, WEEKEND, DAILY};
enum Modeset {MODESET_OPENING, MODESET_CLOSING};
enum Days {MON, TUE, WED, THU, FRI, SAT, SUN};

// enum wakeup_reason {TIMER, ALARM, BTN_UP, BTN_DOWN, BTN_RIGHT, BTN_LEFT, END_UP, END_LOW};


#define M_INV 0
#define o_INV 1
#define D_INV 2
#define i_INV 3
#define F_INV 4
#define r_INV 5
#define S_INV 6
#define a_INV 7

#define TIME_SYM 0
#define LUX_SYM 1
#define LUX_TIME_SYM 2
#define ARROW_UP 3
#define ARROW_DOWN 4


// typedef struct Time{
//   uint8_t minute;
//   uint8_t hour;
//   uint8_t DoW;
//   uint8_t seconds;
//   uint8_t date;
//   uint8_t month;
//   uint16_t year;

//   bool Time::after (const Time& time2) const {
//     if(year > time2.year) return 
    
//     if (hour > time2.hour) return true;
//     if (hour < time2.hour) return false;

//     if (minute > time2.minute) return true;
//     if (minute < time2.minute) return false;

//     if (seconds > time2.seconds) return true;
//     return false;
//   }

// } mtime_t;


byte arrowUp[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};

byte arrowDown[] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100
};

byte time_sym[] = {
  B00000,
  B01110,
  B10001,
  B11101,
  B10101,
  B10101,
  B01110,
  B00000
};

byte lux_sym[] = {
  B01110,
  B10001,
  B10001,
  B10001,
  B01010,
  B01110,
  B01110,
  B00100
};

byte time_lux_sym[] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
  B00100
};


byte M_inv[] = {
  B01110,
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B01110,
  B11111
};

byte o_inv[] = {
  B11111,
  B11111,
  B10001,
  B01110,
  B01110,
  B01110,
  B10001,
  B11111
};

byte D_inv[] = {
  B00011,
  B01101,
  B01110,
  B01110,
  B01110,
  B01101,
  B00011,
  B11111
};

byte i_inv[] = {
  B11011,
  B11111,
  B10011,
  B11011,
  B11011,
  B11011,
  B10001,
  B11111
};

byte F_inv[] = {
  B00000,
  B01111,
  B01111,
  B00001,
  B01111,
  B01111,
  B01111,
  B11111
};

byte r_inv[] = {
  B11111,
  B11111,
  B01001,
  B00110,
  B01111,
  B01111,
  B01111,
  B11111
};

byte S_inv[] = {
  B10000,
  B01111,
  B01111,
  B10001,
  B11110,
  B11110,
  B00001,
  B11111
};

byte a_inv[] = {
  B11111,
  B11111,
  B11111,
  B10001,
  B11110,
  B10000,
  B01110,
  B10000
};







typedef struct {
    bool done;
    uint8_t hour;
    uint8_t minute;
    uint16_t lux;
    uint16_t delay; //delay till opening in seconds
    openingMode mode; //0=licht, 1=Zeit, 2=Licht&Zeit, 3=nichts
} doorDayAlarm_t;



#endif
