#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <Arduino.h>
#include <LiquidCrystal.h>
#include <ESP32AnalogRead.h>
//#include "RTClib.h"
#include <DS3231.h>
#include <Wire.h>
//#include "arduino-esp32\arduino-esp32-master\tools\sdk\include\driver\driver\rtc_io.h"


#include <menu.h>
#include <menuIO/softKeyIn.h>
#include <menuIO/liquidCrystalOut.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialIn.h>

using namespace Menu;

#define MAX_DEPTH 4

//macros
#define LCD_ON pinMode(LCD_EN, OUTPUT);\
               pinMode(LCD_BL_EN, OUTPUT);\
               digitalWrite(LCD_EN, LOW);\
               digitalWrite(LCD_BL_EN, LOW);

#define LCD_OFF pinMode(LCD_EN, INPUT); \
                pinMode(LCD_BL_EN, INPUT);

#define PREP_FOR_DEEP_SLEEP for (int i=12; i<32; i++) { \
                        if((i!=21)&&(i!=24)&&(i!=29)&&(i!=34)&&(i!=39)&&(i!=35)&&(i!=36)&&(i!=3)){\
                        gpio_set_pull_mode((gpio_num_t)i, GPIO_FLOATING);\
         }\
       }

#define WAKEUP_PINMASK 0xF08000000

//pins
#define LED 2
#define LCD_RS 4
#define LCD_D5 5
#define END_LOW 12
#define M_FWD 13
#define M_BACK 14
#define LCD_BL_EN 15
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
#define SW_EXIT 34
#define SW_SELECT 32
#define SW_BACK 35
#define SW_FWD 33
#define LDR_VAL 36
#define V_BATT 39


#define S_TO_uS 1000000
#define LDR_SLEEP 600 //Seconds Delay between Measurement
#define LDR_MEAS_SLEEP 60 //Seconds Delay between Measurement, when threshold was broken
#define BATT_CRIT 5.3 //Critical Volts for Battery
#define BATT_MIN  5.0 //Minimum Volts for Battery
#define THRESHOLD_COUNT_MAX 2 //How often has the LDR to be over threshold

enum KLAPPENPOSITION {POS_UP, POS_DOWN, POS_DRIVING, POS_BLOCKED};
enum openingMode {LICHT, ZEIT, LICHT_ZEIT, NICHT};
enum TimerLogic {NEXT_OPEN, NEXT_CLOSE, WAIT_CLOSE, NO_TIMER};
enum TimerReturnVal {GO_DOWN, GO_UP, SLEEP_LONG, SLEEP_SHORT};


enum TimerVar {BLOCKED_TIMER, MOVING_TIMER};


typedef struct  {
    bool done;
    byte hour;
    byte minute;
    uint16_t lux;
    uint16_t delay; //delay till opening in seconds
    openingMode mode; //0=licht, 1=Zeit, 2=Licht&Zeit, 3=nichts
} doorDayAlarm_t;



#endif
